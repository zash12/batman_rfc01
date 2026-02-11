/*
 * batman.cc
 * B.A.T.M.A.N. Routing Agent Implementation for NS2
 * Part 1: Initialization and basic functions
 */

#include "batman.h"
#include <ip.h>
#include <random.h>
#include <cmu-trace.h>

/* Static packet offset initialization */
int hdr_batman_ogm::offset_;
int hdr_batman_hna::offset_;

/* Packet header class */
static class BATMANHeaderClass : public PacketHeaderClass {
public:
    BATMANHeaderClass() : PacketHeaderClass("PacketHeader/BATMAN", 
                                             sizeof(hdr_all_batman)) {
        bind_offset(&hdr_batman_ogm::offset_);
        bind_offset(&hdr_batman_hna::offset_);
    }
} class_batman_hdr;

/* Routing agent class */
static class BATMANAgentClass : public TclClass {
public:
    BATMANAgentClass() : TclClass("Agent/BATMAN") {}
    TclObject* create(int, const char*const*) {
        return (new BATMANAgent());
    }
} class_batman_agent;

/* ===== Timer Methods ===== */

void OGMTimer::expire(Event *e) {
    agent_->sendOGM();
    
    // Reschedule with jitter
    double next_time = ORIGINATOR_INTERVAL + JITTER;
    resched(next_time);
}

void PurgeTimer::expire(Event *e) {
    agent_->purgeRoutingTable();
    
    // Reschedule purge timer
    resched(PURGE_TIMEOUT);
}

/* ===== BATMANAgent Methods ===== */

BATMANAgent::BATMANAgent() : Agent(PT_BATMAN),
    ra_addr_(0), accessibility_(0), seqno_(0), ttl_value_(TTL_MAX),
    is_gateway_(false), gw_flags_(0), gw_port_(0),
    ogm_timer_(this), purge_timer_(this),
    port_dmux_(NULL), logtarget_(NULL)
{
    bind("accessibility_", &accessibility_);
    
    // Create routing table
    rtable_ = new BATMANRoutingTable(this);
}

BATMANAgent::~BATMANAgent() {
    delete rtable_;
}

int BATMANAgent::command(int argc, const char*const* argv) {
    if (argc == 2) {
        if (strcasecmp(argv[1], "start") == 0) {
            // Start B.A.T.M.A.N. protocol
            ra_addr_ = getMyAddress();
            
            // Bind to BATMAN port
            port_dmux_ = new PortClassifier();
            
            // Start timers
            ogm_timer_.resched(ORIGINATOR_INTERVAL + JITTER);
            purge_timer_.resched(PURGE_TIMEOUT);
            
            printf("BATMAN: Started on node %d\n", ra_addr_);
            return TCL_OK;
        }
        
        if (strcasecmp(argv[1], "print_rtable") == 0) {
            rtable_->print();
            return TCL_OK;
        }
    }
    
    if (argc == 3) {
        if (strcasecmp(argv[1], "log-target") == 0) {
            logtarget_ = (Trace*)TclObject::lookup(argv[2]);
            if (logtarget_ == NULL)
                return TCL_ERROR;
            return TCL_OK;
        }
        
        if (strcasecmp(argv[1], "port-dmux") == 0) {
            port_dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
            if (port_dmux_ == NULL)
                return TCL_ERROR;
            return TCL_OK;
        }
        
        if (strcasecmp(argv[1], "ttl") == 0) {
            ttl_value_ = atoi(argv[2]);
            if (ttl_value_ < TTL_MIN || ttl_value_ > TTL_MAX) {
                fprintf(stderr, "BATMAN: Invalid TTL value %d\n", ttl_value_);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
    }
    
    if (argc == 4) {
        if (strcasecmp(argv[1], "gateway") == 0) {
            is_gateway_ = (atoi(argv[2]) != 0);
            gw_flags_ = atoi(argv[2]);
            gw_port_ = atoi(argv[3]);
            printf("BATMAN: Gateway mode %s (flags=%d, port=%d)\n",
                   is_gateway_ ? "enabled" : "disabled",
                   gw_flags_, gw_port_);
            return TCL_OK;
        }
    }
    
    return Agent::command(argc, argv);
}

void BATMANAgent::recv(Packet *p, Handler*) {
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    
    // Check if this is a BATMAN packet
    if (ch->ptype() == PT_BATMAN) {
        recvOGM(p);
    } else {
        // Data packet - route it
        recvData(p);
    }
}

/* ===== OGM Broadcasting ===== */

void BATMANAgent::sendOGM() {
    Packet *p = createOGM();
    
    if (p != NULL) {
        // Log outgoing packet
        if (logtarget_) {
            log(p);
        }
        
        // Send broadcast
        send(p, 0);
        
        // Update own sequence number
        seqno_++;
        if (seqno_ > SEQNO_MAX)
            seqno_ = 0;
    }
}

Packet* BATMANAgent::createOGM() {
    Packet *p = allocpkt();
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_batman_ogm *oh = hdr_batman_ogm::access(p);
    
    // Common header
    ch->ptype() = PT_BATMAN;
    ch->direction() = hdr_cmn::DOWN;
    ch->size() = IP_HDR_LEN + sizeof(hdr_batman_ogm);
    ch->next_hop() = IP_BROADCAST;
    ch->addr_type() = NS_AF_INET;
    
    // IP header
    ih->saddr() = ra_addr_;
    ih->daddr() = IP_BROADCAST;
    ih->sport() = BATMAN_PORT;
    ih->dport() = BATMAN_PORT;
    ih->ttl() = ttl_value_;
    
    // BATMAN OGM header
    oh->version() = BATMAN_VERSION;
    oh->flags() = 0;
    oh->ttl() = ttl_value_;
    oh->seqno() = seqno_;
    oh->orig_addr() = ra_addr_;
    oh->gw_flags() = gw_flags_;
    oh->gw_port() = gw_port_;
    
    return p;
}

/* ===== OGM Reception ===== */

void BATMANAgent::recvOGM(Packet *p) {
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_batman_ogm *oh = hdr_batman_ogm::access(p);
    
    // Preliminary checks
    if (!preliminaryChecks(p)) {
        Packet::free(p);
        return;
    }
    
    nsaddr_t sender = ih->saddr();
    nsaddr_t originator = oh->orig_addr();
    u_int16_t seqno = oh->seqno();
    bool is_directlink = oh->is_directlink();
    
    // Check if this is our own OGM being echoed back
    if (originator == ra_addr_) {
        // This is our own OGM - update bidirectional link info
        if (is_directlink) {
            rtable_->updateBidirLinkSeqno(sender, seqno);
        }
        Packet::free(p);
        return;
    }
    
    // Check for duplicate
    if (checkDuplicate(originator, seqno)) {
        // Duplicate - may still need to forward
        if (shouldForward(p, sender)) {
            forwardOGM(p);
        } else {
            Packet::free(p);
        }
        return;
    }
    
    // Log this broadcast
    logBroadcast(originator, seqno);
    
    // Check bidirectional link
    bool bidir = checkBidirectionalLink(p);
    if (!bidir) {
        // Mark as unidirectional and don't process further
        oh->set_unidirectional();
        Packet::free(p);
        return;
    }
    
    // Update neighbor ranking
    updateNeighborRanking(p);
    
    // Update gateway information if present
    if (oh->gw_flags() != 0) {
        rtable_->updateGateway(originator, oh->gw_flags(), oh->gw_port());
    }
    
    // Forward OGM if appropriate
    nsaddr_t nexthop;
    if (shouldForward(p, nexthop)) {
        forwardOGM(p);
    } else {
        Packet::free(p);
    }
}

void BATMANAgent::forwardOGM(Packet *p) {
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_batman_ogm *oh = hdr_batman_ogm::access(p);
    
    // Decrement TTL
    oh->ttl()--;
    ih->ttl()--;
    
    if (oh->ttl() == 0) {
        Packet::free(p);
        return;
    }
    
    // Set direct link flag if forwarding on same interface
    nsaddr_t sender = ih->saddr();
    if (sender == oh->orig_addr()) {
        oh->set_directlink();
    } else {
        oh->clear_directlink();
    }
    
    // Update IP header for forwarding
    ih->saddr() = ra_addr_;
    ih->daddr() = IP_BROADCAST;
    
    // Add small random delay to avoid collisions
    double delay = Random::uniform(BROADCAST_DELAY_MAX);
    
    // Log forwarding
    if (logtarget_) {
        log(p);
    }
    
    // Schedule send with delay
    Scheduler::instance().schedule(this, p, delay);
}

/* ===== Packet Processing ===== */

bool BATMANAgent::preliminaryChecks(Packet *p) {
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_batman_ogm *oh = hdr_batman_ogm::access(p);
    
    // Check version
    if (oh->version() != BATMAN_VERSION) {
        trace("BATMAN: Version mismatch, dropping packet");
        return false;
    }
    
    // Check if sender is ourselves
    if (ih->saddr() == ra_addr_) {
        return false;
    }
    
    // Check if sender address is broadcast
    if (ih->saddr() == IP_BROADCAST) {
        return false;
    }
    
    // Check if originator is ourselves (except for bidirectional check)
    if (oh->orig_addr() == ra_addr_) {
        return true; // Allow for bidirectional link check
    }
    
    // Check unidirectional flag
    if (oh->is_unidirectional()) {
        trace("BATMAN: Unidirectional link detected, dropping");
        return false;
    }
    
    return true;
}

bool BATMANAgent::checkDuplicate(nsaddr_t orig, u_int16_t seqno) {
    // Search broadcast log
    std::list<BroadcastLogEntry>::iterator it;
    for (it = bcast_log_.begin(); it != bcast_log_.end(); ++it) {
        if (it->orig_addr_ == orig && it->seqno_ == seqno) {
            return true;
        }
    }
    return false;
}

void BATMANAgent::logBroadcast(nsaddr_t orig, u_int16_t seqno) {
    bcast_log_.push_back(BroadcastLogEntry(orig, seqno, CURRENT_TIME));
    purgeBroadcastLog();
}

void BATMANAgent::purgeBroadcastLog() {
    double timeout = PURGE_TIMEOUT;
    
    std::list<BroadcastLogEntry>::iterator it = bcast_log_.begin();
    while (it != bcast_log_.end()) {
        if ((CURRENT_TIME - it->timestamp_) > timeout) {
            bcast_log_.erase(it++);
        } else {
            ++it;
        }
    }
}

bool BATMANAgent::checkBidirectionalLink(Packet *p) {
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_batman_ogm *oh = hdr_batman_ogm::access(p);
    
    nsaddr_t sender = ih->saddr();
    nsaddr_t originator = oh->orig_addr();
    u_int16_t seqno = oh->seqno();
    bool is_directlink = oh->is_directlink();
    
    // If this is a direct link OGM
    if (is_directlink && sender == originator) {
        // Check if we can find our own echoed OGM
        return rtable_->checkBidirectionalLink(originator, sender, seqno);
    }
    
    // For forwarded OGMs, assume bidirectional if the forwarder is bidirectional
    return true;
}

void BATMANAgent::updateNeighborRanking(Packet *p) {
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_batman_ogm *oh = hdr_batman_ogm::access(p);
    
    nsaddr_t sender = ih->saddr();
    nsaddr_t originator = oh->orig_addr();
    u_int16_t seqno = oh->seqno();
    u_int8_t ttl = oh->ttl();
    
    // Update routing table with this information
    rtable_->updateNeighborRanking(originator, sender, seqno, ttl);
}

bool BATMANAgent::shouldForward(Packet *p, nsaddr_t &nexthop) {
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_batman_ogm *oh = hdr_batman_ogm::access(p);
    
    nsaddr_t sender = ih->saddr();
    nsaddr_t originator = oh->orig_addr();
    u_int16_t seqno = oh->seqno();
    bool is_directlink = oh->is_directlink();
    
    // Forward if:
    // 1. Received from single hop neighbor (direct link)
    // 2. Received via best link AND (not duplicate OR same TTL as last)
    
    OriginatorEntry *oe = rtable_->findOriginator(originator);
    if (oe == NULL)
        return false;
    
    // Case 1: Direct link from originator
    if (is_directlink && sender == originator) {
        nexthop = IP_BROADCAST;
        return true;
    }
    
    // Case 2: Via best link
    if (sender == oe->best_next_hop_) {
        NeighborInfo *ni = oe->getNeighborInfo(sender);
        if (ni != NULL) {
            // Check if duplicate or not
            if (!ni->isInWindow(seqno) || 
                (oh->ttl() == ni->last_ttl_)) {
                nexthop = IP_BROADCAST;
                return true;
            }
        }
    }
    
    return false;
}

/* ===== Data Packet Handling ===== */

void BATMANAgent::recvData(Packet *p) {
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    
    nsaddr_t dest = ih->daddr();
    
    // Check if we are the destination
    if (dest == ra_addr_ || dest == IP_BROADCAST) {
        // Deliver locally
        port_dmux_->recv(p, (Handler*)0);
        return;
    }
    
    // Look up next hop
    nsaddr_t nexthop = rtable_->lookup(dest);
    
    if (nexthop != 0) {
        // Forward packet
        forwardData(p, nexthop);
    } else {
        // No route - drop packet
        trace("BATMAN: No route to %d, dropping packet", dest);
        drop(p, DROP_RTR_NO_ROUTE);
    }
}

void BATMANAgent::forwardData(Packet *p, nsaddr_t nexthop) {
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    
    // Update common header
    ch->direction() = hdr_cmn::DOWN;
    ch->next_hop() = nexthop;
    
    // Decrement TTL
    ih->ttl()--;
    if (ih->ttl() == 0) {
        drop(p, DROP_RTR_TTL);
        return;
    }
    
    // Log forwarding
    if (logtarget_) {
        log(p);
    }
    
    // Send packet
    send(p, 0);
}

/* ===== Route Table Maintenance ===== */

void BATMANAgent::purgeRoutingTable() {
    rtable_->purge(CURRENT_TIME);
}

void BATMANAgent::updateRoutes() {
    // Routes are updated automatically through neighbor ranking
    // This method can be used for additional route optimization
}

/* ===== Utility Functions ===== */

void BATMANAgent::trace(char *fmt, ...) {
    va_list ap;
    
    if (!logtarget_)
        return;
    
    va_start(ap, fmt);
    vsprintf(logtarget_->buffer(), fmt, ap);
    va_end(ap);
    
    logtarget_->dump();
}

void BATMANAgent::log(Packet *p) {
    if (!logtarget_)
        return;
    
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    
    sprintf(logtarget_->buffer(),
            "B %d %d %s %d %d %d",
            ra_addr_,
            ch->ptype(),
            (ch->direction() == hdr_cmn::UP) ? "UP" : "DOWN",
            ih->saddr(),
            ih->daddr(),
            ch->size());
    
    logtarget_->dump();
}

nsaddr_t BATMANAgent::getMyAddress() {
    // Get node address from mobile node
    MobileNode *mn = getMobileNode();
    if (mn != NULL) {
        return mn->address();
    }
    return 0;
}

MobileNode* BATMANAgent::getMobileNode() {
    Node *n = (Node*)this->node();
    if (n == NULL)
        return NULL;
    
    MobileNode *mn = (MobileNode*)n;
    return mn;
}

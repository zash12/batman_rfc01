/*
 * batman.h
 * B.A.T.M.A.N. Routing Agent for NS2
 */

#ifndef __batman_h__
#define __batman_h__

#include <agent.h>
#include <packet.h>
#include <trace.h>
#include <timer-handler.h>
#include <classifier-port.h>
#include <mobilenode.h>

#include "batman_pkt.h"
#include "batman_rtable.h"

#define CURRENT_TIME Scheduler::instance().clock()
#define JITTER (Random::uniform(ORIGINATOR_INTERVAL_JITTER) - ORIGINATOR_INTERVAL_JITTER/2)

/* Forward declarations */
class BATMANAgent;

/* Timer for periodic OGM broadcasts */
class OGMTimer : public TimerHandler {
public:
    OGMTimer(BATMANAgent *a) : TimerHandler(), agent_(a) {}
    void expire(Event *e);
protected:
    BATMANAgent *agent_;
};

/* Timer for route table purging */
class PurgeTimer : public TimerHandler {
public:
    PurgeTimer(BATMANAgent *a) : TimerHandler(), agent_(a) {}
    void expire(Event *e);
protected:
    BATMANAgent *agent_;
};

/* Broadcast buffer entry */
class BroadcastLogEntry {
public:
    nsaddr_t orig_addr_;
    u_int16_t seqno_;
    double timestamp_;
    
    BroadcastLogEntry(nsaddr_t addr, u_int16_t seqno, double time) :
        orig_addr_(addr), seqno_(seqno), timestamp_(time) {}
};

/* B.A.T.M.A.N. Routing Agent */
class BATMANAgent : public Agent {
    friend class OGMTimer;
    friend class PurgeTimer;
    friend class BATMANRoutingTable;
    
public:
    BATMANAgent();
    ~BATMANAgent();
    
    /* NS2 interface methods */
    int command(int argc, const char*const* argv);
    void recv(Packet *p, Handler*);
    
protected:
    /* Configuration parameters */
    nsaddr_t ra_addr_;          // Router agent address
    int accessibility_;         // Accessibility to base stations
    u_int32_t seqno_;          // Sequence number for OGMs
    u_int8_t ttl_value_;       // TTL for OGMs
    
    /* Gateway configuration */
    bool is_gateway_;
    u_int8_t gw_flags_;
    u_int16_t gw_port_;
    
    /* Routing table */
    BATMANRoutingTable *rtable_;
    
    /* Timers */
    OGMTimer ogm_timer_;
    PurgeTimer purge_timer_;
    
    /* Port binding */
    PortClassifier *port_dmux_;
    Trace *logtarget_;
    
    /* Broadcast log */
    std::list<BroadcastLogEntry> bcast_log_;
    
    /* OGM Broadcasting */
    void sendOGM();
    void forwardOGM(Packet *p);
    Packet* createOGM();
    
    /* Packet reception */
    void recvOGM(Packet *p);
    void recvData(Packet *p);
    
    /* Packet processing */
    bool preliminaryChecks(Packet *p);
    bool checkDuplicate(nsaddr_t orig, u_int16_t seqno);
    void logBroadcast(nsaddr_t orig, u_int16_t seqno);
    void purgeBroadcastLog();
    
    /* Bidirectional link check */
    bool checkBidirectionalLink(Packet *p);
    
    /* Neighbor ranking and routing */
    void updateNeighborRanking(Packet *p);
    void updateRoutes();
    
    /* Forwarding decision */
    bool shouldForward(Packet *p, nsaddr_t &nexthop);
    void forwardData(Packet *p, nsaddr_t nexthop);
    
    /* Utility functions */
    void trace(char *fmt, ...);
    void log(Packet *p);
    nsaddr_t getMyAddress();
    MobileNode* getMobileNode();
    
    /* Table maintenance */
    void purgeRoutingTable();
};

#endif /* __batman_h__ */

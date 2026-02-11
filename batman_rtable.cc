/*
 * batman_rtable.cc
 * B.A.T.M.A.N. Routing Table Implementation
 */

#include "batman_rtable.h"
#include "batman.h"
#include <stdlib.h>
#include <stdio.h>

/* ===== NeighborInfo Methods ===== */

void NeighborInfo::updateWindow(u_int16_t seqno) {
    // Add sequence number to sliding window
    sliding_window_.insert(seqno);
    
    // Remove old sequence numbers outside the window
    u_int16_t lower_bound = (curr_seqno_ >= WINDOW_SIZE) ? 
                            (curr_seqno_ - WINDOW_SIZE + 1) : 0;
    
    std::set<u_int16_t>::iterator it = sliding_window_.begin();
    while (it != sliding_window_.end()) {
        if (seqno_less_than(*it, lower_bound)) {
            sliding_window_.erase(it++);
        } else {
            ++it;
        }
    }
    
    // Update packet count
    packet_count_ = sliding_window_.size();
}

bool NeighborInfo::isInWindow(u_int16_t seqno) {
    if (sliding_window_.find(seqno) != sliding_window_.end())
        return true;
    
    u_int16_t lower_bound = (curr_seqno_ >= WINDOW_SIZE) ? 
                            (curr_seqno_ - WINDOW_SIZE + 1) : 0;
    
    return (seqno_greater_than(seqno, lower_bound) || seqno == lower_bound) &&
           (seqno_less_than(seqno, curr_seqno_) || seqno == curr_seqno_);
}

double NeighborInfo::calculateTQ() {
    if (packet_count_ == 0)
        return 0.0;
    
    // TQ = (packets_received / WINDOW_SIZE) * 100
    tq_value_ = (double)packet_count_ / (double)WINDOW_SIZE;
    return tq_value_;
}

/* ===== OriginatorEntry Methods ===== */

OriginatorEntry::~OriginatorEntry() {
    // Delete all neighbor information
    std::map<nsaddr_t, NeighborInfo*>::iterator it;
    for (it = neighbor_info_.begin(); it != neighbor_info_.end(); ++it) {
        delete it->second;
    }
    neighbor_info_.clear();
}

NeighborInfo* OriginatorEntry::getNeighborInfo(nsaddr_t neighbor) {
    std::map<nsaddr_t, NeighborInfo*>::iterator it = neighbor_info_.find(neighbor);
    if (it != neighbor_info_.end()) {
        return it->second;
    }
    
    // Create new neighbor info
    NeighborInfo *ni = new NeighborInfo();
    ni->neighbor_addr_ = neighbor;
    neighbor_info_[neighbor] = ni;
    return ni;
}

void OriginatorEntry::updateBestNextHop() {
    nsaddr_t old_best = best_next_hop_;
    int max_count = 0;
    nsaddr_t best_neighbor = 0;
    
    // Find neighbor with highest packet count
    std::map<nsaddr_t, NeighborInfo*>::iterator it;
    for (it = neighbor_info_.begin(); it != neighbor_info_.end(); ++it) {
        NeighborInfo *ni = it->second;
        if (ni->packet_count_ > max_count) {
            max_count = ni->packet_count_;
            best_neighbor = it->first;
        }
    }
    
    best_next_hop_ = best_neighbor;
    best_route_count_ = max_count;
    
    // Debug output if best route changed
    if (old_best != best_next_hop_ && best_next_hop_ != 0) {
        printf("BATMAN: Updated best route to %d via %d (count=%d)\n",
               orig_addr_, best_next_hop_, max_count);
    }
}

void OriginatorEntry::purgeOldNeighbors(double current_time) {
    std::map<nsaddr_t, NeighborInfo*>::iterator it = neighbor_info_.begin();
    while (it != neighbor_info_.end()) {
        NeighborInfo *ni = it->second;
        if ((current_time - ni->last_valid_time_) > PURGE_TIMEOUT) {
            delete ni;
            neighbor_info_.erase(it++);
        } else {
            ++it;
        }
    }
}

/* ===== BATMANRoutingTable Methods ===== */

BATMANRoutingTable::~BATMANRoutingTable() {
    // Delete all originator entries
    std::map<nsaddr_t, OriginatorEntry*>::iterator it;
    for (it = rt_table_.begin(); it != rt_table_.end(); ++it) {
        delete it->second;
    }
    rt_table_.clear();
}

OriginatorEntry* BATMANRoutingTable::findOriginator(nsaddr_t dest) {
    std::map<nsaddr_t, OriginatorEntry*>::iterator it = rt_table_.find(dest);
    if (it != rt_table_.end()) {
        return it->second;
    }
    return NULL;
}

OriginatorEntry* BATMANRoutingTable::addOriginator(nsaddr_t dest) {
    OriginatorEntry *oe = findOriginator(dest);
    if (oe != NULL)
        return oe;
    
    oe = new OriginatorEntry();
    oe->orig_addr_ = dest;
    oe->last_aware_time_ = CURRENT_TIME;
    rt_table_[dest] = oe;
    
    printf("BATMAN: Added new originator %d\n", dest);
    return oe;
}

void BATMANRoutingTable::removeOriginator(nsaddr_t dest) {
    std::map<nsaddr_t, OriginatorEntry*>::iterator it = rt_table_.find(dest);
    if (it != rt_table_.end()) {
        delete it->second;
        rt_table_.erase(it);
        printf("BATMAN: Removed originator %d\n", dest);
    }
}

nsaddr_t BATMANRoutingTable::lookup(nsaddr_t dest) {
    // First check if dest is a known originator
    OriginatorEntry *oe = findOriginator(dest);
    if (oe != NULL && oe->best_next_hop_ != 0) {
        return oe->best_next_hop_;
    }
    
    // Check HNA tables
    nsaddr_t hna_next_hop = lookupHNA(dest);
    if (hna_next_hop != 0)
        return hna_next_hop;
    
    return 0; // No route found
}

bool BATMANRoutingTable::hasRoute(nsaddr_t dest) {
    return (lookup(dest) != 0);
}

void BATMANRoutingTable::purge(double current_time) {
    std::map<nsaddr_t, OriginatorEntry*>::iterator it = rt_table_.begin();
    while (it != rt_table_.end()) {
        OriginatorEntry *oe = it->second;
        
        // Check if originator is still valid
        if ((current_time - oe->last_aware_time_) > PURGE_TIMEOUT) {
            delete oe;
            rt_table_.erase(it++);
        } else {
            // Purge old neighbors
            oe->purgeOldNeighbors(current_time);
            oe->updateBestNextHop();
            ++it;
        }
    }
}

void BATMANRoutingTable::updateNeighborRanking(nsaddr_t orig, nsaddr_t neighbor,
                                                u_int16_t seqno, u_int8_t ttl) {
    OriginatorEntry *oe = findOriginator(orig);
    if (oe == NULL) {
        oe = addOriginator(orig);
    }
    
    oe->last_aware_time_ = CURRENT_TIME;
    
    NeighborInfo *ni = oe->getNeighborInfo(neighbor);
    ni->last_valid_time_ = CURRENT_TIME;
    ni->last_ttl_ = ttl;
    
    // Check if this is a new sequence number
    if (seqno_greater_than(seqno, oe->curr_seqno_) ||
        (oe->curr_seqno_ == 0 && seqno != 0)) {
        
        // Update current sequence number
        u_int16_t old_seqno = oe->curr_seqno_;
        oe->curr_seqno_ = seqno;
        ni->curr_seqno_ = seqno;
        ni->last_valid_seqno_ = seqno;
        
        // Add to sliding window
        ni->updateWindow(seqno);
        
        // Recalculate TQ
        ni->calculateTQ();
        
        // Update best next hop
        oe->updateBestNextHop();
    } 
    else if (ni->isInWindow(seqno)) {
        // Duplicate within window
        ni->updateWindow(seqno);
        ni->calculateTQ();
    }
}

bool BATMANRoutingTable::checkBidirectionalLink(nsaddr_t orig, nsaddr_t neighbor,
                                                 u_int16_t seqno) {
    OriginatorEntry *oe = findOriginator(orig);
    if (oe == NULL)
        return false;
    
    // Check if sequence number matches
    if (seqno == oe->bidir_link_seqno_) {
        return true;
    }
    
    // Check timeout
    double time_diff = CURRENT_TIME - oe->last_aware_time_;
    if (time_diff > BI_LINK_TIMEOUT)
        return false;
    
    return (seqno_diff(oe->bidir_link_seqno_, seqno) <= 
            (BI_LINK_TIMEOUT / ORIGINATOR_INTERVAL));
}

void BATMANRoutingTable::updateBidirLinkSeqno(nsaddr_t orig, u_int16_t seqno) {
    OriginatorEntry *oe = findOriginator(orig);
    if (oe != NULL) {
        oe->bidir_link_seqno_ = seqno;
    }
}

void BATMANRoutingTable::addHNA(nsaddr_t orig, nsaddr_t network, u_int8_t netmask) {
    OriginatorEntry *oe = findOriginator(orig);
    if (oe == NULL) {
        oe = addOriginator(orig);
    }
    
    // Add or update HNA entry
    std::pair<nsaddr_t, u_int8_t> hna_entry(network, netmask);
    
    // Check if already exists
    bool found = false;
    for (size_t i = 0; i < oe->hna_list_.size(); i++) {
        if (oe->hna_list_[i].first == network) {
            oe->hna_list_[i] = hna_entry;
            found = true;
            break;
        }
    }
    
    if (!found) {
        oe->hna_list_.push_back(hna_entry);
    }
}

void BATMANRoutingTable::removeHNA(nsaddr_t orig) {
    OriginatorEntry *oe = findOriginator(orig);
    if (oe != NULL) {
        oe->hna_list_.clear();
    }
}

nsaddr_t BATMANRoutingTable::lookupHNA(nsaddr_t dest) {
    // Search through all originators' HNA lists
    std::map<nsaddr_t, OriginatorEntry*>::iterator it;
    for (it = rt_table_.begin(); it != rt_table_.end(); ++it) {
        OriginatorEntry *oe = it->second;
        
        for (size_t i = 0; i < oe->hna_list_.size(); i++) {
            nsaddr_t network = oe->hna_list_[i].first;
            u_int8_t netmask = oe->hna_list_[i].second;
            
            // Simple network matching (simplified for NS2)
            if ((dest & netmask) == (network & netmask)) {
                return oe->best_next_hop_;
            }
        }
    }
    
    return 0;
}

void BATMANRoutingTable::updateGateway(nsaddr_t orig, u_int8_t gw_flags,
                                       u_int16_t gw_port) {
    OriginatorEntry *oe = findOriginator(orig);
    if (oe == NULL) {
        oe = addOriginator(orig);
    }
    
    oe->is_gateway_ = (gw_flags != 0);
    oe->gw_flags_ = gw_flags;
    oe->gw_port_ = gw_port;
}

nsaddr_t BATMANRoutingTable::selectBestGateway() {
    nsaddr_t best_gw = 0;
    int best_metric = 0;
    
    std::map<nsaddr_t, OriginatorEntry*>::iterator it;
    for (it = rt_table_.begin(); it != rt_table_.end(); ++it) {
        OriginatorEntry *oe = it->second;
        
        if (oe->is_gateway_ && oe->best_next_hop_ != 0) {
            // Simple metric: packet count * gateway class
            int metric = oe->best_route_count_ * (int)oe->gw_flags_;
            
            if (metric > best_metric) {
                best_metric = metric;
                best_gw = oe->orig_addr_;
            }
        }
    }
    
    return best_gw;
}

void BATMANRoutingTable::print() {
    printf("\n========== BATMAN Routing Table ==========\n");
    printf("%-10s %-10s %-10s %-10s\n", "Dest", "NextHop", "Count", "GW");
    
    std::map<nsaddr_t, OriginatorEntry*>::iterator it;
    for (it = rt_table_.begin(); it != rt_table_.end(); ++it) {
        OriginatorEntry *oe = it->second;
        printf("%-10d %-10d %-10d %-10s\n",
               oe->orig_addr_,
               oe->best_next_hop_,
               oe->best_route_count_,
               oe->is_gateway_ ? "YES" : "NO");
    }
    printf("==========================================\n\n");
}

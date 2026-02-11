/*
 * batman_rtable.h
 * B.A.T.M.A.N. Routing Table Implementation
 */

#ifndef __batman_rtable_h__
#define __batman_rtable_h__

#include <map>
#include <list>
#include <set>
#include <vector>
#include <assert.h>

/* Forward declarations */
class BATMANAgent;

/* Neighbor information for a specific originator */
class NeighborInfo {
public:
    nsaddr_t neighbor_addr_;    // Address of the neighbor
    u_int16_t curr_seqno_;      // Current sequence number
    u_int16_t last_valid_seqno_; // Last valid sequence number  
    std::set<u_int16_t> sliding_window_; // Sliding window of received seqnos
    int packet_count_;          // Number of packets in window
    double last_valid_time_;    // Time of last valid OGM
    u_int8_t last_ttl_;        // TTL of last received OGM
    double tq_value_;          // Transmit Quality value
    
    NeighborInfo() : 
        neighbor_addr_(0), curr_seqno_(0), last_valid_seqno_(0),
        packet_count_(0), last_valid_time_(0), last_ttl_(0), tq_value_(0.0) {}
    
    void updateWindow(u_int16_t seqno);
    bool isInWindow(u_int16_t seqno);
    double calculateTQ();
};

/* Originator entry in the routing table */
class OriginatorEntry {
public:
    nsaddr_t orig_addr_;        // Originator address
    u_int16_t curr_seqno_;      // Current sequence number from this originator
    double last_aware_time_;    // Last time we heard from this originator
    std::map<nsaddr_t, NeighborInfo*> neighbor_info_; // Info per neighbor
    nsaddr_t best_next_hop_;    // Best next hop to reach this originator
    int best_route_count_;      // Packet count of best route
    u_int16_t bidir_link_seqno_; // Sequence number for bidirectional link check
    std::vector<std::pair<nsaddr_t, u_int8_t> > hna_list_; // HNA announcements
    
    // Gateway information
    bool is_gateway_;
    u_int8_t gw_flags_;
    u_int16_t gw_port_;
    
    OriginatorEntry() :
        orig_addr_(0), curr_seqno_(0), last_aware_time_(0),
        best_next_hop_(0), best_route_count_(0), bidir_link_seqno_(0),
        is_gateway_(false), gw_flags_(0), gw_port_(0) {}
    
    ~OriginatorEntry();
    
    NeighborInfo* getNeighborInfo(nsaddr_t neighbor);
    void updateBestNextHop();
    void purgeOldNeighbors(double current_time);
};

/* B.A.T.M.A.N. Routing Table */
class BATMANRoutingTable {
protected:
    std::map<nsaddr_t, OriginatorEntry*> rt_table_;
    BATMANAgent *agent_;
    
public:
    BATMANRoutingTable(BATMANAgent *agent) : agent_(agent) {}
    ~BATMANRoutingTable();
    
    /* Routing table operations */
    OriginatorEntry* findOriginator(nsaddr_t dest);
    OriginatorEntry* addOriginator(nsaddr_t dest);
    void removeOriginator(nsaddr_t dest);
    
    /* Route lookup */
    nsaddr_t lookup(nsaddr_t dest);
    bool hasRoute(nsaddr_t dest);
    
    /* Table maintenance */
    void purge(double current_time);
    void print();
    
    /* Neighbor ranking */
    void updateNeighborRanking(nsaddr_t orig, nsaddr_t neighbor, 
                               u_int16_t seqno, u_int8_t ttl);
    
    /* Bidirectional link check */
    bool checkBidirectionalLink(nsaddr_t orig, nsaddr_t neighbor, u_int16_t seqno);
    void updateBidirLinkSeqno(nsaddr_t orig, u_int16_t seqno);
    
    /* HNA support */
    void addHNA(nsaddr_t orig, nsaddr_t network, u_int8_t netmask);
    void removeHNA(nsaddr_t orig);
    nsaddr_t lookupHNA(nsaddr_t dest);
    
    /* Gateway support */
    void updateGateway(nsaddr_t orig, u_int8_t gw_flags, u_int16_t gw_port);
    nsaddr_t selectBestGateway();
    
    /* Statistics */
    int size() { return rt_table_.size(); }
};

/* Sequence number comparison considering wraparound */
inline bool seqno_greater_than(u_int16_t s1, u_int16_t s2) {
    return ((s1 > s2) && (s1 - s2 < SEQNO_MAX/2)) ||
           ((s2 > s1) && (s2 - s1 > SEQNO_MAX/2));
}

inline bool seqno_less_than(u_int16_t s1, u_int16_t s2) {
    return seqno_greater_than(s2, s1);
}

inline u_int16_t seqno_diff(u_int16_t s1, u_int16_t s2) {
    if (s1 >= s2)
        return s1 - s2;
    else
        return (SEQNO_MAX - s2 + s1);
}

#endif /* __batman_rtable_h__ */

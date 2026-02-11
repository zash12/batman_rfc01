/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * batman-routing-protocol.h
 * B.A.T.M.A.N. Routing Protocol for NS3
 */

#ifndef BATMAN_ROUTING_PROTOCOL_H
#define BATMAN_ROUTING_PROTOCOL_H

#include "batman-packet.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/random-variable-stream.h"
#include "ns3/timer.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include <map>
#include <set>

namespace ns3 {
namespace batman {

#define WINDOW_SIZE 128
#define SEQNO_MAX 65535
#define PURGE_TIMEOUT_FACTOR 10

/**
 * \ingroup batman
 * \brief Neighbor information for an originator
 */
class NeighborInfo
{
public:
    NeighborInfo ();
    
    Ipv4Address m_neighborAddr;
    uint16_t m_currSeqNo;
    std::set<uint16_t> m_slidingWindow;
    uint32_t m_packetCount;
    Time m_lastValidTime;
    uint8_t m_lastTtl;
    double m_tqValue;
    
    void UpdateWindow (uint16_t seqno);
    bool IsInWindow (uint16_t seqno) const;
    double CalculateTQ ();
};

/**
 * \ingroup batman
 * \brief Originator entry in routing table
 */
class OriginatorEntry
{
public:
    OriginatorEntry ();
    ~OriginatorEntry ();
    
    Ipv4Address m_origAddr;
    uint16_t m_currSeqNo;
    Time m_lastAwareTime;
    std::map<Ipv4Address, NeighborInfo*> m_neighborInfo;
    Ipv4Address m_bestNextHop;
    uint32_t m_bestRouteCount;
    uint16_t m_bidirLinkSeqNo;
    
    // Gateway info
    bool m_isGateway;
    uint8_t m_gwFlags;
    uint16_t m_gwPort;
    
    NeighborInfo* GetNeighborInfo (Ipv4Address neighbor);
    void UpdateBestNextHop ();
    void PurgeOldNeighbors (Time currentTime, Time timeout);
};

/**
 * \ingroup batman
 * \brief B.A.T.M.A.N. Routing Protocol
 */
class BatmanRoutingProtocol : public Ipv4RoutingProtocol
{
public:
    static TypeId GetTypeId (void);
    
    BatmanRoutingProtocol ();
    virtual ~BatmanRoutingProtocol ();
    
    // Inherited from Ipv4RoutingProtocol
    virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p,
                                       const Ipv4Header &header,
                                       Ptr<NetDevice> oif,
                                       Socket::SocketErrno &sockerr);
    
    virtual bool RouteInput (Ptr<const Packet> p,
                            const Ipv4Header &header,
                            Ptr<const NetDevice> idev,
                            UnicastForwardCallback ucb,
                            MulticastForwardCallback mcb,
                            LocalDeliverCallback lcb,
                            ErrorCallback ecb);
    
    virtual void NotifyInterfaceUp (uint32_t interface);
    virtual void NotifyInterfaceDown (uint32_t interface);
    virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
    virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
    virtual void SetIpv4 (Ptr<Ipv4> ipv4);
    virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;
    
    // Protocol configuration
    void SetOgmInterval (Time interval);
    Time GetOgmInterval () const;
    void SetPurgeTimeout (Time timeout);
    void SetTtl (uint8_t ttl);
    void SetGateway (uint8_t flags, uint16_t port);
    
protected:
    virtual void DoDispose ();
    virtual void DoInitialize ();
    
private:
    // Protocol parameters
    Time m_ogmInterval;
    Time m_purgeTimeout;
    uint8_t m_ttl;
    uint16_t m_seqNo;
    
    // Gateway parameters
    bool m_isGateway;
    uint8_t m_gwFlags;
    uint16_t m_gwPort;
    
    // Network parameters
    Ptr<Ipv4> m_ipv4;
    Ptr<Socket> m_socket;
    std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
    
    // Routing table
    std::map<Ipv4Address, OriginatorEntry*> m_routingTable;
    
    // Broadcast log for duplicate detection
    struct BroadcastLogEntry
    {
        Ipv4Address origAddr;
        uint16_t seqNo;
        Time timestamp;
    };
    std::list<BroadcastLogEntry> m_broadcastLog;
    
    // Timers
    Timer m_ogmTimer;
    Timer m_purgeTimer;
    
    // Random variable for jitter
    Ptr<UniformRandomVariable> m_uniformRandomVariable;
    
    // Protocol methods
    void Start ();
    void SendOgm ();
    void RecvBatman (Ptr<Socket> socket);
    void ProcessOgm (Ptr<Packet> packet, Ipv4Address senderAddr);
    void ForwardOgm (Ptr<Packet> packet, Ipv4Address senderAddr);
    
    // Packet validation
    bool PreliminaryChecks (Ptr<Packet> packet, Ipv4Address senderAddr);
    bool CheckDuplicate (Ipv4Address origAddr, uint16_t seqNo);
    void LogBroadcast (Ipv4Address origAddr, uint16_t seqNo);
    void PurgeBroadcastLog ();
    
    // Link checking
    bool CheckBidirectionalLink (Ptr<Packet> packet, Ipv4Address senderAddr);
    void UpdateBidirLinkSeqNo (Ipv4Address origAddr, uint16_t seqNo);
    
    // Route management
    void UpdateNeighborRanking (Ipv4Address origAddr, Ipv4Address neighbor,
                               uint16_t seqNo, uint8_t ttl);
    OriginatorEntry* FindOriginator (Ipv4Address dest);
    OriginatorEntry* AddOriginator (Ipv4Address dest);
    void RemoveOriginator (Ipv4Address dest);
    Ipv4Address Lookup (Ipv4Address dest);
    
    // Forwarding decision
    bool ShouldForward (Ptr<Packet> packet, Ipv4Address senderAddr,
                       Ipv4Address &nexthop);
    
    // Table maintenance
    void PurgeRoutingTable ();
    
    // Utility functions
    Ipv4Address GetMainInterface () const;
    void SendPacket (Ptr<Packet> packet, Ipv4Address destination);
};

/**
 * \brief Sequence number comparison with wraparound
 */
inline bool SeqNoGreaterThan (uint16_t s1, uint16_t s2)
{
    return ((s1 > s2) && (s1 - s2 < SEQNO_MAX/2)) ||
           ((s2 > s1) && (s2 - s1 > SEQNO_MAX/2));
}

inline bool SeqNoLessThan (uint16_t s1, uint16_t s2)
{
    return SeqNoGreaterThan (s2, s1);
}

inline uint16_t SeqNoDiff (uint16_t s1, uint16_t s2)
{
    if (s1 >= s2)
        return s1 - s2;
    else
        return (SEQNO_MAX - s2 + s1);
}

} // namespace batman
} // namespace ns3

#endif /* BATMAN_ROUTING_PROTOCOL_H */

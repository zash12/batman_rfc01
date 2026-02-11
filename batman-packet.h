/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * batman-packet.h
 * B.A.T.M.A.N. Protocol Packet Definitions for NS3
 *
 * Based on RFC draft-openmesh-b-a-t-m-a-n-00
 */

#ifndef BATMAN_PACKET_H
#define BATMAN_PACKET_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include <iostream>

namespace ns3 {
namespace batman {

/**
 * \ingroup batman
 * \brief B.A.T.M.A.N. Protocol Constants
 */
#define BATMAN_VERSION 4
#define BATMAN_PORT 4305

/**
 * \ingroup batman
 * \brief B.A.T.M.A.N. Packet Type
 */
enum MessageType {
    BATMANTYPE_OGM = 1,
    BATMANTYPE_HNA = 2
};

/**
 * \ingroup batman
 * \brief Originator Message (OGM) Header
 *
 * OGM format:
 * \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |    Version    |U|D|           |      TTL      |    GWFlags    |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |        Sequence Number        |             GW Port           |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                      Originator Address                       |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   \endverbatim
 */
class OriginatorMessageHeader : public Header
{
public:
    OriginatorMessageHeader ();
    virtual ~OriginatorMessageHeader ();

    /**
     * \brief Set the protocol version
     * \param version the version number
     */
    void SetVersion (uint8_t version);
    
    /**
     * \brief Get the protocol version
     * \return the version number
     */
    uint8_t GetVersion () const;
    
    /**
     * \brief Set flags
     * \param flags the flags byte
     */
    void SetFlags (uint8_t flags);
    
    /**
     * \brief Get flags
     * \return the flags byte
     */
    uint8_t GetFlags () const;
    
    /**
     * \brief Set direct link flag
     */
    void SetDirectLink (bool enable);
    
    /**
     * \brief Check if direct link flag is set
     * \return true if direct link
     */
    bool IsDirectLink () const;
    
    /**
     * \brief Set unidirectional flag
     */
    void SetUnidirectional (bool enable);
    
    /**
     * \brief Check if unidirectional flag is set
     * \return true if unidirectional
     */
    bool IsUnidirectional () const;
    
    /**
     * \brief Set TTL
     * \param ttl the time to live value
     */
    void SetTtl (uint8_t ttl);
    
    /**
     * \brief Get TTL
     * \return the time to live value
     */
    uint8_t GetTtl () const;
    
    /**
     * \brief Set sequence number
     * \param seqno the sequence number
     */
    void SetSeqNo (uint16_t seqno);
    
    /**
     * \brief Get sequence number
     * \return the sequence number
     */
    uint16_t GetSeqNo () const;
    
    /**
     * \brief Set originator address
     * \param address the originator IPv4 address
     */
    void SetOriginatorAddress (Ipv4Address address);
    
    /**
     * \brief Get originator address
     * \return the originator IPv4 address
     */
    Ipv4Address GetOriginatorAddress () const;
    
    /**
     * \brief Set gateway flags
     * \param flags gateway class flags
     */
    void SetGatewayFlags (uint8_t flags);
    
    /**
     * \brief Get gateway flags
     * \return gateway class flags
     */
    uint8_t GetGatewayFlags () const;
    
    /**
     * \brief Set gateway port
     * \param port the gateway tunnel port
     */
    void SetGatewayPort (uint16_t port);
    
    /**
     * \brief Get gateway port
     * \return the gateway tunnel port
     */
    uint16_t GetGatewayPort () const;

    // Inherited from Header
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

private:
    uint8_t m_version;       ///< Protocol version
    uint8_t m_flags;         ///< Flags byte
    uint8_t m_ttl;           ///< Time to live
    uint8_t m_gwFlags;       ///< Gateway flags
    uint16_t m_seqNo;        ///< Sequence number
    uint16_t m_gwPort;       ///< Gateway port
    Ipv4Address m_origAddr;  ///< Originator address
    
    static const uint8_t DIRECTLINK_FLAG = 0x40;
    static const uint8_t UNIDIRECTIONAL_FLAG = 0x20;
};

/**
 * \ingroup batman
 * \brief Host Network Announcement (HNA) Header
 *
 * HNA format:
 * \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                        Network Address                        |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |    Netmask    |
 +-+-+-+-+-+-+-+-+
   \endverbatim
 */
class HnaMessageHeader : public Header
{
public:
    HnaMessageHeader ();
    virtual ~HnaMessageHeader ();

    /**
     * \brief Set network address
     * \param address the network address
     */
    void SetNetworkAddress (Ipv4Address address);
    
    /**
     * \brief Get network address
     * \return the network address
     */
    Ipv4Address GetNetworkAddress () const;
    
    /**
     * \brief Set netmask
     * \param netmask the network mask bits
     */
    void SetNetmask (uint8_t netmask);
    
    /**
     * \brief Get netmask
     * \return the network mask bits
     */
    uint8_t GetNetmask () const;

    // Inherited from Header
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

private:
    Ipv4Address m_networkAddr;  ///< Network address
    uint8_t m_netmask;          ///< Netmask in CIDR notation
};

} // namespace batman
} // namespace ns3

#endif /* BATMAN_PACKET_H */

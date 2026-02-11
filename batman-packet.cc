/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * batman-packet.cc
 * B.A.T.M.A.N. Protocol Packet Implementation for NS3
 */

#include "batman-packet.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

namespace ns3 {
namespace batman {

NS_LOG_COMPONENT_DEFINE ("BatmanPacket");

/* ===== OriginatorMessageHeader Implementation ===== */

NS_OBJECT_ENSURE_REGISTERED (OriginatorMessageHeader);

OriginatorMessageHeader::OriginatorMessageHeader ()
    : m_version (BATMAN_VERSION),
      m_flags (0),
      m_ttl (64),
      m_gwFlags (0),
      m_seqNo (0),
      m_gwPort (0),
      m_origAddr (Ipv4Address ())
{
}

OriginatorMessageHeader::~OriginatorMessageHeader ()
{
}

TypeId
OriginatorMessageHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::batman::OriginatorMessageHeader")
        .SetParent<Header> ()
        .SetGroupName ("Batman")
        .AddConstructor<OriginatorMessageHeader> ()
    ;
    return tid;
}

TypeId
OriginatorMessageHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void
OriginatorMessageHeader::Print (std::ostream &os) const
{
    os << "OGM: orig=" << m_origAddr
       << " seqno=" << m_seqNo
       << " ttl=" << (uint32_t)m_ttl
       << " flags=" << (uint32_t)m_flags;
}

uint32_t
OriginatorMessageHeader::GetSerializedSize (void) const
{
    return 12; // OGM is 12 bytes
}

void
OriginatorMessageHeader::Serialize (Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    
    i.WriteU8 (m_version);
    i.WriteU8 (m_flags);
    i.WriteU8 (m_ttl);
    i.WriteU8 (m_gwFlags);
    i.WriteHtonU16 (m_seqNo);
    i.WriteHtonU16 (m_gwPort);
    WriteTo (i, m_origAddr);
}

uint32_t
OriginatorMessageHeader::Deserialize (Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    
    m_version = i.ReadU8 ();
    m_flags = i.ReadU8 ();
    m_ttl = i.ReadU8 ();
    m_gwFlags = i.ReadU8 ();
    m_seqNo = i.ReadNtohU16 ();
    m_gwPort = i.ReadNtohU16 ();
    ReadFrom (i, m_origAddr);
    
    return GetSerializedSize ();
}

void
OriginatorMessageHeader::SetVersion (uint8_t version)
{
    m_version = version;
}

uint8_t
OriginatorMessageHeader::GetVersion () const
{
    return m_version;
}

void
OriginatorMessageHeader::SetFlags (uint8_t flags)
{
    m_flags = flags;
}

uint8_t
OriginatorMessageHeader::GetFlags () const
{
    return m_flags;
}

void
OriginatorMessageHeader::SetDirectLink (bool enable)
{
    if (enable)
    {
        m_flags |= DIRECTLINK_FLAG;
    }
    else
    {
        m_flags &= ~DIRECTLINK_FLAG;
    }
}

bool
OriginatorMessageHeader::IsDirectLink () const
{
    return (m_flags & DIRECTLINK_FLAG) != 0;
}

void
OriginatorMessageHeader::SetUnidirectional (bool enable)
{
    if (enable)
    {
        m_flags |= UNIDIRECTIONAL_FLAG;
    }
    else
    {
        m_flags &= ~UNIDIRECTIONAL_FLAG;
    }
}

bool
OriginatorMessageHeader::IsUnidirectional () const
{
    return (m_flags & UNIDIRECTIONAL_FLAG) != 0;
}

void
OriginatorMessageHeader::SetTtl (uint8_t ttl)
{
    m_ttl = ttl;
}

uint8_t
OriginatorMessageHeader::GetTtl () const
{
    return m_ttl;
}

void
OriginatorMessageHeader::SetSeqNo (uint16_t seqno)
{
    m_seqNo = seqno;
}

uint16_t
OriginatorMessageHeader::GetSeqNo () const
{
    return m_seqNo;
}

void
OriginatorMessageHeader::SetOriginatorAddress (Ipv4Address address)
{
    m_origAddr = address;
}

Ipv4Address
OriginatorMessageHeader::GetOriginatorAddress () const
{
    return m_origAddr;
}

void
OriginatorMessageHeader::SetGatewayFlags (uint8_t flags)
{
    m_gwFlags = flags;
}

uint8_t
OriginatorMessageHeader::GetGatewayFlags () const
{
    return m_gwFlags;
}

void
OriginatorMessageHeader::SetGatewayPort (uint16_t port)
{
    m_gwPort = port;
}

uint16_t
OriginatorMessageHeader::GetGatewayPort () const
{
    return m_gwPort;
}

/* ===== HnaMessageHeader Implementation ===== */

NS_OBJECT_ENSURE_REGISTERED (HnaMessageHeader);

HnaMessageHeader::HnaMessageHeader ()
    : m_networkAddr (Ipv4Address ()),
      m_netmask (0)
{
}

HnaMessageHeader::~HnaMessageHeader ()
{
}

TypeId
HnaMessageHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::batman::HnaMessageHeader")
        .SetParent<Header> ()
        .SetGroupName ("Batman")
        .AddConstructor<HnaMessageHeader> ()
    ;
    return tid;
}

TypeId
HnaMessageHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void
HnaMessageHeader::Print (std::ostream &os) const
{
    os << "HNA: network=" << m_networkAddr
       << " netmask=" << (uint32_t)m_netmask;
}

uint32_t
HnaMessageHeader::GetSerializedSize (void) const
{
    return 5; // HNA is 5 bytes (4 for address + 1 for netmask)
}

void
HnaMessageHeader::Serialize (Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    
    WriteTo (i, m_networkAddr);
    i.WriteU8 (m_netmask);
}

uint32_t
HnaMessageHeader::Deserialize (Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    
    ReadFrom (i, m_networkAddr);
    m_netmask = i.ReadU8 ();
    
    return GetSerializedSize ();
}

void
HnaMessageHeader::SetNetworkAddress (Ipv4Address address)
{
    m_networkAddr = address;
}

Ipv4Address
HnaMessageHeader::GetNetworkAddress () const
{
    return m_networkAddr;
}

void
HnaMessageHeader::SetNetmask (uint8_t netmask)
{
    m_netmask = netmask;
}

uint8_t
HnaMessageHeader::GetNetmask () const
{
    return m_netmask;
}

} // namespace batman
} // namespace ns3

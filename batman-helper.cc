/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * batman-helper.cc
 * B.A.T.M.A.N. Helper Implementation for NS3
 */

#include "batman-helper.h"
#include "ns3/batman-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3 {

BatmanHelper::BatmanHelper ()
{
    m_agentFactory.SetTypeId ("ns3::batman::BatmanRoutingProtocol");
}

BatmanHelper::~BatmanHelper ()
{
}

BatmanHelper::BatmanHelper (const BatmanHelper &o)
    : m_agentFactory (o.m_agentFactory)
{
}

BatmanHelper*
BatmanHelper::Copy (void) const
{
    return new BatmanHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
BatmanHelper::Create (Ptr<Node> node) const
{
    Ptr<batman::BatmanRoutingProtocol> agent = m_agentFactory.Create<batman::BatmanRoutingProtocol> ();
    node->AggregateObject (agent);
    return agent;
}

void
BatmanHelper::Set (std::string name, const AttributeValue &value)
{
    m_agentFactory.Set (name, value);
}

Ptr<Ipv4RoutingProtocol>
BatmanHelper::Install (Ptr<Node> node) const
{
    return Create (node);
}

int64_t
BatmanHelper::AssignStreams (NodeContainer c, int64_t stream)
{
    int64_t currentStream = stream;
    Ptr<Node> node;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
        node = (*i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
        NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
        Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
        NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
        Ptr<batman::BatmanRoutingProtocol> batman = DynamicCast<batman::BatmanRoutingProtocol> (proto);
        if (batman)
        {
            currentStream += batman->AssignStreams (currentStream);
            continue;
        }
        // BATMAN may also be in a list
        Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
        if (list)
        {
            int16_t priority;
            Ptr<Ipv4RoutingProtocol> listProto;
            Ptr<batman::BatmanRoutingProtocol> listBatman;
            for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
                listProto = list->GetRoutingProtocol (i, priority);
                listBatman = DynamicCast<batman::BatmanRoutingProtocol> (listProto);
                if (listBatman)
                {
                    currentStream += listBatman->AssignStreams (currentStream);
                    break;
                }
            }
        }
    }
    return (currentStream - stream);
}

} // namespace ns3

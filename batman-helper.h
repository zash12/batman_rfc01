/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * batman-helper.h
 * B.A.T.M.A.N. Helper for NS3
 */

#ifndef BATMAN_HELPER_H
#define BATMAN_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"
#include <map>

namespace ns3 {

/**
 * \ingroup batman
 * \brief Helper class to make it easier to use B.A.T.M.A.N. routing
 */
class BatmanHelper : public Ipv4RoutingHelper
{
public:
    /**
     * \brief Constructor
     */
    BatmanHelper ();
    
    /**
     * \brief Destructor
     */
    virtual ~BatmanHelper ();
    
    /**
     * \brief Construct a BatmanHelper from another previously initialized instance
     * \param o object to copy
     */
    BatmanHelper (const BatmanHelper &o);
    
    /**
     * \returns pointer to clone of this BatmanHelper
     *
     * This method is mainly for internal use by the other helpers;
     * clients are expected to free the dynamic memory allocated by this method
     */
    BatmanHelper* Copy (void) const;
    
    /**
     * \param node the node on which the routing protocol will run
     * \returns a newly-created routing protocol
     *
     * This method will be called by ns3::InternetStackHelper::Install
     */
    virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;
    
    /**
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     *
     * This method controls the attributes of ns3::batman::BatmanRoutingProtocol
     */
    void Set (std::string name, const AttributeValue &value);
    
    /**
     * \brief Install BATMAN routing on the specified node
     * \param node Pointer to node
     * \returns Pointer to the created routing protocol
     */
    Ptr<Ipv4RoutingProtocol> Install (Ptr<Node> node) const;
    
    /**
     * \brief Assign a fixed random variable stream number to the random variables
     * used by this model
     * \param stream first stream index to use
     * \param c NodeContainer of the set of nodes for which the BatmanRoutingProtocol
     *          should be modified to use a fixed stream
     * \return the number of stream indices assigned by this helper
     */
    int64_t AssignStreams (NodeContainer c, int64_t stream);

private:
    ObjectFactory m_agentFactory; ///< Object factory for BATMAN agent
};

} // namespace ns3

#endif /* BATMAN_HELPER_H */

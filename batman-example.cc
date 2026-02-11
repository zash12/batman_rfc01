/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * batman-example.cc
 * Example simulation for B.A.T.M.A.N. routing protocol in NS3
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/batman-helper.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BatmanExample");

/**
 * \ingroup batman
 * \brief B.A.T.M.A.N. routing example
 *
 * This example demonstrates a mobile ad-hoc network using B.A.T.M.A.N.
 * routing protocol with:
 * - 20 mobile nodes in a 1000x1000m area
 * - Random waypoint mobility
 * - UDP traffic between multiple source-destination pairs
 * - Performance analysis using FlowMonitor
 */

int
main (int argc, char *argv[])
{
    // Simulation parameters
    uint32_t nNodes = 20;
    double simTime = 200.0;
    double txpDistance = 250.0;
    bool pcap = false;
    bool verbose = false;

    // Parse command line arguments
    CommandLine cmd;
    cmd.AddValue ("nodes", "Number of nodes", nNodes);
    cmd.AddValue ("time", "Simulation time (s)", simTime);
    cmd.AddValue ("txp", "Transmission distance (m)", txpDistance);
    cmd.AddValue ("pcap", "Enable PCAP tracing", pcap);
    cmd.AddValue ("verbose", "Enable verbose logging", verbose);
    cmd.Parse (argc, argv);

    // Enable logging
    if (verbose)
    {
        LogComponentEnable ("BatmanRoutingProtocol", LOG_LEVEL_INFO);
        LogComponentEnable ("BatmanExample", LOG_LEVEL_INFO);
    }

    NS_LOG_INFO ("Creating nodes...");

    // Create nodes
    NodeContainer nodes;
    nodes.Create (nNodes);

    // Configure WiFi
    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue ("DsssRate11Mbps"),
                                  "ControlMode", StringValue ("DsssRate1Mbps"));

    // Configure WiFi PHY
    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel",
                                    "MaxRange", DoubleValue (txpDistance));
    wifiPhy.SetChannel (wifiChannel.Create ());

    // Configure WiFi MAC
    WifiMacHelper wifiMac;
    wifiMac.SetType ("ns3::AdhocWifiMac");

    // Install WiFi devices
    NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

    NS_LOG_INFO ("Installing mobility model...");

    // Configure mobility
    MobilityHelper mobility;
    ObjectFactory pos;
    pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
    pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
    pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));

    Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
    
    mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                               "Speed", StringValue ("ns3::UniformRandomVariable[Min=5.0|Max=15.0]"),
                               "Pause", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=5.0]"),
                               "PositionAllocator", PointerValue (taPositionAlloc));
    mobility.SetPositionAllocator (taPositionAlloc);
    mobility.Install (nodes);

    NS_LOG_INFO ("Installing Internet stack with B.A.T.M.A.N. routing...");

    // Install B.A.T.M.A.N. routing
    BatmanHelper batman;
    batman.Set ("OgmInterval", TimeValue (Seconds (1.0)));
    
    InternetStackHelper internet;
    internet.SetRoutingHelper (batman);
    internet.Install (nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign (devices);

    NS_LOG_INFO ("Creating applications...");

    // Create UDP traffic flows
    uint16_t port = 9;
    
    // Traffic flow configuration
    struct TrafficFlow
    {
        uint32_t source;
        uint32_t sink;
        double startTime;
    };
    
    std::vector<TrafficFlow> flows = {
        {1, 10, 10.0},
        {5, 15, 15.0},
        {8, 18, 20.0},
        {12, 3, 25.0},
        {16, 7, 30.0}
    };

    // Create applications for each flow
    for (const auto& flow : flows)
    {
        if (flow.source < nNodes && flow.sink < nNodes)
        {
            // UDP Echo Server
            UdpEchoServerHelper echoServer (port);
            ApplicationContainer serverApp = echoServer.Install (nodes.Get (flow.sink));
            serverApp.Start (Seconds (1.0));
            serverApp.Stop (Seconds (simTime - 1.0));

            // UDP Echo Client
            UdpEchoClientHelper echoClient (interfaces.GetAddress (flow.sink), port);
            echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
            echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
            echoClient.SetAttribute ("PacketSize", UintegerValue (512));

            ApplicationContainer clientApp = echoClient.Install (nodes.Get (flow.source));
            clientApp.Start (Seconds (flow.startTime));
            clientApp.Stop (Seconds (simTime - 1.0));
        }
        port++;
    }

    NS_LOG_INFO ("Installing FlowMonitor...");

    // Install FlowMonitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    // Enable PCAP tracing if requested
    if (pcap)
    {
        wifiPhy.EnablePcapAll ("batman-example");
    }

    NS_LOG_INFO ("Running simulation...");

    // Schedule simulation stop
    Simulator::Stop (Seconds (simTime));

    // Run simulation
    Simulator::Run ();

    NS_LOG_INFO ("Analyzing results...");

    // Print flow monitor statistics
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    std::cout << "\n========== Flow Statistics ==========\n";
    std::cout << "FlowID\tSrc\t\tDst\t\tTx\tRx\tPDR(%)\tDelay(ms)\n";
    
    double totalPdr = 0.0;
    double totalDelay = 0.0;
    uint32_t flowCount = 0;

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        
        double pdr = 0.0;
        if (i->second.txPackets > 0)
        {
            pdr = (static_cast<double>(i->second.rxPackets) / static_cast<double>(i->second.txPackets)) * 100.0;
        }
        
        double avgDelay = 0.0;
        if (i->second.rxPackets > 0)
        {
            avgDelay = (i->second.delaySum.GetSeconds() / i->second.rxPackets) * 1000.0;
        }

        std::cout << i->first << "\t"
                  << t.sourceAddress << "\t"
                  << t.destinationAddress << "\t"
                  << i->second.txPackets << "\t"
                  << i->second.rxPackets << "\t"
                  << std::fixed << std::setprecision(2) << pdr << "\t"
                  << std::fixed << std::setprecision(3) << avgDelay << "\n";

        if (i->second.txPackets > 0)
        {
            totalPdr += pdr;
            totalDelay += avgDelay;
            flowCount++;
        }
    }

    std::cout << "=====================================\n";
    if (flowCount > 0)
    {
        std::cout << "Average PDR: " << std::fixed << std::setprecision(2) 
                  << (totalPdr / flowCount) << "%\n";
        std::cout << "Average Delay: " << std::fixed << std::setprecision(3)
                  << (totalDelay / flowCount) << " ms\n";
    }

    // Save FlowMonitor results
    monitor->SerializeToXmlFile ("batman-flowmon.xml", true, true);

    // Cleanup
    Simulator::Destroy ();

    NS_LOG_INFO ("Simulation complete!");

    return 0;
}

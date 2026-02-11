# B.A.T.M.A.N. (Better Approach To Mobile Ad-hoc Networking) Implementation
## Complete Implementation for NS2 and NS3

This repository contains a complete implementation of the B.A.T.M.A.N. routing protocol for both NS-2 and NS-3 network simulators, based on the RFC draft-openmesh-b-a-t-m-a-n-00.

---

## Table of Contents
1. [Overview](#overview)
2. [Features](#features)
3. [NS2 Implementation](#ns2-implementation)
4. [NS3 Implementation](#ns3-implementation)
5. [Installation](#installation)
6. [Usage Examples](#usage-examples)
7. [Testing](#testing)
8. [Performance Analysis](#performance-analysis)
9. [References](#references)

---

## Overview

B.A.T.M.A.N. is a proactive routing protocol for Wireless Ad-hoc Mesh Networks (MANET). The protocol:
- Maintains information about all accessible nodes
- Uses Originator Messages (OGM) for route discovery
- Employs statistical analysis for route quality assessment
- Supports bidirectional link checking
- Implements opportunistic routing deletion

### Key Protocol Features

1. **Sequence Number Based**: Uses sequence numbers for loop-free routing
2. **Sliding Window**: Implements a sliding window mechanism for packet counting
3. **Neighbor Ranking**: Ranks neighbors based on received OGM count
4. **Bidirectional Link Check**: Ensures symmetric communication
5. **Gateway Support**: Supports internet gateway announcement and selection
6. **HNA Messages**: Host Network Announcement for network reachability

---

## Features

### Implemented Features

- ✅ Originator Message (OGM) broadcasting
- ✅ Sequence number management with wraparound handling
- ✅ Sliding window mechanism (configurable window size)
- ✅ Neighbor ranking and best route selection
- ✅ Bidirectional link verification
- ✅ Route table management and purging
- ✅ TTL-based packet forwarding
- ✅ Gateway announcement and selection
- ✅ HNA (Host Network Announcement) support
- ✅ Broadcast duplicate detection
- ✅ Opportunistic route deletion policy
- ✅ Transmit Quality (TQ) metric calculation

### Protocol Constants

```
VERSION = 4
TTL_MIN = 2
TTL_MAX = 255
SEQNO_MAX = 65535
ORIGINATOR_INTERVAL = 1.0 second
WINDOW_SIZE = 128
PURGE_TIMEOUT = 10 × WINDOW_SIZE × ORIGINATOR_INTERVAL
BATMAN_PORT = 4305
```

---

## NS2 Implementation

### File Structure

```
ns2/
├── batman_pkt.h          # Packet format definitions
├── batman_rtable.h       # Routing table structure
├── batman_rtable.cc      # Routing table implementation
├── batman.h              # Main agent header
├── batman.cc             # Main agent implementation
├── batman_example.tcl    # Example simulation script
└── INSTALL.md           # Installation instructions
```

### Key Classes

1. **BATMANAgent**: Main routing agent
   - Inherits from NS2 Agent class
   - Manages OGM broadcasting and forwarding
   - Handles route table updates

2. **BATMANRoutingTable**: Routing table management
   - Stores originator entries
   - Manages neighbor information
   - Implements route lookup and maintenance

3. **OriginatorEntry**: Per-originator routing information
   - Tracks sequence numbers
   - Maintains neighbor rankings
   - Stores HNA announcements

4. **NeighborInfo**: Per-neighbor statistics
   - Sliding window of received packets
   - TQ (Transmit Quality) calculation
   - Last packet timestamp

---

## NS3 Implementation

### File Structure

```
ns3/batman/
├── model/
│   ├── batman-packet.h          # Packet headers
│   ├── batman-packet.cc         # Packet implementation
│   ├── batman-routing-protocol.h   # Main protocol header
│   ├── batman-routing-protocol.cc  # Protocol implementation
│   └── batman-rtable.h          # Routing table
├── helper/
│   ├── batman-helper.h          # Helper class
│   └── batman-helper.cc
├── examples/
│   ├── batman-example.cc        # Basic example
│   └── batman-performance.cc    # Performance evaluation
├── test/
│   └── batman-test-suite.cc     # Unit tests
└── wscript                      # Build configuration
```

### Key Classes

1. **BatmanRoutingProtocol**: Main routing protocol
   - Implements Ipv4RoutingProtocol interface
   - Manages packet routing decisions
   - Handles OGM generation and processing

2. **BatmanHelper**: Configuration and installation helper
   - Simplifies protocol deployment
   - Configures protocol parameters
   - Installs on node containers

---

## Installation

### Prerequisites (Ubuntu 22.04)

```bash
# Common dependencies
sudo apt-get update
sudo apt-get install build-essential gcc g++ python3
sudo apt-get install git wget cmake

# For NS2
sudo apt-get install gcc-9 g++-9
sudo apt-get install tcl8.6 tcl8.6-dev tk8.6 tk8.6-dev
sudo apt-get install libx11-dev libxmu-dev

# For NS3
sudo apt-get install python3-pip
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```

### NS2 Installation

See detailed instructions in `ns2/INSTALL.md`

**Quick Start:**

1. Download and patch NS-2.35
2. Copy B.A.T.M.A.N. files to ns-2.35/batman/
3. Modify Makefile.in, packet.h, and ns-packet.tcl
4. Recompile NS2
5. Run example: `ns batman_example.tcl`

### NS3 Installation

See detailed instructions in `ns3/INSTALL.md`

**Quick Start:**

1. Clone NS-3 (version 3.35 or later recommended)
```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev
```

2. Copy B.A.T.M.A.N. module
```bash
cp -r batman src/
```

3. Configure and build
```bash
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

4. Run example
```bash
./ns3 run batman-example
```

---

## Usage Examples

### NS2 Example

```tcl
# Create simulator
set ns [new Simulator]

# Configure nodes with B.A.T.M.A.N.
$ns node-config -adhocRouting BATMAN

# Create nodes
for {set i 0} {$i < 10} {incr i} {
    set node_($i) [$ns node]
}

# Configure B.A.T.M.A.N. agent
set batman [$node_(0) set ragent_]
$batman ttl 64
$batman gateway 128 5000  # Set as gateway

# Run simulation
$ns run
```

### NS3 Example

```cpp
// Create nodes
NodeContainer nodes;
nodes.Create (20);

// Install internet stack with B.A.T.M.A.N.
BatmanHelper batman;
InternetStackHelper internet;
internet.SetRoutingHelper (batman);
internet.Install (nodes);

// Configure wireless
WifiHelper wifi;
YansWifiPhyHelper wifiPhy;
WifiMacHelper wifiMac;
// ... configure wifi ...

// Assign IP addresses
Ipv4AddressHelper address;
address.SetBase ("10.1.1.0", "255.255.255.0");
address.Assign (devices);

// Run simulation
Simulator::Stop (Seconds (200.0));
Simulator::Run ();
```

---

## Testing

### NS2 Testing

```bash
# Run example simulation
cd ns2
ns batman_example.tcl

# Analyze results
grep "^s" batman_trace.tr | wc -l  # Sent packets
grep "^r" batman_trace.tr | wc -l  # Received packets
grep "^D" batman_trace.tr | wc -l  # Dropped packets

# Visualize with NAM
nam batman_nam.nam
```

### NS3 Testing

```bash
# Run unit tests
./test.py --suite=batman

# Run examples
./ns3 run batman-example
./ns3 run batman-performance

# Enable logging
export NS_LOG=BatmanRoutingProtocol=level_all
./ns3 run batman-example
```

### Performance Metrics

The implementation tracks:
- Packet Delivery Ratio (PDR)
- End-to-End Delay
- Routing Overhead
- Route Discovery Time
- Route Stability
- Energy Consumption (NS3 with energy model)

---

## Performance Analysis

### Analysis Scripts

**AWK script for packet statistics (NS2):**

```awk
# pdr.awk - Calculate Packet Delivery Ratio
BEGIN {
    sent = 0;
    recv = 0;
}
{
    if ($1 == "s" && $7 == "cbr") sent++;
    if ($1 == "r" && $7 == "cbr") recv++;
}
END {
    if (sent > 0) {
        printf "PDR: %.2f%%\n", (recv/sent)*100;
    }
}
```

Usage:
```bash
awk -f pdr.awk batman_trace.tr
```

### Python Analysis (NS3)

```python
# analyze.py - Parse and analyze NS3 flowmonitor output
import xml.etree.ElementTree as ET
import sys

def analyze_flow(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    
    total_pdr = 0
    total_delay = 0
    flows = 0
    
    for flow in root.findall('FlowStats/Flow'):
        tx_packets = int(flow.get('txPackets'))
        rx_packets = int(flow.get('rxPackets'))
        delay_sum = float(flow.get('delaySum').replace('ns', ''))
        
        if tx_packets > 0:
            pdr = (rx_packets / tx_packets) * 100
            avg_delay = (delay_sum / rx_packets) / 1e9 if rx_packets > 0 else 0
            
            total_pdr += pdr
            total_delay += avg_delay
            flows += 1
    
    if flows > 0:
        print(f"Average PDR: {total_pdr/flows:.2f}%")
        print(f"Average Delay: {total_delay/flows:.6f}s")

if __name__ == "__main__":
    analyze_flow(sys.argv[1])
```

---

## References

### Primary RFC
- **draft-openmesh-b-a-t-m-a-n-00** (March 2008)
  - Authors: A. Neumann, C. Aichele, M. Lindner, S. Wunderlich

### Research Papers

1. **"Better Approach To Mobile Ad-hoc Networking (B.A.T.M.A.N.)"**
   - Neumann et al., 2008
   - IETF Internet-Draft

2. **"An Even Better Approach – Improving the B.A.T.M.A.N. Protocol Through Formal Modelling and Analysis"**
   - Fehnker et al., NASA Formal Methods 2018

3. **"Performance Evaluation and Optimization of B.A.T.M.A.N. V Routing for Aerial and Ground-based Mobile Ad-hoc Networks"**
   - Sliwa et al., 2019

4. **"BATMAN Store-and-Forward: the Best of the Two Worlds"**
   - Delosières and Nadjm-Tehrani, 2012

5. **"Routing Performance of Wireless Mesh Networks"**
   - Seither et al., 2011

6. **"A simple pragmatic approach to mesh routing using BATMAN"**
   - David Johnson, IFIP 2008

### Online Resources
- Official B.A.T.M.A.N. project: https://www.open-mesh.org/
- NS-2 Documentation: https://www.isi.edu/nsnam/ns/
- NS-3 Documentation: https://www.nsnam.org/

---

## Implementation Details

### Sequence Number Handling

The implementation properly handles sequence number wraparound (0-65535):

```cpp
bool seqno_greater_than(uint16_t s1, uint16_t s2) {
    return ((s1 > s2) && (s1 - s2 < SEQNO_MAX/2)) ||
           ((s2 > s1) && (s2 - s1 > SEQNO_MAX/2));
}
```

### Sliding Window Algorithm

```cpp
void updateWindow(uint16_t seqno) {
    // Add to window
    sliding_window.insert(seqno);
    
    // Remove old entries
    uint16_t lower_bound = (curr_seqno >= WINDOW_SIZE) ?
                           (curr_seqno - WINDOW_SIZE + 1) : 0;
    
    // Prune entries outside window
    for (auto it = sliding_window.begin(); it != sliding_window.end();) {
        if (seqno_less_than(*it, lower_bound)) {
            sliding_window.erase(it++);
        } else {
            ++it;
        }
    }
    
    packet_count = sliding_window.size();
}
```

### TQ (Transmit Quality) Calculation

```cpp
double calculateTQ() {
    if (packet_count == 0)
        return 0.0;
    
    // TQ = (packets_received / WINDOW_SIZE)
    tq_value = (double)packet_count / (double)WINDOW_SIZE;
    return tq_value;
}
```

---

## Troubleshooting

### Common Issues

1. **NS2: "command not found: ns"**
   - Solution: Check PATH environment variable, source ~/.bashrc

2. **NS2: "can't find package Tcl"**
   - Solution: Install tcl8.6-dev, set TCL_LIBRARY

3. **NS3: "No module named 'batman'"**
   - Solution: Copy batman folder to src/, rebuild

4. **Compilation errors**
   - NS2: Use gcc-9/g++-9
   - NS3: Check NS-3 version (≥3.35 recommended)

5. **No routes established**
   - Check OGM interval and PURGE_TIMEOUT
   - Verify wireless range and node positions
   - Enable debug logging

---

## Contributing

To contribute to this implementation:

1. Fork the repository
2. Create a feature branch
3. Implement your changes
4. Add tests
5. Submit a pull request

---

## License

This implementation is provided for educational and research purposes. Please cite the original RFC and this implementation in academic work.

---

## Authors

Implementation by: [Your Name]
Based on RFC: draft-openmesh-b-a-t-m-a-n-00
Original Protocol: Neumann, Aichele, Lindner, Wunderlich

---

## Version History

- v1.0.0 (2026-02-11): Initial implementation
  - Complete NS2 implementation
  - Complete NS3 implementation
  - Documentation and examples

---

For detailed installation instructions, see:
- NS2: `ns2/INSTALL.md`
- NS3: `ns3/INSTALL.md`

For example simulations, see:
- NS2: `ns2/batman_example.tcl`
- NS3: `ns3/batman/examples/batman-example.cc`

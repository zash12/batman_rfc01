# NS3 B.A.T.M.A.N. Installation Guide
## Complete Installation Instructions for Ubuntu 22.04

---

## Prerequisites

### Install Required Packages

```bash
# Update system
sudo apt-get update
sudo apt-get upgrade

# Install build tools
sudo apt-get install g++ python3 python3-dev
sudo apt-get install cmake ninja-build
sudo apt-get install git mercurial

# Install NS3 dependencies
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
sudo apt-get install python3-setuptools python3-pip
sudo apt-get install libxml2 libxml2-dev
sudo apt-get install libboost-all-dev
sudo apt-get install libsqlite3-dev

# Install optional packages for visualization
sudo apt-get install gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython3
sudo apt-get install tcpdump wireshark

# Install Python dependencies
pip3 install cppyy
```

---

## NS-3 Installation

### Method 1: Development Version (Recommended)

```bash
# Clone NS-3 repository
cd ~
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev

# Check out stable release (optional)
git checkout ns-3.39
```

### Method 2: Release Tarball

```bash
# Download NS-3 release
cd ~
wget https://www.nsnam.org/releases/ns-allinone-3.39.tar.bz2
tar xjf ns-allinone-3.39.tar.bz2
cd ns-allinone-3.39/ns-3.39
```

---

## B.A.T.M.A.N. Module Installation

### Step 1: Copy B.A.T.M.A.N. Module

```bash
# Assuming you're in the ns-3 directory (ns-3-dev or ns-3.39)
cd src

# Copy the batman directory
cp -r /path/to/batman-ns-implementation/ns3/batman ./

# Verify structure
ls batman/
# Should see: model/ helper/ examples/ test/ wscript doc/
```

### Step 2: Verify File Structure

```bash
tree batman/
# Output should be:
# batman/
# ├── model/
# │   ├── batman-packet.h
# │   ├── batman-packet.cc
# │   ├── batman-routing-protocol.h
# │   └── batman-routing-protocol.cc
# ├── helper/
# │   ├── batman-helper.h
# │   └── batman-helper.cc
# ├── examples/
# │   ├── batman-example.cc
# │   └── wscript
# ├── test/
# │   └── batman-test-suite.cc
# ├── wscript
# └── doc/
```

---

## Build NS-3 with B.A.T.M.A.N.

### Configure NS-3

```bash
# Return to ns-3 root directory
cd ~/ns-3-dev  # or cd ~/ns-allinone-3.39/ns-3.39

# Configure with examples and tests enabled
./ns3 configure --enable-examples --enable-tests

# Optional: Enable specific features
./ns3 configure --enable-examples --enable-tests --enable-asserts --enable-logs
```

### Build

```bash
# Build NS-3 with B.A.T.M.A.N.
./ns3 build

# Or build with verbose output
./ns3 build -v

# Build only batman module (faster for testing)
./ns3 build batman
```

### Verify Build

```bash
# Check if batman module compiled successfully
./ns3 show targets | grep batman

# Should output something like:
# batman
# batman-example
# batman-test-suite
```

---

## Testing Installation

### Run Example Simulation

```bash
# Basic run
./ns3 run batman-example

# Run with parameters
./ns3 run "batman-example --nodes=30 --time=300 --verbose=true"

# Run with command-line help
./ns3 run "batman-example --help"
```

### Run Unit Tests

```bash
# Run all batman tests
./test.py --suite=batman

# Run with verbose output
./test.py --suite=batman --verbose
```

---

## Configuration and Usage

### Basic Configuration in Simulation

```cpp
#include "ns3/batman-helper.h"

// Create nodes
NodeContainer nodes;
nodes.Create (20);

// Install B.A.T.M.A.N.
BatmanHelper batman;

// Configure parameters (optional)
batman.Set ("OgmInterval", TimeValue (Seconds (1.0)));
batman.Set ("PurgeTimeout", TimeValue (Seconds (1280.0)));
batman.Set ("Ttl", UintegerValue (64));

// Install internet stack with B.A.T.M.A.N.
InternetStackHelper internet;
internet.SetRoutingHelper (batman);
internet.Install (nodes);
```

### Enable Logging

```bash
# Enable all B.A.T.M.A.N. logging
export NS_LOG=BatmanRoutingProtocol=level_all

# Enable specific logging levels
export NS_LOG=BatmanRoutingProtocol=info

# Enable logging for multiple components
export NS_LOG="BatmanRoutingProtocol=level_all|BatmanHelper=info"

# Run simulation with logging
./ns3 run batman-example
```

### Generate PCAP Traces

```cpp
// In your simulation code
wifiPhy.EnablePcapAll ("batman-trace");

// Or for specific interfaces
wifiPhy.EnablePcap ("batman-trace", devices.Get (0));
```

---

## Advanced Features

### Gateway Configuration

```cpp
// Configure node as gateway
Ptr<Ipv4> ipv4 = nodes.Get (0)->GetObject<Ipv4> ();
Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
Ptr<batman::BatmanRoutingProtocol> batman = DynamicCast<batman::BatmanRoutingProtocol> (proto);
if (batman)
{
    batman->SetGateway (128, 5000);  // gateway class, port
}
```

### Visualization with NetAnim

```cpp
#include "ns3/netanim-module.h"

// Create animation
AnimationInterface anim ("batman-animation.xml");

// Set node positions
for (uint32_t i = 0; i < nodes.GetN (); ++i)
{
    anim.SetConstantPosition (nodes.Get (i), x, y);
}

// Run simulation
Simulator::Run ();
```

Then view with:
```bash
netanim batman-animation.xml
```

---

## Performance Analysis

### Using FlowMonitor

```cpp
#include "ns3/flow-monitor-module.h"

// Install FlowMonitor
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

// Run simulation
Simulator::Run ();

// Print statistics
monitor->CheckForLostPackets ();
Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
monitor->SerializeToXmlFile ("flowmon-results.xml", true, true);

// Process results
std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
for (auto& stat : stats)
{
    // Calculate PDR, delay, etc.
}
```

### Analyze FlowMonitor XML

```python
# analyze-flowmon.py
import xml.etree.ElementTree as ET

tree = ET.parse('flowmon-results.xml')
root = tree.getroot()

for flow in root.findall('FlowStats/Flow'):
    tx = int(flow.get('txPackets'))
    rx = int(flow.get('rxPackets'))
    
    if tx > 0:
        pdr = (rx / tx) * 100
        print(f"Flow {flow.get('flowId')}: PDR = {pdr:.2f}%")
```

---

## Debugging

### Enable Debug Output

```bash
# Configure with debug
./ns3 configure --enable-examples --enable-tests --build-profile=debug

# Build
./ns3 build

# Run with GDB
./ns3 shell
gdb --args ./build/scratch/my-simulation
```

### Common Issues and Solutions

**Issue 1: Module not found**
```bash
# Solution: Verify batman directory is in src/
ls src/batman/

# Reconfigure and rebuild
./ns3 clean
./ns3 configure --enable-examples
./ns3 build
```

**Issue 2: Compilation errors**
```bash
# Check NS-3 version compatibility
./ns3 --version

# Use compatible version (3.35+)
```

**Issue 3: Runtime errors**
```bash
# Enable logging to diagnose
export NS_LOG="*=level_all"
./ns3 run batman-example 2>&1 | less
```

---

## Customization

### Modify Protocol Parameters

Edit `batman/model/batman-routing-protocol.h`:

```cpp
// Default values
#define WINDOW_SIZE 128
#define SEQNO_MAX 65535
#define PURGE_TIMEOUT_FACTOR 10
```

Then rebuild:
```bash
./ns3 build batman
```

### Add Custom Metrics

Create custom tracing in your simulation:

```cpp
// Trace routing table changes
Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
                MakeCallback (&MacTxCallback));
```

---

## Validation

### Compare with Reference Implementation

```bash
# Run validation tests
./test.py --suite=batman --verbose

# Check test results
cat test-results.xml
```

### Benchmark Performance

```bash
# Run performance tests
./ns3 run "batman-performance --nodes=50 --time=500"

# Compare with other protocols
./ns3 run "aodv-performance --nodes=50 --time=500"
./ns3 run "olsr-performance --nodes=50 --time=500"
```

---

## Documentation

### Generate Doxygen Documentation

```bash
# Configure with documentation
./ns3 configure --enable-examples --enable-tests

# Generate docs
cd doc
make html

# View documentation
firefox build/html/index.html
```

---

## Troubleshooting

### Check NS-3 Installation

```bash
# Verify Python bindings
./ns3 shell
python3
>>> import ns.core
>>> import ns.network
>>> import ns.internet
```

### Verify B.A.T.M.A.N. Integration

```bash
# Check if module is recognized
./ns3 show modules | grep batman

# List batman programs
./ns3 show programs | grep batman
```

### Clean Build

```bash
# Remove all build artifacts
./ns3 clean

# Reconfigure
./ns3 configure --enable-examples --enable-tests

# Rebuild
./ns3 build
```

---

## Environment Setup

Add to `~/.bashrc` for convenience:

```bash
# NS-3 environment
export NS3_HOME=~/ns-3-dev
alias ns3="cd $NS3_HOME && ./ns3"
alias ns3-run="cd $NS3_HOME && ./ns3 run"
alias ns3-build="cd $NS3_HOME && ./ns3 build"

# Logging presets
alias ns3-log-all="export NS_LOG='*=level_all'"
alias ns3-log-info="export NS_LOG='*=level_info'"
alias ns3-log-batman="export NS_LOG='BatmanRoutingProtocol=level_all'"
```

---

## Next Steps

1. Run the example simulation
2. Modify parameters and observe behavior
3. Create custom scenarios
4. Compare with other routing protocols
5. Analyze performance metrics
6. Publish your research!

---

## Support

For issues and questions:
- Check NS-3 documentation: https://www.nsnam.org/documentation/
- NS-3 users mailing list: ns-3-users@googlegroups.com
- B.A.T.M.A.N. project: https://www.open-mesh.org/

---

## Quick Reference

```bash
# Install NS-3
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev

# Add B.A.T.M.A.N.
cp -r /path/to/batman src/

# Build
./ns3 configure --enable-examples --enable-tests
./ns3 build

# Run
./ns3 run batman-example

# Test
./test.py --suite=batman

# Clean
./ns3 clean
```

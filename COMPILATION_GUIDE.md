# Complete B.A.T.M.A.N. Compilation & Patching Guide
## Ubuntu 22.04 - NS2 and NS3

This guide provides complete step-by-step instructions for compiling and patching both NS2 and NS3 with B.A.T.M.A.N. routing protocol on Ubuntu 22.04.

---

## Table of Contents
1. [System Preparation](#system-preparation)
2. [NS2 Complete Setup](#ns2-complete-setup)
3. [NS3 Complete Setup](#ns3-complete-setup)
4. [Verification](#verification)
5. [Troubleshooting](#troubleshooting)

---

## System Preparation

### Update System

```bash
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get autoremove -y
```

### Install Common Dependencies

```bash
# Build essentials
sudo apt-get install -y build-essential autoconf automake
sudo apt-get install -y git wget curl vim
sudo apt-get install -y python3 python3-pip python3-dev

# Compilers
sudo apt-get install -y gcc-9 g++-9 gcc-11 g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90

# Development libraries
sudo apt-get install -y libxml2 libxml2-dev
sudo apt-get install -y libboost-all-dev
sudo apt-get install -y libsqlite3-dev
```

---

## NS2 Complete Setup

### Step 1: Install NS2 Dependencies

```bash
# TCL/TK dependencies
sudo apt-get install -y tcl8.6 tcl8.6-dev tk8.6 tk8.6-dev

# X11 libraries
sudo apt-get install -y libx11-dev libxmu-dev libxext-dev

# Additional tools
sudo apt-get install -y perl xgraph gnuplot
```

### Step 2: Download and Extract NS-2.35

```bash
cd ~
mkdir ns2-batman
cd ns2-batman

# Download NS-2.35
wget https://sourceforge.net/projects/nsnam/files/allinone/ns-allinone-2.35/ns-allinone-2.35.tar.gz
tar -xzf ns-allinone-2.35.tar.gz
cd ns-allinone-2.35
```

### Step 3: Create Ubuntu 22.04 Compatibility Patches

Create `ns2-ubuntu22.patch`:

```bash
cat > ns2-ubuntu22.patch << 'EOF'
--- ns-2.35/linkstate/ls.h.orig 2024-01-01 00:00:00.000000000 +0000
+++ ns-2.35/linkstate/ls.h      2024-01-01 00:00:00.000000000 +0000
@@ -137,7 +137,7 @@
        void eraseAll() { erase(baseMap::begin(), baseMap::end()); }
        iterator lookup(int nodeId) { return find(nodeId); }
 };
-typedef map<int, LsIdSeq> LsIdSeqMap;
+typedef std::map<int, LsIdSeq> LsIdSeqMap;
 
 /* NOTE: The message fields need to be in network byte order,
  *       the internal data structures should be in host-byte order.

--- ns-2.35/tools/ranvar.h.orig 2024-01-01 00:00:00.000000000 +0000
+++ ns-2.35/tools/ranvar.h      2024-01-01 00:00:00.000000000 +0000
@@ -200,2 +200,3 @@
+#include <cstdlib>
 #include "rng.h"

--- ns-2.35/common/mobilenode.h.orig    2024-01-01 00:00:00.000000000 +0000
+++ ns-2.35/common/mobilenode.h 2024-01-01 00:00:00.000000000 +0000
@@ -41,6 +41,7 @@
 #include "topography.h"
 #include "phy.h"
 #include "propagation.h"
+#include <cstring>
 
 class MobileNode : public Node {
 public:
EOF
```

Apply patches:

```bash
patch -p0 < ns2-ubuntu22.patch
```

### Step 4: Configure Build Environment

```bash
# Set compiler versions
export CC=gcc-9
export CXX=g++-9
export CPP=cpp-9

# Configure installation paths
export NS2_HOME=$HOME/ns2-batman/ns-allinone-2.35
export PATH=$NS2_HOME/bin:$NS2_HOME/tcl8.5.10/unix:$NS2_HOME/tk8.5.10/unix:$PATH
export LD_LIBRARY_PATH=$NS2_HOME/otcl-1.14:$NS2_HOME/lib:$LD_LIBRARY_PATH
export TCL_LIBRARY=$NS2_HOME/tcl8.5.10/library
```

### Step 5: Build NS-2.35

```bash
cd ~/ns2-batman/ns-allinone-2.35

# Run installer
./install 2>&1 | tee install.log

# Check for successful installation
if [ $? -eq 0 ]; then
    echo "NS-2 installation successful!"
else
    echo "NS-2 installation failed. Check install.log"
    exit 1
fi
```

### Step 6: Add B.A.T.M.A.N. Protocol

```bash
cd ns-allinone-2.35/ns-2.35

# Create batman directory
mkdir batman

# Copy B.A.T.M.A.N. files
cp /path/to/batman-ns-implementation/ns2/batman_pkt.h batman/
cp /path/to/batman-ns-implementation/ns2/batman_rtable.h batman/
cp /path/to/batman-ns-implementation/ns2/batman_rtable.cc batman/
cp /path/to/batman-ns-implementation/ns2/batman.h batman/
cp /path/to/batman-ns-implementation/ns2/batman.cc batman/
```

### Step 7: Modify NS2 Core Files

#### Modify packet.h

```bash
cd ~/ns2-batman/ns-allinone-2.35/ns-2.35/common

# Backup original
cp packet.h packet.h.bak

# Edit packet.h - Add to enum packet_t (around line 80)
# Add: PT_BATMAN,

sed -i '/PT_NTYPE/i\    PT_BATMAN,' packet.h

# Add to name_ array initialization (around line 240)
# Add: name_[PT_BATMAN] = "BATMAN";

# Find the line with PT_NTYPE and add before it
awk '/name_\[PT_NTYPE\]/ { print "    name_[PT_BATMAN] = \"BATMAN\";"; } { print; }' \
    packet.h > packet.h.tmp && mv packet.h.tmp packet.h
```

#### Modify ns-packet.tcl

```bash
cd ~/ns2-batman/ns-allinone-2.35/ns-2.35/tcl/lib

# Backup original
cp ns-packet.tcl ns-packet.tcl.bak

# Add BATMAN to packet types list
sed -i '/PGM/a\    BATMAN' ns-packet.tcl
```

#### Modify Makefile.in

```bash
cd ~/ns2-batman/ns-allinone-2.35/ns-2.35

# Backup original
cp Makefile.in Makefile.in.bak

# Add BATMAN object files to OBJ_CC
# Find the line containing "diffusion3/" and add batman objects after it

awk '/diffusion3\/ns\/diffrtg.o/ {
    print $0;
    print "\tbatman/batman.o \\";
    print "\tbatman/batman_rtable.o \\";
    next;
} { print; }' Makefile.in > Makefile.in.tmp && mv Makefile.in.tmp Makefile.in
```

### Step 8: Recompile NS2 with B.A.T.M.A.N.

```bash
cd ~/ns2-batman/ns-allinone-2.35/ns-2.35

# Clean previous build
make clean

# Build with B.A.T.M.A.N.
make 2>&1 | tee build.log

# Check for compilation errors
if [ $? -eq 0 ]; then
    echo "NS-2 with B.A.T.M.A.N. compiled successfully!"
else
    echo "Compilation failed. Check build.log"
    exit 1
fi
```

### Step 9: Test NS2 Installation

```bash
# Copy test script
cp /path/to/batman-ns-implementation/ns2/batman_example.tcl .

# Run test
./ns batman_example.tcl

# Check output
if [ -f batman_trace.tr ] && [ -f batman_nam.nam ]; then
    echo "Test successful! Files generated:"
    ls -lh batman_trace.tr batman_nam.nam
else
    echo "Test failed. Files not generated."
fi
```

### Step 10: Set Permanent Environment Variables

```bash
# Add to ~/.bashrc
cat >> ~/.bashrc << 'EOF'

# NS-2 with B.A.T.M.A.N. environment
export NS2_HOME=$HOME/ns2-batman/ns-allinone-2.35
export PATH=$NS2_HOME/bin:$NS2_HOME/tcl8.5.10/unix:$NS2_HOME/tk8.5.10/unix:$PATH
export LD_LIBRARY_PATH=$NS2_HOME/otcl-1.14:$NS2_HOME/lib:$LD_LIBRARY_PATH
export TCL_LIBRARY=$NS2_HOME/tcl8.5.10/library

# NS-2 aliases
alias ns2='cd $NS2_HOME/ns-2.35 && ./ns'
alias ns2-dir='cd $NS2_HOME/ns-2.35'
EOF

# Reload
source ~/.bashrc
```

---

## NS3 Complete Setup

### Step 1: Install NS3 Dependencies

```bash
# Python dependencies
sudo apt-get install -y python3-setuptools python3-pip
pip3 install --user cppyy

# Build tools
sudo apt-get install -y cmake ninja-build ccache

# Qt5 for visualization
sudo apt-get install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools

# Optional visualization tools
sudo apt-get install -y gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo
sudo apt-get install -y python3-pygraphviz gir1.2-gtk-3.0 ipython3

# Network tools
sudo apt-get install -y tcpdump wireshark
```

### Step 2: Download and Setup NS-3

```bash
cd ~
mkdir ns3-batman
cd ns3-batman

# Clone NS-3 development version
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev

# OR download release version
# wget https://www.nsnam.org/releases/ns-allinone-3.39.tar.bz2
# tar xjf ns-allinone-3.39.tar.bz2
# cd ns-allinone-3.39/ns-3.39

# Check version
./ns3 --version
```

### Step 3: Install B.A.T.M.A.N. Module

```bash
cd ~/ns3-batman/ns-3-dev/src

# Copy batman module
cp -r /path/to/batman-ns-implementation/ns3/batman ./

# Verify structure
tree batman/ -L 2
```

### Step 4: Configure NS-3

```bash
cd ~/ns3-batman/ns-3-dev

# Basic configuration
./ns3 configure --enable-examples --enable-tests

# Advanced configuration with optimizations
./ns3 configure \
    --enable-examples \
    --enable-tests \
    --enable-logs \
    --enable-asserts \
    --build-profile=optimized

# Debug configuration (for development)
# ./ns3 configure --enable-examples --enable-tests --build-profile=debug
```

### Step 5: Build NS-3 with B.A.T.M.A.N.

```bash
# Build all modules
./ns3 build 2>&1 | tee build.log

# OR build only batman module (faster)
./ns3 build batman

# Check build status
if [ $? -eq 0 ]; then
    echo "NS-3 with B.A.T.M.A.N. compiled successfully!"
    ./ns3 show modules | grep batman
else
    echo "Compilation failed. Check build.log"
    exit 1
fi
```

### Step 6: Test NS3 Installation

```bash
# Run example
./ns3 run batman-example

# Run with parameters
./ns3 run "batman-example --nodes=20 --time=200 --verbose=true"

# Run tests
./test.py --suite=batman --verbose
```

### Step 7: Set Permanent Environment Variables

```bash
# Add to ~/.bashrc
cat >> ~/.bashrc << 'EOF'

# NS-3 with B.A.T.M.A.N. environment
export NS3_HOME=$HOME/ns3-batman/ns-3-dev

# NS-3 aliases
alias ns3='cd $NS3_HOME && ./ns3'
alias ns3-run='cd $NS3_HOME && ./ns3 run'
alias ns3-build='cd $NS3_HOME && ./ns3 build'
alias ns3-test='cd $NS3_HOME && ./test.py'
alias ns3-dir='cd $NS3_HOME'

# Logging shortcuts
alias ns3-log-all='export NS_LOG="*=level_all"'
alias ns3-log-batman='export NS_LOG="BatmanRoutingProtocol=level_all"'
EOF

# Reload
source ~/.bashrc
```

---

## Verification

### Verify NS2 Installation

```bash
# Check NS2 version
cd $NS2_HOME/ns-2.35
./ns -version

# Test basic simulation
./ns $NS2_HOME/ns-2.35/tcl/ex/simple.tcl

# Test B.A.T.M.A.N.
./ns batman_example.tcl
```

### Verify NS3 Installation

```bash
# Check NS3 version
cd $NS3_HOME
./ns3 --version

# List modules
./ns3 show modules | grep batman

# List examples
./ns3 show programs | grep batman

# Run tests
./test.py --suite=batman
```

### Performance Benchmarks

**NS2 Performance Test:**
```bash
cd $NS2_HOME/ns-2.35
time ns batman_example.tcl

# Check output
awk -f analyze_trace.awk batman_trace.tr
```

**NS3 Performance Test:**
```bash
cd $NS3_HOME
time ./ns3 run "batman-example --nodes=50 --time=300"

# Analyze results
python3 analyze-flowmon.py batman-flowmon.xml
```

---

## Troubleshooting

### NS2 Common Issues

**Issue: "ns: command not found"**
```bash
# Solution:
export PATH=$HOME/ns2-batman/ns-allinone-2.35/bin:$PATH
source ~/.bashrc
```

**Issue: "can't find package Tcl"**
```bash
# Solution:
export TCL_LIBRARY=$HOME/ns2-batman/ns-allinone-2.35/tcl8.5.10/library
sudo apt-get install tcl8.6-dev
```

**Issue: Compilation errors with GCC**
```bash
# Solution: Use GCC-9
export CC=gcc-9
export CXX=g++-9
make clean && make
```

**Issue: "undefined reference to BATMAN"**
```bash
# Solution: Verify Makefile.in includes batman objects
grep -A 5 "batman/batman.o" Makefile.in

# If not found, re-edit Makefile.in and rebuild
make clean && make
```

### NS3 Common Issues

**Issue: Module not found**
```bash
# Solution:
ls src/batman  # Verify directory exists
./ns3 clean
./ns3 configure --enable-examples
./ns3 build
```

**Issue: Python errors**
```bash
# Solution:
pip3 install --user cppyy pybindgen
export PYTHONPATH=$NS3_HOME/build/bindings/python:$PYTHONPATH
```

**Issue: Build fails**
```bash
# Solution: Check dependencies
sudo apt-get install -y libboost-all-dev libxml2-dev
./ns3 clean
./ns3 configure
./ns3 build
```

---

## Post-Installation

### Create Workspace Directory

```bash
mkdir -p ~/batman-simulations/{ns2,ns3}
```

### Copy Example Scripts

```bash
# NS2
cp $NS2_HOME/ns-2.35/batman_example.tcl ~/batman-simulations/ns2/
cp /path/to/batman-ns-implementation/ns2/*.tcl ~/batman-simulations/ns2/

# NS3
cp /path/to/batman-ns-implementation/ns3/batman/examples/*.cc ~/batman-simulations/ns3/
```

### Documentation

**Generate NS3 Documentation:**
```bash
cd $NS3_HOME
./ns3 docs doxygen

# View in browser
firefox build/doc/html/index.html
```

---

## Quick Start Commands

### NS2 Quick Start
```bash
# Navigate to NS2
cd $NS2_HOME/ns-2.35

# Run simulation
./ns batman_example.tcl

# View in NAM
nam batman_nam.nam
```

### NS3 Quick Start
```bash
# Navigate to NS3
cd $NS3_HOME

# Run simulation
./ns3 run batman-example

# Run with logging
NS_LOG="BatmanRoutingProtocol=info" ./ns3 run batman-example

# Analyze results
python3 analyze-flowmon.py batman-flowmon.xml
```

---

## Backup and Version Control

### Backup Configurations

```bash
# Backup NS2
tar -czf ns2-batman-backup.tar.gz ~/ns2-batman/ns-allinone-2.35/ns-2.35/batman

# Backup NS3
tar -czf ns3-batman-backup.tar.gz ~/ns3-batman/ns-3-dev/src/batman
```

### Git Repository (Optional)

```bash
cd ~/batman-simulations
git init
git add .
git commit -m "Initial B.A.T.M.A.N. implementation"
```

---

## Conclusion

You now have a complete B.A.T.M.A.N. routing protocol implementation for both NS2 and NS3 on Ubuntu 22.04!

**Next steps:**
1. Run example simulations
2. Modify parameters and analyze performance
3. Compare with other routing protocols (AODV, OLSR, DSR)
4. Create custom scenarios for your research
5. Publish your findings!

**For support:**
- NS2: https://www.isi.edu/nsnam/ns/
- NS3: https://www.nsnam.org/
- B.A.T.M.A.N.: https://www.open-mesh.org/

Happy simulating!

# NS2 B.A.T.M.A.N. Installation Instructions
# For Ubuntu 22.04 and NS-2.35

## PREREQUISITES
============================================================

1. Install required packages:
```bash
sudo apt-get update
sudo apt-get install build-essential autoconf automake libxmu-dev
sudo apt-get install gcc-9 g++-9
sudo apt-get install tcl8.6 tcl8.6-dev tk8.6 tk8.6-dev
sudo apt-get install libx11-dev libxext-dev
sudo apt-get install perl python3
```

2. Download NS-2.35:
```bash
cd ~
wget https://sourceforge.net/projects/nsnam/files/allinone/ns-allinone-2.35/ns-allinone-2.35.tar.gz
tar -xzf ns-allinone-2.35.tar.gz
cd ns-allinone-2.35
```

## INSTALLATION STEPS
============================================================

### Step 1: Patch NS2 for Ubuntu 22.04

Create a file `ns2-ubuntu22-fix.patch`:

```patch
--- ns-2.35/linkstate/ls.h.orig
+++ ns-2.35/linkstate/ls.h
@@ -137,7 +137,7 @@
        void eraseAll() { erase(baseMap::begin(), baseMap::end()); }
        iterator lookup(int nodeId) { return find(nodeId); }
 };
-typedef map<int, LsIdSeq> LsIdSeqMap;
+typedef std::map<int, LsIdSeq> LsIdSeqMap;
 
 /* NOTE: The message fields need to be in network byte order,
  *       the internal data structures should be in host-byte order.
```

Apply the patch:
```bash
cd ns-allinone-2.35
patch -p0 < ns2-ubuntu22-fix.patch
```

### Step 2: Build NS2

```bash
cd ns-allinone-2.35
export CC=gcc-9
export CXX=g++-9
export CPP=cpp-9
./install
```

### Step 3: Add B.A.T.M.A.N. protocol files

```bash
# Copy B.A.T.M.A.N. files to NS2 directory
cd ns-allinone-2.35/ns-2.35
mkdir batman
cp /path/to/batman_pkt.h batman/
cp /path/to/batman_rtable.h batman/
cp /path/to/batman_rtable.cc batman/
cp /path/to/batman.h batman/
cp /path/to/batman.cc batman/
```

### Step 4: Modify NS2 Makefile

Edit `ns-allinone-2.35/ns-2.35/Makefile.in`:

Add to OBJ_CC section (around line 230):
```makefile
batman/batman.o \
batman/batman_rtable.o \
```

### Step 5: Modify packet.h

Edit `ns-allinone-2.35/ns-2.35/common/packet.h`:

Add to enum packet_t (around line 80):
```c
PT_BATMAN,
```

Add to p_info::name_ array initialization (around line 240):
```c
name_[PT_BATMAN] = "BATMAN";
```

### Step 6: Modify ns-packet.tcl

Edit `ns-allinone-2.35/ns-2.35/tcl/lib/ns-packet.tcl`:

Add to the foreach loop (around line 100):
```tcl
BATMAN
```

### Step 7: Recompile NS2

```bash
cd ns-allinone-2.35/ns-2.35
make clean
make
```

### Step 8: Set environment variables

Add to ~/.bashrc:
```bash
export PATH=$PATH:~/ns-allinone-2.35/bin:~/ns-allinone-2.35/tcl8.5.10/unix:~/ns-allinone-2.35/tk8.5.10/unix
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/ns-allinone-2.35/otcl-1.14:~/ns-allinone-2.35/lib
export TCL_LIBRARY=~/ns-allinone-2.35/tcl8.5.10/library
```

Reload:
```bash
source ~/.bashrc
```

### Step 9: Test installation

```bash
cd batman
ns batman_example.tcl
```

## MAKEFILE MODIFICATIONS DETAIL
============================================================

Complete Makefile.in additions:

```makefile
# Add BATMAN source files
OBJ_CC = \
    ... existing files ... \
    batman/batman.o \
    batman/batman_rtable.o

# Add BATMAN to dependencies
batman/batman.o: batman/batman.cc batman/batman.h batman/batman_pkt.h batman/batman_rtable.h
batman/batman_rtable.o: batman/batman_rtable.cc batman/batman_rtable.h batman/batman_pkt.h
```

## TESTING
============================================================

1. Run the example simulation:
```bash
ns batman_example.tcl
```

2. View with NAM:
```bash
nam batman_nam.nam
```

3. Analyze trace file:
```bash
grep "^s" batman_trace.tr | wc -l  # Sent packets
grep "^r" batman_trace.tr | wc -l  # Received packets
grep "^D" batman_trace.tr | wc -l  # Dropped packets
```

## TROUBLESHOOTING
============================================================

1. If "command not found: ns":
   - Check PATH environment variable
   - Ensure NS2 compiled successfully

2. If "can't find package Tcl":
   - Check TCL_LIBRARY environment variable
   - Reinstall tcl8.6-dev

3. If compilation errors with gcc:
   - Use gcc-9 and g++-9
   - Check compiler version with: gcc --version

4. For "undefined reference to BATMAN":
   - Verify .o files are added to Makefile
   - Run make clean && make

## VERIFICATION
============================================================

To verify B.A.T.M.A.N. is installed:

```bash
cd ns-allinone-2.35/ns-2.35
./ns
```

In NS2 console:
```tcl
Agent/BATMAN set ttl_ 64
Agent/BATMAN set ogm_interval_ 1.0
puts "B.A.T.M.A.N. loaded successfully!"
```

You should see the success message.

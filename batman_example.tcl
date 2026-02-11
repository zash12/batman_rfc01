# batman_example.tcl
# Example simulation script for B.A.T.M.A.N. routing protocol
# Simulates a mobile ad-hoc network with 20 nodes

# ======================================================================
# Define options
# ======================================================================
set val(chan)           Channel/WirelessChannel    ;# channel type
set val(prop)           Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)          Phy/WirelessPhy            ;# network interface type
set val(mac)            Mac/802_11                 ;# MAC type
set val(ifq)            Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)             LL                         ;# link layer type
set val(ant)            Antenna/OmniAntenna        ;# antenna model
set val(ifqlen)         50                         ;# max packet in ifq
set val(nn)             20                         ;# number of mobilenodes
set val(rp)             BATMAN                     ;# routing protocol
set val(x)              1000                       ;# X dimension of topography
set val(y)              1000                       ;# Y dimension of topography
set val(simtime)        200                        ;# simulation time
set val(energymodel)    EnergyModel                ;# Energy Model
set val(initialenergy)  100                        ;# Initial energy in Joules

# ======================================================================
# Initialize Global Variables
# ======================================================================
set ns [new Simulator]

# Set up trace files
set tracefd  [open batman_trace.tr w]
set namtrace [open batman_nam.nam w]

$ns trace-all $tracefd
$ns namtrace-all-wireless $namtrace $val(x) $val(y)

# Set up topography object
set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)

# Create GOD (General Operations Director)
create-god $val(nn)

# ======================================================================
# Configure nodes
# ======================================================================
$ns node-config -adhocRouting $val(rp) \
                -llType $val(ll) \
                -macType $val(mac) \
                -ifqType $val(ifq) \
                -ifqLen $val(ifqlen) \
                -antType $val(ant) \
                -propType $val(prop) \
                -phyType $val(netif) \
                -channelType $val(chan) \
                -topoInstance $topo \
                -agentTrace ON \
                -routerTrace ON \
                -macTrace OFF \
                -movementTrace ON \
                -energyModel $val(energymodel) \
                -initialEnergy $val(initialenergy) \
                -rxPower 0.3 \
                -txPower 0.6 \
                -idlePower 0.1 \
                -sleepPower 0.05

# ======================================================================
# Create nodes
# ======================================================================
for {set i 0} {$i < $val(nn)} {incr i} {
    set node_($i) [$ns node]
    $node_($i) random-motion 0  ;# disable random motion
}

# ======================================================================
# Define node positions & movements
# ======================================================================
# Create a grid pattern initially
set grid_size [expr int(sqrt($val(nn)))]
set spacing [expr $val(x) / ($grid_size + 1)]

for {set i 0} {$i < $val(nn)} {incr i} {
    set row [expr $i / $grid_size]
    set col [expr $i % $grid_size]
    set x_pos [expr ($col + 1) * $spacing]
    set y_pos [expr ($row + 1) * $spacing]
    
    $node_($i) set X_ $x_pos
    $node_($i) set Y_ 0.0
    $node_($i) set Z_ 0.0
    
    # Move nodes to their positions
    $ns at 0.0 "$node_($i) setdest $x_pos $y_pos 0.0"
}

# Add some mobility after initial setup
for {set i 0} {$i < [expr $val(nn) / 2]} {incr i} {
    set dest_x [expr rand() * $val(x)]
    set dest_y [expr rand() * $val(y)]
    set speed [expr 5.0 + rand() * 10.0]  ;# 5-15 m/s
    set start_time [expr 20.0 + rand() * 50.0]
    
    $ns at $start_time "$node_($i) setdest $dest_x $dest_y $speed"
}

# ======================================================================
# Configure BATMAN specific parameters
# ======================================================================
for {set i 0} {$i < $val(nn)} {incr i} {
    set batman_agent [$node_($i) set ragent_]
    
    # Set TTL
    $batman_agent ttl 64
    
    # Configure node 0 as gateway
    if {$i == 0} {
        $batman_agent gateway 128 5000  ;# gateway class=128, port=5000
    }
}

# ======================================================================
# Setup traffic connections
# ======================================================================
# Create CBR traffic from multiple sources to destinations
proc create_cbr_traffic {src dst start_time} {
    global ns node_
    
    # Create UDP agent for sender
    set udp_($src:$dst) [new Agent/UDP]
    $ns attach-agent $node_($src) $udp_($src:$dst)
    
    # Create CBR application
    set cbr_($src:$dst) [new Application/Traffic/CBR]
    $cbr_($src:$dst) set packetSize_ 512
    $cbr_($src:$dst) set interval_ 0.5
    $cbr_($src:$dst) set random_ 1
    $cbr_($src:$dst) attach-agent $udp_($src:$dst)
    
    # Create receiver (Null agent)
    set null_($src:$dst) [new Agent/Null]
    $ns attach-agent $node_($dst) $null_($src:$dst)
    
    # Connect agents
    $ns connect $udp_($src:$dst) $null_($src:$dst)
    
    # Schedule traffic
    $ns at $start_time "$cbr_($src:$dst) start"
    $ns at [expr $val(simtime) - 10.0] "$cbr_($src:$dst) stop"
}

# Create multiple traffic flows
create_cbr_traffic 1 10 10.0
create_cbr_traffic 5 15 15.0
create_cbr_traffic 8 18 20.0
create_cbr_traffic 12 3 25.0
create_cbr_traffic 16 7 30.0

# ======================================================================
# Print routing table periodically
# ======================================================================
proc print_rtable {} {
    global ns node_ val
    
    set now [$ns now]
    puts "\n========== Time: $now =========="
    
    for {set i 0} {$i < $val(nn)} {incr i} {
        set batman_agent [$node_($i) set ragent_]
        puts "Node $i routing table:"
        $batman_agent print_rtable
    }
    
    $ns at [expr $now + 20.0] "print_rtable"
}

# Schedule first routing table print
$ns at 30.0 "print_rtable"

# ======================================================================
# Tell nodes when the simulation ends
# ======================================================================
for {set i 0} {$i < $val(nn)} {incr i} {
    $ns at $val(simtime) "$node_($i) reset"
}

# ======================================================================
# Finish procedure
# ======================================================================
proc finish {} {
    global ns tracefd namtrace
    $ns flush-trace
    close $tracefd
    close $namtrace
    
    puts "\nSimulation finished!"
    puts "Trace file: batman_trace.tr"
    puts "NAM file: batman_nam.nam"
    puts "\nRun 'nam batman_nam.nam' to visualize"
    
    exit 0
}

# ======================================================================
# Run simulation
# ======================================================================
puts "Starting B.A.T.M.A.N. simulation..."
puts "Nodes: $val(nn)"
puts "Simulation time: $val(simtime) seconds"
puts "Area: $val(x) x $val(y) meters"

$ns at $val(simtime) "finish"
$ns run

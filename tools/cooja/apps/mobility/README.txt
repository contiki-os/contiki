Mobility COOJA plugin

(Clarification: Fredrik Osterlind is the author of the mobility-plugin and I have
his permission to attach it and distribute it in this Contiki project.)
When I (Marcus Lunden) wanted to evaluate a communication primitive that I hypo-
thesized being particularly well suited for low latency neighbor discovery, I
needed a way to make the nodes in the simulation move. Unfortunately, there is no
built in function for this in the version of COOJA that can be obtained from the
web site (www.sics.se/contiki) or the Contiki CVS. However, Fredrik Osterlind, the
main author of COOJA had an experimental plugin called Mobility. As input, it takes
line separated position data in the following format:
nodeid time xpos ypos

Here is an example with two nodes:
0 0.00000000 1 0
1 0.00000000 2 0
0 0.25000000 1.5000000000 0
1 0.25000000 2.5000000000 0
0 0.50000000 2 0
1 0.50000000 3 0

The time is in seconds and the coordinates are in meters, with origin
in the top left corner. In the simulation, nodes are enumerated
beginning with 0 and increasing; this nodeid is not the Rime-address. In COOJA
they will correspond to nodes with Rime addresses 1.0 and 2.0 respectively.

This input format is compatible with the output from BonnMotion, the mobility
simulator from Bonn University. I had some trouble installing it and saw the
opportunity to a learning experience so I wrote my own mobility simulator in
Python. In contrast to BonnMotion, it is much simpler (only Random Waypoint
mobility model vs that+Manhattan grid+disaster area etc) but it works nice and
was fun to write. I will add it to this project as soon as I cleared up the code
a bit. I have included a file with movement data for 15 nodes and the following
parameters:
  # Number of nodes: 15
  # Time [s]: 600.0 seconds
  # Min speed [m/s]: 1.0
  # Max speed [m/s]: 4.0
  # Min pause time [s]: 2.0
  # Max pause time [s]: 10.0
  # Currently disregarding the first 10.0 seconds
  # Resolution [s]: 0.2
  # Maximum X-size [m]: 150.0
  # Maximum Y-size [m]: 150.0
However, a small default position.dat is included with position data for only
one node to simplify when installing. To use the large file, unzip it and
replace the small file.


Usage:
  Setting up the environment
    * put the mobility-folder in your contiki-folder/tools/cooja/apps/
    * put the movement data file (positions.dat) in that folder,
        contiki-folder/tools/cooja/apps/mobility/positions.dat
    * build the plugin
        cd contiki-folder/tools/cooja/apps/mobility
        ant jar
    * start cooja:
        cd contiki-folder/tools/cooja
        ant run
    * create a new simulation
        file/new simulation
    * setup the project directories
        settings/Manage project directories/Enter path manually:
          ../apps/mobility/
        Save as default
    * quit COOJA
        
  Changing the position.dat
  Should you want to change to another position data file, then do this:
    * Quit Cooja
    * Replace position.dat
    * Clean and rebuild the plugin
        cd contiki-folder/tools/cooja/apps/mobility
        ant clean
        ant jar
    * Start Cooja and run your simulation...

  Using mobility in simulations
    * Create a new simulation
    * Start mobility
        plugins/Mobility
        A textbox should appear with text like "Parsing positions.dat" etc
    * Add nodes and start the simulation!

  Notes on usage
    * Mobility can be started and stopped (by closing the text box) at any
      moment in the simulations.
    * If there are more nodes than position data, they will not move (but those
      that have will).
    * When simulation time overflows the position data time, it loops back again.
    * The plugin can be started at any moment, making that t = 0. This way, one
      can have the sim stand still for X seconds and then start the movements,
      without having to recompile or generate new movement data.


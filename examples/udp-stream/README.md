UDP stream
==========

This is an example of bursts support in CSMA/ContikiMAC, together with storage
of long packet queue in CFS. This is useful to support large fragmented UDP
datagrams or continuous data streaming. The current implementation is a
simplified version of the techniques presented in "Lossy Links, Low Power, High
Throughput", published in the proceeding of ACM SenSys 2011.

In this example, node with ID==5 sends bursts of UDP datagrams to node with
ID==1, the root of the RPL dodag.

Testing in cooja:
-----------------

    make TARGET=cooja udp-stream.csc

Testing on Tmote sky:
---------------------

1. Set node IDs to different motes so node 5 sends to node 1 (using
   examples/sky-shell)

2. Compile and program:

    make TARGET=sky udp-stream.upload

3. Monitor motes with:

    make login MOTE=xxx

The native border router connects a TTY with an ethernet TUN device
via the RPL protocol.  This works with the ../slip-radio example to permit
a Linux host to have a 802.15.4 radio connected via a serial interface.
What's on the SLIP interface is really not Serial Line IP, but SLIP framed
15.4 packets.

The border router supports a number of commands on it's stdin.
Each are prefixed by !:
     !G - global RPL repair root.
     !M - set MAC address (if coming from RADIO, i.e. SLIP link)
     !C - show channel (if coming from RADIO, i.e. SLIP link)
     !D - sensor data received
     !Q - exit

Queries are prefixed by ?:
     ?M - just a test
     ?C - writes stuff to SLIP port!
     ?S - shows stats for SLIP port.




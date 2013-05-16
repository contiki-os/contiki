A very simple sniffer for sensinode devices.
============================================

The cc2430 RF driver supports outputting all captured packets in hexdump
format. We turn this on, and turn everything else off. We use a stub RDC driver
to make sure no incoming packet ever goes up the stack and no packet is ever
sent out.

We only initialise the radio driver instead of the entire stack by over-riding
the default netstack.c with the one in this directory.

You can then pipe the sniffer's output to the n601-cap util, which will convert
the hexdumps to pcap format, which can in turn be piped to wireshark. This is
handy if we want live capture of the lowpan traffic from wireshark. See the
README in n601-cap for more details

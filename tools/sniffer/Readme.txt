
This directory holds the code of a generic sniffer.
It was implemented by Etienne Duble (CNRS / LIG).
A part of this work was done for the ANR ARESA2 project.

Quick start 
-----------
$ make TARGET=<target>                       # compilation
$ make TARGET=<target> sniffer.upload        # flashing
$ make TARGET=<target> sniff                 # on-line display (wireshark)

Troubleshooting
---------------
- No packets: check that the sniffer and the sensors use the same channel and pan ID.
- Malformed intermediary packets: most likely the sniffer was compiled with debug 
traces enabled, which are interfering with packets also going through the serial
interface. You should disable any debug traces and recompile / reflash the sniffer.

Only the first packet displayed in wireshark should be bad (it is caused by the startup 
traces of contiki).

Implementation details
----------------------
A fake RDC layer catches packets and sends them on the serial link 
using the SLIP protocol. On the development machine, the slip2pcap
binary handles the conversion SLIP -> PCAP, and the PCAP-encoded stream
is forwarded to wireshark for online display.

Supporting a new platform
-------------------------
In contiki, the radio_driver is specific to a given platform, 
whereas the rdc_driver is not. Therefore, implementing the sniffing feature
as a (fake) RDC layer allowed it to be plugged on any platform-specific 
radio_driver, in order to support any platform.

As a result, very little adaptation should be needed to make the
sniffer work on a given platform:
- set any platform-specific parameter to allow sniffing-mode (1)
- check if the frames detected include a FCS (frame checksum) or not (2)

Note: the serial connection to the sniffer board is done by calling "make login"
(see the Makefile). If there is no "login" Makefile target defined for your platform,
you will also have to implement this (i.e. call the appropriate tool to connect
to the board, see platform/sky/Makefile.common for example).

(1) radio chips usually have features implemented in hardware, like auto-ack 
or address filtering (discard packets whose destination address does not match
the address of the board). These features should be disabled in order to make
the board act as a sniffer, e.g. pass all packets to the RDC layer. If the
radio_driver also implements this kind of features (in software), they should
also be disabled. 
Disabling these features may involve updating macro definitions in project-conf.h 
(see CC2420_CONF_AUTOACK for example) or adding platform-specific calls (see 
function calls ST_<something> in sniffer.c for example).

(2) depending on the radio chip and the radio_driver code, the frame checksum
may or may not be passed to the RDC layer. If this setting is wrong, packets 
will appear malformed in wireshark. In this case, you should update the Makefile
and set -DSNIFFER_FRAMES_HAVE_FCS appropriately for your platform. 


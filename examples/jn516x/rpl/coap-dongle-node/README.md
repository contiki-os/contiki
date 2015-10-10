dongle-node is an example of a RICH node containing a JN516x microcontroller on a USB dongle. 
The dongle is part of the NXP JN516x Evaluation Kit (see http://www.nxp.com/documents/leaflet/75017368.pdf) 
The dongle-node connects to the network in the same way as described in `examples\jn5168/rpl/README.md`

The dongle contains 2 LEDs that are available as a CoAP resource. They can be accessed via a CoAP client at the IPv6 interface 
of the border router (e.g. via Copper plug-in on Firefox).
The following list gives an overview of the resources:

URI                        Description
---                        -----------
Dongle\LED-toggle          When doing a PUT/POST method on this resource (no payload needed), the LEDs will run through
                           the following states with wrap-around:
                           - GREEN LED on
                           - RED LED on
                           - All LEDs off
                           - RED and GREEN LED alternatively on with 1 sec period time

# Placeholder

This code connects a 802.15.4 radio over TTY with the full uIPv6 stack of Contiki including 6LoWPAN and 802.15.4 framing / parsing. The native border router also acts as a RPL Root and handles the routing and maintains the RPL network. Finally the native border router connects the full 6LoWPAN/RPL network to the host (linux/os-x) network stack making it possible for applications on the host to transparently reach all the nodes in the 6LoWPAN/RPL network.

This is designed to interact with the `slip-radio` (in the node folder) example running on a mote that is either directly USB/TTY connected, or is remote via a TCP connect.  What's on the SLIP interface is really not Serial Line IP, but SLIP framed 15.4 packets.

## Native border router

The border router supports a number of commands on it's stdin.
Each are prefixed by !:
* !G - global RPL repair root.
* !M - set MAC address (if coming from RADIO, i.e. SLIP link)
* !C - show channel (if coming from RADIO, i.e. SLIP link)
* !D - sensor data received
* !Q - exit

Queries are prefixed by ?:
* ?M is used for requesting the MAC address from the radio in order to use it for uIP6 and its stateless address auto configuration of its IPv6 address. This will make the native border router have the address that correspond to the MAC address of the slip-radio. (response is !M from the slip-radio)

* ?C is used for requesting the currently used channel for the slip-radio. The response is !C with a channel number (from the slip-radio).

* !C is used for setting the channel of the slip-radio (useful if the motes are using another channel than the one used in the slip-radio).

## To compile and run:

````
make border-router
sudo ./border-router.native -L -v6 -s /dev/ttyUSB0 -B 115200 aaaa::1/64
````

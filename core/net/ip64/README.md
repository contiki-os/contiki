The `ip64` module lets an IPv6 Contiki network be connected to an IPv4
network without any additional configuration or outside software. The
`ip64` module runs on the RPL root node and translates outgoing IPv6
packets into IPv4 packets nd incoming IPv4 packets to IPv6 packets.

The `ip64` module uses stateful NAT64 (RFC6164) to do the packet
translation and DNS64 (RFC6147) to catch DNS requests for IPv6
addresses, turn them into requests for IPv4 addresses, and turn the
replies into responses for IPv6 addresses. This allows devices on the
inside IPv6 network to connect to named servers on the outside IPv4
network.

The `ip64` module hooks into the IPv6 stack via a fallback
interface. Any packet that can not be routed into the local RPL mesh
will be sent over the fallback interface, where `ip64` picks it up,
translates it into an IPv4 packet, and sends it over its outgoing
interface.

In addition to providing NAT64 and DNS64 services, the `ip64` module
also performs DHCPv4 to request IPv4 address for devices connected to
a medium such as Ethernet. The `ip64` module also performs ARP
processing to communicate over the Ethernet.

The `ip64` module uses a configuration file called `ip64-conf.h` that
specifies what device to use for the IPv4 network. This file is
intended to be placed in the platform directory. An example
configuration file called `ip64-conf-example.h` is provided in this
directory.


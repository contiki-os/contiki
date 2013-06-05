See siscslow_ethernet.c for information about translation between 802.15.4 and
802.3 addresses.  Devices must have a proper EUI-64 address for this bridge to
work.  If the EUI-64 address of the devices cannot be changed and they are
incompatible with the translation mechanism, you have to change the translation
rules (they are defined in mac_createSicslowpanLongAddr() and
mac_createEthernetAddr()).

Usage example:

    run tapslip6 (source file are located in tools folder).

    ./tapslip6 -p 2001:db8:bbbb:abce::/64

where 2001:db8:bbbb:abce:: is the network address and 64 is the the prefix
length. The remaining 64 bits will be derived from the EUI-64 (two middle bytes
will be removed).

You can also simply create an edge router in one of the network nodes: you have
to enable routing and set the appropriate forwarding rules, as shown in
contiki-init-net.c. Then you have to add the corresponding rule on your
machine, for example:

    route -A inet6 add 2001:db8:bbbb:abcd::/64 gw 2001:db8:bbbb:abce:280:e102::8a.

where 2001:db8:bbbb:abce:280:e102::6c is the address of the edge node and
2001:db8:bbbb:abcd::/64 is the lowpan address.

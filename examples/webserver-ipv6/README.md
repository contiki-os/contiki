This example features a simple webserver running on top of the IPv6 contiki
stack.

For this example to run properly the UIP_CONF_TCP compilation flag must be set
to 1 in the contiki-conf.h file of the platform.

By default contiki ipv6 nodes are configured with the low-power RPL protocol
and direct outgoing packets through the RPL parent to a mesh border router.
Access to the webserver thus requires at least one other node to as the RPL
root. The Makefile changes the default to no RPL for the minimal-net target.
Override the RPL choice with

        make TARGET=minimal-net UIP_CONF_RPL=1
        make TARGET=any-other UIP_CONF_RPL=0

The RPL mesh border router can be configured with this webserver using the
/examples/ipv6/rpl-border-router/ example.

If you are using the minimal-net platform without RPL you can access the
webserver through the link local address by appending the interface descriptor
shown at launch, e.g.

        wget http://[fe80::296:98ff:fe00:0232%tap0]  (linux)
        ping http://[fe80::206:98ff:fe00:0202%nnnn]  (Windows)

The lower 64 bits are derived from the ethernet EUI-48 "mac address" in uip6.c:

        uip_lladdr_t uip_lladdr = {{0x00,0x06,0x98,0x00,0x02,0x32}};

The ipv6 prefix can be hard-coded in the build or assigned through a
host router advertisement. If hard-coded just assign the prefix to the
interface:

        sudo ip -6 address add fdfd::1/64 dev tap0 (linux)
        netsh commands or the interface GUI        (Windows)

On linux you can set up router advertisements as follows:

        ifconfig tap0 inet6 3ffe:0501:ffff:0100:0206:98ff:fe00:0231
        ip -6 address add 3ffe:0501:ffff:0100:0206:98ff:fe00:0231 dev tap0

- You might need to add a route:

        ip -6 route add fd00:0000:0000:0000:0206:98ff:fe00:0232/64 dev tap0

- Then configure a global address by sending a router advertisement (RA) with a
  prefix option. You can use radvd for example to generate such a packet.
  Note: You should set the preferred and valid lifetime to reasonable value to
  avoid clock wrap-around. E.g.:

        AdvPreferredLifetime 400;
        AdvValidLifetime 600;

  Assume the prefix in the RA is: 3ffe:0501:ffff:0100:0000:0000:0000:0000/64.
  and that the resulting address created by the contiki stack is:
  3ffe:0501:ffff:0100:0206:98ff:fe00:0232
- Finally you can use:

        wget http://[3ffe:0501:ffff:0100:0206:98ff:fe00:0232]

The default webserver and content is in /apps/webserver/...
Change that using e.g.

    make clean
    make WITH_WEBSERVER=webserver-nano
    make TARGET=redbee-econotag WITH_WEBSERVER=webserver-nano
    make TARGET=avr-raven WITH_WEBSERVER=raven-webserver

*Beware: Make clean before switching make options!*

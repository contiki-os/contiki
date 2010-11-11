Contiki RPL Quickstart using Econotags
======================================

Contiki RPL implements the IETF ROLL working group's RPL: IPv6 Routing
Protocol for Low power and Lossy Networks. This routing standard
enables you to set up a route-over 6LowPAN network. 

This tutorial will show you how to run Contiki's RPL demos on Redwire
Econotags. 

1) Obtain Redwire Econotags, or other mc1322x hardware.
---------------------------------------------------------------

See [the mc1322x hardware](http://mc1322x.devl.org/hardware.html) for
an incomplete, but growing list of mc1322x hardware. 

We recommend a few [Econotags](http://www.redwirellc.com/store/node/1)
to get started. In addition to the mc13224v, the Econotag also has the
FT2232H USB dual port controller. One port is connected to UART1 and
all the necessary pins to load, erase, and reset the mc13224v. The
other port is connected to the JTAG pins for run-time debugging. **No
other programming hardware is necessary. Just your computer!**

2) Get a toolchain, and Contiki source.
----------------------------------------

Everything you need is in [Instant Contiki
2.5](http://www.sics.se/contiki/news/contiki-2.5-release-candidate-1-avaliable.html). If
you are going to be working primarily with Contiki then this is the
way to go.

Instant Contiki contains the
[mc1322x-oe](http://git.devl.org/?p=malvira/mc1322x-oe.git;a=summary)
[[ubuntu]](http://mc1322x.devl.org/ubuntu.html)
[[mac]](http://mc1322x.devl.org/mac.html) toolchain.

Development of the mc1322x port occurs in a different branch from the
main Contiki CVS repository. The most current mc1322x contiki port can be found here:
[contik-mc1322x.git](http://git.devl.org/?p=malvira/contiki-mc1322x.git)

3) Build the border-router and rpl-udp
---------------------------------------

The RPL demo consists of a "Border router" and at least one other RPL
enabled node. For this tutorial, the border router creates a RPL DAG
and is the ROOT. The other node will be a RPL udp-client. It joins the
RPL network, acts as an RPL router, and sends a UDP datagram to a
hardcoded IPV6 address. This tutorial only handles a two node network,
but can be extended for any number of nodes.

The general command to make a Contiki binary for the Econotag is:

    make TARGET=redbee-econotag

To build the border-router:

    cd contiki-2.x/examples/ipv6/rpl-border-router
    make TARGET=redbee-econotag

This will produce the binary image
'border-router_redbee-econotag.bin', which you can load directly on to
an mc1322x and execute.

To build the udp-client, do:

    cd contiki-2.x/examples/ipv6/udp-client
    make TARGET=redbee-econotag

which will build 'udp-client_redbee-econotag.bin'.

4) Load the binaries on the Econotags:
-------------------------------------------

### a) Run the econotags with randomized MAC addresses (random, but persistent)

After building border-router_redbee-econotag.bin, you load the code
with:

    mc1322x-load.pl -f border-router_redbee-econotag.bin -t /dev/ttyUSB1 -c 'bbmc -l redbee-econotag reset'

The econotag will flash itself with a random Redwire MAC address if
you haven't programmed one in:

    address in flash blank, setting to defined IAB with a random extention.
    flashing blank address
    Rime started with address 00:50:C2:FF:FE:A8:C2:66
    CSMA sicslowmac, channel check rate 100 Hz, radio channel 26
    Tentative link-local IPv6 address
    fe80:0000:0000:0000:0250:c2ff:fea8:c266
    Tentative global IPv6 address aaaa:0000:0000:0000:0250:c2ff:fea8:c266

This mac address is used to derive its IPv6 address. In this example,
it's aaaa::0250:c2ff:fea8:c266.

The border router connects to your computer over a SLIP tunnel formed
by `tunslip6` --- a tool included in Contiki.

To make tunslip6:

    cd contiki-2.x/tools
    make tunslip6

You need to run tunslip6 to create this tunnel and SLIP connection:

    tunslip6 -s /dev/ttyUSB1 aaaa::1/64

### b) Program the MAC addresses on the Econotag

To erase the econotag (and the MAC address), first make sure you've
built BBMC:

    cd contiki-2.x/cpu/mc1322x/tool/ftditools
    make

If you have trouble, see the [BBMC
page](http://mc1322x.devl.org/bbmc.html) for details. BBMC controls
the FT2232H over USB through libftdi. It can reset or erase an
mc13224v. To erase:

    bbmc -l redbee-econotag erase

Then to  flash just the MAC address you need to build flasher from
libmc1322x. See:

    http://mc1322x.devl.org/libmc1322x.html

Then to flash the MAC address you can do:

    mc1322x-load.pl -e -f flasher_redbee-econotag.bin -z -t /dev/ttyUSB1 -c 'bbmc -l redbee-econotag reset' 0x1e000,0xABC25000,0x010000C0

+ -z to indicate you are sending a zero length binary to flasher. 
+ -e causes mc1322x-load.pl to exit when it's done, instead of 
  dropping to a terminal

The next time you run Contiki you'll get:

    loading rime address from flash.
    Rime started with address 00:50:C2:AB:C0:00:00:01
    CSMA sicslowmac, channel check rate 100 Hz, radio channel 26
    Tentative link-local IPv6 address
    fe80:0000:0000:0000:0250:c2ab:c000:0001
    Tentative global IPv6 address aaaa:0000:0000:0000:0250:c2ab:c000:0001

N.B. for all of the bbmc commands, you'll need to use -i if you have
more than one econotag plugged in. On most systems the indexes are
enumerated backwards from the terminals. E.g. if you have 23 nodes
connected, then /dev/ttyUSB1 will be on ftdi index 22 and the load
command would be:

    mc1322x-load.pl -f foo.bin -t /dev/ttyUSB1 -c 'bbmc -l redbee-econotag -i 22 reset'

5) Start the udp-client
-----------------------

Use mc1322x-load.pl to run `udp-client_redbee-econotag.bin` on the
second econotag:

    mc1322x-load.pl -f udp-client_redbee-econotag.bin -t /dev/ttyUSB3
    'bbmc -l redbee-econotag -i 0'

6) Check the border router's webpage for status
------------------------------------------------

The RPL border-router serves a webpage that displays the known
neighbors and the known routes to all the node on the DAG. In firefox,
you can type in the following to go to a IPv6 address:

    http://[aaaa:0000:0000:0000:0250:c2ff:fea8:c266]

#### Neighbors
fe80::250:c2ff:fea8:cbf4
#### Routes
aaaa::250:c2ff:fea8:cbf4/128 (via fe80::250:c2ff:fea8:cbf4)

7) Ping6 all the nodes
----------------------

After a short time, once the DAG forms to a node, you should see a
route appear on the border-router's status page. You can ping this
node from the computer running 'tunslip6'

    ping6 aaaa::250:c2ff:fea8:cbf4

A successful ping shows that both a downward and upward route has
formed.

8) That's it!
-------------

Now go mesh and stuff.


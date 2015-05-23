A Quick Introduction to the Erbium (Er) REST Engine
===================================================

EXAMPLE FILES
-------------

- er-example-server.c: A RESTful server example showing how to use the REST
  layer to develop server-side applications (at the moment only CoAP is
  implemented for the REST Engine).
- er-example-client.c: A CoAP client that polls the /actuators/toggle resource
  every 10 seconds and cycles through 4 resources on button press (target
  address is hard-coded).
- er-plugtest-server.c: The server used for draft compliance testing at ETSI
  IoT CoAP Plugtests. Erbium (Er) participated in Paris, France, March 2012 and
  Sophia-Antipolis, France, November 2012 (configured for minimal-net).

PRELIMINARIES
-------------

- Make sure rpl-border-router has the same stack and fits into mote memory:
  You can disable RDC in border-router project-conf.h (not really required as BR keeps radio turned on).
    #undef NETSTACK_CONF_RDC
    #define NETSTACK_CONF_RDC     nullrdc_driver
- Alternatively, you can use the native-border-router together with the slip-radio.
- For convenience, define the Cooja addresses in /etc/hosts
      aaaa::0212:7401:0001:0101 cooja1
      aaaa::0212:7402:0002:0202 cooja2
      ...
- Get the Copper (Cu) CoAP user-agent from
  [https://addons.mozilla.org/en-US/firefox/addon/copper-270430](https://addons.mozilla.org/en-US/firefox/addon/copper-270430)
- Optional: Save your target as default target
      make TARGET=sky savetarget

COOJA HOWTO
-----------

###Server only:

    make TARGET=cooja server-only.csc

Open new terminal

    make connect-router-cooja

- Start Copper and discover resources at coap://cooja2:5683/
- Choose "Click button on Sky 2" from the context menu of mote 2 (server) after
  requesting /test/separate
- Do the same when observing /test/event

###With client:

    make TARGET=cooja server-client.csc

Open new terminal

    make connect-router-cooja

- Wait until red LED toggles on mote 2 (server)
- Choose "Click button on Sky 3" from the context menu of mote 3 (client) and
  watch serial output

TMOTES HOWTO
------------

###Server:

1. Connect two Tmote Skys (check with $ make TARGET=sky sky-motelist)

        make TARGET=sky er-example-server.upload MOTE=2
        make TARGET=sky login MOTE=2

2. Press reset button, get address, abort with Ctrl+C:
   Line: "Tentative link-local IPv6 address fe80:0000:0000:0000:____:____:____:____"

        cd ../ipv6/rpl-border-router/
        make TARGET=sky border-router.upload MOTE=1
        make connect-router

    For a BR tty other than USB0:

        make connect-router-port PORT=X

3. Start Copper and discover resources at:

        coap://[aaaa::____:____:____:____]:5683/

### Add a client:

1. Change the hard-coded server address in er-example-client.c to aaaa::____:____:____:____
2. Connect a third Tmote Sky

        make TARGET=sky er-example-client.upload MOTE=3

MINIMAL-NET HOWTO
-----------------

With the target minimal-net you can test your CoAP applications without
constraints, i.e., with large buffers, debug output, memory protection, etc.
The er-plugtest-server is thought for the minimal-net platform, as it requires
an 1280-byte IP buffer and 1024-byte blocks.

        make TARGET=minimal-net er-plugtest-server
        sudo ./er-plugtest-server.minimal-net

Open new terminal

        make connect-minimal

- Start Copper and discover resources at coap://[fdfd::ff:fe00:10]:5683/
- You can enable the ETSI Plugtest menu in Copper's preferences

Under Windows/Cygwin, WPCAP might need a patch in
<cygwin>\usr\include\w32api\in6addr.h:

    21,23c21
    < #ifdef __INSIDE_CYGWIN__
    <     uint32_t __s6_addr32[4];
    < #endif
    ---
    >     u_int __s6_addr32[4];
    36d33
    < #ifdef __INSIDE_CYGWIN__
    39d35
    < #endif

DETAILS
-------

Erbium implements the Proposed Standard of CoAP. Central features are commented
in er-example-server.c.  In general, apps/er-coap supports:

- All draft-18 header options
- CON Retransmissions (note COAP_MAX_OPEN_TRANSACTIONS)
- Blockwise Transfers (note REST_MAX_CHUNK_SIZE, see er-plugtest-server.c for
  Block1 uploads)
- Separate Responses (no rest_set_pre_handler() required anymore, note
  coap_separate_accept(), _reject(), and _resume())
- Resource Discovery
- Observing Resources (see EVENT_ and PRERIODIC_RESOURCE, note
  COAP_MAX_OBSERVERS)

TODOs
-----

- Dedicated Observe buffers
- Optimize message struct variable access (directly access struct without copying)
- Observe client
- Multiple If-Match ETags
- (Message deduplication)

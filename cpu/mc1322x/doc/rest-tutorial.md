Contiki REST/CoAP Quickstart using Econotags
======================================

Contiki has an implementation of the IETF CORE (Constrained RESTful
Environments) working group's CoAP layer (Constrained Application
Protocol). CoAP is a RESTful application layer that uses HTTP-like
methods (GET, POST, PUT, DELETE) to interact with constrained
networks. CoAP operates over UDP and supports reliable transmission.

This tutorial will show you how to run Contiki's CoAP demo on Redwire
Econotags using an RPL border-router.

1) Run a RPL border-router and tunnel
---------------------------------------------------------------

See [the RPL
HOWTO](http://mc1322x.devl.org/repos/contiki-mc1322x/cpu/mc1322x/doc/rpl-tutorial.md)
for details about running a RPL border-router.

2) Build and run the `rest-server-example` on a second Econotag
----------------------------------------

__The following must be done on the contiki-mc1322x.git tree__

    git clone git://git.devl.org/git/malvira/contiki-mc1322x.git

__Contiki CVS is currently down and the new SCM system hasn't been set
up yet. The necessary changes will be pushed as soon as the new SCM is
available. - 6 Februrary 2011__

To build the rest-server-example:

    cd contiki-mc1322x/examples/rest-example
    make TARGET=redbee-econotag

This will produce the binary image
`rest-server-example_redbee-econotag.bin`, which you can load directly
on to an mc1322x and execute.

    mc1322x-load.pl -f rest-server-example_redbee-econotag.bin -t /dev/ttyUSB3

Then press the reset button to connect to the bootloader.

In this example, we are loading the CoAP server on to the econotag on `/dev/ttyUSB3`

You should see boot up messages similar to this:

    CONNECT
    Size: 62096 bytes
    Sending rest-server-example_redbee-econotag.bin
    done sending files.
    performing ring osc cal
    crm_status: 0xc0000
    sys_cntl: 0x18
    ring osc cal complete
    cal_count: 0x17e17e0
    cal factor: 100
    hib_wake_secs: 2000
    loading rime address from flash.
    Rime started with address 00:50:C2:AB:C0:00:00:23
    nullmac nullrdc, channel check rate 100 Hz, radio channel 26
    Tentative link-local IPv6 address
    fe80:0000:0000:0000:0250:c2ab:c000:0023
    Tentative global IPv6 address aaaa:0000:0000:0000:0250:c2ab:c000:0023
    Starting 'Rest Server Example'
    COAP Server

The last line indicates that the server will be using
COAP. As an alternative, you can build the server to use HTTP instead with:

    make TARGET=redbee-econotag WITH_COAP=0 

3) Download and install the `Copper` Firefox Plugin
-----------------------------------------------------

The `Copper` Plugin for Firefox provides the `coap:` URL access method
as well as an interface to easily send `coap` requests.

Download and install the plugin from here:

+ [Copper plugin homepage](http://people.inf.ethz.ch/~mkovatsc/)
+ Install link:[copper-0.3.0pre2.xpi](http://people.inf.ethz.ch/~mkovatsc/resources/copper/copper-0.3.0pre2.xpi)

4) Open Copper
--------------

Open a new Firefox tab and click on the orange CU button in the lower
right.

[![Open CU](http://mc1322x.devl.org/files/coap-blanktab-t.png)](http://mc1322x.devl.org/files/coap-blanktab.png)

The initial CU screen will look like this:

[![CU startup](http://mc1322x.devl.org/files/coap-opencu-t.png)](http://mc1322x.devl.org/files/coap-opencu.png)

Type in the the URL of the coap node with the default port number of
"61616":

[![CU startup](http://mc1322x.devl.org/files/coap-url-t.png)](http://mc1322x.devl.org/files/coap-url.png)

+ Don't forget the brackets ( [ ] ) in the URL 

+ Make sure to use the IPv6 address of your coap server. You can get
  this from the boot up messages or from the webpage served by your
  border-router.

+ You must always press Enter after changing the URL.

5) GET `.well-known/core` resources
------------------------------------

Now click on the red "./well-known/core" button: this changes the URL
to the `.well-known/core` resource.

Then click GET to perform a get. You should see an acknowledgement
that the GET was successful (returns 200 OK). The payload should
return:

    </helloworld>;n="HelloWorld",</led>;n="LedControl",</light>;n="Light

Which is are the well-known resources that this node advertises; see
the [COAP
specification](https://datatracker.ietf.org/doc/draft-ietf-core-coap/)
for details.

[![Open CU](http://mc1322x.devl.org/files/coap-wellknown-t.png)](http://mc1322x.devl.org/files/coap-wellknown.png)

6) PUT,POST the `led` resource state
------------------------------------

You can PUT or POST to change the state of the LED. 

Type in the following URL and press enter:

    coap://[aaaa::250:c2ff:fea8:c48e]:61616/led?color=green

Be sure to use the proper IP address. For this URL: we will perform
actions on the `led` resource with a query string of `color=green`.

In the payload, type:

    mode=on

That is the payload that will be PUT or POSTed. The COAP server
detects the mode string and activates the LED accordingly (with the
color chosen by the query string).

Then click PUT or POST to perform the request.

[![CU startup](http://mc1322x.devl.org/files/coap-led-t.png)](http://mc1322x.devl.org/files/coap-led.png)

You should get a successful return code (200 OK) and the green LED
should turn on. If you PUT/POST `mode=off` the led will turn off.

The econotag only has two LEDs: a green and a red. The red LED is used
to indicate radio transmission by default and so cannot be used in
this demo. The Coniki blue LED is connected to GPIO 43. You can toggle
it, but you won't see anything unless you hook something up to this
pin.

7) Other resources
------------------

The `rest-server-example` also provides `helloworld` and `light` as
GETtable resources.

    coap://[aaaa::250:c2ff:fea8:c48e]:61616/helloworld
    coap://[aaaa::250:c2ff:fea8:c48e]:61616/light

The econotag does not have a light sensor. The light resource will
always return 0. At a latter date, this sensor will be connected to
one of the ADC pins.

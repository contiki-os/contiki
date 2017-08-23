sensniff Contiki Project
========================
This example can be used to create an IEEE 802.15.4 wireless sniffer firmware,
meant to be used in parallel with
[sensniff](https://github.com/g-oikonomou/sensniff).

Running
=======
* Build this example and program your device
* Connect your device to a host
* Run sensniff on your host
* Fire up wireshark and enjoy.

Make sure your device's UART baud rate matches the `-b` argument passed to
sensniff. I strongly recommend using at least 460800. This comment does not
apply if your device is using native USB.

Subsequently, make absolutely certain that your device is tuned to the same RF
channel as the network you are trying to sniff. You can change sniffing channel
through sensniff's text interface.

More details in sensniff's README.

Adding support for more platforms
=================================
Firstly, this example will try to turn off frame filtering and automatic h/w
ACKs by calling `NETSTACK_RADIO.set_value(RADIO_PARAM_RX_MODE, 0)`. If your
radio does not support this, then implementing this is your first step towards
running this example on your board.

Secondly, in order to be able to switch channels and retrieve current/min/max
RF channel supported from sensniff's text interface, your device's radio driver
must also support:

    NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, ...)
    NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, ...)
    NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, ...)
    NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, ...)

The following radios have been tested:

* CC13xx/CC26xx in PROP and IEEE modes
* CC2538
* CC2530/CC2531
* CC1200

One you have the radio sorted out, you also need to configure character I/O.
The firmware captures wireless frames and streams them over a serial line to
the host where your device is connected. This can be achieved over UART or over
CDC-ACM. The example makes zero assumptions about your hardware's capability,
you have to configure thnigs explicitly. 

* Firstly, create a directory named the same as your platform. Crate a header
file therein called `target-conf.h`. This will get included automatically.
* Then look at the header files under `pool`, perhaps your device's CPU is
already supported. If that's the case, then within your `target-conf.h` you
simply need to add a line like this:

        #define SENSNIFF_IO_DRIVER_H "pool/cc2538-io.h"
    
choosing the header that corresponds to your device's CPU. Just look for any of
the platforms already supported to see how you can configure things in a more
fine-grained fashion (e.g. to select UART instance, switch between UART/USB
etc).

* If your CPU is not already supported, then you need to create an additional
header file. In that header file, you will need to define the following three:

        #define sensniff_io_byte_out()  <driver function that prints bytes>
        #define sensniff_io_flush()     <for buffered I/O. Can be empty>
        #define sensniff_io_set_input() <driver function that sets an input callback>

Those should map to functions implemented by your device's peripheral driver,
e.g. your UART driver. `_byte_out()` and `set_input()` are required, but
`_flush()` is optional and is only really helpful in case of drivers/hardware
that support buffered I/O (as is the case for some Contiki's USB drivers). Once
you have provided those defines, then simple go back to your `target-conf.h`
and:

        #define SENSNIFF_IO_DRIVER_H "header-with-my-own-defines.h"

That should be it!


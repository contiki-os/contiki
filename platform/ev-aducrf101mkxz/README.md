Building Contiki for the EV-ADuCRF101MKxZ Board
===============================================

On Debian/Ubuntu Linux:
-----------------------

For older versions of Ubuntu (prior to 14.04), add the external
package repository that provides recent versions of GCC for ARM:

    sudo add-apt-repository -y ppa:terry.guo/gcc-arm-embedded
    sudo apt-get update

For all systems, install the required development packages:

    sudo apt-get install git make gcc-arm-none-eabi python-serial

Obtain the Contiki source code:

	git clone https://github.com/contiki-os/contiki.git

Build Contiki's `example-abc`:

	make -C contiki/examples/rime \
        TARGET=ev-aducrf101mkxz \
        example-abc.ev-aducrf101mkxz.hex

The default radio frequency can optionally be specified on the
command-line as follows.  A clean rebuild may be needed when changing
it:

	make -C contiki/examples/rime \
        TARGET=ev-aducrf101mkxz \
        RF_CHANNEL=915000000 \
        clean \
        example-abc.ev-aducrf101mkxz.hex

The code can be flashed to the eval board and tested using
[adi-cm3sd](https://github.com/jimparis/adi-cm3sd.git).  Obtain
`adi-cm3sd`:

    git clone https://github.com/jimparis/adi-cm3sd.git

Connect the evaluation board using its J-Link board, or any other
serial adapter.  Flash `example-adc` and open a terminal by running:

    adi-cm3sd/cm3sd.py -a contiki/examples/rime/example-abc.ev-aducrf101mkxz.hex \
        /dev/serial/by-id/usb-SEGGER_J-Link_000541011111-if00

replacing `/dev/serial/by-id/usb-SEGGER_J-Link_000541011111-if00` with
the path to the correct serial device.  Flash the same code on a
second evaluation board to see them communicate.

### IPv6 Example ###

#### Border Router ####

First, build and run the IPv6 `border-router` example:

	make -C contiki/examples/ipv6/rpl-border-router \
        TARGET=ev-aducrf101mkxz \
        SERIAL_ID='"00001234"' \
        border-router.ev-aducrf101mkxz.hex

    adi-cm3sd/cm3sd.py -a contiki/examples/ipv6/rpl-border-router/border-router.ev-aducrf101mkxz.hex \
        /dev/serial/by-id/usb-SEGGER_J-Link_000541011111-if00

After flashing, close the terminal with `CTRL-C`, then build and run
the SLIP tunnel on the host machine:

	make -C contiki/tools tunslip6

    sudo contiki/tools/tunslip6 \
        -s /dev/serial/by-id/usb-SEGGER_J-Link_000541011111-if00 \
        -B 115200 -v3 aaaa::1/64

Press the RESET button on the eval board, then open the border router
home page at: http://[aaaa::3230:3030:3132:3334]/

#### Web Server ####

Then, build and flash the IPv6 `webserver6` example on another eval
board.  The different `SERIAL_ID` ensures that the webserver uses a
link-local IP address that is different from that of the border
router:

	make -C contiki/examples/webserver-ipv6 \
        TARGET=ev-aducrf101mkxz \
        SERIAL_ID='"00005678"' \
        webserver6.ev-aducrf101mkxz.hex

    adi-cm3sd/cm3sd.py -a contiki/examples/webserver-ipv6/webserver6.ev-aducrf101mkxz.hex \
        /dev/serial/by-id/usb-SEGGER_J-Link_000541022222-if00

Open the web server's home page at: http://[aaaa::3230:3030:3536:3738]/

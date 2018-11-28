Getting Started with Contiki on the Weptech Saker
=================================================

This guide aims to get you started with Contiki and the Weptech Saker.

Saker features
================

The Saker features the following components: 
 * A Texas Instruments CC2538 Cortex®- M3 Microcontroller with 512KB Flash +
   32KB RAM. The integrated 2.4GHz low power radio is connected to an Inverted
   F-Antenna (optional U.FL connector (assembly variant)).
 * A Texas Instruments Sub-Ghz Transceiver CC1200 connected to a Monopole PCB
   Antenna (optional U.FL connector (assembly variant)). The CC1200 is
   controlled by the CC2538 via SPI.
 * A Microchip ENC28J60 10BASE-T Ethernet Controller connected to a RJ45
   modular jack. The ENC28J60 is controlled by the CC2538 via SPI (not shared
   with CC1200).
 * A Microchip 24AA02E48 EEPROM providing a (read-only) EUI-48 (“MAC address”)
   for the Ethernet Interface. In addition, this EEPROM provides 128Byte of
   writeable memory which can be used e.g. for configuration purposes. The
   EEPROM is controlled by the CC2538 via I2C.
 * A FTDI FT234XD USB-to-UART converter providing a virtual COM port to the host
   system.
 * Two buttons, one of which is freely available for programming.
 * Three LEDs.

The CC2538 SoC at the core of the Saker offers the following key
features:

 * Deep Sleep support with RAM retention for ultra-low energy consumption.
 * Native USB support (CDC-ACM). SLIP over UART for border routers is no longer
   a bottleneck.
 * DMA transfers for increased performance (RAM to/from RF, RAM to/from USB).
 * Standard Cortex M3 peripherals (NVIC, SCB, SysTick)
 * Sleep Timer (underpins rtimers)
 * SysTick (underpins the platform clock and Contiki's timers infrastructure)
 * UART
 * Watchdog (in watchdog mode)
 * USB (in CDC-ACM)
 * uDMA Controller (RAM to/from USB and RAM to/from RF)
 * Random number generator
 * Low Power Modes
 * General-Purpose Timers. NB: GPT0 is in use by the platform code, the
   remaining GPTs are available for application development.


Requirements
============
To get started with the Saker, you will need the following:

 * A toolchain to compile Contiki for the CC2538.
 * Maybe additional drivers so that your OS can communicate with your hardware.
 * Software to upload images to the CC2538.
 * Ideally some more 6LoWPAN hardware to have a mesh for which the Saker can
   serve as a border router. TI's SmartRF06 board with Evaluation Modules for
   the CC1310 or CC1350 SoCs, as well as TI Sensortags based on the CC2650 or
   CC1350 have been tested. Additionally, the SmartRF06 features a JTAG
   interface which you can use to program your Saker.

This guide was tested on Linux, but as the Saker is based on the CC2538, most
of the instructions in the README.md for the CC2538DK *should* apply. In that
manual, they also describe how to get that board running under Windows and Mac
OS X.

Install a Toolchain
-------------------
The toolchain used to build contiki is arm-gcc, also used by other arm-based 
Contiki ports. If you are using Instant Contiki, you will have a version 
pre-installed in your system. To find out if this is the case, try this:

    $ arm-none-eabi-gcc -v
    Using built-in specs.
    COLLECT_GCC=arm-none-eabi-gcc
    COLLECT_LTO_WRAPPER=/usr/bin/../lib/gcc/arm-none-eabi/4.9.3/lto-wrapper
    Target: arm-none-eabi
    Configured with: /build/gcc-arm-none-eabi-Yjx9PL/gcc-arm-none-eabi-4.9.3.2015q2/src/gcc/configure
    ...
    (skip)
    ...
    Thread model: single
    gcc version 4.9.3 20150529 (release) [ARM/embedded-4_9-branch revision 224288] (GNU Tools for ARM Embedded Processors)

The platform is currently being used/tested with the following toolchains:

* GNU Tools for ARM Embedded Processors. This is the recommended version.
  <https://launchpad.net/gcc-arm-embedded>
* Alternatively, you can use this older version for Linux. At the time of 
  writing, this is the version used by Contiki's regression tests. 
  <https://sourcery.mentor.com/public/gnu_toolchain/arm-none-eabi/arm-2008q3-66-arm-none-eabi-i686-pc-linux-gnu.tar.bz2>

Drivers
-------
On Windows, the drivers for the FTDI234 USB chip on the Saker will be
installed along with [SmartRF][smart-rf-flashprog]. You can also
install these drivers directly from [FTDI][ftdidrivers].

On Linux, the ftdi\_sio kernel module works out of the box. You should see a
ttyUSB file under /dev as soon as you plug in the Saker.

If you are using a SmartRF06 board for programming the device or to act as a
6LoWPAN communication partner, please refer to the README of the srf06-cc26xx
platform and follow the driver installation instructions there as well. Under
Linux, the kernel module needs a few adjustments. You will also find information
to install the drivers on Mac OS X.

Flashing your device
--------------------
After you have everything set up and compiled your firmware, you will need to
flash it onto the hardware in order to run it. Here's two different ways how to
do that. Both should work on Windows and Linux. On Mac OS X, the serial
bootloader method is the only one available.

### Using the Serial Bootloader
If the device is configured correctly\*, it is possible to start the device in
bootloader mode. It is then possible to load firmware onto the device using the
`cc2538-bsl.py` python script that ships with contiki under tools/cc2538-bsl.

To start the bootloader mode, connect the device to your computer via USB, then
reset the device with the right button while holding down the left button
as well.

Then, flash the firmware on your device by running
    python cc2538-bsl.py -e -w -v path/to/image.bin
The given parameters instruct the script to erase the entire flash, write the
given image, and verify whether writing succeeded.

Please note that this script uses the .bin image that will be produced by the
compilation along with numerous others.

In the platform's `Makefile.saker`, there is also an `%.upload`-target that
calls `cc2538-bsl.py`, so that you can run e.g. 
`make TARGET=saker border-router.bin border-router.upload` to do both 
the compilation and the flashing in one step. The platform `Makefile` is sourced
by the Contiki `Makefile.include`, so ultimately by any project's `Makefile`, so
the `%.upload` target should be available for every example.

> \* "Configured correctly": The backdoor is configured correctly with the
> default values for the macros `FLASH_CCA_CONF_BOOTLDR_BACKDOOR` and related in
> the platform's `contiki-conf.h`. It is possible to overwrite those values in
> your `project-conf.h`, which will disable or might otherwise break the
> backdoor mechanism. It is then necessary to flash the device via JTAG and a
> corresponding program.

### Using a JTAG Interface like TI SmartRF06 + Uniflash
The CC2538's JTAG interface is exposed via the 10-pin fine pitch header X201.
The SmartRF06 features that same header (labelled: "10-pin ARM Cortex Debug
Connector".) Connect these two using an applicable cable and paying close
attention to the Pin1 markers on the boards.

On the SmartRF board, make sure that the jumpers "VDD to EM" and "Enable UART
over XDS100v3" are on, and the power source switch is set to USB. Connect the
SRF06 to your computer and turn the board on.

You can use TI's [SmartRF Flash Programmer 2][smart-rf-flashprog] or [Uniflash][uniflash]
programs to flash the device.  These instructions continue with Uniflash, but
finding the corresponding settings in Flash Programmer 2 should be
straightforward.

Select "CC2538SF53" as your device and "Texas Instruments XDS100v3 USB Debug
Probe" as your connection. As your flash image, select the .bin image of your
firmware. Set the load address to 0x200000 (two hundred thousand). On the
Settings tab, make sure that "Erase" is set to "Entire Flash". Back on the
Program tab, click on "Load Image". The console at the bottom should tell you
that everything went alright. To make sure, verify the image by pressing
"Verify Image".

> Note: In our experience Uniflash had trouble halting the CPU and continuing to
> flash it. The load process would abort, giving the error "Load failed" and the
> log message "Timed out waiting for target to halt." Just stubbornly trying
> again would then succeed.

Examples
========
Under `examples/saker` you will find the following examples.

ip64-rpl-border-router
----------------------
This example will have the Saker act as a border router between a 6LoWPAN
wireless network and some extern network with internet access, allowing the
wireless devices to communicate with any internet server. The example we will
use on the client device will send sensor data via MQTT to a pre-configured
server. Per default, that is IBM's Internet of Things Quickstart Server, but you
may adapt it to send data to a server of your choice.

The Saker firmware can be configured to either connect to a IPv6 network with
a statically configured address via SLIP, or to a IPv4 ethernet with DHCP via
Ethernet. We are currently working on improving the example code to make it more
flexible in using SLIP/Ethernet, IPv4/IPv6 and DHCP/Static Configuration.

### Preparing a wireless node
To give the Saker some traffic to route, prepare a wireless node that tries to
connect to an internet server. The example mqtt-client can be found under
`examples/cc26xx/cc26xx-web-demo`. That example can be compiled for the
SmartRF06 board with the CC1310EM, CC1350EM, or CC2538EM evaluation modules, the
CC26XX based Sensortag, or the CC2650 or CC1310 Launchpads.

> This section is really just a quick run-down of the actual README under
> `platform/srf06-cc26xx/README.md`. Should questions arise at any point, please
> refer there.

#### Prepare the code
1. Configure what the firmware will do by editing
`examples/cc26xx/cc26xx-web-demo/project-conf.h`. Take note of the configured
`IEEE802154_CONF_PANID` and `RF_CORE_CONF_CHANNEL`. The defaults here should
work fine, but if problems arise, these parameters will be the first ones to
double check. For this example, set `RF_BLE_CONF_ENABLED`,
`CC26XX_WEB_DEMO_CONF_6LBR_CLIENT`, `CC26XX_WEB_DEMO_CONF_COAP_SERVER`, and
`CC26XX_WEB_DEMO_CONF_NET_UART` to 0, leaving only
`CC26XX_WEB_DEMO_CONF_MQTT_CLIENT` enabled.
2. Fix the IP address `broker_ip` in `mqtt-client.c`. The standard value,
`0064:ff9b:0000:0000:0000:0000:b8ac:7cbd`, attempts to wrap the IPv4 address
`184.172.124.189` (in hexadecimal: `0xb8ac7cbd`) into an IPv6 address using the
Well-Known NAT64 prefix `64:ff9b::/96`. However, Contiki's own IP64 module does
not use that Well-Known  Prefix, but the prefix for IPv4-mapped addresses,
`::ffff:0000/24`. Hence, change the address to
`0000:0000:0000:0000:0000:ffff:b8ac:7cb` instead. (See core/net/ip/ip64-addr.h
with links to the relevant RFCs for the gory details.)
3. Other configurations for your device, e.g. `BOARD_CONF_DEBUGGER_DEVPACK` if
you are using a Sensortag with a Dbugger DevPack. Please refer to your device's
README.md.

#### Compile the code
Strictly speaking, to build it you need to run 
`make TARGET=srf06-cc26xx BOARD=srf06/cc26xx`. However, the example directories 
contain a `Makefile.target` which is automatically included and specifies the
correct `TARGET=` argument. The `BOARD=` environment variable defaults to
`srf06/cc26xx` (which is the SmartRF06 EB + CC26XXEM). Thus, for examples under
the `cc26xx` directory, and when using the SmartRF06 EB, you can simply run
`make`.

Other options for the `BOARD` make variable are:

* Srf06+CC26xxEM: Set `BOARD=srf06/cc26xx`
* Srf06+CC13xxEM: Set `BOARD=srf06/cc13xx`
* CC2650 tag: Set `BOARD=sensortag/cc2650`
* CC2650 Launchpad: Set `BOARD=launchpad/cc2650`
* CC1310 Launchpad: Set `BOARD=launchpad/cc1310`

#### Flash the device
Please refer to your device's manual on how to flash it. For the SRF06-based
boards and Launchpads, the serial bootloader is available. All devices,
including the Sensortags, support flashing via JTAG and a program like Flash
Programmer 2 or Uniflash.

### Preparing the Saker
The Saker's firmware code is under
`examples/saker/ip64-rpl-border-router`. 

#### Prepare the code
Configure the example.
1. In `project-conf.h`, configure `PLATFORM_CONF_USE_CC1200`. If it is set to 1,
the CC2538 2.4GHz RF core will be disabled and the CC120 will be started to
drive its Sub-GHz antenna. This configuration will work with all CC13xx based
communication partners. If it is set to 0 or not defined (the default is 0), the
CC1200 will be disabled and the CCC2538 RF Core will be enabled. This will work
with all CC26xx based devices.
2. In the `Makefile`, configure whether you want to use SLIP by setting or
unsetting `WITH_SLIP`. As you can see right below, so far `WITH_SLIP` and
`WITH_IP64` exclude each other, so turning SLIP on will turn IP64 off.
3. Also in the `Makefile`, choose whether you want the Saker to offer a HTTP
webserver that offers some technical information about the running network.
Choose `1` for the simple internal webserver, `0` for no server, or any name to
run a webserver app of that name from the `apps` directory, like `webserver`.
For both the internal webserver and `apps/webserver`, there's a few additional
`#define`s in the `project-conf.h` that you might want to take a look over.
4. In `project-conf.h, take a look at the other `#define`s that there are. They
control debugging out put at startup, regular statistics outputs while the
device is running, blinkenlights, and so on.

#### Compile the Code
Compile the code by running `make TARGET=saker`.

#### Flash the Device
Flash the `border-router.bin` image onto your device with any of the recipes
described under [Flashing your device](#flashing-your-device). 

### Running it all
Once the device and the Saker are ready, run it all by connecting the Saker
to USB and, if you are not using SLIP, to Ethernet. Also power up the client
device. Check the `printf` output of the devices using a serial program like
`minicom` or `hterm`. For both our example and the `cc26xx-web-demo`, the serial
consoles are preconfigured for 8 data bits, 1 Stop bit, no parity at 115200
baud.

#### Determining the client device's device ID
The Saker will output the IP Address of its LAN interface like so:
    Starting DHCPv4
    Inited
    Requested
    Requesting IPv4 address...........DHCP Configured with 192.0.2.1

Navigate to that IP address with a web browser to access the Saker's web
interface. There, you should be able to see the client device's local IPv6
address under "Neighbors". Under "Routes", there should be another entry listing
the client device's global IPv6 address as being available via the device's
local IP address.

The device ID that IBM uses is constructed from the client device's MAC address'
first three bytes and its IPv6 address' last three bytes. The default MAC
address begins with 0x00124b. The IPv6 address of the device can be read from
the Saker's web interface. So, for a device with the IPv6 address
`fe80::212:5b00:89ab:cdef`, your device ID is `00124babcdef`.

> The device's MAC address be overridden by setting `IEEE_ADDR_CONF_HARDCODED`
> to 1 and `IEEE_ADDR_CONF_ADDRESS` to an address of your choice in the
> `project-conf.h` of the MQTT example. This is turned off by default, but is
> worth double checking if you struggle with finding the device ID.

#### Viewing the data
With your device ID, check out the data that the MQTT server has received. For
IBM's server, go to <https://quickstart.internetofthings.ibmcloud.com>. 

> Note: IBM's server uses websockets to display the device data. Many corporate networks block websockets in their firewall. While IBM will then correctly receive the device data, the webpage will not display it. Please check with your network administrator if websockets work in your environment if you experience trouble.

Sensniff
--------
The Saker can be programmed to act as a sniffer to record wireless 802.15.4
communications using [Sensniff](https://github.com/g-oikonomou/sensniff). The
firmware code is available in Contiki under `examples/sensniff`.

### Prepare the code
In `target-conf.h` under `examples/sensniff/saker`, select a value
for `PLATFORM_CONF_USE_CC1200`. Set it to 1 to use the CC1200 radio chip to
sniff out Sub-GHz traffic. For 2.4GHz traffic, leave it to 0. Take note of the
`UART0_CONF_BAUD_RATE`, which is set to 460800 by default.

### Compile and flash the firmware
Compile the sniffer firmware by running `make TARGET=saker` and
flash it on the Saker using one of the recipes under [Flashing your
device](#flashing-your-device).

### Run the host tool
The host tool is available from Github, see above. Check out that repository and
read its `README.md`. You can run the host tool by running 
`python sensniff.py -b 460800 -d /dev/ttyUSBX`, exchanging `USBX` for whatever 
port where your Saker is connected. The host tool should report 
`Sniffing in channel: 25`.  Make sure to use the baud rate that is configured in
`target-conf.h` as the `-b` parameter. 

A list of available commands will be printed when starting the tool or when you
type `h` or `?`. There are commands available to set the channel in which you
want to sniff.

### Run wireshark
Open the FIFO `/tmp/sensniff` in wireshark by running `wireshark -k -i
/tmp/sensniff`. Packets in the configured channel should appear. 

Advanced Topics
===============
The platform's functionality can be customised by tweaking the various
configuration directives in `platform/saker/contiki-conf.h`. Bear in
mind that defines specified in `contiki-conf.h` can be over-written by defines
specified in `project-conf.h`, which is a file commonly encountered in example
directories.

Thus, if you want to modify the platform's default behaviour, change values in
`contiki-conf.h`. If you want to configure custom behaviour for a specific
example, modify this example's `project-conf.h`. If you have any changes that
you think would be good global defaults, we would love to hear from you on
[Github](https://github.com/Weptech_elektronik).

> Note: Some defines in `contiki-conf.h` are not meant to be modified.

Node IEEE/RIME/IPv6 Addresses
-----------------------------
Nodes will generally autoconfigure their IPv6 address based on their IEEE
address. The IEEE address can be read directly from the CC2538 Info Page, or it
can be hard-coded. Additionally, the user may specify a 2-byte value at build
time, which will be used as the IEEE address' 2 LSBs.

To configure the IEEE address source location (Info Page or hard-coded), use the
`IEEE_ADDR_CONF_HARDCODED` define in contiki-conf.h:

* 0: Info Page
* 1: Hard-coded

If `IEEE_ADDR_CONF_HARDCODED` is defined as 1, the IEEE address will take its
value from the `IEEE_ADDR_CONF_ADDRESS` define. If `IEEE_ADDR_CONF_HARDCODED` is
defined as 0, the IEEE address can come from either the primary or secondary
location in the Info Page. To use the secondary address, define
`IEEE_ADDR_CONF_USE_SECONDARY_LOCATION` as 1.

Additionally, you can override the IEEE's 2 LSBs, by using the `NODEID` make
variable. The value of `NODEID` will become the value of the `IEEE_ADDR_NODE_ID`
pre-processor define. If `NODEID` is not defined, `IEEE_ADDR_NODE_ID` will not
get defined either. For example:

    make NODEID=0x79ab

This will result in the 2 last bytes of the IEEE address getting set to 0x79
0xAB.

UART Baud Rate
--------------
By default, the CC2538 UART is configured with a baud rate of 115200. It is easy
to increase this to 230400 by changing the value of `UART0_CONF_BAUD_RATE` in
`contiki-conf.h` or `project-conf.h`.

    #define UART0_CONF_BAUD_RATE 230400

RF and USB DMA
--------------
Transfers between RAM and the RF core will be conducted with DMA. If for
whatever reason you wish to disable this, here are the relevant configuration
lines.

    #define CC2538_RF_CONF_TX_USE_DMA            1
    #define CC2538_RF_CONF_RX_USE_DMA            1

Low-Power Modes
---------------
The CC2538 port supports power modes for low energy consumption. The SoC will
enter a low power mode as part of the main loop when there are no more events to
service.

LPM support can be disabled in its entirety by setting `LPM_CONF_ENABLE` to 0 in
`contiki-conf.h` or `project-conf.h`.

The Low-Power module uses a simple heuristic to determine the best power mode,
depending on anticipated Deep Sleep duration and the state of various
peripherals.

In a nutshell, the algorithm first answers the following questions:

* Is the RF off?
* Are all registered peripherals permitting PM1+?
* Is the Sleep Timer scheduled to fire an interrupt?

If the answer to any of the above question is "No", the SoC will enter PM0. If
the answer to all questions is "Yes", the SoC will enter one of PMs 0/1/2
depending on the expected Deep Sleep duration and subject to user configuration
and application requirements.

At runtime, the application may enable/disable some Power Modes by making calls
to `lpm_set_max_pm()`. For example, to avoid PM2 an application could call
`lpm_set_max_pm(1)`. Subsequently, to re-enable PM2 the application would call
`lpm_set_max_pm(2)`.

The LPM module can be configured with a hard maximum permitted power mode.

    #define LPM_CONF_MAX_PM        N

Where N corresponds to the PM number. Supported values are 0, 1, 2. PM3 is not
supported. Thus, if the value of the define is 1, the SoC will only ever enter
PMs 0 or 1 but never 2 and so on.

The configuration directive `LPM_CONF_MAX_PM` sets a hard upper boundary. For
instance, if `LPM_CONF_MAX_PM` is defined as 1, calls to `lpm_set_max_pm()` can
only enable/disable PM1. In this scenario, PM2 can not be enabled at runtime.

When setting `LPM_CONF_MAX_PM` to 0 or 1, the entire SRAM will be available.
Crucially, when value 2 is used the linker will automatically stop using the
SoC's SRAM non-retention area, resulting in a total available RAM of 16MB
instead of 32MB.

> Note: If your application requires information being received via UART, e.g.
> when you use SLIP, `LPM_CONF_MAX_PM` must be set to 0. Otherwise the UART will
> not be able to wake up and have a stable clock quick enough to receive
> incoming messages, which will garble your input.

### LPM and Duty Cycling Driver
LPM is highly related to the operations of the Radio Duty Cycling (RDC) driver
of the Contiki network stack and will work correctly with ContikiMAC and
NullRDC.

* With ContikiMAC, PMs 0/1/2 are supported subject to user configuration.
* When NullRDC is in use, the radio will be always on. As a result, the
  algorithm discussed above will always choose PM0 and will never attempt to
  drop to PM1/2.

Build headless nodes
--------------------
It is possible to turn off all character I/O for nodes not connected to a PC.
Doing this will entirely disable the UART as well as the USB controller,
preserving energy in the long term. The define used to achieve this is (1:
Quiet, 0: Normal output):

    #define CC2538_CONF_QUIET      0

Setting this define to 1 will automatically set the following to 0:

* `USB_SERIAL_CONF_ENABLE`
* `UART_CONF_ENABLE`
* `STARTUP_CONF_VERBOSE`

Code Size Optimisations
-----------------------
The build system currently uses optimization level `-Os`, which is controlled
indirectly through the value of the `SMALL` make variable. This value can be
overridden by example makefiles, or it can be changed directly in
`platform/saker/Makefile.saker`.

Historically, the `-Os` flag has caused problems with some toolchains. If you
are using one of the toolchains documented in this README, you should be able to
use it without issues. If for whatever reason you do come across problems, try
setting `SMALL=0` or replacing `-Os` with `-O2` in
`cpu/cc2538/Makefile.cc2538`.

Doxygen Documentation
=====================
This port's code has been documented with doxygen. To build the documentation,
navigate to `$(CONTIKI)/doc` and run `make`. This will build the entire contiki
documentation and may take a while.

If you want to build this platform's documentation only and skip the remaining
platforms, run this:

    make basedirs="platform/saker core cpu/cc2538 examples/saker"

Once you've built the docs, open `$(CONTIKI)/doc/html/index.html` and enjoy.

[smart-rf-flashprog]: http://www.ti.com/tool/flash-programmer "SmartRF Flash Programmer"
[uniflash]: http://processors.wiki.ti.com/index.php/Category:CCS_UniFlash "UniFlash"
[ftdidrivers]: http://www.ftdichip.com/FTDrivers.htm "FTDI Drivers"




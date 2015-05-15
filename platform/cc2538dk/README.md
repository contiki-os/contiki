Getting Started with Contiki for TI CC2538DK
============================================

This guide's aim is to help you start using Contiki for TI's CC2538 Development Kit. By
CC2538DK we mean a TI CC2538 Evaluation Module (EM), either standalone and powered by USB or attached to a SmartRF06 Evaluation Board (EB) or Battery Board (BB). The general usage scenario assumes that the EM is attached to a SmartRF06EB and is powered by it.

This guide assumes that you have basic understanding of how to use the command line and perform basic admin tasks on UNIX family OSs.

Port Features
=============
The platform has the following key features:

 * Deep Sleep support with RAM retention for ultra-low energy consumption.
 * Native USB support (CDC-ACM). SLIP over UART for border routers is no longer a bottleneck.
 * DMA transfers for increased performance (RAM to/from RF, RAM to/from USB).

In terms of hardware support, the following drivers have been implemented:

  * CC2538 System-on-Chip:
    * Standard Cortex M3 peripherals (NVIC, SCB, SysTick)
    * Sleep Timer (underpins rtimers)
    * SysTick (underpins the platform clock and Contiki's timers infrastructure)
    * RF
    * UART
    * Watchdog (in watchdog mode)
    * USB (in CDC-ACM)
    * uDMA Controller (RAM to/from USB and RAM to/from RF)
    * Random number generator
    * Low Power Modes
    * General-Purpose Timers. NB: GPT0 is in use by the platform code, the remaining GPTs are available for application development.
    * ADC
    * Cryptoprocessor (AES-CCM-256, SHA-256)
  * SmartRF06 EB and BB peripherals
    * LEDs
    * Buttons
    * ADC sensors (on-chip VDD / 3 and temperature, ambient light sensor)
    * UART connectivity over the XDS100v3 backchannel (EB only)

Requirements
============
To start using Contiki, you will need the following:

 * A toolchain to compile Contiki for the CC2538.
 * Drivers so that your OS can communicate with your hardware.
 * Software to upload images to the CC2538.

Different tasks can be performed under different operating systems. The table below summarises what task can be performed on which OS:

                       Windows     Linux     OS-X
    Building Contiki      Y          Y         Y
    Node Programming      Y          Y         Y
    Console output
      (UART)              Y          Y         Y
      (USB CDC-ACM)       Y          Y         Y
    Border Routers
      (UART)              N          Y         Y
      (USB CDC-ACM)       N          Y         Y
    Sniffer
      (UART)              N          Y         Y
      (USB CDC-ACM)       N          Y         Y

The platform has been developed and tested under Windows XP, Mac OS X 10.9.1 and Ubuntu 12.04 and 12.10. The matrix above has been populated based on information for those OSs.

Install a Toolchain
-------------------
The toolchain used to build contiki is arm-gcc, also used by other arm-based Contiki ports. If you are using Instant Contiki, you will have a version pre-installed in your system. To find out if this is the case, try this:

    $ arm-none-eabi-gcc -v
    Using built-in specs.
    Target: arm-none-eabi
    Configured with: /scratch/julian/lite-respin/eabi/src/gcc-4.3/configure
    ...
    (skip)
    ...
    Thread model: single
    gcc version 4.3.2 (Sourcery G++ Lite 2008q3-66)

The platform is currently being used/tested with "GNU Tools for ARM Embedded Processors". This is the recommended version and the one being used by Contiki's regression tests on Travis. <https://launchpad.net/gcc-arm-embedded>

The older version (Sourcery G++ Lite 2008q3-66) shown above should still work, but the port is no longer being tested with it. <http://sourcery.mentor.com/public/gnu_toolchain/arm-none-eabi>

Drivers
-------
You will need to install drivers so that your Operating System can communicate with the hardware

### For the SmartRF06 EB (UART)
The SmartRF communicates with the PC with a piece of hardware called the TI XDS100v3 Emulator (from now on simply XDS). This is a combined JTAG/UART interface and is used to program the EM, for debugging and for UART character I/O.

You will need to install XDS drivers if you want to do anything useful with the CC2538 UART.

  * **For Windows**: Installing [SmartRF Studio][smart-rf-studio] will install the drivers (A beta version is needed, not the one currently distributed on the TI site). Read the [SmartRF User Guide][smart-rf-ug] for more detailed instructions. After driver installation, the XDS will appear as a COM port.
  * **For Linux**: The XDS is based on an FTDI chip and new Linux kernels provide very good support for FTDI chips. If the kernel module does not kick in automatically, perform the following steps:
    * Connect the SmartRF to the linux box. Find the device's VID and PID (0403:a6d1 in the output below):

            $ lsusb
            ...
            Bus 001 Device 002: ID 0403:a6d1 Future Technology Devices International, Ltd
            ...


    * As root or with `sudo`, run the command below (if necessary, replace the `vendor` and `product` arguments with the values you got from `lsusb`):

            modprobe ftdi_sio vendor=0x403 product=0xa6d1
            
    * From Kernel 3.12 run the command below:
            
            modprobe ftdi_sio
            echo 0403 a6d1 > /sys/bus/usb-serial/drivers/ftdi_sio/new_id

    * You may have have to remove package `brltty`, if it's installed.
    * The board should have enumerated as `/dev/ttyUSB{0,1}`. `ttyUSB1` will be the UART backchannel.
  * **For OS X**: We need to hack the kernel extension (kext) a little bit:
    * First, install the FTDI VCP driver from <http://www.ftdichip.com/Drivers/VCP.htm>
    * Edit `/System/Library/Extensions/FTDIUSBSerialDriver.kext/Contents/Info.plist` with a text editor.
    * Add the following block somewhere under `IOKitPersonalities`.

            <key>TI_XDS100v3</key>
            <dict>
            <key>CFBundleIdentifier</key>
              <string>com.FTDI.driver.FTDIUSBSerialDriver</string>
              <key>IOClass</key>
              <string>FTDIUSBSerialDriver</string>
              <key>IOProviderClass</key>
              <string>IOUSBInterface</string>
              <key>bConfigurationValue</key>
              <integer>1</integer>
              <key>bInterfaceNumber</key>
              <integer>1</integer>
              <key>idProduct</key>
              <integer>42705</integer>
              <key>idVendor</key>
              <integer>1027</integer>
            </dict>

    * If the kext is loaded at the time you perform this change, you will have to either reload it or reboot the Mac. At the time of writing this guide, reloading the kext would fail with errors so rebooting appears to be the only solution.
    * After you have rebooted, plug in the SmartRF, turn it on and then load the kext manually:

            sudo kextload /System/Library/Extensions/FTDIUSBSerialDriver.kext

If everything worked, the XDS will have enumerated as `/dev/tty.usbserial-<serial-number>`

### For the CC2538EM (USB CDC-ACM)
The CC2538 EM's USB Vendor and Product IDs are the following:

  * VID 0x0451
  * PID 0x16C8

The implementation in Contiki is pure CDC-ACM: The Linux and OS X kernels know exactly what to do and drivers are not required.

On windows, you will need to provide a driver. You have two options:

  * Use the signed or unsigned driver provided by TI in [CC2538 Foundation Firmware](http://www.ti.com/tool/cc2538-sw). You will find them both under the `drivers` directory.
  * Download a generic Virtual Serial Port driver and modify it so it works for the CC2538.

For the latter option:

  * Download this [LUFA CDC-ACM driver](https://raw.githubusercontent.com/abcminiuser/lufa/master/Demos/Device/LowLevel/VirtualSerial/LUFA%20VirtualSerial.inf).
  * Adjust the VID and PID near the end with the values at the start of this section.
  * Next time you get prompted for the driver, include the directory containing the .inf file in the search path and the driver will be installed.

### Improve Stability on Linux
There are some issues under recent Ubuntu versions (e.g. 12.10). The problem manifests itself as frequent connects/disconnects for the first approximately 30 seconds after the device has been connected to the host (Both UART and USB). The reason for this is that, as soon as the device is connected, the modem manager kicks in and starts probing it. To prevent this, we can tell the modem manager to leave this device alone:

* edit `/lib/udev/rules.d/77-mm-usb-device-blacklist.rules`
* Add the following line somewhere:

        ATTRS{idVendor}=="0451", ATTRS{idProduct}=="16c8", ENV{ID_MM_DEVICE_IGNORE}="1"

* This line will instruct modem-manager to ignore the EM's USB port. To achieve the same for the SmartRF's XDS port, add a similar line but replace `idVendor` and `idProduct` with the XDS' VID/PID: 0403/a6d1.

* restart the modem-manager process:

        sudo service modemmanager restart

This will tell modem manager to suppress probing for these VID/PID combinations. Keep in mind that the `blacklist.rules` file may get overwritten by future modem-manager updates and you may have to re-apply the fix in the future.

### Jumper Settings
Be careful with jumper settings on the CC2538 EM. The EM can be powered from the SmartRF or it can be powered from its own USB port.

* Locate the pair of adjacent jumpers on the EM.
* To power the EM from the SmartRF, place the jumper on the inner two pins (the ones closest to the SoC).
* To power the EM from its USB, place the jumper on the two pins nearest to the USB port.

The USB functionality will work on both situations, the jumper only controlls power supply.

### Device Enumerations
For the UART, serial line settings are 115200 8N1, no flow control.

Once all drivers have been installed correctly:

On windows, devices will appear as a virtual COM port (applies to both the UART/XDS as well as USB CDC-ACM).

On Linux and OS X, devices will appear under `/dev/`.

On OS X:

* XDS backchannel: `tty.usbserial-<serial number>`
* EM in CDC-ACM: `tty.usbmodemf<X><ABC>` (X a letter, ABC a number e.g. `tty.usbmodemfd121`)

On Linux:

* XDS backchannel: `ttyUSB1`
* EM in CDC-ACM: `ttyACMn` (n=0, 1, ....)

Software to Program the Nodes
-----------------------------
The CC2538 can be programmed via the jtag interface or via the serial boot loader on the chip.

* On Windows:
    * Nodes can be programmed with TI's ArmProgConsole or the [SmartRF Flash Programmer 2][smart-rf-flashprog]. The README should be self-explanatory. With ArmProgConsole, upload the file with a `.bin` extension. (jtag + serial)
    * Nodes can also be programmed via the serial boot loader in the cc2538. In `tools/cc2538-bsl/` you can find `cc2538-bsl.py` this is a python script that can download firmware to your node via a serial connection. If you use this option you just need to make sure you have a working version of python installed. You can read the README in the same directory for more info. (serial)

* On Linux:
    * Nodes can be programmed with TI's [UniFlash] tool. With UniFlash, use the file with `.elf` extension. (jtag + serial)
    * Nodes can also be programmed via the serial boot loader in the cc2538. No extra software needs to be installed. (serial)

* On OSX:
    * The `cc2538-bsl.py` script in `tools/cc2538-bsl/` is the only option. No extra software needs to be installed. (serial)

The file with a `.cc2538dk` extension is a copy of the `.elf` file.

Use the Port
============
The following examples are intended to work off-the-shelf:

* Examples under `examples/cc2538dk`
* Border router: `examples/ipv6/rpl-border-router`
* Webserver: `examples/webserver-ipv6`

We can also use the CoAP example from `examples/er-rest-example/`. However, the example's `Makefile` is slightly problematic at the time of writing this guide. As a workaround, open it and delete this entire block:

    ifneq ($(TARGET), minimal-net)
    ifneq ($(TARGET), native)
    ifneq ($(TARGET), econotag)
    ifneq ($(findstring avr,$(TARGET)), avr)
    PROJECT_SOURCEFILES += static-routing.c
    endif
    endif
    endif
    endif

The key is to prevent compilation of `static-routing.c`. If you're curious about more info, see the discussion here:
<http://thread.gmane.org/gmane.os.contiki.devel/16543>

and the related bug report here:
<https://github.com/contiki-os/contiki/issues/87>

Build your First Examples
-------------------------
It is recommended to start with the `cc2538-demo` and `timer-test` examples under `examples/cc2538dk/`. These are very simple examples which will help you get familiar with the hardware and the environment.

Strictly speaking, to build them you need to run `make TARGET=cc2538dk`. However, the example directories contain a `Makefile.target` which is automatically included and specifies the correct `TARGET=` argument. Thus, for examples under the `cc2538dk` directory, you can simply run `make`.

If you want to upload the compiled firmware to a node via the serial boot loader you need to manually enable the boot loader and then use `make cc2538-demo.upload`. On the SmartRF06 board you enable the boot loader by resetting the board (EM RESET button) while holding the `select` button. (The boot loader backdoor needs to be enabled on the chip for this to work, see README in the `tools/cc2538-bsl` directory for more info)

For the `cc2538-demo`, the comments at the top of `cc2538-demo.c` describe in detail what the example does.

To generate an assembly listing of the compiled firmware, run `make cc2538-demo.lst`. This may be useful for debugging or optimizing your application code. To intersperse the C source code within the assembly listing, you must instruct the compiler to include debugging information by adding `CFLAGS += -g` to the project Makefile and rebuild by running `make clean cc2538-demo.lst`.

Node IEEE/RIME/IPv6 Addresses
-----------------------------

Nodes will generally autoconfigure their IPv6 address based on their IEEE address. The IEEE address can be read directly from the CC2538 Info Page, or it can be hard-coded. Additionally, the user may specify a 2-byte value at build time, which will be used as the IEEE address' 2 LSBs.

To configure the IEEE address source location (Info Page or hard-coded), use the `IEEE_ADDR_CONF_HARDCODED` define in contiki-conf.h:

* 0: Info Page
* 1: Hard-coded

If `IEEE_ADDR_CONF_HARDCODED` is defined as 1, the IEEE address will take its value from the `IEEE_ADDR_CONF_ADDRESS` define. If `IEEE_ADDR_CONF_HARDCODED` is defined as 0, the IEEE address can come from either the primary or secondary location in the Info Page. To use the secondary address, define `IEEE_ADDR_CONF_USE_SECONDARY_LOCATION` as 1.

Additionally, you can override the IEEE's 2 LSBs, by using the `NODEID` make variable. The value of `NODEID` will become the value of the `IEEE_ADDR_NODE_ID` pre-processor define. If `NODEID` is not defined, `IEEE_ADDR_NODE_ID` will not get defined either. For example:

    make NODEID=0x79ab

This will result in the 2 last bytes of the IEEE address getting set to 0x79 0xAB

Note: Some early production devices do not have am IEEE address written on the Info Page. For those devices, using value 0 above will result in a Rime address of all 0xFFs. If your device is in this category, define `IEEE_ADDR_CONF_HARDCODED` to 1 and specify `NODEID` to differentiate between devices.

### Scripted multi-image builds

You can build multiple nodes with different `NODEID`s sequentially. The only platform file relying on the value of `NODEID` (or more accurately `IEEE_ADDR_NODE_ID`) is `ieee-addr.c`, which will get recompiled at each build invocation. As a result, the build system can be scripted to build multiple firmware images, each one with a different MAC address. Bear in mind that, if you choose to do such scripting, you will need to make a copy of each firmware before invoking the next build, since each new image will overwrite the previous one. Thus, for example, you could do something like this:

    for image in 1 2 3 4; do make cc2538-demo NODEID=$image && \
    cp cc2538-demo.cc2538dk cc2538-demo-$image.cc2538dk; done

Which would build `cc2538-demo-1.cc2538dk`, `cc2538-demo-2.cc2538dk` etc

As discussed above, only `ieee-addr.c` will get recompiled for every build. Thus, if you start relying on the value of `IEEE_ADDR_NODE_ID` in other code modules, this trick will not work off-the-shelf. In a scenario like that, you would have to modify your script to touch those code modules between every build. For instance, if you are using an imaginary `foo.c` which needs to see changes to `NODEID`, the script above could be modified like so:

    for image in 1 2 3 4; do make cc2538-demo NODEID=$image && \
    cp cc2538-demo.cc2538dk cc2538-demo-$image.cc2538dk && \
    touch foo.c; done


Build a 6LoWPAN Testbed
-----------------------
Once you are familiar with the basics, get a mini 6LoWPAN testbed.

Start by building a border router from `examples/ipv6/rpl-border-router`

  * Turn on debugging output by changing `#define DEBUG DEBUG_NONE` to `#define DEBUG DEBUG_PRINT` in `border-router.c`.
  * The border router's configuration (`project-conf.h`), sets the maximum size of the uIP buffer (`UIP_CONF_BUFFER_SIZE`). This is a bit restrictive for this platform: we can afford to allocate more memory of we want to. It's not necessary, but feel free to remove the lines below from `project-conf.h`, allowing the platform to use its own default value.

        #ifndef UIP_CONF_BUFFER_SIZE
        #define UIP_CONF_BUFFER_SIZE    140
        #endif

  * `make TARGET=cc2538dk`
  * Flash your device with `border-router.cc2538dk` or `border-router.bin`.
  * Connect device to Linux or OS X over its XDS port.
  * `cd $(CONTIKI)/tools`
  * `make tunslip6`
  * `sudo $(CONTIKI)/tools/tunslip6 -s /dev/<device> aaaa::1/64`
  * The router will print its own IPv6 address. Use it below.

        Got configuration message of type P
        Setting prefix aaaa::
        created a new RPL dag
        Server IPv6 addresses:
         aaaa::212:4b00:89ab:cdef
         fe80::212:4b00:89ab:cdef

  * `ping6 <address>`
  * `curl -g "http://[<address-inside-the-brackets>]"` and the border router will serve you a web-page. Try from a browser too.

Afterwards, build RPL nodes in `examples/cc2538dk/udp-ipv6-echo-server`

  * If you are not reading node MAC addresses from the Info Page, make sure you assign a new MAC address for each node by passing `NODEID=xyz` to the make command line, as discussed in an earlier section.
  * `make` (or `make NODEID=xyz`). You don't need to specify `TARGET=` as this is saved in `Makefile.target`
  * Flash device with `udp-echo-server.cc2538dk` or `.bin`.
  * If you want to see console output, connect the device to a PC over its XDS port. You don't need to do that though, this example will work on 'headless' nodes. This may be a good chance to try out your BB, if you have one.
  * Repeat for more nodes, each one with a new `NODEID` if necessary.

More things to play around with

  * Feel free to throw some webservers in the mix. In `examples/webserver-ipv6`, run `make TARGET=cc2538dk NODEID=<value>`
  * `ping6` and `netcat` the RPL nodes (the echo server listens on UDP 3000): `nc -6u <address> 3000`
  * Retrieve a webpage from a node. Use `curl` as above, or you can `wget` or you can fire up a browser and navigate to the websever's address.

Build a Sniffer - Live Traffic Capture with Wireshark
-----------------------------------------------------
There is a sniffer example in `examples/cc2538dk/sniffer/`

Diverging from platform defaults, this example configures the UART to use a baud rate of 460800. The reason is that sniffers operating at 115200 are liable to corrupt frames. This is almost certain to occur when sniffing a ContikiMAC-based deployment. See more details on how to configure UART baud rates in the "Advanced Topics" section.

Once you have built it and flashed your device, download and run `sensniff` on your PC (Linux or OS X). Get it from:
<https://github.com/g-oikonomou/sensniff>

Instructions on what to do with `sensniff` are in its README. Make sure to set the `-b` command line parameter correctly to match the sniffer's UART baud rate. Lastly, bear in mind that Host-to-Peripheral commands will not work with the CC2538 at this stage.

Mix & Match with CC2530s
------------------------
Every aspect of the CC2538 port is interoprable with the existing CC2530 port. Same 6LoWPAN prefix, same .15.4 channel and PAN ID etc. Thus, you can throw in CC2530s at will, for as long as you are using NullRDC. For instance, you can have a CC2531 border router with SmartRF06 + CC2538 EMs as RPL nodes. Or you can have a CC2538 border router with SmartRF05 + CC2530EM RPL nodes etc.

If you want to add CC2530s to the network, make sure you have followed the CC2530 how-to on the main contiki wiki:
<https://github.com/contiki-os/contiki/wiki/8051-Based-Platforms>

Advanced Topics
===============
The platform's functionality can be customised by tweaking the various configuration directives in `platform/cc2538dk/contiki-conf.h`. Bear in mind that defines specified in `contiki-conf.h` can be over-written by defines specified in `project-conf.h`, which is a file commonly encountered in example directories.

Thus, if you want to modify the platform's default behaviour, change values in `contiki-conf.h`. If you want to configure custom behaviour for a specific example, modify this example's `project-conf.h`.

N.B. Some defines in `contiki-conf.h` are not meant to be modified.

Switching between UART and USB (CDC-ACM)
----------------------------------------
By default, everything is configured to use the UART (stdio, border router's SLIP, sniffer's output stream). If you want to change this, these are the relevant lines in contiki-conf.h (0: UART, 1: USB):

    #define SLIP_ARCH_CONF_USB          0 /** SLIP over UART by default */
    #define CC2538_RF_CONF_SNIFFER_USB  0 /** Sniffer out over UART by default */
    #define DBG_CONF_USB                0 /** All debugging over UART by default */

You can multiplex things (for instance, SLIP as well as debugging over USB or SLIP over USB but debugging over UART and other combinations).

Selecting UART0 and/or UART1
----------------------------
By default, everything is configured to use the UART0 (stdio, border router's SLIP, sniffer's output stream). If you want to change this, these are the relevant lines in contiki-conf.h (0: UART0, 1: UART1):

    #define SERIAL_LINE_CONF_UART       0
    #define SLIP_ARCH_CONF_UART         0
    #define CC2538_RF_CONF_SNIFFER_UART 0
    #define DBG_CONF_UART               0
    #define UART1_CONF_UART             0

A single UART is available on CC2538DK, so all the configuration values above should be the same (i.e. either all 0 or all 1), but 0 and 1 could be mixed for other CC2538-based platforms supporting 2 UARTs.

The chosen UARTs must have their ports and pins defined in board.h:

    #define UART0_RX_PORT            GPIO_A_NUM
    #define UART0_RX_PIN             0
    #define UART0_TX_PORT            GPIO_A_NUM
    #define UART0_TX_PIN             1

Only the UART ports and pins implemented on the board can be defined.

UART Baud Rate
--------------
By default, the CC2538 UART is configured with a baud rate of 115200. It is easy to increase this to 230400 by changing the value of `UART0_CONF_BAUD_RATE` or `UART1_CONF_BAUD_RATE` in `contiki-conf.h` or `project-conf.h`, according to the UART instance used.

    #define UART0_CONF_BAUD_RATE 230400
    #define UART1_CONF_BAUD_RATE 230400

RF and USB DMA
--------------
Transfers between RAM and the RF and USB will be conducted with DMA. If for whatever reason you wish to disable this, here are the relevant configuration lines.

    #define USB_ARCH_CONF_DMA                    1
    #define CC2538_RF_CONF_TX_USE_DMA            1
    #define CC2538_RF_CONF_RX_USE_DMA            1

Low-Power Modes
---------------
The CC2538 port supports power modes for low energy consumption. The SoC will enter a low power mode as part of the main loop when there are no more events to service.

LPM support can be disabled in its entirety by setting `LPM_CONF_ENABLE` to 0 in `contiki-conf.h` or `project-conf.h`.

NOTE: If you are using PG2 version of the Evaluation Module, the SoC will refuse to enter Power Modes 1+ if the debugger is connected and will always enter PM0 regardless of configuration. In order to get real low power mode functionality, make sure the debugger is disconnected. The Battery Board is ideal to test this.

The Low-Power module uses a simple heuristic to determine the best power mode, depending on anticipated Deep Sleep duration and the state of various peripherals.

In a nutshell, the algorithm first answers the following questions:

* Is the RF off?
* Are all registered peripherals permitting PM1+?
* Is the Sleep Timer scheduled to fire an interrupt?

If the answer to any of the above question is "No", the SoC will enter PM0. If the answer to all questions is "Yes", the SoC will enter one of PMs 0/1/2 depending on the expected Deep Sleep duration and subject to user configuration and application requirements.

At runtime, the application may enable/disable some Power Modes by making calls to `lpm_set_max_pm()`. For example, to avoid PM2 an application could call `lpm_set_max_pm(1)`. Subsequently, to re-enable PM2 the application would call `lpm_set_max_pm(2)`.

The LPM module can be configured with a hard maximum permitted power mode.

    #define LPM_CONF_MAX_PM        N

Where N corresponds to the PM number. Supported values are 0, 1, 2. PM3 is not supported. Thus, if the value of the define is 1, the SoC will only ever enter PMs 0 or 1 but never 2 and so on.

The configuration directive `LPM_CONF_MAX_PM` sets a hard upper boundary. For instance, if `LPM_CONF_MAX_PM` is defined as 1, calls to `lpm_set_max_pm()` can only enable/disable PM1. In this scenario, PM2 can not be enabled at runtime.

When setting `LPM_CONF_MAX_PM` to 0 or 1, the entire SRAM will be available. Crucially, when value 2 is used the linker will automatically stop using the SoC's SRAM non-retention area, resulting in a total available RAM of 16MB instead of 32MB.

### LPM and Duty Cycling Driver
LPM is highly related to the operations of the Radio Duty Cycling (RDC) driver of the Contiki network stack and will work correctly with ContikiMAC and NullRDC.

* With ContikiMAC, PMs 0/1/2 are supported subject to user configuration.
* When NullRDC is in use, the radio will be always on. As a result, the algorithm discussed above will always choose PM0 and will never attempt to drop to PM1/2.

Build headless nodes
--------------------
It is possible to turn off all character I/O for nodes not connected to a PC. Doing this will entirely disable the UART as well as the USB controller, preserving energy in the long term. The define used to achieve this is (1: Quiet, 0: Normal output):

    #define CC2538_CONF_QUIET      0

Setting this define to 1 will automatically set the following to 0:

* `USB_SERIAL_CONF_ENABLE`
* `UART_CONF_ENABLE`
* `STARTUP_CONF_VERBOSE`

Code Size Optimisations
-----------------------
The build system currently uses optimization level `-Os`, which is controlled indirectly through the value of the `SMALL` make variable. This value can be overridden by example makefiles, or it can be changed directly in `platform/cc2538dk/Makefile.cc2538dk`.

Historically, the `-Os` flag has caused problems with some toolchains. If you are using one of the toolchains documented in this README, you should be able to use it without issues. If for whatever reason you do come across problems, try setting `SMALL=0` or replacing `-Os` with `-O2` in `cpu/cc2538/Makefile.cc2538`.

Doxygen Documentation
=====================
This port's code has been documented with doxygen. To build the documentation, navigate to `$(CONTIKI)/doc` and run `make`. This will build the entire contiki documentation and may take a while.

If you want to build this platform's documentation only and skip the remaining platforms, run this:

    make basedirs="platform/cc2538dk core cpu/cc2538 examples/cc2538dk"

Once you've built the docs, open `$(CONTIKI)/doc/html/index.html` and enjoy.

Other Versions of this Guide
============================
If you prefer this guide in other formats, use the excellent [pandoc] to convert it.

* **pdf**: `pandoc -s --toc README.md -o README.pdf`
* **html**: `pandoc -s --toc README.md -o README.html`

More Reading
============
1. [SmartRF06 Evaluation Board User's Guide, (SWRU321)][smart-rf-ug]
2. [CC2538 System-on-Chip Solution for 2.4-GHz IEEE 802.15.4 and ZigBee&reg;/ZigBee IP&reg; Applications, (SWRU319B)][cc2538]

[smart-rf-studio]: http://www.ti.com/tool/smartrftm-studio "SmartRF Studio"
[smart-rf-flashprog]: http://www.ti.com/tool/flash-programmer "SmartRF Flash Programmer"
[smart-rf-ug]: http://www.ti.com/litv/pdf/swru321a     "SmartRF06 Evaluation Board User's Guide"
[cc2538]: http://www.ti.com/product/cc2538     "CC2538"
[uniflash]: http://processors.wiki.ti.com/index.php/Category:CCS_UniFlash "UniFlash"
[pandoc]: http://johnmacfarlane.net/pandoc/ "Pandoc - a universal document converter"

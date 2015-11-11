Zolertia Re-Mote platform
============================================

<a href="url"><img src="http://goo.gl/3AGZkI" align="center" height="320" ></a>

The Re-Mote platform is a IoT Hardware development platform based on TI's CC2538
system on chip (SoC), featuring an ARM Cortex-M3 with 512KB flash, 32Kb RAM, 
double RF interface, and the following goodies:

* ISM 2.4-GHz IEEE 802.15.4 & Zigbee compliant.
* ISM 868-, 915-, 920-, 950-MHz ISM/SRD Band.
* AES-128/256, SHA2 Hardware Encryption Engine.
* ECC-128/256, RSA Hardware Acceleration Engine for Secure Key Exchange.
* Power consumption down to 3uA using our shutdown mode.
* Co-Processor to allow peripheral management, programming over BSL without requiring to press any button to enter bootloader mode.
* Built-in battery charger (500mA), Energy Harvesting and Solar Panels to be connected to standards LiPo batteries.
* Power input with wide range 2-26VDC.
* Built-in TMP102 temperature sensor
* Small form-factor (as the Z1 mote, half the size of an Arduino) 57x35 mm.


Port Features
=============
The platform has the following key features:

 * Deep Sleep support with RAM retention for ultra-low energy consumption.
 * Native USB support (CDC-ACM). SLIP over UART for border routers is no longer a bottleneck.
 * DMA transfers for increased performance (RAM to/from RF, RAM to/from USB).
 * Shutdown mode controlled externally by the Co-processor, completely power the
   platform OFF, further reducing current consumption.

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
    * Public Key Accelerator (ECDH, ECDSA)
    * LEDs
    * Buttons
    * Internal/external 2.4GHz antenna switch controllable by SW.
    * Built-in core temperature and battery sensor.
    * TMP102 temperature sensor driver.
  * CC1120 sub-1GHz radio interface:
    * Ported in Contiki, SPI based.
  * Micro-SD external storage:
    * Pending to port, SPI based.

Requirements
============
To start using Contiki, the following is required:

 * A toolchain to compile Contiki for the CC2538.
 * Drivers so that your OS can communicate with your hardware.
 * Software to upload images to the CC2538.


Install a Toolchain
-------------------
Forked from `platform/cc2538dk/README.md`.
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

The platform is currently being used/tested with the following toolchains:

* GNU Tools for ARM Embedded Processors. This is the recommended version. Works nicely on OS X. <https://launchpad.net/gcc-arm-embedded>
* Alternatively, you can use this older version for Linux. At the time of writing, this is the version used by Contiki's regression tests. <https://sourcery.mentor.com/public/gnu_toolchain/arm-none-eabi/arm-2008q3-66-arm-none-eabi-i686-pc-linux-gnu.tar.bz2>

Drivers
-------
The Re-Mote features a FTDI serial-to-USB module, the driver is commonly found in most OS, but if required it can be downloaded
from <http://www.ftdichip.com/Drivers/VCP.htm>
 

### For the CC2538EM (USB CDC-ACM)
The Re-Mote has built-in support for USB 2.0 USB, Vendor and Product IDs are the following:

  * VID 0x0451
  * PID 0x16C8

The implementation in Contiki is pure CDC-ACM: The Linux and OS X kernels know exactly what to do and drivers are not required.

On windows, you will need to provide a driver:

  * Download this LUFA CDC-ACM driver:
<https://github.com/abcminiuser/lufa/blob/master/Demos/Device/LowLevel/VirtualSerial/LUFA%20VirtualSerial.inf>
  * Adjust the VID and PID near the end with the values at the start of this section.
  * Next time you get prompted for the driver, include the directory containing the .inf file in the search path and the driver will be installed.

### Device Enumerations
For the UART, serial line settings are 115200 8N1, no flow control.

Once all drivers have been installed correctly:

On windows, devices will appear as a virtual COM port.

On Linux and OS X, devices will appear under `/dev/`.

On OS X:

* XDS backchannel: `tty.usbserial-<serial number>`
* EM in CDC-ACM: `tty.usbmodemf<X><ABC>` (X a letter, ABC a number e.g. `tty.usbmodemfd121`)

On Linux:

* Re-Mote over FTDI: `ttyUSB1`
* Re-Mote over USB driver (in CDC-ACM): `ttyACMn` (n=0, 1, ....)

Software to Program the Nodes
-----------------------------
The Re-Mote can be programmed via the jtag interface or via the serial boot loader on the chip.

* On Windows:
    * Nodes can be programmed with TI's ArmProgConsole or the [SmartRF Flash Programmer 2][smart-rf-flashprog]. The README should be self-explanatory. With ArmProgConsole, upload the file with a `.bin` extension. (jtag + serial)
    * Nodes can also be programmed via the serial boot loader in the cc2538. In `tools/cc2538-bsl/` you can find `cc2538-bsl.py` this is a python script that can download firmware to your node via a serial connection. If you use this option you just need to make sure you have a working version of python installed. You can read the README in the same directory for more info. (serial)

* On Linux:
    * Nodes can be programmed with TI's [UniFlash] tool. With UniFlash, use the file with `.elf` extension. (jtag + serial)
    * Nodes can also be programmed via the serial boot loader in the cc2538. No extra software needs to be installed. (serial)

* On OSX:
    * The `cc2538-bsl.py` script in `tools/cc2538-bsl/` is the only option. No extra software needs to be installed. (serial)

The file with a `.remote` extension is a copy of the `.elf` file.

Use the Port
============
The following examples are intended to work off-the-shelf:

* Examples under `examples/remote`
* MQTT example `examples/cc2538dk/mqtt-demo`
* Border router: `examples/ipv6/rpl-border-router`
* Webserver: `examples/webserver-ipv6`
* CoAP example: `examples/er-rest-example/`

Build your First Examples
-------------------------
It is recommended to start with the `remote-demo`, it is a simple example that walkthroughs the platform features, such as
the built-in sensors, LEDs, user button operation modes (press, release, hold-press), radio (Rime broadcast).

The `Makefile.target` includes the `TARGET=` argument, predefining which is the target platform to compile for, it is automatically included at compilation.  To generate or override an existing one, you can run:

`make TARGET=remote savetarget`

Then you can just run `make` to compile an application, otherwise you will need to do `make TARGET=remote`.

If you want to upload the compiled firmware to a node via the serial boot loader you need first to either manually enable the boot loader, or just let the Co-Processor detect the flash sequence and do it on your behalf, as simple as not pressing anything at all!

Then use `make remote-demo.upload PORT=/dev/ttyUSB1`. 

The `PORT` argument is used to specify in which port the device is connected, as we are currently using a dual-channel FTDI chip, at the moment the programming channel is always assigned to the second FTDI channel, so the FTDI will enumerate as `/dev/ttyUSB0`, `/dev/ttyUSB1`, and we would need to use the later one and specify when flashing using the `cc2538-bsl` script, as it will use the first port found by default.  This will be improved in the next release.

To manually enable the boot loader, press the `reset` button on the board while holding the `user` button. (The boot loader backdoor needs to be enabled on the chip for this to work, see README in the `tools/cc2538-bsl` directory for more info)

To generate an assembly listing of the compiled firmware, run `make remote-demo.lst`. This may be useful for debugging or optimizing your application code. To intersperse the C source code within the assembly listing, you must instruct the compiler to include debugging information by adding `CFLAGS += -g` to the project Makefile and rebuild by running `make clean remote-demo.lst`.

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

### Shutdown Mode
The Re-Mote allows to further reduce power consumption by shutting down entirely all the components but the Co-Processor, powering completely of the CC2538 and CC1120 even preventing quiescent current from being drawn, allowing the platform to awake after a given period governed by the Co-Processor and the built-in battery management IC.  This effectively reduces the power consumption down to 3-4uA.

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
The build system currently uses optimization level `-Os`, which is controlled indirectly through the value of the `SMALL` make variable. This value can be overridden by example makefiles, or it can be changed directly in `platform/remote/Makefile.remote`.

Historically, the `-Os` flag has caused problems with some toolchains. If you are using one of the toolchains documented in this README, you should be able to use it without issues. If for whatever reason you do come across problems, try setting `SMALL=0` or replacing `-Os` with `-O2` in `cpu/cc2538/Makefile.cc2538`.

Doxygen Documentation
=====================
This port's code has been documented with doxygen. To build the documentation, navigate to `$(CONTIKI)/doc` and run `make`. This will build the entire contiki documentation and may take a while.

If you want to build this platform's documentation only and skip the remaining platforms, run this:

    make basedirs="platform/remote core cpu/cc2538 examples/remote examples/cc2538dk"

Once you've built the docs, open `$(CONTIKI)/doc/html/index.html` and enjoy.

Other Versions of this Guide
============================
If you prefer this guide in other formats, use the excellent [pandoc] to convert it.

* **pdf**: `pandoc -s --toc README.md -o README.pdf`
* **html**: `pandoc -s --toc README.md -o README.html`

More Reading
============
1. [Zolertia Re-Mote website][remote-site]
2. [CC2538 System-on-Chip Solution for 2.4-GHz IEEE 802.15.4 and ZigBee&reg;/ZigBee IP&reg; Applications, (SWRU319B)][cc2538]
3. [CC1120 sub-1GHz RF transceiver][cc1120]

[remote-site]: http://www.zolertia.io/products "Zolertia Re-Mote"
[cc1120]: http://www.ti.com/cc1120 "CC1120"
[smart-rf-studio]: http://www.ti.com/tool/smartrftm-studio "SmartRF Studio"
[smart-rf-flashprog]: http://www.ti.com/tool/flash-programmer "SmartRF Flash Programmer"
[cc2538]: http://www.ti.com/product/cc2538     "CC2538"
[uniflash]: http://processors.wiki.ti.com/index.php/Category:CCS_UniFlash "UniFlash"
[pandoc]: http://johnmacfarlane.net/pandoc/ "Pandoc - a universal document converter"

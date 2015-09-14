# NXP JN516x platform

## Overview

The JN516x series is a range of ultra low power, high performance wireless microcontrollers from NXP. They feature an enhanced 32-bit RISC processor (256kB/32kB/4kB Flash/RAM/EEPROM for JN5168), and also include a 2.4GHz IEEE802.15.4-compliant transceiver.
These system on chip (SoC) devices have the following main [features][jn516x-datasheet]:
* 32-bit RISC CPU (Beyond Architecture -- BA), 1 to 32MHz clock speed
* 2.4GHz IEEE802.15.4-compliant transceiver
* 128-bit AES security processor
* MAC accelerator with packet formatting, CRCs, address check, auto-acks, timers
* Transmit power 2.5dBm
* Receiver sensitivity -95dBm
* RX current 17mA, TX 15mA
* Integrated ultra low power sleep oscillator – 0.6μA
* Deep sleep current 0.12μA (Wake-up from IO)
* Time of Flight engine for ranging
* Antenna Diversity (Auto RX)
* 2.0V to 3.6V battery operation
* Supply voltage monitor with 8 programmable thresholds
* Built-in battery and temperature sensors
* Infra-red remote control transmitter
* Peripherals: I2C, SPI, 2x UART, 4-input 10-bit ADC, comparator, 5x PWM

## Maintainers and Contact

Long-term maintainers:
* Theo Van Daele, NXP, theo.van.daele@nxp.com, github user: [TeVeDe](https://github.com/TeVeDe)
* Simon Duquennoy, SICS, simonduq@sics.se, github user: [simonduq](https://github.com/simonduq)

Other contributors:
* Beshr Al Nahas, SICS (now Chalmers University), beshr@chalmers.se, github user: [beshrns](https://github.com/beshrns)
* Atis Elsts, SICS, atis.elsts@sics.se, github user: [atiselsts](https://github.com/atiselsts)

Additional long-term contact:
* Hugh Maaskant, NXP, hugh.maaskant@nxp.com, github user: [hugh-maaskant](https://github.com/hugh-maaskant)

## License

All files are under BSD license, as described by the copyright statement in every source file.

## Port Features

The following features have been implemented:
  * A radio driver with two modes (polling and interrupt based)
  * CCM* driver with HW accelerated AES
  * UART driver (with HW and SW flow control, 1'000'000 baudrate by default)
  * Contiki system clock and rtimers (16MHz tick frequency based on 32 MHz crystal)
  * 32.768kHz external oscillator
  * Periodic DCO recalibration
  * CPU "doze" mode
  * HW random number generator used as a random seed for pseudo-random generator
  * Watchdog, JN516x HW exception handlers

The following hardware platforms have been tested:
  * DR1174 evaluation board (with a button sensor)
  * DR1175 sensor board (with humidity/temperature and light sensors)
  * DR1199 sensor board (with potentiometer and button sensors)
  * USB dongle

## TODO list

The following features are planned:
  * CPU deeper sleep mode support (where the 32 MHz crystal is turned off)
  * Time-accurate radio primitives ("send at", "listen until")
  * External storage

## Requirements

To start using JN516x with Contiki, the following are required:
 * The toolchain and Software Development Kit to compile Contiki for JN516x
 * Drivers so that your OS can communicate with your hardware
 * Software to upload images to the JN516x

### Install a Toolchain

The toolchain used to build Contiki for JN516x is `ba-elf-gcc`.
The compiler as well as the binary libraries required to link the executables can be downloaded from NXP. To express your interest in obtaining them, go to [NXP 802.15.4 software page][NXP-802.15.4-software], select "JN-SW-4163", and contact the NXP support through the web form. The download link is then obtained via e-mail (allow 1+ working day for a reply).
The example applications in this port have been tested with compiler version `gcc-4.7.4-ba-r36379`.

Linux and Windows instructions:
* On Linux: A compiled version for linux 64-bit is available: download [this](http://simonduq.github.io/resources/ba-elf-gcc-4.7.4-part1.tar.bz2) and [this](http://simonduq.github.io/resources/ba-elf-gcc-4.7.4-part2.tar.bz2) file, extract both in `/usr/ba-elf-gcc` such and add `/usr/ba-elf-gcc/bin` to your `$PATH` environment variable. Place the JN516x SDK under `/usr/jn516x-sdk`.
* On Windows: Run the setup program and select `C:/NXP/bstudio_nxp/` as install directory. Also make sure your PATH environment variable points to the compiler binaries (by default in `C:/NXP/bstudio_nxp/sdk/Tools/ba-elf-ba2-r36379/bin`).

### Drivers

The JN516x platforms feature FTDI serial-to-USB modules. The driver is commonly found in most OS, but if required it can be downloaded from <http://www.ftdichip.com/Drivers/VCP.htm>
 
### Device Enumerations
For the UART, serial line settings are 1000000 8N1, no flow control.

Once all drivers have been installed correctly:
* On Windows, devices will appear as a virtual COM port.
* On Linux and OS X, devices will appear as `/dev/tty*`.

### Software to Program the Nodes

The JN516x can be programmed via the serial boot loader on the chip.
* On Linux, nodes can be programmed via the serial boot loader using the [JennicModuleProgrammer] tool. It is pre-installed under `tools/jn516x/JennicModuleProgrammer`.
* On Windows, nodes are programmed using NXP's Flash Programmer. There are two versions of it: GUI and command line. The Contiki make system is configured to use the command line version. By default it looks for the programmer in the SDK base directory under `Tools/flashprogrammer/FlashCLI.exe`. With the default SDK installation path the file should be located under `C:/NXP/bstudio_nxp/sdk/JN-SW-4163/Tools/flashprogrammer/FlashCLI.exe`. Modify `platforms/jn516x/Makefile.common` to change this default location.

## Using the Port

The following examples are intended to work off-the-shelf:
* Platform-specific examples under `examples/jn516x`
* All platform-independent Contiki application examples

### Building an example

To build the classic "hello world" example, navigate to `examples/hello-world`. It is recommended to either set the `TARGET` environmental variable or to save the `jn516x` platform as the default make target. To do that, run:

`make TARGET=jn516x savetarget`

Then run `make hello-world` to compile the application for JN516x platform.

### Uploading an example

Run the `upload` command to program the binary image on it:
`make hello-world.upload MOTE=0`

The `MOTE` argument is used to specify to which of the ports the device is connected. For example, if there is a single mote connected to `/dev/ttyUSB3` in Linux (or, alternatively, `COM3` in Windows), the right command would be:
`make hello-world.upload MOTE=3`

Note that on Windows, the FTDI drivers are able to switch the board to programming mode before uploading the image.

On Linux, the drivers are not able to do so yet. We use a modified bootloader for JN516x, where nodes wait 5s in programming mode after a reset. You simply need to reset them before using `make upload`. The modified bootloader can be downloaded [here](http://simonduq.github.io/resources/BootLoader_JN5168.ba2.bin) and installed using a JTAG programmer, or alternatively, [this image](http://simonduq.github.io/resources/BootLoaderUpdater_JN5168.bin) can be installed as a normal application using the normal Windows tools.  Once the device resets, this application will run and will then install the new boot loader. It generates some status output over UART0 at 115200 baud during this process. **Warning**: use the images above at your risk; NXP does not accept responsibility for any devices that are rendered unusable as a result of using it.

### Listening to output

Run the `login` command to start the `serialdump` application.
`make login MOTE=3`

On Linux: after the application has started, press the reset button on the node.

### Platform-specific make targets

* `<application>.flash` - flash the (pre-compiled) application to a JN516x mote (specified via the `MOTE` variable)
* `<application>.flashall` - flash the (pre-compiled) application to all all connected JN516x motes
* `<application>.upload` - compile and flash the application to a JN516x mote (specified via the `MOTE` variable)
* `<application>.uploadall` - compile and flash the application to all all connected JN516x motes
* `login`, `serialview`, `serialdump` - dump serial port output from a JN516x mote (specified via the `MOTE` variable)
* `serialdumpall` - dump serial port output from all connected JN516x motes
* `motelist` - list all connected JN516x motes.
* `motelistmac` - list MAC addresses of all connected JN516x motes (Note: not implemented on Linux!)
* `motelistinfo` - list info about all connected JN516x motes (Note: very limited functionality on Linux!)

*Troubleshooting:* you need a working Python installation for these commands to work. On Windows, make sure Python executable is in your `PATH`.

### Compiling for different MCUs and boards

The platforms can selected by using `Makefile` variables.

The following MCU models are supported:
* `JN5164` - 160kB/32kB/4kB Flash/RAM/EEPROM
* `JN5168` - 256kB/32kB/4kB Flash/RAM/EEPROM (default MCU)
* `JN5169` - 512kB/32kB/4kB Flash/RAM/EEPROM

Set `CHIP` variable to change this; for example, to select JN5164 use:
`make CHIP=JN5164`

The following platform-specific configurations are supported:
* DR1174 evaluation kit; enable this with `JN516x_WITH_DR1174 = 1` in your makefile
* DR1174 with DR1175 sensor board; enable this with `JN516x_WITH_DR1175 = 1` (will set `JN516x_WITH_DR1174` automatically)
* DR1174 with DR1199 sensor board; enable this with `JN516x_WITH_DR1199 = 1` (will set `JN516x_WITH_DR1174` automatically)
* USB dongle; enable this with `JN516x_WITH_DONGLE = 1`

### Node IEEE/RIME/IPv6 Addresses

Nodes will autoconfigure their IPv6 address based on their 64-bit IEEE/MAC address. The 64-bit MAC address is read directly from JN516x System on Chip.
The 16-bit RIME address and the Node ID are set from the last 16-bits of the 64-bit MAC address.

## Additional documentation

1. [Data Sheet: JN516x IEEE802.15.4 Wireless Microcontroller][jn516x-datasheet]
2. [JN516x web page][jn516x-web]
3. [JN5168 web page][jn5168-web]
4. [JN516x user manuals][user-manuals]

[jn516x-datasheet]: http://www.nxp.com/documents/data_sheet/JN516X.pdf
[jn516x-web]: http://www.nxp.com/products/microcontrollers/product_series/jn516x
[jn5168-web]: http://www.nxp.com/products/microcontrollers/product_series/jn516x/JN5168.html
[user-manuals]: http://www.nxp.com/technical-support-portal/#/tid=1,sid=,bt=,tab=usermanuals,p=1,rpp=,sc=,so=,jump=
[NXP-802.15.4-software]: http://www.nxp.com/techzones/wireless-connectivity/ieee802-15-4.html
[JennicModuleProgrammer]: https://github.com/WRTIOT/JennicModuleProgrammer

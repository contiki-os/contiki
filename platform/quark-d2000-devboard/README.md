Intel Quark D2000 Development Platform
======================================

This README file contains information about Contiki port for Intel Quark
D2000 Development Platform [1].

Hardware Information
--------------------

This section present some features about the Intel Quark D2000 platform.
More detailed information can be found in [2] as well as documents such
as datasheets and schematics.

Processor Core
* 32 MHz clock frequency
* Pentium 586 ISA Compatible without x87 Floating Point Unit
* Integrated Local APIC and I/O APIC

Memory
* 8 KB SRAM
* 32 KB flash
* 8 KB Code OTP
* 4 KB Data OTP

I/O Peripherals
* 1 I2C interface
* 2 UART interfaces
* 1 SPI master interface
* 25 GPIO
* 2 PWM channels
* Analog to digital convertor (ADC)
* Analog comparators

Features
--------

This section presents the current features (e.g. device drivers and Contiki
API) supported by the Quark D2000 Devboard port.

This port is based on the Intel Quark Microcontroller Software Interface (QMSI)
[3] which is a hardware abstraction layer for Intel Quark Microcontroller
products, including Quark D2000. QMSI BSP provides several components that are
reused by this port such as bootstrap code, linker script, interrupt system and
device drivers.

QMSI BSP provides device drivers for almost all I/O peripheral from the Quark
D2000 SoC. Contiki applications can access these devices through QMSI APIs
(see [4]). QMSI BSP also provides some example applications to show how to use
the APIs [5].

Some peripherals are used by the Contiki OS itself. User applications must not
use these peripherals otherwise the Contiki OS might not work properly. Below
follows the devices already used by the OS:
* UART A
* PIC timer
* Real-Time Clock (RTC)
* Watchdog

Building
--------

Before building applications, make sure the BSP components (toolchain and QMSI)
are properly installed and built.

To download and install the toolchain run the following command. The toolchain
is installed in platform/quark-d2000-devboard/bsp/ directory. The download can
take a while.
```
$ ./platform/quark-d2000-devboard/bsp/toolchain/install_toolchain.sh
```

If the toolchain is installed successfully, you should see the output:
```
IAMCU toolchain installed successfully.
```

To download and build QMSI, run the command:
```
$ ./platform/quark-d2000-devboard/bsp/qmsi/build_qmsi.sh
```

If QMSI is built successfully, you should see the output:
```
QMSI built successfully.
```

Once the BSP components are properly installed and built, we are ready to build
Contiki applications. Make sure the toolchain binaries are in your PATH:
```
export PATH=<CONTIKI ROOT DIR>/platform/quark-d2000-devboard/bsp/toolchain/iamcu_toolchain_Linux_issm_2016.0.019/tools/compiler/bin:$PATH
```

In order to build application for this port you should set TARGET variable to
'quark-d2000-devboard'. For instance, to build the hello world application run
the command:
```
$ make TARGET=quark-d2000-devboard -C examples/hello-world/
```

By default, our build system generates applications for development purposes
(e.g. non-stripped elf image, debugging information is provided and no code
optimization is performed). To generate optimized applications you should set
'BUILD_RELEASE=1' in build command line. For instance, to build hello world
application in 'release' mode, run the command.
```
$ make TARGET=quark-d2000-devboard -C examples/hello-world/ BUILD_RELEASE=1
```

Flashing
--------

Flashing Contiki applications on Quark D2000 Devboard is quite simple. Our
build system provides the 'flash' rule which runs _openocd_ with the right
parameters. This rule will also build the application in case it has not
been built yet.

For instance, to flash the hello-world application run the following command:
```
$ make flash TARGET=quark-d2000-devboard -C examples/hello-world/
```

Keep in mind this port is based on the default ROM firmware provided by QMSI
BSP so Contiki applications might not work properly if the ROM firmware is
customized. If you are flashing a Contiki application for the first time, you
should flash the ROM firmware to ensure you have the right firmware running on
your Quark D2000 Devboard. To flash the ROM firmware use the 'flash-firmware'
rule within the application directory.
```
$ make flash-firmware TARGET=quark-d2000-devboard -C examples/hello-world/
```

Console Output
--------------

The port uses the UART_A device as console output, configured with 115200
baud rate. You can you your favorite serial communication program to read
the console output. Here follows an example using picocom.
```
$ picocom -b 115200 --imap lfcrlf /dev/ttyUSB0
```

References
----------

[1] http://www.intel.com/content/www/us/en/embedded/products/quark/mcu/d2000/overview.html

[2] http://www.intel.com/content/www/us/en/embedded/products/quark/mcu/d2000/documentation.html

[3] https://github.com/01org/qmsi

[4] https://github.com/01org/qmsi/tree/master/drivers/include

[5] https://github.com/01org/qmsi/tree/master/examples

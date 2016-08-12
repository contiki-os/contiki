Contiki for nRF52 Development Kit
=================================
This guide's aim is to help you with using Contiki for 
Nordic Semiconductor's nRF52 DK. 

The port depends on Nordic Semiconductor IoT SDK for nRF52. 
The IoT SDK contains source code and libraries which are 
required for successfull port compilation. It also contains 
SoftDevice binary driver which is required for BLE operation. 
See prerequisites section for details on how to set up the SDK. 

For more information about SoftDevice please refer to the SDK 
docummentation [nRF52 Datasheet and SDK documentation].

This port supports DK versions PCA10040 and PCA10036.

Port Features
=============
The following features have been implemented:
* Support for IPv6 over BLE using Contiki 6LoWPAN implementation
* Contiki system clock and rtimers (using 32kHz and 1MHz timers)
* UART driver
* Watchdog driver
* Hardware RNG
* Temperature sensor driver
* DK LED driver
* DK Buttons driver
* Real Time Transfer (RTT) I/O support

Note that this port supports only IPv6 network stack.

The port is organized as follows:
* nRF52832 CPU and BLE drivers are located in `cpu/nrf52832` folder
* nRF52 Development Kit drivers are located in `platform/nrf52dk` folder
* Platform examples are located in `examples/nrf52dk` folder 

Prerequisites and Setup
=======================
In order to compile for the nRF52 DK platform you'll need:

* nRF5 IOT SDK
  https://developer.nordicsemi.com
  
  Download nRF5 IOT SDK, extract it to a folder of your choice,
  and point `NRF52_SDK_ROOT` environmental variable to it, e.g.,:
    
    ```
    wget https://developer.nordicsemi.com/nRF5_IoT_SDK/nRF5_IoT_SDK_v0.9.x/nrf5_iot_sdk_3288530.zip
    unzip nrf5_iot_sdk_3288530.zip -d /path/to/sdk
    export NRF52_SDK_ROOT=/path/to/sdk
    ```
    
* An ARM compatible toolchain
  The port has been tested with GNU Tools for ARM Embedded Processors 
  version 5.2.1. 
  
  For Ubuntu you can use package version provided by your distribution:
    ```
    sudo apt-get install gcc-arm-none-eabi
    ```

  Alternatively, install the toolchain from PPA to get the latest version
  of the compiler: https://launchpad.net/~team-gcc-arm-embedded/+archive/ubuntu/ppa
  
  For other systems please download and install toolchain available at 
  https://launchpad.net/gcc-arm-embedded

* GNU make

* Segger JLink Software for Linux
  https://www.segger.com/jlink-software.html

  This package contains tools necessary for programming and debugging nRF52 DK. 

  For Ubuntu you can download and install a .deb package.  Alternatively download 
  tar.gz archive and extract it to a folder of your choice. In this case you 
  need to set `NRF52_JLINK_PATH` environmental variable to point to the
  JLink tools location:
  
    ```
    export NRF52_JLINK_PATH=/path/to/jlink/tools
    ```
    
  To keep this variable set between sessions please add the above line to your
  `rc.local` file. 
  
  In order to access the DK as a regular Linux user create a `99-jlink.rules`
  file in your udev rules folder (e.g., `/etc/udev/rules.d/`) and add the 
  following line to it:
    
    ```
    ATTRS{idProduct}=="1015", ATTRS{idVendor}=="1366", MODE="0666"
    ```
  When installing from a deb package, the `99-jlink.rules` file is added
  automatically to /etc/udev/rules.d folder. However, the syntax of the file
  doesn't work on newer udev versions. To fix this problem edit this file and
  replace ATTR keyword with ATTRS.

To fully use the platform a BLE enabled router device is needed. Please refer
to `Preqrequisites` section in `README-BLE-6LoWPAN.md` for details.

Getting Started
===============
Once all tools are installed it is recommended to start by compiling 
and flashing `examples/hello-word` application. This allows to verify 
that toolchain setup is correct.

To compile the example, go to `examples/hello-world` and execute:

    make TARGET=nrf52dk

If you haven't used the device with Contiki before we advise to
erase the device and flash new SoftDevice:

    make TARGET=nrf52dk erase
    make TARGET=nrf52dk softdevice.flash

If the compilation is completed without errors flash the board:

    make TARGET=nrf52dk hello-world.flash

The device will start BLE advertising as soon as initialized. By
default the device name is set to 'Contiki nRF52 DK'. To verify
that the device is advertising properly run:

    sudo hcitool lescan

And observe if the device name appears in the output. Also, observe
if LED1 is blinking what indicates that device is waiting for a connection
from BLE master.

If device is functioning as expected you can test IPv6 connection 
to the device. Please refer to `README-BLE-6LoWPAN.md` on details how to do
this.

Examples
========
Examples specific for nRF52 DK can be found in `examples/nrf52dk` folder. Please 
refer to README.md in respective examples for detailed description.

The DK has also been tested with the `examples/hello-world` and `examples/webserver-ipv6`
generic examples. 

Compilation Options
===================
The Contiki TARGET name for this port is `nrf52dk`, so in order to compile 
an application you need to invoke GNU make as follows:

    make TARGET=nrf52dk

In addition to this port supports the following variables which can be
set on the compilation command line:

* `NRF52_SDK_ROOT=<SDK PATH>`
  This variable allows to specify a path to the nRF52 SDK which should 
  be used for the build.
  
* `NRF52_WITHOUT_SOFTDEVICE={0|1}`
  Disables SoftDevice support if set to 1. By default, SoftDevice support
  is used. Note that SoftDevice must be present (flashed) in the device
  before you run an application that requires it's presence. 
  
* `NRF52_USE_RTT={0|1}`
  Enables RealTime Terminal I/O. See VCOM and RTT for details. By default,
  RTT is disabled and IO is done using Virtual COM port.

* `NRF52_JLINK_SN=<serial number>`
  Allows to choose a particular DK by its serial number (printed on the
  label). This is useful if you have more than one DK connected to your
  PC and whish to flash a particular device. 

* `NRF52_DK_REVISION={pca10040|pca10036}`
  Allows to specify DK revision. By default, pca10040 is used.

Compilation Targets
===================
Invoking make solely with the `TARGET` variable set will build all
applications in a given folder. A particular application can be built
by invoking make with its name as a compilation target:

    make TARGET=nrf52dk hello-world 

In order to flash the application binary to the device use `<application>.flash`
as make target, e.g.: 

    make TARGET=nrf52dk hello-world.flash

In addition, the SoftDevice binary can be flashed to the DK by invoking:

    make TARGET=nrf52dk softdevice.flash

To remove all build results invoke:

    make TARGET=nrf52dk clean

The device memory can be erased using:

    make TARGET=nrf52dk erase

Note, that once the device is erased, the SoftDevice must be programmed again.

Virtual COM and Real Time Transfer
==================================
By default, the nRF52 DK uses a Virtual COM port to output logs. Once
the DK is plugged in a `/tty/ACM<n>` or `/ttyUSB<n>` device should appear in
your filesystem. A terminal emulator, such as picocom or minicom, can be 
used to connect to the device. Default serial port speed is 38400 bps. 

To connect to serial port using picocom invoke:

    picocom -fh -b 38400 --imap lfcrlf /dev/ttyACM0 

Note, that if you have not fixed file permissions for `/dev/ttyACM0` 
according to section `Segger JLink Software for Linux` you'll need to use
root or sudo to open the port with `picocom`.

In addition to Virtual COM the port supports SEGGER's Real Time Transfer
for low overhead I/O support. This allows for outputting debugging information
at much higher rate with significantly lower overhead than regular I/O.

To compile an application with RTT rather that VCOM set `NRF52_USE_RTT` to 1 on
the compilation command line:

    make TARGET=nrf52dk NRF52_USE_RTT=1 hello-world

You can then connect to the device terminal using `JLinkRTTClient`. Note that
a JLlink gdb or commander must be connected to the target for the RTT to work.

More details regarding RTT can be found at https://www.segger.com/jlink-rtt.html

Docummentation
==============
This port provides doxygen source code docummentation. To build the 
docummentation please run:

    sudo apt-get install doxygen
    cd <CONTIKI_ROOT>\doc
    make

Support
=======
This port is officially supported by Nordic Semiconductor. Please send bug 
reports or/and suggestions to <wojciech.bober@nordicsemi.no>.

License
=======
All files in the port are under BSD license. nRF52 SDK and SoftDevice are
licensed on a separate terms.

Resources
=========
* nRF52 Datasheet and SDK documentation (http://infocenter.nordicsemi.com)
* nRF52 SDK Downloads (https://developer.nordicsemi.com/)
* JLink Tools (https://www.segger.com/)
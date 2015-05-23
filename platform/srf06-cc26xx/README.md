Getting Started with Contiki for TI CC26xx
==========================================

This guide's aim is to help you start using Contiki for TI's CC26xx. The
platform supports two different boards:

* SmartRF 06 Evaluation Board with a CC26xx Evaluation Module (relevant files
  and drivers are under `srf06/`)
* CC26xx SensorTag 2.0 (relevant drivers under `sensortag/`)

The CPU code, common for both platforms, can be found under `$(CONTIKI)/cpu/cc26xx`.
The port was developed and tested with CC2650s, but the intention is for it to
work with the CC2630 as well. Thus, bug reports are welcome for both chips.
Bear in mind that the CC2630 does not have BLE capability.

This port is only meant to work with 7x7mm chips

This guide assumes that you have basic understanding of how to use the command
line and perform basic admin tasks on UNIX family OSs.

Port Features
=============
The platform has the following key features:

* Deep Sleep support with RAM retention for ultra-low energy consumption.
* Support for CC26xx RF in IEEE as well as BLE mode (BLE support is very basic
  since Contiki does not provide a BLE stack).

In terms of hardware support, the following drivers have been implemented:

* SmartRF06 EB peripherals
  * LEDs
  * Buttons
  * UART connectivity over the XDS100v3 backchannel
* SensorTag 2.0
  * LEDs
  * Buttons (One of the buttons can be used as a shutdown button)
  * Reed relay
  * Motion Processing Unit (MPU9250 - Accelerometer, Gyro)
  * BMP280 sensor
  * TMP007 sensor
  * HDC1000 sensor
  * OPT3001 sensor
  * Buzzer
  * External SPI flash

Examples
========
The port comes with two examples: A very basic example and a mode advanced one
(web demo). The former demonstrates how to read sensors and how to use board
peripherals. It also demonstrates how to send out BLE advertisements.
The latter includes a CoAP server, an MQTT client which connects and publishes
to the IBM quickstart service, a net-based UART and lastly a web server that
can be used to configure the rest of the example.

More details about those two examples can be found in their respective READMEs.

Requirements
============
To use the port you need:

* TI's CC26xxware sources. The correct version will be installed automatically
  as a submodule when you clone Contiki.
* Software to program the nodes. Use TI's SmartRF Flash Programmer
* A toolchain to build firmware: The port has been developed and tested with
  GNU Tools for ARM Embedded Processors <https://launchpad.net/gcc-arm-embedded>.
  The port was developed and tested using this version:

    $ arm-none-eabi-gcc -v
    [...]
    gcc version 4.9.3 20141119 (release) [ARM/embedded-4_9-branch revision 218278] (GNU Tools for ARM Embedded Processors)

* srecord (http://srecord.sourceforge.net/)
* You may also need other drivers so that the SmartRF can communicate with your
operating system and so that you can use the chip's UART for I/O. Please read
the section ["Drivers" in the CC2538DK readme](https://github.com/contiki-os/contiki/tree/master/platform/cc2538dk#drivers).

Filename conflicts between Contiki and CC26xxware
=================================================
There is a file called `timer.c` both in Contiki as well as in CC26xxware. The
way things are configured now, we don't use the latter. However, if you need to
start using it at some point, you will need to rename it:

From `cpu/cc26xx/lib/cc26xxware/driverlib/timer.c` to `driverlib-timer.c`

Sensortag vs Srf06
==================
To build for the sensortag, set `BOARD=sensortag`. You can do that by exporting
it as an environment variable, by adding it to your Makefile or by adding it to
your make command as an argument

If the `BOARD` variable is not equal to `sensortag`, an image for the Srf06
CC26XXEM will be built instead.

If you want to switch between building for one platform to the other, make
certain to `make clean` before building for the new one, or you will get linker
errors.

Low Power Operation
===================
The platform takes advantage of the CC26xx's power saving features. In a
nutshell, here is how things work:

* When the RF is TXing, the CPU will enter sleep mode and will resume after TX
  has completed.
* When there are no events in the Contiki event queue, the chip will enter
  'some' low power mode (more below).

We do not use pre-defined power profiles (e.g. as mentioned in the TRM or as
we do for the CC2538 with LPM1, LPM2 etc). Each time we enter low power
operation, we either put the CM3 to sleep or to deep sleep. The latter case is
highly configurable: the LPM engine allows other code modules to register
themselves for notifications and to configure low power operation. With these
facilities, a module can e.g. prohibit deep sleep altogether, or it can request
that a power domain be kept powered. The LPM engine will turn off as many
CC26xx components as it can while satisfying all restrictions set by registered
modules.

To determine which power mode to use, the following logic is followed:

* The deepest available low power mode can be hard-coded by using
  the `LPM_MODE_MAX_SUPPORTED` macro in the LPM driver (`lpm.[ch]`). Thus, it
  is possible to prohibit deep sleep altogether.
* Code modules which are affected by low power operation can 'register'
  themselves with the LPM driver.
* If the projected low-power duration is lower than `STANDBY_MIN_DURATION`,
  the chip will simply sleep.
* If the projected low power duration is sufficiently long, the LPM will visit
  all registered modules to query the maximum allowed power mode (maximum means
  sleep vs deep sleep in this context). It will then drop to this power mode.
  This is where a code module can forbid deep sleep if required.
* All registered modules will be notified when the chip is about to enter
  deep sleep, as well as after wake-up.

When the chip does enter deep sleep:

* The RF Core, VIMS, SYSBUS and CPU power domains are always turned off. Due to
  the way the RF driver works, the RFCORE PD should be off already.
* Peripheral clocks stop
* The Serial and Peripheral power domains are turned off, unless an LPM module
  requests them to stay operational. For example, the net-uart demo keeps the
  serial power domain powered on and the UART clocked under sleep and deep
  sleep in order to retain UART RX functionality.
* If both SERIAL and PERIPH PDs are turned off, we also switch power source to
  the uLDO for ultra low leakage under deep sleep.

The chip will come out of low power mode by one of the following events:

* Button press or, in the case of the SensorTag, a reed relay trigger
* Software clock tick (timer). The clock ticks at 128Hz, therefore the maximum
  time we will ever spend in a sleep mode is 7.8125ms. In hardware terms, this
  is an AON RTC Channel 2 compare interrupt.
* Rtimer triggers, as part of ContikiMAC's sleep/wake-up cycles. The rtimer
  sits on the AON RTC channel 0.

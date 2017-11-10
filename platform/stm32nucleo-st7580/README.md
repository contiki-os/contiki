Getting Started with Contiki for STM32 Nucleo equipped with ST7580 FSK, PSK multi-mode power 
line networking system-on-chip expansion boards
=============================================================================================

This guide explains how to get started with the STM32 Nucleo and expansion boards port to Contiki.

Maintainers and Contacts
========================

Long-term maintainers:
* Stefano Bosisio, stefano.bosisio@st.com, github user: [stebosisio](https://github.com/stebosisio)
* Giorgio Balbusso, giorgio.balbusso@st.com, github user: [giorgiobalbussost](https://github.com/giorgiobalbussost)

Contributors:
* Stefano Bosisio, stefano.bosisio@st.com, github user: [stebosisio](https://github.com/stebosisio)
* Giorgio Balbusso, giorgio.balbusso@st.com, github user: [giorgiobalbussost](https://github.com/giorgiobalbussost)

Port Feature
============

The port supports the following boards from ST:
-   NUCLEO-L152RE board, based on the STM32L152RET6 ultra-low power microcontroller
-   X-NUCLEO-PLM01A1 based on ST7580 FSK, PSK multi-mode power line networking system-on-chip

The following drivers are included:
- LEDs and user button
- USB
- ST7580 FSK, PSK multi-mode power line networking system-on-chip

Hardware Requirements
=====================

* NUCLEO-L152RE development board

 >The NUCLEO-L152RE board belongs to the STM32 Nucleo family.
It features an STM32L152RET6 ultra-low power microcontroller based on ARM Cortex M3 MCU.
For detailed information on the NUCLEO-L152RE development board, please go to http://www.st.com and search for the "NUCLEO-L152RE" part number.


* X-NUCLEO-PLM01A1 Power line communication expansion board 

 >For detailed information on the X-NUCLEO-PLM01A1 expansion board, or the ST7580 FSK, PSK multi-mode power line networking system-on-chip please go to http://www.st.com and search for the "X-NUCLEO-PLM01A1" part number.
 
* USB type A to Mini-B USB cable, to connect the STM32 Nucleo board to the PC

Software Requirements
=====================

The following software are needed:

* ST port of Contiki for STM32 Nucleo and expansion boards. 
 
 >The port is automatically installed when both the Contiki and the submodule repository are cloned: the former hosts the Contiki distribution and the ST platform interface, the latter hosts the actual library. The following command downloads the full porting: 

	git clone --recursive https://github.com/contiki-os/contiki

Alternatively, if you cloned the contiki repository without the "--recursive" option, you can simply download the submodule repository with the following commands:

	cd contiki/
	git submodule init
	git submodule update

The platform name is: stm32nucleo-st7580

* A toolchain to build the firmware: The port has been developed and tested with GNU Tools 
for ARM Embedded Processors.
 >The toolchain can be found at: https://launchpad.net/gcc-arm-embedded


Examples
========

The following examples have been successfully tested:

* examples/ipv6 (rpl-border-router, rpl-udp)


Build an example
================
In order to build an example go to the selected example directory (see a list of tested
examples in the previous section).

For example, go to examples/ipv6/rpl-border-router directory.

The following must be run:

	make TARGET=stm32nucleo-st7580 clean
	make TARGET=stm32nucleo-st7580

This will create executables for rpl-border-router node.

In order to generate binary file that can be flashed on the STM32 Nucleo the following command must be run:

	arm-none-eabi-objcopy -O binary border-router.stm32nucleo-st7580 border_rouer.bin

This executable can be programmed on the node using the procedure described hereafter.

System setup
============ 

1. Connect the X-NUCLEO-PLM01A1 board to the STM32 Nucleo board (NUCLEO-L152RE) from the top.

2. Power the STM32 Nucleo board using the Mini-B USB cable connected to the PC.

3. Program the firmware on the STM32 Nucleo board. 
This can be done by copying the binary file on the USB mass storage that is 
automatically created when plugging the STM32 Nucleo board to the PC.

4. Reset the MCU by using the reset button on the STM32 Nucleo board


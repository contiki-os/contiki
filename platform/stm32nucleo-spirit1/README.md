Getting Started with Contiki for STM32 Nucleo equipped with sub-1GHz SPIRIT1 expansion boards
=============================================================================================

This guide explains how to get started with the STM32 Nucleo and expansion boards port to Contiki.

Maintainers and Contacts
========================

Long-term maintainers:
* Marco Grella, marco.grella@st.com, github user: [STclab](https://github.com/STclab)
* Alok Mittal, alok.mittal@st.com, github user: [STclab](https://github.com/STclab)
* Indar Prakash Singhal, indar.singhal@st.com, github user: [STclab](https://github.com/STclab)

Contributors:
* David Siorpaes, david.siorpaes@st.com, github user: [siorpaes](https://github.com/siorpaes)
* Luca Celetto, luca.celetto@st.com

Port Feature
============

The port supports the following boards from ST:
-   NUCLEO-L152RE board, based on the STM32L152RET6 ultra-low power microcontroller
-	X-NUCLEO-IDS01A4 based on sub-1GHz SPSGRF-868 SPIRIT1 module (operating at 868 MHz)
-	X-NUCLEO-IDS01A5 based on sub-1GHz SPSGRF-915 SPIRIT1 module (operating at 915 MHz)
-   X-NUCLEO-IKS01A1 featuring motion MEMS and environmental sensors (optional)

The following drivers are included:
- LEDs and buttons (user, reset)
- USB
- SPIRIT1 sub-1GHz transceiver  
- HTS221, LIS3MDL, LPS25HB, LSM6DS0 sensors


Hardware Requirements
=====================

* NUCLEO-L152RE development board

 >The NUCLEO-L152RE board belongs to the STM32 Nucleo family.
It features an STM32L152RET6 ultra-low power microcontroller based on ARM Cortex M3 MCU.
Detailed information on the NUCLEO-L152RE development board can be found at:
http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/LN1847/PF260002


* X-NUCLEO-IDS01Ax sub-1GHz expansion board

 >The X-NUCLEO-IDS01A4 and X-NUCLEO-IDS01A5 are STM32 Nucleo expansion boards that use 
the module SPSGRF-868 or SPSGRF-915 based on SPIRIT1 low data rate, low power sub-1 GHz transceiver.

 >The user can select the X-NUCLEO-IDS01A4 board to operate the SPIRIT1 transceiver at 868MHz or the X-NUCLEO-IDS01A5 board to operate the SPIRIT1 transceiver at 915MHz.

 >Detailed information on the X-NUCLEO-IDS01A4 expansion board can be found at:
http://www.st.com/web/catalog/tools/FM146/CL2167/SC2006/PF261982

 >Detailed information on the X-NUCLEO-IDS01A5 expansion board can be found at:
http://www.st.com/web/catalog/tools/FM146/CL2167/SC2006/PF261983 

 >Detailed information on the SPIRIT1 sub-1GHz transceiver can be found at:
http://www.st.com/web/catalog/sense_power/FM2185/SC1845/PF253167

* X-NUCLEO-IKS01A1, motion MEMS and environmental sensors expansion board (OPTIONAL)

 >The X-NUCLEO-IKS01A1 is a motion MEMS and environmental sensor evaluation board.
The use of this board is optional in the stm32nucleo-spirit1 Contiki platform. 

 >Detailed information on the X-NUCLEO-IKS01A1 expansion board can be found at:
http://www.st.com/web/catalog/tools/FM146/CL2167/SC2006/PF261191


* USB type A to Mini-B USB cable, to connect the STM32 Nucleo board to the PC

Software Requirements
=====================

The following software are needed:

* ST port of Contiki for STM32 Nucleo and expansion boards. 
 
 >The port is automatically installed when both the Contiki and the submodule repository are cloned: the former hosts the Contiki distribution and the ST platform interface, the latter hosts the actual library. The following commands are needed to download the full porting: 

	git clone https://github.com/STclab/contiki.git
	cd contiki/
	git checkout stm32nucleo-spirit1
	git submodule init
	git submodule update


Note: the first and third steps are required only if using the STclab GitHub repository, they won't be needed any more once the Pull Request is accepted.

The platform name is: stm32nucleo-spirit1

* A toolchain to build the firmware: The port has been developed and tested with GNU Tools 
for ARM Embedded Processors.
 >The toolchain can be found at: https://launchpad.net/gcc-arm-embedded
The port was developed and tested using this version: gcc-arm-none-eabi v4.8.3


Examples
========

The following examples have been successfully tested:

* examples/stm32nucleo-spirit1/sensor-demo
* examples/ipv6/simple-udp-rpl (multicast, rpl-border-router, simple-udp-rpl)


Build an example
================
In order to build an example go to the selected example directory (see a list of tested
examples in the previous section).

For example, go to examples/ipv6/simple-udp-rpl directory.

	
If the X-NUCLEO-IDS01A4 sub-1GHz RF expansion board is used, the following must be run:

	make TARGET=stm32nucleo-spirit1 BOARD=ids01a4 clean
	make TARGET=stm32nucleo-spirit1 BOARD=ids01a4

If the X-NUCLEO-IDS01A5 sub-1GHz RF expansion board is used, the following must be run:

	make TARGET=stm32nucleo-spirit1 BOARD=ids01a5 clean
	make TARGET=stm32nucleo-spirit1 BOARD=ids01a5
	
	
This will create executables for UDP sender and receiver nodes.

In order to generate binary files that can be flashed on the STM32 Nucleo the following command must be run:

	arm-none-eabi-objcopy -O binary unicast-sender.stm32nucleo-spirit1 unicast-sender.bin
	arm-none-eabi-objcopy -O binary unicast-receiver.stm32nucleo-spirit1 unicast-receiver.bin

These executables can be programmed on the nodes using the procedure described hereafter.


In case you need to build an example that uses the additional sensors expansion board 
(for example, considering a system made of NUCLEO-L152RE, X-NUCLEO-IDS01A4 and X-NUCLEO-IKS01A1)
then the command to be run would be:

	make TARGET=stm32nucleo-spirit1 BOARD=ids01a4 SENSORBOARD=iks01a1

System setup
============ 

1. Check that the jumper on the J1 connector on the X-NUCLEO-IDS01Ax expansion board is connected. 
This jumper provides the required voltage to the devices on the board.

2. Connect the X-NUCLEO-IDS01Ax board to the STM32 Nucleo board (NUCLEO-L152RE) from the top.

3. If the optional X-NUCLEO-IKS01A1 board is used, connect it on top of the X-NUCLEO-IDS01Ax board.

4. Power the STM32 Nucleo board using the Mini-B USB cable connected to the PC.

5. Program the firmware on the STM32 Nucleo board. 
This can be done by copying the binary file on the USB mass storage that is 
automatically created when plugging the STM32 Nucleo board to the PC.

6. Reset the MCU by using the reset button on the STM32 Nucleo board










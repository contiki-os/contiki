Sensors Demo
============

The sensors demo can be used to read the value of all sensors and print it every 5 seconds on the terminal.

In order to use this example the X-NUCLEO-IKS01A1 expansion board featuring ST environmental and motions sensors 
must used. It needs to be connected on top of the STM32 Nucleo L1 and the X-NUCLEO-IDS01A4 (or A5) 
sub-1GHz RF communication boards.

To build the example type: 

	make TARGET=stm32nucleo-spirit1 USE_SUBGHZ_BOARD=IDS01A4 USE_SENSOR_BOARD=1




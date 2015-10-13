Sensor Demo
============

The sensor demo can be used to read the values of all sensors and print them every 5 seconds on the terminal.

In order to use this example the X-NUCLEO-IKS01A1 expansion board featuring ST environmental and motion MEMS sensors 
must be used. It needs to be connected on top of the NUCLEO-L152RE (MCU) and the X-NUCLEO-IDS01Ax 
(sub-1GHz RF communication) boards.

To build the example type: 

	make TARGET=stm32nucleo-spirit1 USE_SUBGHZ_BOARD=IDS01A4 USE_SENSOR_BOARD=1
or

	make TARGET=stm32nucleo-spirit1 USE_SUBGHZ_BOARD=IDS01A5 USE_SENSOR_BOARD=1

depending on the sub GHz board you have.

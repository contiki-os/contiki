STM32W Flasher 2.0.0b2 for Linux
================================

A programmer for development boards based on STM32W108 microcontroller.
It works with the following boards (with FT232R or STM32F103 as USB-serial converter):
MB850, MB851, MB950, MB951, MB954

Installation
------------

Installation is not required.
Hal package has to be present in your system.


Usage
-----

Run the program with -h option for usage info.


Notes
-----

- This program may require root privileges when programming boards with FT232R chip.
- In Ubuntu, if you want to flash using Make, type 'sudo -s' before that.
- This version of STM32W Flasher does not support jlink yet.

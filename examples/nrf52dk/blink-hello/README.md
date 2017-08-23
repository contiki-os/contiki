Blink Hello example
===================
This example shows basic usage of DK's buttons and LEDs. It also shows basic 
usage of Contiki's processes. The application autostarts 5 processes: 4 processes
for button and LED control and 1 to display current temperature to the console.

A process reacts to a press of a respective button (process 1 reacts to button 1, etc.)
and doubles the current blinking frequency. The cycle restarts for beginning when blinking
frequency is greater than 8Hz.  

The example requires one DK and it doesn't use SoftDevice. To compile and flash the
example run:

	make TARGET=nrf52dk blink-hello.flash
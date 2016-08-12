Timers test
===========
Timers test is an application allows for testing clocks implementation for nRF52 DK. 
The results of different tests are output to the console.

There are 4 tests performed:

	1) TEST clock_delay_usec() - measures duration of a NOP delay using rtimer. It's expected
	                             that difference is close to 0.
	                             
	2) TEST rtimer - schedules an rtimer callback after 1 second. Prints actual time difference
	                 in rtimer and clock ticks. It's expected that both values are close to 0.
	                 
	3) TEST etimer - schedules an event timer and measures time difference. It is expected that
	                 the value is close to 0.

The example requires one DK and it doesn't use SoftDevice. To compile and flash the
example run:

	make TARGET=nrf52dk timer-test.flash
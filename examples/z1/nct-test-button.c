#include "contiki.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include <stdio.h>
/*-------------------------------------------------*/
PROCESS(button_process, "button process");
AUTOSTART_PROCESSES(&button_process);
/*-------------------------------------------------*/
PROCESS_THREAD(button_process, ev, data) {
	PROCESS_BEGIN();
	SENSORS_ACTIVATE(button_sensor);
	while (1) {
		PROCESS_WAIT_EVENT_UNTIL(
				(ev==sensors_event) && (data == &button_sensor));
		printf("I pushed the button! \n");
	}
	PROCESS_END();
}

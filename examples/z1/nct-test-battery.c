/**
 * \file
 *         Testing the internal MSP430 battery sensor on the Zolertia Z1 Platform.
 * \author
 *         Enric M. Calvo <ecalvo@zolertia.com>
 */

#include "contiki.h"
#include "dev/battery-sensor.h"
#include <stdio.h>		/* For printf() */

float floor(float x) {
	if (x >= 0.0f) {
		return (float) ((int) x);
	} else {
		return (float) ((int) x - 1);
	}
}

/*---------------------------------------------------------------------------*/
PROCESS(test_battery_process, "Battery Sensor Test");
AUTOSTART_PROCESSES(&test_battery_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_battery_process, ev, data) {

	PROCESS_BEGIN()
		;

		SENSORS_ACTIVATE(battery_sensor);

		while (1) {
			uint16_t bateria = battery_sensor.value(0);
			float mv = (bateria * 2.500 * 2) / 4096;
			printf("Battery: %i (%ld.%03d mV)\n", bateria, (long) mv,
					(unsigned) ((mv - floor(mv)) * 1000));
		}

		SENSORS_DEACTIVATE(battery_sensor);

	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

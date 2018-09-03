#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contiki.h"
#include "netstack.h"
#include "net/rime/rime.h"
#include "net/packetbuf.h"
#include "dev/temp-sensor.h"
#include "dev/pwrpin.h"


static double temp=0.0;
static double duty=0.0;

PROCESS(temp_process, "Temperature Process");
AUTOSTART_PROCESSES(&temp_process);

PROCESS_THREAD(temp_process, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	pwr_pin_init();
	etimer_set(&et, CLOCK_SECOND/300);
	while(1) {  
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);	
        SENSORS_ACTIVATE(temp_sensor);
		temp = (double)(temp_sensor.value(0)*1.0/100);
		SENSORS_DEACTIVATE(temp_sensor);
		printf("t=%-4.2f",temp);
	}
	PROCESS_END();
}

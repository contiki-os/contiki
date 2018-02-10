/**
* \file
*         WIMEA-ICT Gen3 AWS Gateway
* \details
*   ATMEGA256RFR2 RSS2 MOTE with RTC, SD card and Electron 3G uplink
* \author
*         Maximus Byamukama <maximus.byamukama@cedat.mak.ac.ug>
*/
#include <stdio.h>
#include "contiki.h"

PROCESS(sinknode_process, "Sink Node Process");
AUTOSTART_PROCESSES(&sinknode_process);

PROCESS_THREAD(sinknode_process, ev, data)
{
	PROCESS_BEGIN();
	static struct timer t;
	while(1) {  
		timer_set(&t, CLOCK_SECOND/200);
		if(timer_expired(&t)){
        printf("n");
     }
	}
	PROCESS_END();
}
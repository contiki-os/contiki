/**
* \file
*         WIMEA-ICT Gen3 Sink Node with RTC and SD card
* \author
*         Maximus Byamukama <maximus.byamukama@cedat.mak.ac.ug>
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include "contiki.h"
#include "dev/leds.h"
#include <util/delay.h>
#include <dev/watchdog.h>

int i=0;
static struct etimer et;


PROCESS(uplink_process, "Uplink Process");
AUTOSTART_PROCESSES(&uplink_process);

PROCESS_THREAD(uplink_process, ev, data)
{
	PROCESS_BEGIN();
	while(1) {		
	etimer_set(&et, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("AT+CGSOCKCONT=1,\"IP\",\"INTERNET\"\n");
    etimer_set(&et, CLOCK_SECOND * 1);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("AT+CSOCKSETPN = 1\n");
    etimer_set(&et, CLOCK_SECOND * 1);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("AT+CIPMODE = 0\n");
    etimer_set(&et, CLOCK_SECOND * 1);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("AT+NETOPEN\n");
    etimer_set(&et, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    printf("AT+CIPOPEN = 0,\"TCP\",\"wimea.mak.ac.ug\",10000\n");
    etimer_set(&et, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));	
	while (i<30)
     {
      printf("AT+CIPSEND = 0,\n");
      etimer_set(&et, CLOCK_SECOND/4);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));	
      printf(" Count = %d:  TXT=mak-gen3 THIS IS A TEST\n%c",i,26);
      etimer_set(&et, CLOCK_SECOND/4);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	  i++;
    }
      printf("AT + CIPCLOSE = 0\n");
	  etimer_set(&et, CLOCK_SECOND/2);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));	
	}
	PROCESS_END();
}

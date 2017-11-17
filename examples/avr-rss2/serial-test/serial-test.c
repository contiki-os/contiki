
#include "contiki.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dev/serial-line.h"

int uplink_error;

char *strip_copy(const char *s) {
	char *p = malloc(strlen(s) + 1);
	if(p) 
	{
		char *p2 = p;
		while(*s != '\0') 
		{
			if(*s != '\t' && *s != '\n' && *s != '\r' && *s != ' ') {
				*p2++ = *s++;
			} else {
				++s;
			}
		}
		*p2 = '\0';
	}
	return p;
}


PROCESS(serial_recv, "Serial line receive process");
PROCESS(serial_send, "Serial line send process");
AUTOSTART_PROCESSES(&serial_recv, &serial_send);

PROCESS_THREAD(serial_send, ev, data)
{
	struct etimer et;
	PROCESS_BEGIN();
	while(1) 
	{
		printf("AT+CGSOCKCONT=1,\"IP\",\"INTERNET\"\n");
		etimer_set(&et, CLOCK_SECOND * 1);
		
	}
	PROCESS_END();
}

PROCESS_THREAD(serial_recv, ev, data)
{
	PROCESS_BEGIN();
	while(1) 
	{
		PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message);
		char* response = strip_copy((char*)data);
		char *pos = strstr(response,"ERROR");	
		if(pos-response > -1) //"ERROR" was found in response string
		{
			uplink_error=1;
		}
		else uplink_error=0;
		
	}
	PROCESS_END();
}
#ifndef EVENT_TRACE_H
#define EVENT_TRACE_H

#include "sys/clock.h"
#include "contiki.h"
#include "er-coap-engine.h"
PROCESS_NAME(event_trace_process);

extern int event_now;
extern int intensity;
extern coap_packet_t teste[1];
void event_trace_start(unsigned seconds, clock_time_t period);
int detect();
int get_event();
int isEvent(int time);
#endif /*__EVENT-TRACE__*/

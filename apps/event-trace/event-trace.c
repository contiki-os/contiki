#include "contiki.h"
#include "contiki-lib.h"
#include <stdio.h>
#include "event-trace.h"
#include <data.c>
#include "node-id.h"
#include <math.h>


int isEVent(int time);

//VARIABLES
unsigned timer, interval_min = 1, interval_sec;
int event_now = 0, last_event = 0, current_event = 0, intensity;
int node_removed = 0;



PROCESS(event_trace_process, "Event Trace");

PROCESS_THREAD(event_trace_process, ev, data){

	static struct etimer periodic;
	clock_time_t *period;

	PROCESS_BEGIN();

	period = data;
	if(period == NULL)
		PROCESS_EXIT();

	etimer_set(&periodic, 60*CLOCK_SECOND);

	while(1){
		PROCESS_WAIT_UNTIL(etimer_expired(&periodic));
		printf("EVENT_TRACE - %d - %d ||| %d\n", timer, last_event, node_removed);
		etimer_reset(&periodic);

		//SYNC THE EVENT TIMERS WITH SIMULATION TIMER
		timer += interval_min;

		//VERIFIES IF CURRENT TIME IS IN THE RANGE OF THE EVENT SPAM
		event_now = (abs(timer - last_event) <= 5) ? 1 : 0;

		//IF CURRENT TIMER CORRESPONDS TO AN EVENT, THE NEXT 5 MINUTES ARE EVENTFUL.
		if(isEVent(timer)){
			if(detect()){
				intensity = e[current_event];
				last_event = datetime[current_event];
				printf("Event at %d minutes, at [%d, %d] position, intensity of %d\n", last_event, xpos[current_event], ypos[current_event], e[current_event]);
			}
		current_event++;
		}
	}
	PROCESS_END();
}

//FUNCTION TO START THIS PROCESS
void event_trace_start(unsigned seconds, clock_time_t period){
	interval_sec = seconds;
	interval_min = seconds/60;
	process_start(&event_trace_process, (void *)&period);
}

//RUNS TROUGH THE LIST OF EVENT TIMERS AND RETURNS IF THE CURRENT TIME IS EVENTFUL
int isEVent(int time){
	int arr_size = sizeof(datetime)/sizeof(datetime[0]);
	int i;
	for(i = 0; i < arr_size; i++)
		if(time == datetime[i]) 
			return 1;
	return 0;
}

//VERIFIES IF NODE IS IN RANGE OF EVENT
int detect(){
	/*
	double dist = sqrt(pow((px-pcx), 2) + pow((py-pcy), 2));
	if(dist < 50){
		printf("Event[%d, %d] detected by Node %d [%d, %d] \n",px, py, node_id, pcx, pcy);
		printf("Distance: %lu\n", (unsigned long) dist);
		event_now = 1;
		return 1;
	}
	*/	
	return 1;
}

int get_event() {
	if(node_removed == 0)
		return event_now;
	return 0;
}
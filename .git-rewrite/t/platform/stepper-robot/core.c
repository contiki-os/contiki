#include <stepper-process.h>
#include <dev/leds.h>
#include <loader/codeprop-otf.h>
#include <loader/ram-segments.h>
#include <sys/autostart.h>
#include "codeprop-otf.h"
#include <stdio.h>
#include <stdlib.h>

PROCESS(blink_process, "LED blink process");

PROCESS_THREAD(blink_process, ev , data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
  etimer_set(&timer, CLOCK_SECOND/2);
   while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXIT ||
			     ev== PROCESS_EVENT_TIMER);
    if (ev == PROCESS_EVENT_EXIT) break;
    leds_invert(LEDS_RED);
    etimer_reset(&timer);
  }
  PROCESS_END();
}

AUTOSTART_PROCESSES(&codeprop_process, &ram_segments_cleanup_process, &stepper_process,&blink_process);

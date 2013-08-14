/* This file demonstrates the interaction between two processes via events.
 *
 * - "Sensor process": Throws an event periodically. This can be for example a
 * 				       sensor value.
 * - "Print process" : Waits for events from "Sensor" and prints a message when
 *                     an event occurs (e.g. prints the sensor value)
 *
 * This example is derived from the contiki code examples for the WSN430
 * (SensTools - Inria: http://senstools.gforge.inria.fr/)
 *
 * Author: George Oikonomou <G.Oikonomou@lboro.ac.uk>
 */

#include "contiki.h"
//#include "dev/leds.h"
#include <limits.h>
#include <stdio.h> /* For printf() */
#include "event-post.h"

/* This is our event type */
static process_event_t event_data_ready;

/*---------------------------------------------------------------------------*/
/* Declare the two processes here */
PROCESS(sensor_process, "Sensor process");
PROCESS(print_process, "Print process");

/* Tell Contiki that we want them to start automatically */
AUTOSTART_PROCESSES(&sensor_process, &print_process);

/*---------------------------------------------------------------------------*/
/* Implementation "Sensor Process" */
PROCESS_THREAD(sensor_process, ev, data)
{
	/* static variables to preserve values across consecutive calls of this
	 * process. */
	/* Set an etimer */
	static struct etimer timer;
	/* And the 'sensor' monitoring variable */
    static struct event_struct es;

    PROCESS_BEGIN();

    /* Set some near-the-limit initial values */
    /* signed primitives */
    es.s_val = SHRT_MAX-2;
    es.i_val = INT_MAX-2;
    es.l_val = LONG_MAX-2;
    /* sizeof(long long) == sizeof(long) on sensinodes - see other examples*/
    es.ll_val = LONG_MAX-2;
    /* and some typedef-ed unsigned variables */
    es.u8_val = UCHAR_MAX-2;
    es.u16_val = USHRT_MAX-2;
    es.u32_val = ULONG_MAX-2;

    /* allocate the required event */
    event_data_ready = process_alloc_event();
    
    /* process_event_t is actually a u_char. What did the OS allocate for us? */
    printf("Contiki allocated event ID %d.\r\n", event_data_ready);
    
    /* Set a timer here. We will generate an event every times this timer expires
     * etimer_set accepts clock ticks as its 2nd argument.
     * CLOCK_CONF_SECOND is the number of ticks per second.
     * This CLOCK_CONF_SECOND * N = N seconds */
    etimer_set(&timer, CLOCK_CONF_SECOND * 2);
    
    while (1)
    {
        printf("Sensor process: Wait for timer event...\r\n");
        /* Wait on our timer */
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

        /* blip */
        /* leds_toggle(LEDS_BLUE); */

        /* Set the 'sensor' value before throwing the event */
        printf("Sensor Process: Incrementing values...\r\n");
        es.s_val++;
        es.i_val++;
        es.l_val++;
        es.ll_val++;
        es.u8_val++;
        es.u16_val++;
        es.u32_val++;

        /* Post our event.
         * N.B. es is declared static.
         * Try passing a volatile variable and observe the results... */
        printf("Sensor Process: Generating 'Data Ready' event.\r\n");
        process_post(&print_process, event_data_ready, &es);

        /* reset the timer so we can wait on it again */
        etimer_reset(&timer);

    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Implementation of "Print Process" */
PROCESS_THREAD(print_process, ev, data)
{

	struct event_struct * sd;

    PROCESS_BEGIN();
    
    while (1)
    {
        /* Stop here and wait until "event_data_ready" occurs */
        PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
        
        /* When the event occurs, the incoming data will be stored in
         * process_data_t data        (careful, this is void *)
         *
         * Print away...
         * es is volatile, we need to set it = data again and dereference it. */
        sd = data;
        printf("Print Process - Data Ready:\r\n");
        printf("    s: %d\r\n", sd->s_val);
        printf("    i: %d\r\n", sd->i_val);
        printf("    l: %ld\r\n", sd->l_val);
        printf("   ll: %lld\r\n", sd->ll_val);
        printf("   u8: %u\r\n", sd->u8_val);
        printf("  u16: %u\r\n", sd->u16_val);
        printf("  u32: %lu\r\n", sd->u32_val);

        /* aaaaand back to waiting for the next event */
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/

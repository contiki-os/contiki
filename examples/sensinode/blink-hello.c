/* This is a very simple hello_world program.
 * It aims to demonstrate the co-existence of two processes:
 * One of them prints a hello world message and the other blinks the LEDs
 *
 * It is largely based on hello_world in $(CONTIKI)/examples/sensinode
 *
 * Author: George Oikonomou <G.Oikonomou@lboro.ac.uk>
 */

#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
/* We declare the two processes */
PROCESS(hello_world_process, "Hello world process");
PROCESS(blink_process, "LED blink process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&hello_world_process, &blink_process);
/*---------------------------------------------------------------------------*/
/* Implementation of the first process */
PROCESS_THREAD(hello_world_process, ev, data)
{
    /* variables are declared static to ensure their values are maintained
       between subsequent calls.
       All the code between PROCESS_THREAD and PROCESS_BEGIN() runs each time
       the process is invoked. */
    static struct etimer timer;
    static int count;
    
    /* any process must start with this. */
    PROCESS_BEGIN();
    
    /* set the etimer module to generate an event in one second.
       CLOCK_CONF_SECOND is #define as 128 */
    etimer_set(&timer, CLOCK_CONF_SECOND * 4);
    count = 0;
    /* Don't declare variables after PROCESS_BEGIN.
     * While it will compile fine with TARGET=native (gcc is happy),
     * SDCC doesn't like it. Soon as you try TARGET=sensinode you will get:
     * syntax error: token -> 'int' ;
     * Try uncommenting the line below and observe the results */
    /* int whoops = 0;
     * whoops = 0; */
    while (1)
    {
        /* wait here for an event to happen */
        PROCESS_WAIT_EVENT();
        /* This achieves the same
         * PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER); */

        /* if the event is the timer event as expected... */
        if(ev == PROCESS_EVENT_TIMER)
        {
            /* do the process work */
            printf("Sensor says no... #%d\r\n", count);
            count ++;
            
            /* reset the timer so it will generate an other event
             * the exact same time after it expired (periodicity guaranteed) */
            etimer_reset(&timer);
        }
        
        /* and loop */
    }
    /* any process must end with this, even if it is never reached. */
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Implementation of the second process */
PROCESS_THREAD(blink_process, ev, data)
{
    static struct etimer timer;
    PROCESS_BEGIN();
    
    while (1)
    {
        /* we set the timer from here every time */
        etimer_set(&timer, CLOCK_CONF_SECOND);
        
        /* and wait until the event we receive is the one we're waiting for */
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        
        printf("Blink... (state %0.2X).\r\n", leds_get()); 
        /* update the LEDs */
        leds_toggle(LEDS_GREEN);
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*
 * This file contains an example of how to call a service.
 *
 * This program implements a process that calls the service defined in
 * example-service.h every second.
 */

#include <stdio.h>

#include "contiki.h"

/*
 * We must include the header file for the service.
 */
#include "example-service.h"

/*
 * All Contiki programs must have a process, and we declare it here.
 */
PROCESS(example_use_service_process, "Use example");

/*
 * The program is to call the service once every second, so we use an
 * event timer in order to run every second.
 */
static struct etimer timer;

/*---------------------------------------------------------------------------*/
/*
 * Here we implement the process.
 */
PROCESS_THREAD(example_use_service_process, ev, data)
{
  /*
   * A process thread starts with PROCESS_BEGIN() and ends with
   * PROCESS_END().
   */  
  PROCESS_BEGIN();

  /*
   * We loop for ever, calling the service once every second.
   */
  while(1) {

    /*
     * We set a timer that wakes us up once every second. 
     */
    etimer_set(&timer, CLOCK_SECOND);
    PROCESS_YIELD_UNTIL(etimer_expired(&timer));

    /*
     * We call the service. If the service is not registered, the
     * SERVICE_CALL() statement does nothing. If we need to know if
     * the service exists, we can use the SERVICE_FIND() function.
     */
    printf("use example: calling example\n");
    SERVICE_CALL(example_service, example_function());
  }

  /*
   * And finally the process ends.
   */
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

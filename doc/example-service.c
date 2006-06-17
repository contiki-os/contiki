/*
 * This file is an example of how to implement a service in
 * Contiki. The header file example-service.h defines a service called
 * "example_service", which we implement in this file.
 *
 * This example shows how to define an instance of a service, and how
 * to write the service's controlling process.
 *
 * See the file example-use-service.c for an example of how to call a
 * service.
 */

#include <stdio.h>

#include "example-service.h"
#include "contiki.h"

/*---------------------------------------------------------------------------*/
/*
 * We start by implementing all the functions that the service
 * offers. In this case, there is only a single function (called
 * example_function()) and we implement it here. We give it the name
 * example() and declare it with the "static" keyword to keep the
 * scope local to this file.
 */
static void
example(void) {
  printf("Example service called\n");
}
/*---------------------------------------------------------------------------*/
/*
 * This is the instantiation of the service called
 * "example_service". The service interface is defined in the header
 * file example-service.h.
 *
 * This statement defines the name of this implementation of the
 * service - example_service_implementation - and defines the
 * functions that actually implement the functions offered by the
 * service. In this example, the service consists of a single function
 * called "example_function()". We implement this function in the
 * function called "example()" defined above.
 *
 */
SERVICE(example_service_implementation,  /* The name of this instance
					    of the service - used with
					    SERVICE_REGISTER(). */
	example_service,                 /* The name of the serivce
					    that is instantiated. */
        { example });                    /* The list of functions
					    required by the
					    service. In this case, we
					    only have one function. */

/*
 * All services needs a controlling process. The controlling process
 * registers the service with the system when it starts, and is also
 * notified if the service is removed or replaced.
 *
 * We simply call the process "example_service_process" and gives it a
 * similar textual name.
 */
PROCESS(example_service_process, "Example service process");

/*
 * For this example, we use a timer to remove the service after a
 * certain time. We declare the timer here.
 */ 
static struct etimer timer;

/*
 * Finally, we implement the controlling process.
 */
PROCESS_THREAD(example_service_process, ev, data)
{

  /*
   * A process thread starts with PROCESS_BEGIN() and ends with
   * PROCESS_END().
   */
  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  /*
   * We register the service instance with a SERVICE_REGISTER()
   * statement.
   */
  printf("Registering example service\n");
  SERVICE_REGISTER(example_service_implementation);

  /*
   * We set a timer for four seconds and wait for it to expire - or
   * for the process to receive an event which requests it to exit.
   *
   * The only purpose for the timer is to demonstrate how a service is
   * removed - it is not something that is commonly done.
   */
  etimer_set(&timer, 4 * CLOCK_SECOND);
  PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_SERVICE_REMOVED ||
		      etimer_expired(&timer));

  /*
   * And we remove the service before the process ends. This is a
   * *very* important step - if the process exits and is unloaded
   * without first removing its services, the system may crash!
   */
  printf("Removing example service\n");

  /*
   * And finally the process ends.
   */
 exit:
  SERVICE_REMOVE(example_service_implementation);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

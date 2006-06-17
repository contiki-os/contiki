/*
 * This file is an example of how to define a service in Contiki. The
 * example shows how to define a service interface, and how to give
 * the service a name.
 */
#ifndef __EXAMPLE_SERVICE_H__
#define __EXAMPLE_SERVICE_H__

#include "sys/service.h"

/*
 * This is how we define the service interface, and give the service a
 * name. The name of this particular service is "example_service" and
 * the interface consists of a single function, called
 * example_function().
 */
SERVICE_INTERFACE(example_service,
{
  void (* example_function)(void);
  /* More functions can be added here, line by line. */
});

/*
 * We must also give the service a textual name. We do this by using a
 * special #define statment - we define a macro with the same name as
 * the service, but postfixed with "_name".
 *
 * The textual name is used when looking up services. The name must be
 * unique within the system.
 */
#define example_service_name "Example service"

#endif /* __EXAMPLE_SERVICE_H__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"

#include "erbium.h"
/* Define which resources to include to meet memory constraints. */
#define REST_RES_RELAY 1

#if defined (PLATFORM_HAS_BUTTON)
#include "dev/leds.h" // we have check it
#endif


#if REST_RES_RELAY
/*
 * Resources are defined by the RESOURCE macro.
 * Signature: resource name, the RESTful methods it handles, and its URI path (omitting the leading slash).
 */

RESOURCE(RELAY, METHOD_POST | METHOD_PUT | METHOD_POST,  "sensor\relay" , "title=\"LEDs: ?color=r|g|b, POST/PUT mode=on|off\";rt=\"Control\"");


//"actuators/le
/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
void
RELAY_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *color = NULL;
  const char *mode = NULL;
  uint8_t led = 0;
  int success = 1;

  if ((len=REST.get_query_variable(request, "color", &color))) {
    PRINTF("color %.*s\n", len, color);

    if (strncmp(color, "r", len)==0) {
      led = LEDS_RED;
    } else if(strncmp(color,"g", len)==0) {
      led = LEDS_GREEN;
    } else if (strncmp(color,"b", len)==0) {
      led = LEDS_BLUE;
    } else {
      success = 0;
    }
  } else {
    success = 0;
  }

  if (success && (len=REST.get_post_variable(request, "mode", &mode))) {
    PRINTF("mode %s\n", mode);

    if (strncmp(mode, "on", len)==0) {
      leds_on(led);
    } else if (strncmp(mode, "off", len)==0) {
      leds_off(led);
    } else {
      success = 0;
    }
  } else {
    success = 0;
  }

  if (!success) {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
}

#endif




//Process Function
PROCESS_THREAD(rest_server_example, ev, data)
{
  PROCESS_BEGIN();

  PRINTF("Starting Erbium Example Server\n");


  /* Activate the application-specific resources. */
#if REST_RES_RELAY
  rest_activate_resource(&resource_RELAY);
#endif

  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();

#if REST_RES_EVENT
      /* Call the event_handler for this application-specific event. */
      event_event_handler(&resource_event);
#endif
#if REST_RES_SEPARATE && WITH_COAP>3
      /* Also call the separate response example handler. */
      separate_finalize_handler();
#endif
    }
  } /* while (1) */

  PROCESS_END();
}

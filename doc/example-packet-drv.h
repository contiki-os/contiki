/*
 * This is an example of how to write a network device driver ("packet
 * driver") for Contiki.
 */

#ifndef __EXAMPLE_PACKET_DRV_H__
#define __EXAMPLE_PACKET_DRV_H__

/*
 * We include the "contiki.h" file to get the macro and typedef used below.
 */
#include "contiki.h"

/*
 * Here we declare the process name used to start it (typically from main()).
 */
PROCESS_NAME(example_packet_driver_process);

/*
 * Finally we declare the output function for use with uIP packet forwarding.
 */
uint8_t example_packet_driver_output(void);

#endif /* __EXAMPLE_PACKET_DRV_H__ */

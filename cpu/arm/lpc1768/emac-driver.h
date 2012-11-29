/*
 * emac-driver.h
 *
 *  Created on: Nov 25, 2012
 *      Author: cazulu
 */

#ifndef EMAC_DRIVER_H_
#define EMAC_DRIVER_H_

#include "contiki-net.h"
#include "contiki-conf.h"

PROCESS_NAME(emac_lpc1768);

//This is just a wrapper for the Ethernet module interrupt
//to call a contiki poll_process
void
poll_eth_driver(void);

#if UIP_CONF_IPV6
uint8_t
send_packet(uip_lladdr_t * lladdr);
#else
uint8_t
send_packet(void);
#endif

#endif /* EMAC_DRIVER_H_ */

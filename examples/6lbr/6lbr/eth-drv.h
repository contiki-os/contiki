#ifndef ETH_DRV_H_
#define ETH_DRV_H_

#include "contiki.h"
#include "net/uip.h"

#define UIP_8023_ADDR_LEN 6

#if UIP_CONF_LLH_LEN == 0
#define ETHERNET_LLH_LEN 14
extern uint8_t ll_header[ETHERNET_LLH_LEN];
#endif

PROCESS_NAME(eth_drv_process);

void eth_drv_send(void);

void eth_drv_exit(void);
void eth_drv_init(void);

#endif

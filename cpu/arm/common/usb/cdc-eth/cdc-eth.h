#ifndef __CDC_ETH_H__NUI0ULFC7C__
#define __CDC_ETH_H__NUI0ULFC7C__

#include <net/uip.h>

/* Should be called before usb_cdc_eth_setup */
void
usb_cdc_eth_set_ifaddr(uip_ipaddr_t *addr);

void
usb_cdc_eth_setup();

#endif /* __CDC_ETH_H__NUI0ULFC7C__ */

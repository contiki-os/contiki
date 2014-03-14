#ifndef CDC_ETH_H_NUI0ULFC7C__
#define CDC_ETH_H_NUI0ULFC7C__

#include <net/ip/uip.h>

/* Should be called before usb_cdc_eth_setup */
void
usb_cdc_eth_set_ifaddr(uip_ipaddr_t *addr);

void
usb_cdc_eth_setup();

#endif /* CDC_ETH_H_NUI0ULFC7C__ */

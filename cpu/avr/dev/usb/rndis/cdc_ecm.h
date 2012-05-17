
#ifndef __CDC_ECM_H__
#define __CDC_ECM_H__

#include "contiki.h"
#include <stdint.h>
#include "conf_usb.h"

#define SET_ETHERNET_PACKET_FILTER		(0x43)


extern uint8_t cdc_ecm_process(void);
extern uint8_t ecm_send(uint8_t * senddata, uint16_t sendlen, uint8_t led);
extern void cdc_ecm_set_ethernet_packet_filter(void);
extern void cdc_ecm_configure_endpoints();
extern void cdc_ecm_set_active(uint8_t value);
extern uint8_t cdc_ecm_handle_send_encapsulated_command(uint16_t wLength);
extern uint8_t cdc_ecm_handle_get_encapsulated_command();

#endif // __CDC_ECM_H__
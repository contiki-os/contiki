#ifndef _ER_COAP_COMMUNICATION_H_
#define _ER_COAP_COMMUNICATION_H_

#include "contiki.h"

void
coap_init_communication_layer(uint16_t port);

void
coap_send_message(uip_ipaddr_t *addr, uint16_t port, uint8_t *data, uint16_t length);

void
coap_handle_receive(void);

#endif

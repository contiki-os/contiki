#ifndef _ER_COAP_COMMUNICATION_H_
#define _ER_COAP_COMMUNICATION_H_

#include "contiki.h"

#if WITH_DTLS_COAP
#include "er-coap-dtls.h"
#else
#include "er-coap-udp.h"
#endif

context_t *
coap_init_communication_layer(uint16_t port);

void
coap_send_message(context_t * ctx, uip_ipaddr_t *addr, uint16_t port, uint8_t *data, uint16_t length);

void
coap_handle_receive(context_t * ctx);

#endif

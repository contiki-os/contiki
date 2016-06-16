#include "contiki.h"
#include "contiki-net.h"
#include "er-coap.h"
#include "er-coap-engine.h"

#include <string.h>

#define DEBUG DEBUG_NONE
#include "uip-debug.h"

/*-----------------------------------------------------------------------------------*/
context_t *
coap_init_communication_layer(uint16_t port)
{
  /* new connection with remote host */
  context_t * ctx = udp_new(NULL, 0, NULL);
  udp_bind(ctx, port);
  PRINTF("Listening on port %u\n", uip_ntohs(ctx->lport));
  return ctx;
}
/*-----------------------------------------------------------------------------------*/
void
coap_send_message(context_t * ctx, uip_ipaddr_t *addr, uint16_t port, uint8_t *data, uint16_t length)
{
  /* Configure connection to reply to client */
  uip_ipaddr_copy(&ctx->ripaddr, addr);
  ctx->rport = port;

  uip_udp_packet_send(ctx, data, length);
  PRINTF("-sent UDP datagram (%u)-\n", length);

  /* Restore server connection to allow data from any node */
  memset(&ctx->ripaddr, 0, sizeof(ctx->ripaddr));
  ctx->rport = 0;
}
/*-----------------------------------------------------------------------------------*/
void
coap_handle_receive(context_t * ctx)
{
  coap_receive(ctx);
}

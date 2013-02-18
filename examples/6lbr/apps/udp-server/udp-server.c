#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "node-info.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define MAX_PAYLOAD_LEN 120

static struct uip_udp_conn *server_conn;

PROCESS(udp_server_process, "UDP server process");

static void
tcpip_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  if(uip_newdata()) {
    ((char *)uip_appdata)[uip_datalen()] = 0;
    PRINTF("Server received: '%s' from ", (char *)uip_appdata);
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("\n");

#if CETIC_NODE_INFO
    node_info_t *node = node_info_lookup(&UIP_IP_BUF->srcipaddr);

    if(node == NULL) {
      node = node_info_add(&UIP_IP_BUF->srcipaddr);
    }
    if(node != NULL) {
      node->last_lookup = clock_time();
      strncpy(node->my_info, (char *)uip_appdata, sizeof(node->my_info) - 1);
    }
#endif

    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    PRINTF("Responding with message: ");
    sprintf(buf, "Hello from the server! (%d)", ++seq_id);
    PRINTF("%s\n", buf);

    uip_udp_packet_send(server_conn, buf, strlen(buf));
    /* Restore server connection to allow data from any node */
    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
  }
}

PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("UDP server started\n");

  server_conn = udp_new(NULL, UIP_HTONS(3001), NULL);
  udp_bind(server_conn, UIP_HTONS(3000));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();
}

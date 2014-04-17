
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "net/rpl/labels.h" 
#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
int pos_x;
int pos_y;

uip_ipaddr_t ipaddr;
struct uip_udp_conn* udp_conn;
PROCESS(rpl_sink_process, "RPL sink process");
AUTOSTART_PROCESSES(&rpl_sink_process);

/*static void udp_handler(void){
  routing_entry_t* entry;

  if(uip_newdata()) {
    entry = (routing_entry_t*) uip_appdata;
    printf("Labels received '%u'\n", (unsigned int) entry->labels);
  }
}*/


/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("IPv6 ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rpl_sink_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

  PRINTF("RPL sink started\n");

  init_featurecast();

 // printf("POS:[%d,%d]\n", pos_x, pos_y);
#if UIP_CONF_ROUTER
/* The choice of server address determines its 6LoPAN header compression.
 * Obviously the choice made here must also be selected in udp-client.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
 * e.g. set Context 0 to aaaa::.  At present Wireshark copies Context/128 and then overwrites it.
 * (Setting Context 0 to aaaa::1111:2222:3333:4444 will report a 16 bit compressed address of aaaa::1111:22ff:fe33:xxxx)
 * Note Wireshark's IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */
 
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */
  
  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high 
     packet reception rates. */
  NETSTACK_MAC.off(1);

  init_routing_table(&routing_table);

  struct uip_udp_conn* udp_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(udp_conn, UIP_HTONS(8888));
 

 static struct etimer periodic; 
  etimer_set(&periodic, 10 * CLOCK_SECOND);
  while(1) {
    PROCESS_YIELD();
    if (ev == sensors_event && data == &button_sensor) {
      //PRINTF("Initiaing global repair\n");
      //rpl_repair_root(RPL_DEFAULT_INSTANCE);
	printf("button pressed - sending packet\n");
	label_packet_t packet;
	
	packet.data = 0;
	memcpy(&packet.src, &ipaddr, sizeof(packet.src));
	packet.labels = LABEL_TYPE_LIGHT;
	//put_label_in_addr(LABEL_TYPE_LIGHT, uip_ip6addr_t* addr)
	send_featurecast(&packet, sizeof(packet), 1, LABEL_TYPE_LIGHT);
	//forward_label_packet(&packet);
	//test_children();
    }else if( ev == tcpip_event){
        ;//udp_handler();
    }else if(etimer_expired(&periodic)) {
      //printf("tick\n");
      etimer_reset(&periodic);
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

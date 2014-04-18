#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-udp-packet.h"
#include "sys/ctimer.h"
#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#include "net/featurecast/featurecast.h"
#include "node-id.h"


int pos_x;
int pos_y;
uip_ipaddr_t server_ipaddr;

uip_ipaddr_t ipaddr;
struct uip_udp_conn* udp_conn;
/*---------------------------------------------------------------------------*/
PROCESS(node_process, "Node  process");
AUTOSTART_PROCESSES(&node_process);
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("IPv6 ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_ds6_if.addr_list[i].ipaddr.u16[6] = ipaddr.u16[6];
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}


static void udp_handler(void){
  routing_entry_t* entry;

  if(uip_newdata()) {
	printf("featurecast data received!\n");
 /*   entry = (routing_entry_t*) uip_appdata;
    printf("Labels received '%u'\n", (unsigned int) entry->labels);*/
  }
}

/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  //uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);

  printf("my old ipv6 addr:");
  PRINT6ADDR(&ipaddr);
  PRINTF("\n");
  
  init_addr_for_labels(&ipaddr);
  printf("my  ipv6 addr after init:");
  PRINT6ADDR(&ipaddr);
  PRINTF("\n");

  if(node_id <= 9){
  	printf("add ROOM_1\n");
  	put_label_in_addr(LABEL_ROOM_1, &ipaddr);
	printf("add FLOOR1\n");
	put_label_in_addr(LABEL_FLOOR_1, &ipaddr);
  }else if(node_id <= 17){
	  printf("add ROOM_2\n");
        put_label_in_addr(LABEL_ROOM_2, &ipaddr);
	printf("add FLOOR1\n");
	put_label_in_addr(LABEL_FLOOR_1, &ipaddr);
  }else if(node_id <= 25){
 	printf("add ROOM_3\n");
        put_label_in_addr(LABEL_ROOM_3, &ipaddr);
	printf("add FLOOR2\n");
	put_label_in_addr(LABEL_FLOOR_2, &ipaddr);
  }else{
	printf("add ROOM_4\n");
        put_label_in_addr(LABEL_ROOM_4, &ipaddr);
	printf("add FLOOR2\n");
	put_label_in_addr(LABEL_FLOOR_2, &ipaddr);
  }  

  if(node_id % 2){
	printf("add TEMP\n");
  	put_label_in_addr(LABEL_TYPE_TEMP, &ipaddr);
  }else{
	printf("add LIGHT\n");
	put_label_in_addr(LABEL_TYPE_LIGHT, &ipaddr);
  }

  printf("my  ipv6 addr after adding a label:");
  PRINT6ADDR(&ipaddr);
  PRINTF("\n");
  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
 /* uip_ds6_if.addr_list[1].ipaddr.u16[6] = ipaddr.u16[6];
  uip_ds6_if.addr_list[1].ipaddr.u16[7] = ipaddr.u16[7];
  uip_ds6_if.addr_list[0].ipaddr.u16[6] = ipaddr.u16[6];
  uip_ds6_if.addr_list[0].ipaddr.u16[7] = ipaddr.u16[7];*/
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  set_global_address();

  init_featurecast();
 // static struct etimer periodic;

  //check_against_all_labels(&ipaddr);
  printf("Bits set in addr %d\n", bits_in_addr(&ipaddr));
//  printf("Checking LIGHT %d\nChecking TEMP: %d\n", addr_contains_label(LABEL_TYPE_LIGHT, &ipaddr), addr_contains_label(LABEL_TYPE_TEMP,  &ipaddr));
 // PRINTF("Node process started\n");
 // printf("POS:[%d,%d]\n", pos_x, pos_y); 
  print_local_addresses();

  init_routing_table(&routing_table);

  struct uip_udp_conn* udp_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(udp_conn, UIP_HTONS(8888));

//etimer_set(&periodic, 10 * CLOCK_SECOND);
  while(1) {
    printf("tick\n");
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      udp_handler();
    }
  //  if(etimer_expired(&periodic)) {
   //   etimer_reset(&periodic);
   // }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

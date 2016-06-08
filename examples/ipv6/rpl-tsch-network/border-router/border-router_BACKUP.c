#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/netstack.h"
#include "dev/slip.h"
#include "button-sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

PROCESS(border_router_process, "Border router process");

// #if WEBSERVER==0
// /* No webserver */
// AUTOSTART_PROCESSES(&border_router_process);

// #elif WEBSERVER>1
// /* Use an external webserver application */
// #include "webserver-nogui.h"
// AUTOSTART_PROCESSES(&border_router_process,&webserver_nogui_process);

// #else
// #include "httpd-simple.h"
// /* The internal webserver can provide additional information if
//  * enough program flash is available.
//  */
// #define WEBSERVER_CONF_LOADTIME 0
// #define WEBSERVER_CONF_FILESTATS 0
// #define WEBSERVER_CONF_NEIGHBOR_STATUS 0
// /* Adding links requires a larger RAM buffer. To avoid static allocation
//  * the stack can be used for formatting; however tcp retransmissions
//  * and multiple connections can result in garbled segments.
//  * TODO:use PSOCk_GENERATOR_SEND and tcp state storage to fix this.
//  */
// #define WEBSERVER_CONF_ROUTE_LINKS 0
// #if WEBSERVER_CONF_ROUTE_LINKS
// #define BUF_USES_STACK 1
// #endif

// PROCESS(webserver_nogui_process, "Web server");
// PROCESS_THREAD(webserver_nogui_process, ev, data)
// {
//   PROCESS_BEGIN();

//   httpd_init();

//   while(1) {
//     PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
//     httpd_appcall(data);
//   }

//   PROCESS_END();
// }
// AUTOSTART_PROCESSES(&border_router_process,&webserver_nogui_process);

// static const char *TOP = "<html><head><title>ContikiRPL</title></head><body>\n";
// static const char *BOTTOM = "</body></html>\n";
// #if BUF_USES_STACK
// static char *bufptr, *bufend;
// #define ADD(...) do {                                                   \
//     bufptr += snprintf(bufptr, bufend - bufptr, __VA_ARGS__);      \
//   } while(0)
// #else
// static char buf[256];
// static int blen;
// #define ADD(...) do {                                                   \
//     blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
//   } while(0)
// #endif
// /*---------------------------------------------------------------------------*/
// static void
// ipaddr_add(const uip_ipaddr_t *addr)
// {
//   uint16_t a;
//   int i, f;
//   for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
//     a = (addr->u8[i] << 8) + addr->u8[i + 1];
//     if(a == 0 && f >= 0) {
//       if(f++ == 0) ADD("::");
//     } else {
//       if(f > 0) {
//         f = -1;
//       } else if(i > 0) {
//         ADD(":");
//       }
//       ADD("%x", a);
//     }
//   }
// }
// /*---------------------------------------------------------------------------*/
// static
// PT_THREAD(generate_routes(struct httpd_state *s))
// {
//   static uip_ds6_route_t *r;
//   static uip_ds6_nbr_t *nbr;
// #if BUF_USES_STACK
//   char buf[256];
// #endif
// #if WEBSERVER_CONF_LOADTIME
//   static clock_time_t numticks;
//   numticks = clock_time();
// #endif

//   PSOCK_BEGIN(&s->sout);

//   SEND_STRING(&s->sout, TOP);
// #if BUF_USES_STACK
//   bufptr = buf;bufend=bufptr+sizeof(buf);
// #else
//   blen = 0;
// #endif
//   ADD("Neighbors<pre>");

//   for(nbr = nbr_table_head(ds6_neighbors);
//       nbr != NULL;
//       nbr = nbr_table_next(ds6_neighbors, nbr)) {

// #if WEBSERVER_CONF_NEIGHBOR_STATUS
// #if BUF_USES_STACK
// {char* j=bufptr+25;
//       ipaddr_add(&nbr->ipaddr);
//       while (bufptr < j) ADD(" ");
//       switch (nbr->state) {
//       case NBR_INCOMPLETE: ADD(" INCOMPLETE");break;
//       case NBR_REACHABLE: ADD(" REACHABLE");break;
//       case NBR_STALE: ADD(" STALE");break;
//       case NBR_DELAY: ADD(" DELAY");break;
//       case NBR_PROBE: ADD(" NBR_PROBE");break;
//       }
// }
// #else
// {uint8_t j=blen+25;
//       ipaddr_add(&nbr->ipaddr);
//       while (blen < j) ADD(" ");
//       switch (nbr->state) {
//       case NBR_INCOMPLETE: ADD(" INCOMPLETE");break;
//       case NBR_REACHABLE: ADD(" REACHABLE");break;
//       case NBR_STALE: ADD(" STALE");break;
//       case NBR_DELAY: ADD(" DELAY");break;
//       case NBR_PROBE: ADD(" NBR_PROBE");break;
//       }
// }
// #endif
// #else
//       ipaddr_add(&nbr->ipaddr);
// #endif

//       ADD("\n");
// #if BUF_USES_STACK
//       if(bufptr > bufend - 45) {
//         SEND_STRING(&s->sout, buf);
//         bufptr = buf; bufend = bufptr + sizeof(buf);
//       }
// #else
//       if(blen > sizeof(buf) - 45) {
//         SEND_STRING(&s->sout, buf);
//         blen = 0;
//       }
// #endif
//   }
//   ADD("</pre>Routes<pre>");
//   SEND_STRING(&s->sout, buf);
// #if BUF_USES_STACK
//   bufptr = buf; bufend = bufptr + sizeof(buf);
// #else
//   blen = 0;
// #endif

//   for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {

// #if BUF_USES_STACK
// #if WEBSERVER_CONF_ROUTE_LINKS
//     ADD("<a href=http://[");
//     ipaddr_add(&r->ipaddr);
//     ADD("]/status.shtml>");
//     ipaddr_add(&r->ipaddr);
//     ADD("</a>");
// #else
//     ipaddr_add(&r->ipaddr);
// #endif
// #else
// #if WEBSERVER_CONF_ROUTE_LINKS
//     ADD("<a href=http://[");
//     ipaddr_add(&r->ipaddr);
//     ADD("]/status.shtml>");
//     SEND_STRING(&s->sout, buf); //TODO: why tunslip6 needs an output here, wpcapslip does not
//     blen = 0;
//     ipaddr_add(&r->ipaddr);
//     ADD("</a>");
// #else
//     ipaddr_add(&r->ipaddr);
// #endif
// #endif
//     ADD("/%u (via ", r->length);
//     ipaddr_add(uip_ds6_route_nexthop(r));
//     if(1 || (r->state.lifetime < 600)) {
//       ADD(") %lus\n", (unsigned long)r->state.lifetime);
//     } else {
//       ADD(")\n");
//     }
//     SEND_STRING(&s->sout, buf);
// #if BUF_USES_STACK
//     bufptr = buf; bufend = bufptr + sizeof(buf);
// #else
//     blen = 0;
// #endif
//   }
//   ADD("</pre>");

// #if WEBSERVER_CONF_FILESTATS
//   static uint16_t numtimes;
//   ADD("<br><i>This page sent %u times</i>",++numtimes);
// #endif

// #if WEBSERVER_CONF_LOADTIME
//   numticks = clock_time() - numticks + 1;
//   ADD(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND))/CLOCK_SECOND));
// #endif

//   SEND_STRING(&s->sout, buf);
//   SEND_STRING(&s->sout, BOTTOM);

//   PSOCK_END(&s->sout);
// }
// /*---------------------------------------------------------------------------*/
// httpd_simple_script_t
// httpd_simple_get_script(const char *name)
// {

//   return generate_routes;
// }

// #endif /* WEBSERVER */

//Skip simple web process, go straight to border router
//Delete the line below and uncomment the block above to run a simple webpage
AUTOSTART_PROCESSES(&border_router_process);
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;
  printf("Test [1]: print_local_addresses\n");
  PRINTA("Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINTA(" ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTA("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
void
request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
//  PRINTF("Test [0]: I think this function is executed first!!!!!")
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
  uip_len = 2;
  slip_send();
  uip_clear_buf(); //empty the uip_buf, similar to the code uip_len = 0
}
/*---------------------------------------------------------------------------*/
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
//  printf("\n 11111111111111111111111111");
  rpl_dag_t *dag;
  uip_ipaddr_t ipaddr;
  memcpy(&prefix, prefix_64, 16);
  memcpy(&ipaddr, prefix_64, 16);
  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
 uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

 dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
 if(dag != NULL) {
  rpl_set_prefix(dag, &prefix, 64);
  PRINTF("created a new RPL dag\n");
 }
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
net_init(uip_ipaddr_t *br_prefix)
{
  uip_ipaddr_t global_ipaddr;

  if(br_prefix) { /* We are RPL root. Will be set automatically
                     as TSCH pan coordinator via the tsch-rpl module */
    memcpy(&global_ipaddr, br_prefix, 16);
    uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
    rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
    rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
    rpl_repair_root(RPL_DEFAULT_INSTANCE);
  }

  NETSTACK_MAC.on();
}
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

//  printf("\n 222222222222222222222222");
/* While waiting for the prefix to be sent through the SLIP connection, the future
 * border router can join an existing DAG as a parent or child, or acquire a default
 * router that will later take precedence over the SLIP fallback interface.
 * Prevent that by turning the radio off until we are initialized as a DAG root.
 */
  prefix_set = 0;
  NETSTACK_MAC.off(0); //turn off both the MAC and radio signal

  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

  PRINTF("RPL-Border router started\n");
#if 0
   /* The border router runs with a 100% duty cycle in order to ensure high
     packet reception rates.
     Note if the MAC RDC is not turned off now, aggressive power management of the
     cpu will interfere with establishing the SLIP connection */
  NETSTACK_MAC.off(1);
#endif

  /* Request prefix until it has been received */
  while(!prefix_set) {
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  /* Now turn the radio on, but disable radio duty cycling.
   * Since we are the DAG root, reception delays would constrain mesh throughbut.
   */
//  NETSTACK_MAC.off(1); //turn MAC layer off but still keep the radio signal always on

//Newly added
net_init(&prefix);

#if DEBUG || 1
  print_local_addresses();
#endif

  while(1) {
    PROCESS_YIELD();
    if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiating global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-schedule.h"
#include "net/netstack.h"
#include "dev/slip.h"
#include "button-sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_NONE
#include "net/ip/uip-debug.h"

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

PROCESS(border_router_process, "Border router process");



//Skip simple web process, go straight to border router
AUTOSTART_PROCESSES(&border_router_process);
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;
  printf("Test [1]: print_local_addresses\n");
  PRINTA("Server IPv6 addresses:\n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINTA(" ");
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTA("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
void
request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
//  PRINTF("Test [0]: I think this function is executed first!!!!!")
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
  uip_len = 2;
  slip_send();
  uip_clear_buf(); //empty the uip_buf, similar to the code uip_len = 0
}
/*---------------------------------------------------------------------------*/
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
//  printf("\n 11111111111111111111111111");
  rpl_dag_t *dag;
  uip_ipaddr_t ipaddr;
  memcpy(&prefix, prefix_64, 16);
  memcpy(&ipaddr, prefix_64, 16);
  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
 uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

 dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
 if(dag != NULL) {
  rpl_set_prefix(dag, &prefix, 64);
  PRINTF("created a new RPL dag\n");
 }
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
net_init(uip_ipaddr_t *br_prefix)
{
  uip_ipaddr_t global_ipaddr;

  if(br_prefix) { /* We are RPL root. Will be set automatically
                     as TSCH pan coordinator via the tsch-rpl module */
    memcpy(&global_ipaddr, br_prefix, 16);
    uip_ds6_set_addr_iid(&global_ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&global_ipaddr, 0, ADDR_AUTOCONF);
    rpl_set_root(RPL_DEFAULT_INSTANCE, &global_ipaddr);
    rpl_set_prefix(rpl_get_any_dag(), br_prefix, 64);
    rpl_repair_root(RPL_DEFAULT_INSTANCE);
  }

  NETSTACK_MAC.on();
}
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

//  printf("\n 222222222222222222222222");
/* While waiting for the prefix to be sent through the SLIP connection, the future
 * border router can join an existing DAG as a parent or child, or acquire a default
 * router that will later take precedence over the SLIP fallback interface.
 * Prevent that by turning the radio off until we are initialized as a DAG root.
 */
  prefix_set = 0;
  NETSTACK_MAC.off(0); //turn off both the MAC and radio signal

  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

  PRINTF("RPL-Border router started\n");
#if 0
   /* The border router runs with a 100% duty cycle in order to ensure high
     packet reception rates.
     Note if the MAC RDC is not turned off now, aggressive power management of the
     cpu will interfere with establishing the SLIP connection */
  NETSTACK_MAC.off(1);
#endif

  /* Request prefix until it has been received */
  while(!prefix_set) {
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  /* Now turn the radio on, but disable radio duty cycling.
   * Since we are the DAG root, reception delays would constrain mesh throughbut.
   */
//  NETSTACK_MAC.off(1); //turn MAC layer off but still keep the radio signal always on

//Newly added
net_init(&prefix);

#if DEBUG || 1
  print_local_addresses();
#endif

  while(1) {
    PROCESS_YIELD();
    if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiating global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

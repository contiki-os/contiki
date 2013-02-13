#include "net/uip-nd6.h"
#include "net/uip-ds6.h"
#include "net/uip-ds6-route.h"
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/uip.h"
#include "net/netstack.h"
#include "net/rpl/rpl.h"

#include "sicslow-ethernet.h"
#include "cetic-bridge.h"
#include "nvm-config.h"
#include "rio.h"
#include "node-info.h"

#if CONTIKI_TARGET_ECONOTAG
#include "mc1322x.h"
#include "contiki-maca.h"
#endif

#include <stdio.h> /* For printf() */
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

extern uip_ds6_nbr_t uip_ds6_nbr_cache[];
extern uip_ds6_prefix_t uip_ds6_prefix_list[];

extern rpl_instance_t instance_table[RPL_MAX_INSTANCES];

int count = 0;
/*---------------------------------------------------------------------------*/
/* Use simple webserver with only one page for minimum footprint.
 * Multiple connections can result in interleaved tcp segments since
 * a single static buffer is used for all segments.
 */
#include "httpd-simple.h"
/* The internal webserver can provide additional information if
 * enough program flash is available.
 */
#define WEBSERVER_CONF_LOADTIME 0
#define WEBSERVER_CONF_FILESTATS 0

#define WEBSERVER_CONF_INFO 1
#define WEBSERVER_CONF_RPL 0
#define WEBSERVER_CONF_NETWORK 1
#define WEBSERVER_CONF_CONFIG 1

#define BUF_USES_STACK 0
/*---------------------------------------------------------------------------*/
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
	PROCESS_BEGIN();

	httpd_init();

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
		httpd_appcall(data);
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#define BUF_SIZE 256
static const char *TOP = "<html><head><title>CETIC WSN</title><link rel=\"stylesheet\" type=\"text/css\" href=\"http://www.cetic.be/squelettes/css/cetic_layout.css\" /><link rel=\"stylesheet\" href=\"http://www.cetic.be/prive/spip_style.css\" type=\"text/css\" /></head>";
static const char *BODY = "<body class=\"page_rubrique\"><div id=\"container\"><div id=\"banner\"><img src=\"http://www.cetic.be/squelettes/css/img/logo.jpg\" style=\"position: absolute; top: 0; left: 0;\"><div id=\"banner_fx\"><div id=\"img_bandeau\"><img src=\"http://www.cetic.be/squelettes/img/banner_home.jpg\"></div><div id=\"barre_nav\"><div class=\"menu-general\"><a href=\"/\">Info</a></div></div></div></div>\n";
static const char *BOTTOM = "</div></body></html>\n";
#if BUF_USES_STACK
static char *bufptr, *bufend;
#else
static char buf[BUF_SIZE];
static int blen;
#endif
/*---------------------------------------------------------------------------*/
/*OPTIMIZATIONS to gain space : prototypes*/
static void add(char *str,...);
void add_div_home();
void add_network_cases(const uint8_t state);
static void reset_buf();
/*End optimizations*/
/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
	uint16_t a;
	int i, f;
	for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
		a = (addr->u8[i] << 8) + addr->u8[i + 1];
		if(a == 0 && f >= 0) {
			if(f++ == 0) add("::");
		} else {
			if(f > 0) {
				f = -1;
			} else if(i > 0) {
				add(":");
			}
			add("%x", a);
		}
	}
}
static void
lladdr_add(const uip_lladdr_t *addr)
{
	int i;
	for(i = 0; i < sizeof(uip_lladdr_t); i++) {
		if(i > 0) {
			add(":");
		}
		add("%x", addr->addr[i]);
	}
}
static void
ethaddr_add(ethaddr_t *addr)
{
	int i;
	for(i = 0; i < 6; i++) {
		if(i > 0) {
			add(":");
		}

		add("%x", (*addr)[i]);
	}
}
/*---------------------------------------------------------------------------*/

#if CONTIKI_TARGET_ECONOTAG
extern void _start;
//Code
extern void _etext;
//RO data
extern void __data_start;
//Initialised data
extern void _edata;
//Stack
extern void __bss_start;
//Zero initialised data
extern void _bss_end__;
//Heap
extern void _end;
#endif

/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_index(struct httpd_state *s))
{
	static int i;
	static int j;
	static uip_ds6_route_t *r;
	static uip_ds6_defrt_t *dr;
#if BUF_USES_STACK
	char buf[BUF_SIZE];
#endif
#if WEBSERVER_CONF_LOADTIME
	static clock_time_t numticks;
	numticks = clock_time();
#endif

	PSOCK_BEGIN(&s->sout);

	SEND_STRING(&s->sout, TOP);
	SEND_STRING(&s->sout, BODY);
	reset_buf();
    add("<div id=\"intro_home\"><h1>CETIC Wireless Sensor Network</h1></div>");
	add("<div id=\"left_home\">");
#if WEBSERVER_CONF_INFO
	add("<h2>Info</h2>");
	add("Version : " CONTIKI_VERSION_STRING "<br />");
	add("Mode : ");
#if CETIC_6LBR_SMARTBRIDGE
	add("SMART BRIGDE");
#endif
#if CETIC_6LBR_TRANSPARENTBRIDGE
	add("TRANSPARENT BRIGDE");
#endif
#if CETIC_6LBR_ROUTER
	add("ROUTER");
#endif
	add("<br />\n");
	i = clock_time()/CLOCK_SECOND;
	add("Uptime : %dh %dm %ds<br />", i/3600, (i/60) % 60, i % 60);
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("<br /><h2>WSN</h2>");
	add("MAC: %s<br />RDC: %s (%d Hz)<br />",
			NETSTACK_MAC.name,
			NETSTACK_RDC.name,
			CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0? 1:
			                NETSTACK_RDC.channel_check_interval()));
#if !CETIC_6LBR_TRANSPARENTBRIDGE
	add("Prefix : ");
	ipaddr_add(&cetic_dag->prefix_info.prefix);
	add("/%d", cetic_dag->prefix_info.length);
#endif
	add("<br />");
	add("HW address : ");
	lladdr_add(&uip_lladdr);
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("<br /><h2>Ethernet</h2>");
#if CETIC_6LBR_ROUTER
	add("Address : ");
	ipaddr_add(&eth_ip_addr);
	add("<br />");
	add("Local address : ");
	ipaddr_add(&eth_ip_local_addr);
	add("<br />");
#endif
	add("HW address : ");
	ethaddr_add(&eth_mac_addr);
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if CONTIKI_TARGET_ECONOTAG
	add("<br /><h2>Memory</h2>");

	add("Global : %d (%d %%)<br /><br />", &_end - &_start, (100 * (&_end - &_start)) / (96*1024) );

	add("Code : %d<br />", &_etext - &_start );
	add("Initialised data : %d<br /><br />", &_edata - &_etext );
	add("Data : %d<br />", &_bss_end__ - &__bss_start );
	add("Stack : %d<br />", &__bss_start - &_edata );
	add("Heap : %d<br />", &_end - &_bss_end__ );
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif
#endif

#if WEBSERVER_CONF_RPL
	add("<h2>Configuration</h2>");
	add("Lifetime : %d (%d x %d s)<br />",
		RPL_CONF_DEFAULT_LIFETIME * RPL_CONF_DEFAULT_LIFETIME_UNIT,
		RPL_CONF_DEFAULT_LIFETIME, RPL_CONF_DEFAULT_LIFETIME_UNIT);
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();
	for(i = 0; i < RPL_MAX_INSTANCES; ++i) {
		if ( instance_table[i].used ) {
			add("<h2>Instance %d</h2>", instance_table[i].instance_id);
			for(j = 0; j < RPL_MAX_DAG_PER_INSTANCE; ++j) {
				if ( instance_table[i].dag_table[j].used ) {
					add("<h3>DODAG %d</h3>", j);
					add("DODAG ID : ");
					ipaddr_add(&instance_table[i].dag_table[j].dag_id);
					add("<br />Version : %d", instance_table[i].dag_table[j].version);
					add("<br />Grounded : %s", instance_table[i].dag_table[j].grounded ? "Yes" : "No");
					add("<br />Preference : %d", instance_table[i].dag_table[j].preference);
					add("<br />Mode of Operation : %u", instance_table[i].mop);
					add("<br />Current DIO Interval [%u-%u] : %u", instance_table[i].dio_intmin, instance_table[i].dio_intmin + instance_table[i].dio_intdoubl, instance_table[i].dio_intcurrent);
					add("<br />Objective Function Code Point : %u", instance_table[i].of->ocp);
					add("<br />Joined : %s", instance_table[i].dag_table[j].joined ? "Yes" : "No");
					add("<br />Rank : %d", instance_table[i].dag_table[j].rank);
					add("<br />");
					SEND_STRING(&s->sout, buf);
					reset_buf();
				}
			}
		}
	}
	add("<br /><h3>Actions</h3>");
	add("<a href=\"rpl-gr\">Trigger global repair</a><br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif

#if WEBSERVER_CONF_NETWORK
	add("<br /><h2>Addresses</h2><pre>");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	for (i=0;i<UIP_DS6_ADDR_NB;i++) {
		if (uip_ds6_if.addr_list[i].isused) {
			ipaddr_add(&uip_ds6_if.addr_list[i].ipaddr);
			char flag;
			if ( uip_ds6_if.addr_list[i].state == ADDR_TENTATIVE ) flag ='T';
			else if ( uip_ds6_if.addr_list[i].state == ADDR_PREFERRED ) flag ='P';
			else flag = '?';
			add( " %c", flag);
			if ( uip_ds6_if.addr_list[i].type == ADDR_MANUAL ) flag ='M';
			else if ( uip_ds6_if.addr_list[i].type == ADDR_DHCP ) flag ='D';
			else if ( uip_ds6_if.addr_list[i].type == ADDR_AUTOCONF ) flag = 'A';
			else flag = '?';
			add( " %c", flag);
            if ( ! uip_ds6_if.addr_list[i].isinfinite ) {
            	add(" %u s", stimer_remaining(&uip_ds6_if.addr_list[i].vlifetime));
            }
			add("\n");
			SEND_STRING(&s->sout, buf);
			reset_buf();
		}
	}

	add("</pre><h2>Prefixes</h2><pre>");
	for (i=0;i<UIP_DS6_PREFIX_NB;i++) {
		if (uip_ds6_prefix_list[i].isused) {
			ipaddr_add(&uip_ds6_prefix_list[i].ipaddr);
			add(" ");
#if UIP_CONF_ROUTER
			if (uip_ds6_prefix_list[i].advertise) add("*");
#else
			if (uip_ds6_prefix_list[i].isinfinite) add("I");
#endif
			add("\n");
		}
	}
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("</pre><h2>Neighbors</h2><pre>");
	for(i = 0; i < UIP_DS6_NBR_NB; i++) {
		if(uip_ds6_nbr_cache[i].isused) {
			ipaddr_add(&uip_ds6_nbr_cache[i].ipaddr);
			add(" ");
			lladdr_add(&uip_ds6_nbr_cache[i].lladdr);
			add(" ");
			add_network_cases(uip_ds6_nbr_cache[i].state);
			add("\n");
			SEND_STRING(&s->sout, buf);
			reset_buf();
		}
	}
	add("</pre><h2>Routes</h2><pre>");
	SEND_STRING(&s->sout, buf);
	reset_buf();
	for(r = uip_ds6_route_list_head(), i=0; r != NULL; r = list_item_next(r), ++i) {
		ipaddr_add(&r->ipaddr);
		add("/%u (via ", r->length);
		ipaddr_add(&r->nexthop);
		if(1 || (r->state.lifetime < 600)) {
			add(") %lu s\n", r->state.lifetime);
		} else {
			add(")\n");
		}
		SEND_STRING(&s->sout, buf);
		reset_buf();
	}

	add("</pre><h2>Default Routers</h2><pre>");

	for(dr = uip_ds6_defrt_list_head(); dr != NULL; dr = list_item_next(r)) {
		ipaddr_add(&dr->ipaddr);
		if ( ! dr->isinfinite ) {
			add(" %u s", stimer_remaining(&dr->lifetime));
		}
		add("\n");
		SEND_STRING(&s->sout, buf);
		reset_buf();
	}

#if UIP_CONF_DS6_ROUTE_INFORMATION
	add("</pre><h2>Route info</h2><pre>");
	for (i=0;i<UIP_DS6_ROUTE_INFO_NB;i++) {
		if (uip_ds6_route_info_list[i].isused) {
			ipaddr_add(&uip_ds6_route_info_list[i].ipaddr);
			add("/%u (%x) %us\n", uip_ds6_route_info_list[i].length, uip_ds6_route_info_list[i].flags, uip_ds6_route_info_list[i].lifetime);
		}
	}
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif

#if CETIC_6LBR_TRANSPARENTBRIDGE
	add("</pre><h2>HW Prefixes cache</h2><pre>");
	for (i=0;i<prefixCounter;i++) {
		add("%02x:%02x:%02x\n", prefixBuffer[i][0], prefixBuffer[i][1], prefixBuffer[i][2]);
	}
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif
	add("</pre><br />");
#endif

#if WEBSERVER_CONF_CONFIG
	add("<h2>WSN Network</h2>");
	add("<h3>WSN configuration</h3>");
	add("Channel : %d<br />", nvm_data.channel);
#if CONTIKI_TARGET_ECONOTAG
	add("PanID : 0x%x<br />", *MACA_MACPANID);
#endif
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

    add("<h3>IP configuration</h3>");
#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	add("Network configuration : ");
	if ((nvm_data.mode & CETIC_MODE_WAIT_RA_MASK) != 0 ) {
		add(" autoconfiguration<br />");
	} else {
		add(" static<br />");
		add("Prefix : ");
		ipaddr_add(&wsn_net_prefix);
		add("<br />");
	}
#else
	add("Prefix : ");
	ipaddr_add(&wsn_net_prefix);
	add("<br />");
#endif
	add("Address : ");
	if ((nvm_data.mode & CETIC_MODE_WSN_AUTOCONF) != 0 ) {
		add("autoconfiguration");
	} else {
		ipaddr_add(&wsn_ip_addr);
	}
	add("<br />");
#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	add("Default router : ");
	ipaddr_add(&eth_dft_router);
	add("<br />");
#endif
	SEND_STRING(&s->sout, buf);
	reset_buf();

#if CETIC_6LBR_ROUTER
	add("<br /><h2>Eth Network</h2>");
	add("<h3>IP configuration</h3>");
	add("Prefix : ");
	ipaddr_add(&eth_net_prefix);
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();

	add("Address : ");
	ipaddr_add(&eth_ip_addr);
	add("<br />");
	add("Peer router : ");
	ipaddr_add(&eth_dft_router);
	add("<br />");
	add("RA Daemon : ");
	if ((nvm_data.mode & CETIC_MODE_ROUTER_SEND_CONFIG) != 0 ) {
		add("active (Lifetime : %d)", UIP_CONF_ROUTER_LIFETIME);
	} else {
		add("inactive");
	}
	add("<br />");
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif
#if CETIC_6LBR_SMARTBRIDGE || CETIC_6LBR_TRANSPARENTBRIDGE
	add("<br /><h3>Packet filtering</h3>");
	add("RPL filtering : ");
	if ((nvm_data.mode & CETIC_MODE_FILTER_RPL_MASK) != 0 ) {
		add( "enabled<br />");
	} else {
		add( "disabled<br />");
	}
#endif
#if CETIC_6LBR_ROUTER
	add("<br /><h3>Packet filtering</h3>");
	add("Address rewrite : ");
	if ((nvm_data.mode & CETIC_MODE_REWRITE_ADDR_MASK) != 0 ) {
		add( "enabled<br />");
	} else {
		add( "disabled<br />");
	}
#endif
	add("<br />\n");
	SEND_STRING(&s->sout, buf);
	reset_buf();
#endif

#if WEBSERVER_CONF_FILESTATS
	static uint16_t numtimes;
	add("<br><i>This page sent %u times</i>",++numtimes);
#endif

#if WEBSERVER_CONF_LOADTIME
	numticks = clock_time() - numticks + 1;
	add(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND)/CLOCK_SECOND));
#endif
	add("</div>");
	SEND_STRING(&s->sout, buf);
	SEND_STRING(&s->sout, BOTTOM);

	PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
	static uip_ds6_route_t *r;
	static int i;

	if (strcmp(name, "index.html") == 0 || strcmp(name, "") == 0) {
		return generate_index;
	} else {
		return NULL;
	}
}
/*---------------------------------------------------------------------------*/
void
add_network_cases(const uint8_t state)
{
    switch (state) {
    case NBR_INCOMPLETE: add("I");break;
	case NBR_REACHABLE: add("R");break;
	case NBR_STALE: add("S");break;
	case NBR_DELAY: add("D");break;
	case NBR_PROBE: add("P");break;
	}
}
/*---------------------------------------------------------------------------*/
/*Macro redefined : RESET_BUF()*/
/*---------------------------------------------------------------------------*/
static void
reset_buf()
{
#if BUF_USES_STACK
    bufptr = buf;
    bufend = bufptr + sizeof(buf);
#else
    blen = 0;
#endif
}
/*---------------------------------------------------------------------------*/
/*Macro redefined : ADD()*/
/*---------------------------------------------------------------------------*/
static void
add(char *str, ...)
{
    va_list arg; 
    va_start (arg, str);
    //ADD(str, arg); //TODO : bug while formating
#if BUF_USES_STACK
    bufptr += vsnprintf(bufptr, bufend - bufptr, str, arg);
#else
    blen += vsnprintf(&buf[blen], sizeof(buf) - blen, str, arg);
#endif
    va_end (arg);
}
/*---------------------------------------------------------------------------*/

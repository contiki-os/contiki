#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-udp-packet.h"
#include "sys/ctimer.h"
#ifdef WITH_COMPOWER
#include "powertrace.h"
#endif
#include <stdio.h>
#include <string.h>

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"
//#include "dev/battery-sensor.h"
//#include "dev/i2cmaster.h"
//#include "dev/light-ziglet.h"
#include "dev/sht11.h"

#ifndef PERIOD
#define PERIOD 60
#endif

#define START_INTERVAL		(15 * CLOCK_SECOND)
#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))
#define MAX_PAYLOAD_LEN		30

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;
//static uint16_t light;
static unsigned rh;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void tcpip_handler(void) {
	char *str;
	printf("\n");
	if (uip_newdata()) {
		str = uip_appdata;
		str[uip_datalen()] = '\0';
		printf("DATA recv '%s'\n", str);
	}
}
/*---------------------------------------------------------------------------*/
float floor(float x) {
	if (x >= 0.0f) {
		return (float) ((int) x);
	} else {
		return (float) ((int) x - 1);
	}
}
/*---------------------------------------------------------------------------*/
static void send_packet(void *ptr) {
//	static int seq_id;
	char buf[MAX_PAYLOAD_LEN];

//	seq_id++;
//	PRINTF("DATA send to %d 'Hello %d'\n",
//			server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
//	sprintf(buf, "Hello %d from the client", seq_id);
//	uip_udp_packet_sendto(client_conn, buf, strlen(buf), &server_ipaddr,
//			UIP_HTONS(UDP_SERVER_PORT));


//	bateria sensor
//	uint16_t bateria = battery_sensor.value(0);
//	float mv = (bateria * 2.500 * 2) / 4096;
//	PRINTF("DATA send to %d: %i (%ld.%03d mV)",
//			server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1],
//			bateria,
//			(long) mv,
//			(unsigned) ((mv - floor(mv)) * 1000));

//	sprintf(buf, "BATERIA: %i (%ld.%03d mV)", bateria, (long) mv,
//			(unsigned) ((mv - floor(mv)) * 1000));

//	light sensor
//	light = light_ziglet_read();
//	sprintf(buf, "LIGHT: %u", light);


//	temperature and humidity sensor
    rh = sht11_humidity();
	sprintf(buf, "humidity: %u%%",
			(unsigned) (-4 + 0.0405 * rh - 2.8e-6 * (rh * rh)));

	uip_udp_packet_sendto(client_conn, buf, strlen(buf), &server_ipaddr,
			UIP_HTONS(UDP_SERVER_PORT));
}
/*---------------------------------------------------------------------------*/
static void print_local_addresses(void) {
	int i;
	uint8_t state;

	PRINTF("Client IPv6 addresses: ");
	for (i = 0; i < UIP_DS6_ADDR_NB; i++) {
		state = uip_ds6_if.addr_list[i].state;
		if (uip_ds6_if.addr_list[i].isused
				&& (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
			PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);PRINTF("\n");
			/* hack to make address "final" */
			if (state == ADDR_TENTATIVE) {
				uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
			}
		}
	}
}
/*---------------------------------------------------------------------------*/
static void set_global_address(void) {
	uip_ipaddr_t ipaddr;

	uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
	uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

	/* The choice of server address determines its 6LoPAN header compression.
	 * (Our address will be compressed Mode 3 since it is derived from our link-local address)
	 * Obviously the choice made here must also be selected in udp-server.c.
	 *
	 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
	 * e.g. set Context 0 to aaaa::.  At present Wireshark copies Context/128 and then overwrites it.
	 * (Setting Context 0 to aaaa::1111:2222:3333:4444 will report a 16 bit compressed address of aaaa::1111:22ff:fe33:xxxx)
	 *
	 * Note the IPCMV6 checksum verification depends on the correct uncompressed addresses.
	 */

#if 0
	/* Mode 1 - 64 bits inline */
	uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
#elif 1
	/* Mode 2 - 16 bits inline */
	uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
	/* Mode 3 - derived from server link-local (MAC) address */
	uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0x0250, 0xc2ff, 0xfea8, 0xcd1a); //redbee-econotag
#endif
}
//---------------------------------------------------------------------------//
PROCESS_THREAD(udp_client_process, ev, data) {
	static struct etimer periodic;
	static struct ctimer backoff_timer;
#if WITH_COMPOWER
	static int print = 0;
#endif

	PROCESS_BEGIN()
		;
//		SENSORS_ACTIVATE(battery_sensor);


//	  light_ziglet_init();
	  sht11_init();

		PROCESS_PAUSE();

		set_global_address();

		PRINTF("UDP client process started\n");

		print_local_addresses();

		/* new connection with remote host */
		client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL);
		if (client_conn == NULL) {
			PRINTF("No UDP connection available, exiting the process!\n");
			PROCESS_EXIT();
		}
		udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));

		PRINTF("Created a connection with the server ");
		PRINT6ADDR(&client_conn->ripaddr);
		PRINTF(" local/remote port %u/%u\n",
				UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

#if WITH_COMPOWER
		powertrace_sniff(POWERTRACE_ON);
#endif

		etimer_set(&periodic, SEND_INTERVAL);
		while (1) {
			PROCESS_YIELD();
			if (ev == tcpip_event) {
				tcpip_handler();
			}

			if (etimer_expired(&periodic)) {
				etimer_reset(&periodic);
				ctimer_set(&backoff_timer, SEND_TIME, send_packet, NULL);

#if WITH_COMPOWER
				if (print == 0) {
					powertrace_print("#P");
				}
				if (++print == 3) {
					print = 0;
				}
#endif

			}
		}
//		SENSORS_DEACTIVATE(battery_sensor);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

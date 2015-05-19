/* Soil Moisture Source Code for CC2538 version microcontroller
 * Uses the mesh networking functunality of the RIME stack
 * Uses ContikiMAC for radio duty cycling
 */


//Included files
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/watchdog.h"
//#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "lpm.h"
//#include "usb-serial.h"
#include "dev/uart.h"

#include "random.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/adc-sensor.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"

//Linker definitions
#ifndef PERIOD
#define PERIOD 10
#endif

#define false 0
#define true 1

#define SEND_INTERVAL	(MeasurementPeriod * CLOCK_SECOND)

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define MAX_PAYLOAD_LEN		150

//Global Variables
float Temperature, Voltage, A0;
uint8_t DigitalInput1 = false;

int16_t rv, dec;
float frac;

static struct etimer periodic;
static struct ctimer backoff_timer;

static char weather_rx_buffer[300];
static int weather_rx_buffer_index = 0;

static char StringBuffer[MAX_PAYLOAD_LEN];

unsigned char channel = 25;
unsigned int MeasurementPeriod = 15;			//Default measurement period = 1 min
char TimeoutCount = 0;
bool WaitingResponse;

static char temperature[10];
static char humidity[10];
static char dewPoint[10];
static char pressure[10];
static char windDirection[10];
static char windSpeed[10];
static char solarRadiation[10];
static char batteryVoltage[10];
static char solarVoltage[10];
static char rainLastMin[10];
static char rainLastHour[10];
static char peakWind[10];

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

/*---------------------------- Contiki Processes ----------------------------*/
PROCESS(example_mesh_process, "Mesh example");
PROCESS(weather_serial_process, "Serial");

AUTOSTART_PROCESSES(&example_mesh_process,&weather_serial_process);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

void ExtractDataFromweather_String(void)
{
	char Command;
	//printf("->%s\r",weather_rx_buffer);
	
	Command = weather_rx_buffer[0];    
    if (Command == '*')
	{
		
	}
	else if (Command == 'A') {
		//Extract temperature value from string		
		strncpy(temperature, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",temperature);
	}
	else if (Command == 'B') {
		//Extract humidity value from string		
		strncpy(humidity, weather_rx_buffer + 2, 10);
//printf("!>%s\r",humidity);		
	}
	else if (Command == 'C') {
		//Extract dew point value from string		
		strncpy(dewPoint, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",dewPoint);
	}
	else if (Command == 'D') {
		//Extract pressure value from string		
		strncpy(pressure, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",pressure);
	}
	else if (Command == 'E') {
		//Extract wind direction value from string		
		strncpy(windDirection, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",windDirection);
	}
	else if (Command == 'F') {
		//Extract wind speed value from string		
		strncpy(windSpeed, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",windSpeed);
	}
	else if (Command == 'G') {
		//Extract solar radiation value from string		
		strncpy(solarRadiation, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",solarRadiation);
	}
	else if (Command == 'H') {
		//Extract battery voltage value from string		
		strncpy(batteryVoltage, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",batteryVoltage);
	}
	else if (Command == 'I') {
		//Extract solar panel voltage value from string		
		strncpy(solarVoltage, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",solarVoltage);
	}
	else if (Command == 'J') {
		//Extract last minutes rain value from string		
		strncpy(rainLastMin, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",rainLastMin);
	}
	else if (Command == 'K') {
		//Extract last hour rain value from string		
		strncpy(rainLastHour, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",rainLastHour);
	}
	else if (Command == 'L') {
		//Extract peak wind value from string		
		strncpy(peakWind, weather_rx_buffer + 2, 10); 
		//printf("!>%s\r",rainLastHour);
		
		printf("Temp=%s,Humi=%s,Dew=%s,HPa=%s,Dir=%sm,Spd=%s,Rad=%s,Bat=%s,Sol=%s,Rain Acc=%s,Rain Last H=%s, Peak Wind=%s\r", temperature, humidity, 
			dewPoint, pressure, windDirection, windSpeed, solarRadiation, batteryVoltage, solarVoltage, rainLastMin, rainLastHour, peakWind);
	}
	else
	{
	
	}	
}

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
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

int WeatherSerialDataCallback(unsigned char c)
{
	if (c == 0)
	{
		weather_rx_buffer_index = 0;
		return 0;
	}
	
	if (c == '\r')
	{
		//printf("%s",weather_rx_buffer);
		ExtractDataFromweather_String();
		weather_rx_buffer_index = 0;
		weather_rx_buffer[weather_rx_buffer_index] = 0;			//Null terminate the string
	}
	else
	{
		//Add c to uart buffer
		weather_rx_buffer[weather_rx_buffer_index++] = c;
		weather_rx_buffer[weather_rx_buffer_index] = 0;			//Null terminate the string
	}
	
	if (weather_rx_buffer_index > 299)
	{
		weather_rx_buffer_index = 0;
		weather_rx_buffer[weather_rx_buffer_index] = 0;			//Null terminate the string
	}
	
	return 1;
}

void delay_msec(int time)
{
	while (time > 0)
	{
		clock_delay_usec(1000);
		time--;
		watchdog_periodic();
	}	
}

/*---------------------------------------------------------------------------*/
void send_message(void* ptr)
{
	static int seq_id;
	printf("Prepare Packet 1\r");
	sprintf(StringBuffer, "WE,Temp=%s,Humi=%s,Dew=%s,HPa=%s,Dir=%s,Spd=%s,Rad=%s,", temperature, humidity, dewPoint, pressure, windDirection, windSpeed, solarRadiation);
	sprintf(StringBuffer, "%sBat=%s,Sol=%s,Rain Acc=%s,Rain Last H=%s,Peak Wind=%s\r", StringBuffer, batteryVoltage, solarVoltage, rainLastMin, rainLastHour,peakWind);
				
	printf("%s\r",StringBuffer);

	printf("Sending message\r");
   
	seq_id++;
	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
    uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	
	
}

/*---------------------------------------------------------------------------*/
/*
void send_message2(void* ptr)
{
	static int seq_id;
	printf("Prepare Packet 2\r");
	sprintf(StringBuffer, "WE,Bat=%s,Sol=%s,Rain Acc=%s,Rain Last H=%s,Peak Wind=%s\r", batteryVoltage, solarVoltage, rainLastMin, rainLastHour,peakWind);
			
	printf("%s\r",StringBuffer);

	printf("Sending message\r");

    printf("Sending\n");
    
	seq_id++;
	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
    uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	
	
}
*/
static void print_route_network(void)
{
	char StringBuffer[MAX_PAYLOAD_LEN];				//Buffer for building the string of readings sent over the wireless network
	rpl_dag_t *dag;
	uip_ds6_route_t *r;
	uip_ipaddr_t *addr;
	static int seq_id;
	
	dag = rpl_get_any_dag();
	PRINTF("+++\n");
	addr = &uip_ds6_if.addr_list[2].ipaddr;
	sprintf(StringBuffer,"!+++\r!N=%02x%02x::%02x%02x:%02x%02x:%02x%02x:%02x%02x\r", ((char *)addr)[0], ((char *)addr)[1], ((char *)addr)[8], ((char *)addr)[9], ((char *)addr)[10], ((char *)addr)[11], ((char *)addr)[12], ((char *)addr)[13], ((char *)addr)[14], ((char *)addr)[15]);
	if(dag->preferred_parent != NULL) {
	// send the prefered parent route as a packet
	addr = rpl_get_parent_ipaddr(dag->preferred_parent);			
	sprintf(StringBuffer,"%s!P=%02x%02x::%02x%02x:%02x%02x:%02x%02x:%02x%02x\r", StringBuffer, ((char *)addr)[0], ((char *)addr)[1], ((char *)addr)[8], ((char *)addr)[9], ((char *)addr)[10], ((char *)addr)[11], ((char *)addr)[12], ((char *)addr)[13], ((char *)addr)[14], ((char *)addr)[15]);
			PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
		PRINTF("\n");
	seq_id++;
	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
    uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	}
	for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
		// Display the addresses of the nodes for which we are the parent
		addr = &r->ipaddr;			
		sprintf(StringBuffer,"!R=%02x%02x::%02x%02x:%02x%02x:%02x%02x:%02x%02x\r", ((char *)addr)[0], ((char *)addr)[1], ((char *)addr)[8], ((char *)addr)[9], ((char *)addr)[10], ((char *)addr)[11], ((char *)addr)[12], ((char *)addr)[13], ((char *)addr)[14], ((char *)addr)[15]);
		PRINT6ADDR(&r->ipaddr);
		PRINTF("\n");
		seq_id++;
		PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
		uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	}
	PRINTF("---\n");
	sprintf(StringBuffer,"!---\r");
	PRINT6ADDR(&r->ipaddr);
	PRINTF("\n");
	seq_id++;
	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
	uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}

static void
tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    printf("DATA recv '%s'\n", str);
	
	if (strstr(str, "?") > 0) {					
			print_route_network();				//Send list of routes over network
		}
  }
}


/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
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

//This is the main Current Loop process.
//Here is where we set up I/O, timers and kick of the network processes
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_mesh_process, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_PAUSE();

   set_global_address();
  
  PRINTF("UDP client process started\n");

  print_local_addresses();

  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
  if(client_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT)); 

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

	
	etimer_set(&periodic, SEND_INTERVAL);
	watchdog_periodic();
	
	//Enable high gain mode on cc2592
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(2));			
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(2));
  while(1) {
	PROCESS_YIELD();
	if(ev == tcpip_event) {
      tcpip_handler();
    }
	
		if(etimer_expired(&periodic)) 
		{

			etimer_set(&periodic, SEND_INTERVAL);
			watchdog_periodic();
			ctimer_set(&backoff_timer, (5 * CLOCK_SECOND), send_message, NULL);					//send second message		
			watchdog_periodic();
	    }
    

  }
  PROCESS_END();
}
  
PROCESS_THREAD(weather_serial_process, ev, data)
{
	PROCESS_BEGIN();

	uart_set_input(1, WeatherSerialDataCallback);


	while(1) {
		PROCESS_YIELD();
	}
	PROCESS_END();
}

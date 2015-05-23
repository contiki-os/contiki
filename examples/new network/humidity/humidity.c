/* Knob Source Code for CC2538 version microcontroller
 * Uses the mesh networking functunality of the RIME stack
 * Uses ContikiMAC for radio duty cycling
 */


//Included files
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/watchdog.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "lpm.h"
#include "dev/uart.h"
#include "dev/adc-sensor.h"
#include <stdint.h>
#include "cpu.h"
#include "dev/cc2538-rf.h"
#include "net/netstack.h"
#include "random.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/adc-sensor.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"
#include "dev/sht11.h"

#define false 0
#define true 1

#define SEND_INTERVAL	(MeasurementPeriod * CLOCK_SECOND)			//This is the rate at which data packets are sent

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define MAX_PAYLOAD_LEN		150

#define DEBUG NONE
#include "net/ip/uip-debug.h"

//Global Variables
char ThisNodeAddress[12];											//String containing this nodes address (used for checking if a trickle message is for us)

static struct etimer periodic;										//Declare the timer used for sending measurements to the network

char usart_rx_buffer[500];											//Define a buffer for storing serial port strings
int usart_rx_buffer_index = 0;										//And an index for that buffer

unsigned char channel = 0x19;										//Set the RF channel to 0x19 by default			
unsigned char client[11] = {0,0,0,0,0,0,0,0,0,0,0};					//Declare a variable to store the client name (used as a reference only)
unsigned int MeasurementPeriod = 60;								//Default measurement period = 1 min

int16_t rv, dec;
float frac;
float A0;

//SHT75
float tc, hc;    
 unsigned int temp, humidity;

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

/*---------------------------- Contiki Processes ----------------------------*/
PROCESS(knob_collect_process, "knob Collect");
AUTOSTART_PROCESSES(&knob_collect_process);
/*---------------------------------------------------------------------------*/

// This extends the built in contiki microsecond delay routing to milliseconds
void delay_msec(int time) {
	while (time > 0) {
		clock_delay_usec(1000);
		time--;
		watchdog_periodic();
	}	
}

//This function takes readings from the ADC and sends them over the network to the
//sink (base node)
//Called when the event timer expires in the collect process main loop
/*---------------------------------------------------------------------------*/
void send_message() {
	char StringBuffer[MAX_PAYLOAD_LEN];				//Buffer for building the string of readings sent over the wireless network
	watchdog_periodic();				//Feed the dog
	static int seq_id;
	int32_t value;	
	unsigned char i;
	
	//printf("ADC Set\r");				//Print debug to UART

	//Measure Regulator voltage (VCC / 3 (internal))
	value = 0;							//Reset the temporary 'value' variable
	for (i = 0; i < 100; i++)	{		//Take 100 readings of VDD/3
		value += adc_sensor.value(ADC_SENSOR_VDD_3);
	}
	A0 = ((value / 100) * (3 * 1190)) / (2047 << 4);	//Divide the ADC value by 100 readings, multiply by 3 as VDD/3
	A0 = A0 / 1000.0;								//Divide by 1000 to get mV
	
	dec = tc;
	frac = tc - dec;	
	sprintf(StringBuffer, "HU,T1=%d.%02u,", dec, (unsigned int) (frac*100));				
	
	dec = hc;
	frac = hc - dec;	
	sprintf(StringBuffer, "%sH1=%d.%02u,", StringBuffer, dec, (unsigned int) (frac*100));

	dec = A0;
	frac = A0 - dec;	
	sprintf(StringBuffer, "%sBV=%d.%02u",StringBuffer, dec, (unsigned int) (frac*100));
	
	//printf("%s\r",StringBuffer);			//Print the complete string buffer to the UART (debug)
	//printf("Sending message\r");			//Debug message
	
	//Send the string over the wireless network to the sink using RIME collect

   // printf("Sending\n");					//Debug message
    
	 seq_id++;
	//PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
    uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	
}

/*---------------------------------------------------------------------------*/
//This function converts ascii hexadecimal strings to binary hexadecimal (used for setting the RF channel from a string)
unsigned char ConvertASCIICharToHex(char ASCII) {
	if (ASCII == '0')
		return 0x00;
	else if (ASCII == '1')
		return 0x01;
	else if (ASCII == '2')
		return 0x02;
	else if (ASCII == '3')
		return 0x03;
	else if (ASCII == '4')
		return 0x04;
	else if (ASCII == '5')
		return 0x05;
	else if (ASCII == '6')
		return 0x06;
	else if (ASCII == '7')
		return 0x07;
	else if (ASCII == '8')
		return 0x08;
	else if (ASCII == '9')
		return 0x09;
	else if (ASCII == 'A')
		return 0x0A;
	else if (ASCII == 'B')
		return 0x0B;
	else if (ASCII == 'C')
		return 0x0C;
	else if (ASCII == 'D')
		return 0x0D;
	else if (ASCII == 'E')
		return 0x0E;
	else if (ASCII == 'F')
		return 0x0F;
	else return 0xFF;
}

/*---------------------------------------------------------------------------*/
//This is the UART callback and is called every time the UART buffer has data in it
int uart_rx_callback(unsigned char c) {

	return 1;												//We always return 1 (success)
}

float sht11_TemperatureC(int rawdata)
{
  int _val;                // Raw value returned from sensor
  float _temperature;      // Temperature derived from raw value

  // Conversion coefficients from SHT11 datasheet
  const float D1 = -39.6;
  const float D2 =   0.01;

  // Fetch raw value
  _val = rawdata;

  // Convert raw value to degrees Celsius
  _temperature = (_val * D2) + D1;

  return (_temperature);
}

float sht11_Humidity(int temprawdata,int humidityrawdata)
{
  int _val;                    // Raw humidity value returned from sensor
  float _linearHumidity;       // Humidity with linear correction applied
  float _correctedHumidity;    // Temperature-corrected humidity
  float _temperature;          // Raw temperature value

  // Conversion coefficients from SHT15 datasheet
  const float C1 = -4.0;       // for 12 Bit
  const float C2 =  0.0405;    // for 12 Bit
  const float C3 = -0.0000028; // for 12 Bit
  const float T1 =  0.01;      // for 14 Bit @ 5V
  const float T2 =  0.00008;   // for 14 Bit @ 5V

  _val = humidityrawdata;
   _linearHumidity = C1 + C2 * _val + C3 * _val * _val;

  // Get current temperature for humidity correction
  _temperature = sht11_TemperatureC(temprawdata);

  // Correct humidity value for current temperature
  _correctedHumidity = (_temperature - 25.0 ) * (T1 + T2 * _val) + _linearHumidity;

  return (_correctedHumidity);
}


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

//This is the main Knob process.
//Here is where we set up I/O, timers and kick of the network processes
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(knob_collect_process, ev, data)
{
	PROCESS_BEGIN();
	//NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel);		//Set the radio channel to the value read from SPI EEPROM (this will default to 0x19 if invalid)
	//printf("On channel %d\r",channel);							//Debug message
		
	//sprintf(ThisNodeAddress, "SO %03u.%03u", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);	//Debug message printing node address

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
		
	//Set up the UART for communication with the programmer
	//printf("UART BEGIN\r");
	//uart_set_input(0,uart_rx_callback);
	
	etimer_set(&periodic, SEND_INTERVAL);											//Set up an event timer to send data back to base at a set interval
	
//printf("Init SHT75\r");
  sht11_init();
//  printf("SHT75 Initialised\r");
  
  while(1) {
	PROCESS_YIELD();								//Pause the process until an event is triggered
	if(ev == tcpip_event) {
      tcpip_handler();
    }
	if(etimer_expired(&periodic)) {					//The send message event timer expired variable
//		printf("Reading SHT75\r");
		/* Read temperature value. */
      temp = sht11_temp();
      /* Read humidity value. */
      humidity = sht11_humidity();
	  
	  //printf("temp raw data:%u\nhumidity raw data:%u\n", temp, humidity);
      tc=sht11_TemperatureC(temp);
      hc=sht11_Humidity(temp,humidity);
      //printf("temp:%u.%u\nhumidity:%u.%u\n",(int)tc,((int)(tc*10))%10 , (int)hc,((int)(hc*10))%10);
	  
		//printf("Finished reading SHT75\r");
		etimer_set(&periodic, SEND_INTERVAL);		//Reset the event timer (SEND_INTERVAL may have changed due to a node reconfiguration)

		//printf("Prepare Packet\r");					//Debug message
		
		send_message();
	}
  }
  PROCESS_END();
}

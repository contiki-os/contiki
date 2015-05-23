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
//#include "usb-serial.h"
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
#include "dev/i2c.h"
#include "net/rpl/rpl.h"

#define false 0
#define true 1

#define SEND_INTERVAL	(MeasurementPeriod * CLOCK_SECOND)			//This is the rate at which data packets are sent

// MMA8652 Slave Address
#define MMA8652_SLAVE_ADDR 0x1D
 
// MMA8652 internal register addresses
#define MMA8652_STATUS 0x00
#define MMA8652_OUT_X_MSB 0x01
#define MMA8652_OUT_Y_MSB 0x03
#define MMA8652_OUT_Z_MSB 0x05
#define MMA8652_WHOAMI 0x0D
#define MMA8652_XYZ_DATA_CFG 0x0E
#define MMA8652_CTRL_REG1 0x2A
#define MMA8652_CTRL_REG2 0x2B
#define MMA8652_CTRL_REG3 0x2C
#define MMA8652_CTRL_REG4 0x2D
#define MMA8652_CTRL_REG5 0x2E
#define MMA8652_INT_SOURCE 0x0C
#define MMA8652_WHOAMI_VAL 0x4A
#define MMA8652_SYSMOD 0x0B

#define RADTODEGREES 57.29577951308

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define MAX_PAYLOAD_LEN		150

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

//Global Variables
float A0;										//Variables for storing ADC readings
float x,y,z;									//Variable for accelerometer readings
bool Above45 = false;
uint8_t i = 0;

//Used as temporary variables for printing float values as direct printing of floats is not supported
int16_t dec;
float frac;
float Rho, Phi, Theta;      //Ritch roll and yaw values

char ThisNodeAddress[12];											//String containing this nodes address (used for checking if a trickle message is for us)

static struct etimer periodic;										//Declare the timer used for sending measurements to the network
static struct ctimer backoff_timer;									//Declare the timer for delaying a measurement (get one wire data, delay and send)

char usart_rx_buffer[500];											//Define a buffer for storing serial port strings
int usart_rx_buffer_index = 0;										//And an index for that buffer

unsigned char channel = 0x19;										//Set the RF channel to 0x19 by default			
unsigned char client[11] = {0,0,0,0,0,0,0,0,0,0,0};					//Declare a variable to store the client name (used as a reference only)
unsigned int MeasurementPeriod = 10;								//Default measurement period = 1 min


static struct etimer accel;	
static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;


uint8_t i2c[10];
/*---------------------------- Contiki Processes ----------------------------*/
PROCESS(knob_collect_process, "knob Collect");
PROCESS(knob_accelerometer_process, "knob accelerometer");
AUTOSTART_PROCESSES(&knob_collect_process, &knob_accelerometer_process);
/*---------------------------------------------------------------------------*/

void CheckIfSleepDisabledByPin(void)
{
	if (GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5))){	//If PIND.5 is floating
		leds_off(LEDS_RED);													//Turn the RED led OFF
		lpm_set_max_pm(2);													//Go into power saving mode 2
	}
	else {																	//Else we do not go into power saving mode
		leds_on(LEDS_RED);													//And keep the RED led ON to show this
		lpm_set_max_pm(0);
	}
}


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
void send_message(void* ptr) {
	char StringBuffer[MAX_PAYLOAD_LEN];				//Buffer for building the string of readings sent over the wireless network
	watchdog_periodic();				//Feed the dog
	int32_t value;						//Temporary variable for storing ADC readings. 32bit as readings are averaged
	unsigned char i;				//Temporary variables for storing string CRC and for loop index
	static int seq_id;
	
	printf("ADC Set\r");				//Print debug to UART

	lpm_set_max_pm(0);					//Disable power saving modes as this can affect the ADC / radio messages
	leds_on(LEDS_RED);					//Turn RED leds on to signal that power saving is disabled

	//Measure Regulator voltage (VCC / 3 (internal))
	value = 0;							//Reset the temporary 'value' variable
	for (i = 0; i < 50; i++) {			//Take 50 readings of VDD/3
		value = value + adc_sensor.value(ADC_SENSOR_VDD_3);
	}
	A0 = ((value / 50) * (3 * 1190)) / (2047 << 4);	//Divide the ADC value by 50 readings, multiply by 3 as VDD/3
	A0 = A0 / 1000.0;								//Divide by 1000 to get mV
	
	dec = A0;
	frac = A0 - dec;	
	printf("VDD=%d.%02u\r", dec, (unsigned int) (frac*100));

	sprintf(StringBuffer, "KN,");				//Add KN designator to packet buffer
	
	//Reg voltage
	dec = A0;
	frac = A0 - dec;	
	sprintf(StringBuffer, "%sBV=%d.%02u,",StringBuffer, dec, (unsigned int) (frac*100));	//Add Battery Voltage value to packet buffer
	
	if (Above45){
		sprintf(StringBuffer, "%sVA=1.0,",StringBuffer);					//Valve state
	}
	else{
		sprintf(StringBuffer, "%sVA=0.0,",StringBuffer);					//Valve state
	}
	
	watchdog_periodic();											//Feed the dog

	printf("%s\r",StringBuffer);			//Print the complete string buffer to the UART (debug)
	printf("Sending message\r");			//Debug message
	
	//Send the string over the wireless network to the sink using RIME collect

    printf("Sending\n");					//Debug message
    
	 seq_id++;
	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
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

PROCESS_THREAD(knob_accelerometer_process, ev,data)
{
	uint8_t i2cresult;
	int16_t TempInt;
	PROCESS_BEGIN();
	
	i2c_init();						//Initialize the i2c bus
	
	uint8_t data[2];
    // write 0000 0000 = 0x00 to accelerometer control register 1 to place MMA8652 into
    // standby
    // [7-1] = 0000 000
    // [0]: active=0
	
    data[0] = MMA8652_CTRL_REG1;
    data[1] = 0x00;
    i2c_write_bytes(data, 2, MMA8652_SLAVE_ADDR);
    
    // write 0000 0001= 0x01 to XYZ_DATA_CFG register
    // [7]: reserved
    // [6]: reserved
    // [5]: reserved
    // [4]: hpf_out=0
    // [3]: reserved
    // [2]: reserved
    // [1-0]: fs=00 for accelerometer range of +/-2g range with 0.244mg/LSB
    data[0] = MMA8652_XYZ_DATA_CFG;
    data[1] = 0x00;
    i2c_write_bytes(data, 2, MMA8652_SLAVE_ADDR);
 
	//High resolution mode
    data[0] = MMA8652_CTRL_REG2;
    data[1] = 0x01;
    i2c_write_bytes(data, 2, MMA8652_SLAVE_ADDR);
 
    // write 0000 1101 = 0x0D to accelerometer control register 1
    // [7-6]: aslp_rate=00
    // [5-3]: dr=100 for 50Hz data rate
    // [2]: 0
    // [1]: 0
    // [0]: active=1 to take the part out of standby and enable sampling
    data[0] = MMA8652_CTRL_REG1;
    data[1] = 0x21;
    i2c_write_bytes(data, 2, MMA8652_SLAVE_ADDR);
	
	etimer_set(&accel, CLOCK_SECOND);
	
	while(1) {
		PROCESS_YIELD();
		if(etimer_expired(&accel)){
			etimer_reset(&accel);

			lpm_set_max_pm(0);
			i2c_init();
			i2c_write_byte(0x00, MMA8652_SLAVE_ADDR);
			
			delay_msec(10);
			i2c_read_bytes(i2c, 10, MMA8652_SLAVE_ADDR);
			
			TempInt = i2c[1] << 8;
			TempInt = TempInt | i2c[2];
			TempInt = TempInt / 4;
			
			x = TempInt * 0.196;
			dec = x;
			printf("x=%d mg\r", dec);

			
			TempInt = i2c[3] << 8;
			TempInt = TempInt | i2c[4];
			TempInt = TempInt / 4;
			
			y = TempInt * 0.196;
			dec = y;
			printf("y=%d mg\r", dec);
			
			TempInt = i2c[5] << 8;
			TempInt = TempInt | i2c[6];
			TempInt = TempInt / 4;
			
			z = TempInt * 0.196;
			dec = z;
			printf("z=%d mg\r", dec);
			
					
			Rho = (atan(x / (sqrt(pow(y,2) + pow(z,2))))) * RADTODEGREES;
			dec = Rho;
			frac = Rho - dec;	
			printf("Rho=%d.%02u\r", dec, (unsigned int) (frac*100));
			Phi = (atan(y / (sqrt(pow(x ,2) + pow(z,2))))) * RADTODEGREES;
			dec = Phi;
			frac = Phi - dec;	
			printf("Phi=%d.%02u\r", dec, (unsigned int) (frac*100));
			Theta = (atan((sqrt(pow(x,2) + pow(y,2))) / z)) * RADTODEGREES;  
			dec = Theta;
			frac = Theta - dec;	
			printf("Theta=%d.%02u\r", dec, (unsigned int) (frac*100));

			if(Phi > 45.0 || Phi < -45.0){
				Above45 = true;
				leds_on(LEDS_GREEN);
			}
			else{
				Above45 = false;
				leds_toggle(LEDS_GREEN);
			}
			lpm_set_max_pm(2);
		}
	}
	PROCESS_END();
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
	
	sprintf(ThisNodeAddress, "SO %03u.%03u", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);	//Debug message printing node address

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
	printf("UART BEGIN\r");
	uart_set_input(0,uart_rx_callback);
	
	etimer_set(&periodic, SEND_INTERVAL);											//Set up an event timer to send data back to base at a set interval
	watchdog_periodic();															//Feed the doge
	
  while(1) {
	PROCESS_YIELD();								//Pause the process until an event is triggered
	if(ev == tcpip_event) {
      tcpip_handler();
    }
	if(etimer_expired(&periodic)) {					//The send message event timer expired variable
		
		etimer_set(&periodic, SEND_INTERVAL);		//Reset the event timer (SEND_INTERVAL may have changed due to a node reconfiguration)
		watchdog_periodic();						//Give the dog some food
		printf("Prepare Packet\r");					//Debug message
			
		ctimer_set(&backoff_timer, (random_rand() % (3 * CLOCK_SECOND)), send_message, NULL);		//Wait 3 seconds for soil moisture node to settle and then send data
		watchdog_periodic();				//Feed doge
	}
  }
  PROCESS_END();
}

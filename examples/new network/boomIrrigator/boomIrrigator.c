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
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "lpm.h"
//#include "usb-serial.h"
#include "dev/uart.h"
#include "dev/adc-sensor.h"
#include "spi-arch.h"
#include "spi.h"
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

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define false 0
#define true 1

#define SEND_INTERVAL	(MeasurementPeriod * CLOCK_SECOND)			//This is the rate at which data packets are sent


//EEPROM Memory locations
#define RF_CHANNEL_ADDR	0x00			//1byte
#define CLIENT_ADDR1	0x08			//10bytes
#define CLIENT_ADDR2	0x10			
#define MEASUREMENTPERIOD_ADDR 0x18		//1byte

//EEPROM commands
#define WREN 0x06
#define WRDI 0x04
#define RDSR 0x05
#define WRSR 0x01
#define READ 0x03
#define WRITE 0x02

#define MAX_PAYLOAD_LEN		150

#define WHEEL_PULSE_PORT 	GPIO_C_NUM
#define WHEEL_PULSE_PIN 	4
#define WHEEL_PULSE_VECTOR	NVIC_INT_GPIO_PORT_C
#define WHEEL_PULSE_PORT_BASE    GPIO_PORT_TO_BASE(WHEEL_PULSE_PORT)
#define WHEEL_PULSE_PIN_MASK     GPIO_PIN_MASK(WHEEL_PULSE_PIN)

#define SPOOL_PULSE_PORT 	GPIO_C_NUM
#define SPOOL_PULSE_PIN 	5
#define SPOOL_PULSE_VECTOR	NVIC_INT_GPIO_PORT_C
#define SPOOL_PULSE_PORT_BASE    GPIO_PORT_TO_BASE(SPOOL_PULSE_PORT)
#define SPOOL_PULSE_PIN_MASK     GPIO_PIN_MASK(SPOOL_PULSE_PIN)


static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

float Voltage, A0, A1;
uint8_t DigitalInput1 = false;

int16_t rv, dec;
float frac;

char ThisNodeAddress[12];											//String containing this nodes address (used for checking if a trickle message is for us)

static struct etimer periodic;										//Declare the timer used for sending measurements to the network
static struct ctimer backoff_timer;									//Declare the timer for delaying a measurement (get one wire data, delay and send)

static struct etimer spoolTimer;
static struct etimer wheelTimer;

char usart_rx_buffer[500];											//Define a buffer for storing serial port strings
int usart_rx_buffer_index = 0;										//And an index for that buffer

static char gps_buffer[300];
static int gps_buffer_index = 0;

static bool GettingGPS_String = false; 

bool FoundSatellite = false;

static signed char GPS_TimeString[12]; 
static signed char GPS_DateString[12];
static signed char GPS_LatString[14];
static signed char GPS_Lat_Hemisphere;
static signed char GPS_LongString[14];
static signed char GPS_Long_Hemisphere;
static signed char GPS_SpeedString[10];
static signed char GPS_BearingString[10];

unsigned char channel = 0x19;										//Set the RF channel to 0x19 by default			
unsigned char client[11] = {0,0,0,0,0,0,0,0,0,0,0};					//Declare a variable to store the client name (used as a reference only)
unsigned int MeasurementPeriod = 60;								//Default measurement period = 1 min

uint8_t GPSon[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x4C, 0x37};
uint8_t GPSoff[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B};

bool wheelTurning, spoolTurning;

void sendUBX(uint8_t *MSG, uint8_t len);

//Define a couple of functions that appear out of order
void ReadFromEEPROM(void);
void WriteToEEPROM(void);
static struct timer debouncetimer;
/*---------------------------------------------------------------------------*/
PROCESS(example_mesh_process, "Mesh example");
AUTOSTART_PROCESSES(&example_mesh_process);
/*---------------------------------------------------------------------------*/

int indexOf_shift (char* base, char* str, int startIndex) {
    int result;
    int baselen = strlen(base);
    // str should not longer than base
    if (strlen(str) > baselen || startIndex > baselen) {
        result = -1;
    } else {
        if (startIndex < 0 ) {
            startIndex = 0;
        }
        char* pos = strstr(base+startIndex, str);
        if (pos == NULL) {
            result = -1;
        } else {
            result = pos - base;
        }
    }
    return result;
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

int indexOf (char* base, char* str) {
    return indexOf_shift(base, str, 0);
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
	unsigned char i;				//Temporary variables for storing string loop index
	static int seq_id;

	printf("Send message\r");
	
	if (FoundSatellite == false)
	{
		printf("No satellites found!\r");
	}
	
	//Measure Regulator voltage (VCC / 3 (internal))
	value = 0;							//Reset the temporary 'value' variable
	for (i = 0; i < 20; i++) {			//Take 20 readings of VDD/3
		value = value + adc_sensor.value(ADC_SENSOR_VDD_3);
	}
	A0 = ((value / 20) * (3 * 1195)) / (2047 << 4);	//Divide the ADC value by 20 readings, multiply by 3 as VDD/3
	A0 = A0 / 1000.0;								//Divide by 1000 to get mV
	
	dec = A0;
	frac = A0 - dec;	
	printf("VDD=%d.%02u\r", dec, (unsigned int) (frac*100));
	
	//Measure pressure sensor
    value = 0;
	for (i = 0; i < 20; i++) {
		value = value + adc_sensor.value(ADC_SENSOR_SENS1);
	}
	A1 = ((value / 20) * A0) / (2047 << 4);
	
	//There is no float support for printf so we need to take a float value, get it's integer value and subtract the two.
	//This leaves us with the remainder. We then multiply it by 100 to turn it into an integer and print both values with a '.' in between
	//Analog1
	dec = A1;
	frac = A1 - dec;	
	sprintf(StringBuffer, "IR,A1=%d.%02u,", dec, (unsigned int) (frac*100));				//Add SO designator and Analog1 value to packet buffer
	
	if (FoundSatellite)
	{
		sprintf(StringBuffer, "%sLAT=%s%c,",StringBuffer,GPS_LatString,GPS_Lat_Hemisphere);
		sprintf(StringBuffer, "%sLON=%s%c,",StringBuffer,GPS_LongString,GPS_Long_Hemisphere);	
	}
	
	if (spoolTurning)
		sprintf(StringBuffer, "%sSpool=1,",StringBuffer);	
	else
		sprintf(StringBuffer, "%sSpool=0,",StringBuffer);	

	if (wheelTurning)
		sprintf(StringBuffer, "%sWheel=1,",StringBuffer);	
	else
		sprintf(StringBuffer, "%sWheel=0,",StringBuffer);	
	
	watchdog_periodic();											//Feed the dog

	printf("%s\r",StringBuffer);			//Print the complete string buffer to the UART (debug)
	printf("Sending message\r");			//Debug message
	
	//Send the string over the wireless network to the sink using RIME collect

    printf("Sending\n");					//Debug message
    
	seq_id++;
	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
    uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	
	//Turn GPS OFF
	//sendUBX(GPSoff, sizeof(GPSoff)/sizeof(uint8_t));	
	//Reset packet sending timer
	etimer_set(&periodic, SEND_INTERVAL);
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

//Used to process a full carridge return terminated string received through the serial UART
static void process_line(void) {
	unsigned char ctr;									//FOR loop index
	watchdog_periodic();								//Woof woof bitches
	if (strncmp(usart_rx_buffer,"?",1) == 0) {			//If we received a "?" in position 1, the device connected to the UART wants information about the node
		print_local_addresses();
		ReadFromEEPROM();								//Read settings from EEPROM - the ReadFromEEPROM function will send out the data
	}
	else if (strncmp(usart_rx_buffer,"a",1) == 0) {		//If we received an "a" in position 1, a channel set has been requested. The next 2 chars are the channel no in ASCII hex
		channel = (usart_rx_buffer[1] - 48) * 10;
		channel += (usart_rx_buffer[2] - 48);
		//NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel);		//Set the radio to the new channel
		printf("On channel %d\r",channel);				//Spit out a debug message
	}
	else if (strncmp(usart_rx_buffer,"b",1) == 0) {		//A "b" in position 1 means set a new client name, the next 10 chars are the client name (unless a "\r" is rx before then)
		for (ctr = 0; (ctr < 10) && (ctr < usart_rx_buffer_index); ctr++) {
			if (usart_rx_buffer[ctr+1] == '\r')			//Break the loop for a carridge return
				break;
			client[ctr] = usart_rx_buffer[ctr+1];
		}
		client[ctr] = 0;								//Make sure the string is null nerminated
	}
	else if (strncmp(usart_rx_buffer,"r",1) == 0)						//"r" in position 1 sets the measurement period in seconds. Minimum value = 10, Max is around 300
	{
		if (usart_rx_buffer[3] >= 48 && usart_rx_buffer[3] <= 57)		//We are dealing with a 3 digit number
		{
			MeasurementPeriod = (usart_rx_buffer[1] - 48) * 100;
			MeasurementPeriod += (usart_rx_buffer[2] - 48) * 10;
			MeasurementPeriod += (usart_rx_buffer[3] - 48);
		}
		else
		{
			MeasurementPeriod = (usart_rx_buffer[1] - 48) * 10;
			MeasurementPeriod += (usart_rx_buffer[2] - 48);
		}
	}
	else if (strncmp(usart_rx_buffer,"w",1) == 0) {						//"w" in position 1 commits these new settings to EEPROM
			WriteToEEPROM();
	}
	usart_rx_buffer_index = 0;											//Reset the UART buffer and return from the process line function
}

/*---------------------------------------------------------------------------*/
//This is the UART callback and is called every time the UART buffer has data in it
int uart_rx_callback(unsigned char c) {
	if(usart_rx_buffer_index < 499) {						//The usart buffer string is 500 chars long so this prevents an overflow
	 	usart_rx_buffer[usart_rx_buffer_index++] = c;		//Add a byte to the rx_buffer
		usart_rx_buffer[usart_rx_buffer_index] = '\0';		//Null terminate the next char in the buffer to prevent corruption
	}
	else													//Reset the buffer if it has filled up with crap
		usart_rx_buffer_index = 0;

	if(c == '\r')											//If we rx a full carridge return terminated string, process the string
		process_line();							

	return 1;												//We always return 1 (success)
}

/*---------------------------------------------------------------------------*/
//Function from reading values from the AT25040B SPI EEPROM
//After reading values are output from the serial UART (debug interface)
void ReadFromEEPROM(void) {
	watchdog_periodic();									//Give the dog a bone - we don't want him to bark now
	int i;													//For loop counters	
	spi_set_mode(SSI_CR0_FRF_MOTOROLA, 0, 0, 8);			//Set the SPI bus in motorolla mode 0,0, 8 bit
	spi_enable();											//Enable the spi bus
	
	//Read node channel from EEPROM
	SPI_CS_CLR(GPIO_B_NUM, 1);								//Bring EEPROM CS low to address the chip
	clock_delay_usec(10);									//Wait for the chip to respond
	SPI_WRITE(READ);										//Send the Read command to the EEPROM chip
	SPI_WRITE(RF_CHANNEL_ADDR);								//Set the read address to the address of the RF Channel (defined at top of page)
	SPI_FLUSH();											//Flush the SPI buffer as it gets filled up with 8 bits after every write (bi directional)
	SPI_READ(channel);										//Read the data
	SPI_WAITFORTx_AFTER();									//Wait for the data transfer to stop
	SPI_CS_SET(GPIO_B_NUM, 1);								//AAAnd bring CS high again to deselect the EEPROM chip
	clock_delay_usec(10);
	
	//Refer to top example for how EEPROM SPI Read works
	//Read client name from EEPROM
	SPI_CS_CLR(GPIO_B_NUM, 1);
	clock_delay_usec(10);
	SPI_WRITE(READ);										//Read command
	SPI_WRITE(CLIENT_ADDR1);								//Read address for Client Name
	SPI_FLUSH();
	//Read client name from EEPROM (9 bytes + null)
	for (i = 0; i <= 9; i++)	{
		SPI_READ(client[i]);
	}
	client[10] = 0;											//Null terminate the client name to prevent corruption
	watchdog_periodic();									//Feed goggie
	SPI_WAITFORTx_AFTER();
	SPI_CS_SET(GPIO_B_NUM, 1);
	clock_delay_usec(10);
	
	//Refer to top example for how EEPROM SPI Read works
	//Read Measurement Period from EEPROM
	SPI_CS_CLR(GPIO_B_NUM, 1);
	clock_delay_usec(10);
	SPI_WRITE(READ);				//Read command
	SPI_WRITE(MEASUREMENTPERIOD_ADDR);						//Read address for Measurement Period
	SPI_FLUSH();
	SPI_READ(MeasurementPeriod);
	MeasurementPeriod = 60;
	SPI_WAITFORTx_AFTER();
	SPI_CS_SET(GPIO_B_NUM, 1);
	clock_delay_usec(10);
		
	if (MeasurementPeriod < 10)								//If the value from EEPROM is less than 10, set to 30sec
		MeasurementPeriod = 30;
		
	//Print out everything we have read to the UART
	printf("@A=%d\r", channel);
	printf("@B=%s\r", client);
	printf("@R=%d\r", MeasurementPeriod);
	//printf("@S=%u.%u\r",linkaddr_node_addr.u8[0], 	linkaddr_node_addr.u8[1]);
	printf("@T=BOOM\r");
}

// Write data to AT25040B EEPROM using SPI
// Data is written in 8 byte pages
void WriteToEEPROM(void)
{
	int j;														//Index variable for FOR loop
	spi_set_mode(SSI_CR0_FRF_MOTOROLA, 0, 0, 8);				//Set the SPI bus in motorolla mode 0,0, 8 bit
	spi_enable();												//Enable the spi bus
	
	//Write RF channel variable to EEPROM
	SPI_CS_CLR(GPIO_B_NUM, 1);									//Set CS low to address EEPROM
	SPI_WRITE(WREN);											//Send write Enable latch command
	SPI_CS_SET(GPIO_B_NUM, 1);									//Set CS high to commit write enable command
	
	SPI_CS_CLR(GPIO_B_NUM, 1);									//Set CS low to address EEPROM
	SPI_WRITE(WRITE);											//Send write command
	SPI_WRITE(RF_CHANNEL_ADDR);									//Send write address for RF channel variable
	SPI_WRITE(channel);											//Write the data
	SPI_CS_SET(GPIO_B_NUM, 1);									//Set CS high to start programming

	SPI_FLUSH();												//Flush the SPI buffer whilst we are programming
	watchdog_periodic();										//**Twiddle thumbs****
	delay_msec(10);												//Programming takes a while.....
	printf("RF Channel\r");										//Send a debug message to indicate RF channel programmed
	watchdog_periodic();										//I can has hotdog?
	
	//***Rinse and repeat.....
	//See top example on how to write to EEPROM
	//Write the client name (8 MSBs)
	SPI_CS_CLR(GPIO_B_NUM, 1);									//Set CS low to address EEPROM
	SPI_WRITE(WREN);											//Write Enable latch
	SPI_CS_SET(GPIO_B_NUM, 1);
	
	SPI_CS_CLR(GPIO_B_NUM, 1);
	SPI_WRITE(WRITE);											//Write command
	SPI_WRITE(CLIENT_ADDR1);									//Write address	for client name 8MSBs
	SPI_WRITE(client[0]);
	SPI_WRITE(client[1]);
	SPI_WRITE(client[2]);
	SPI_WRITE(client[3]);
	SPI_WRITE(client[4]);
	SPI_WRITE(client[5]);
	SPI_WRITE(client[6]);
	SPI_WRITE(client[7]);
	SPI_CS_SET(GPIO_B_NUM, 1);									//Set CS high to start programming

	SPI_FLUSH();
	watchdog_periodic();
	delay_msec(10);
	watchdog_periodic();

	//See top example on how to write to EEPROM
	//Write the client name (2 LSBs)
	SPI_CS_CLR(GPIO_B_NUM, 1);
	SPI_WRITE(WREN);											//Write Enable latch
	SPI_CS_SET(GPIO_B_NUM, 1);
	
	SPI_CS_CLR(GPIO_B_NUM, 1);
	SPI_WRITE(WRITE);											//Write command
	SPI_WRITE(CLIENT_ADDR2);									//Write address	for client name 2LSBs
	SPI_WRITE(client[8]);
	SPI_WRITE(client[9]);	
	SPI_CS_SET(GPIO_B_NUM, 1);									//Set CS high to start programming

	SPI_FLUSH();
	watchdog_periodic();
	delay_msec(10);
	printf("Client\r");
	watchdog_periodic();
	
	//See top example on how to write to EEPROM
	//Write the measurement period value to EEPROM
	SPI_CS_CLR(GPIO_B_NUM, 1);
	SPI_WRITE(WREN);											//Write Enable latch
	SPI_CS_SET(GPIO_B_NUM, 1);
	
	SPI_CS_CLR(GPIO_B_NUM, 1);
	SPI_WRITE(WRITE);											//Write command
	SPI_WRITE(MEASUREMENTPERIOD_ADDR);							//Write measurement period address
	SPI_WRITE(MeasurementPeriod);	
	SPI_CS_SET(GPIO_B_NUM, 1);									//Set CS high to start programming

	SPI_FLUSH();
	watchdog_periodic();
	delay_msec(10);
	printf("Measurement Period\r");
	watchdog_periodic();

	//See top example on how to write to EEPROM
	//Write 1 wire addresses to EEPROM
	for (j = 0; j < 2; j++)
	{
		SPI_CS_CLR(GPIO_B_NUM, 1);
		SPI_WRITE(WREN);										//Write Enable latch
		SPI_CS_SET(GPIO_B_NUM, 1);
	
		SPI_CS_CLR(GPIO_B_NUM, 1);
		SPI_WRITE(WRITE);										//Write command
		
		SPI_FLUSH();
		watchdog_periodic();
		delay_msec(10);
		watchdog_periodic();
	}
	printf("SETTINGS SAVED TO EEPROM\r");						//Debug message
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

void sendUBX(uint8_t *MSG, uint8_t len) {
	int i = 0;
	uart_write_byte(1, 0xff);		//Wake up device
	delay_msec(500);
	for(i=0; i<len; i++) {
		uart_write_byte(1, MSG[i]);
		uart_write_byte(1, MSG[i]);
	}
}
  
void ExtractDataFromGPS_String(void)
{
	unsigned char x, i, StrLen;        
        
    //The format of the GPRMC string is:
    //"GPRMC,UTC,FIX,LAT,lATHEM,LONG,LONGHEM,SPEED,BEARING,DATE(UTC),CHECKSUM"
    
    x = strncmp(gps_buffer, "GPRMC", 5);    //Compare the first 5 characters of the Rx0Buffer string with "GPRMC"
    if (x!=0) return;                    //If not matching the exit
    
    //We have identified the string we require form the GPS, so copy it into GPS_String
                                           
    StrLen = strlen(gps_buffer);            //Find out the length of the GPS string
          
    //The first variable is UTC time, starting at character 6
    x = 6;         
    i = 0;     

	//uart_write_byte(0, '\r');
	//uart_write_byte(0, 'U');
	//uart_write_byte(0,'T');
	//uart_write_byte(0,'C');
	//uart_write_byte(0,':');
	
    while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Universal Time
     {
		//uart_write_byte(0,gps_buffer[x]);
        GPS_TimeString[i] = gps_buffer[x];
        x++;
        i++;
     }
     
     x++;
     
     if (gps_buffer[x++] == 'A')
        FoundSatellite = true;
     else
	 {
		FoundSatellite = false;
        return;
	 }
     
     x++;  //Search for next comma
     
     i = 0;
	 
	 //uart_write_byte(0,'\r');
	 //uart_write_byte(0,'L');
	 //uart_write_byte(0,'A');
	 //uart_write_byte(0,'T');
	 //uart_write_byte(0,':');
     while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Decimal Latitude
     {
		//uart_write_byte(0,gps_buffer[x]);
        GPS_LatString[i] = gps_buffer[x];
        x++;
        i++;
     }  
     
     x++;  //Search for next comma
     while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Latitude Hemisphere
     {
		//uart_write_byte(0,gps_buffer[x]);
        GPS_Lat_Hemisphere = gps_buffer[x];
        x++;
     }
     
     x++;  //Search for next comma 
       
     i = 0;

     //uart_write_byte(0,'\r');
	 //uart_write_byte(0,'L');
	 //uart_write_byte(0,'O');
	 //uart_write_byte(0,'N');
	 //uart_write_byte(0,':');
	 while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Decimal Longitude
     {
		//uart_write_byte(0,gps_buffer[x]);
        GPS_LongString[i] = gps_buffer[x];
        x++;
        i++;
     }  
     
     x++;  //Search for next comma
     
     while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Longitude Hemisphere
     {
		//uart_write_byte(0,gps_buffer[x]);
        GPS_Long_Hemisphere = gps_buffer[x];
        x++;
     }
     
    
     x++;  //Search for next comma
     
	 //uart_write_byte(0,'\r');
	 //uart_write_byte(0,'k');
	 //uart_write_byte(0,'M');
	 //uart_write_byte(0,'H');
	 //uart_write_byte(0,':');
    i = 0;
    GPS_SpeedString[0] = '0';
    while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the gps speed
     {
		//uart_write_byte(0,gps_buffer[x]);
        GPS_SpeedString[i] = gps_buffer[x];
        x++;
        i++;
     }  
                               
     x++;  //Search for next comma
     
    i = 0;

	//uart_write_byte(0,'\r');
	//uart_write_byte(0,'D');
	//uart_write_byte(0,'E');
	//uart_write_byte(0,'G');
	//uart_write_byte(0,':');
    GPS_BearingString[0] = '0';
    while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the bearing
     {
		//uart_write_byte(0,gps_buffer[x]);      
		GPS_BearingString[i] = gps_buffer[x];
        x++;
        i++;
     }  
                               
     x++;  //Search for next comma   
     
      i = 0;

	//uart_write_byte(0,'\r');
	//uart_write_byte(0,'D');
	//uart_write_byte(0,'A');
	//uart_write_byte(0,'T');
	//uart_write_byte(0,':');

    while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the UTC Date
     {
		//uart_write_byte(0,gps_buffer[x]);
        GPS_DateString[i] = gps_buffer[x];
        x++;
        i++;
     }  

	//uart_write_byte(0,'\r');
}

//.............................................................................
/*---------------------------------------------------------------------------*/
int uart1_rx_callback(unsigned char c)
{
	//This is data from the GPS

	watchdog_periodic();

    if (c == 0)   //Ignore a null character 
        return 0;   
    
    if (c == '$')
    {
        GettingGPS_String = true;
        gps_buffer_index = 0;
        return 1;
    }
         
    if (c == '\r')
    {
        GettingGPS_String = false; 
        gps_buffer_index = 0; 
		ExtractDataFromGPS_String();     
        return 1;
    }   
         
    if (GettingGPS_String)
    {
        gps_buffer[gps_buffer_index++] = c;
        gps_buffer[gps_buffer_index] = 0;
    } 

	watchdog_periodic();

	return 1;
}

static void pulse_callback(uint8_t port, uint8_t pin)
{
	if(!timer_expired(&debouncetimer)) {
    return;
  }
	timer_set(&debouncetimer, CLOCK_SECOND / 100);
	if (port == GPIO_C_NUM)
	{
		if(pin == 4)
		{
			wheelTurning = true;
			etimer_reset(&wheelTimer);
		}

		if(pin == 5)
		{
			spoolTurning = true;
			etimer_reset(&spoolTimer);
		}
	}
	return ;
}

static void
config(uint32_t port_base, uint32_t pin_mask)
{
  /* Software controlled */
  GPIO_SOFTWARE_CONTROL(port_base, pin_mask);

  /* Set pin to input */
  GPIO_SET_INPUT(port_base, pin_mask);

  /* Enable edge detection */
  GPIO_DETECT_EDGE(port_base, pin_mask);

  /* Single edge */
  GPIO_TRIGGER_SINGLE_EDGE(port_base, pin_mask);

  /* Trigger interrupt on Falling edge */
  GPIO_DETECT_FALLING(port_base, pin_mask);
  
  GPIO_ENABLE_INTERRUPT(port_base, pin_mask);
}

static int
config_wheel_pulse()
{
	config(WHEEL_PULSE_PORT_BASE, WHEEL_PULSE_PIN_MASK);
	ioc_set_over(WHEEL_PULSE_PORT, WHEEL_PULSE_PIN, IOC_OVERRIDE_DIS);
	nvic_interrupt_enable(WHEEL_PULSE_VECTOR);
	gpio_register_callback(pulse_callback, WHEEL_PULSE_PORT, WHEEL_PULSE_PIN);
	return 1;
}

static int
config_spool_pulse()
{
	config(SPOOL_PULSE_PORT_BASE, SPOOL_PULSE_PIN_MASK);
	ioc_set_over(SPOOL_PULSE_PORT, SPOOL_PULSE_PIN, IOC_OVERRIDE_DIS);
	nvic_interrupt_enable(WHEEL_PULSE_VECTOR);
	gpio_register_callback(pulse_callback, SPOOL_PULSE_PORT, SPOOL_PULSE_PIN);
	return 1;
}

//This is the main Gate process.
//Here is where we set up I/O, timers and kick of the network processes
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_mesh_process, ev, data)
{
	PROCESS_BEGIN();
	
	spi_init();											//Initialise the SPI bus (this library is included in Contiki and is cc2538 specific)
	spi_cs_init(GPIO_B_NUM, 1);							//Configure PORTB.1 as the Chip Select (CS) line for the EEPROM chip on the SPI BUS
	SPI_CS_SET(GPIO_B_NUM, 1);							//Set the EEPROM CS line to high (chip not selected - default state)

	ReadFromEEPROM();											//Read all our saved values from SPI EEPROM

	timer_set(&debouncetimer, 0);
	
	config_spool_pulse();
	config_wheel_pulse();
	
	//Turn off 9V output(PORTB.5)
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
			
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
	uart_set_input(1,uart1_rx_callback);
	
	//Turn GPS OFF
	//sendUBX(GPSoff, sizeof(GPSon)/sizeof(uint8_t));
	
	etimer_set(&periodic, SEND_INTERVAL);											//Set up an event timer to send data back to base at a set interval
	etimer_set(&spoolTimer, 60 * CLOCK_SECOND);
	etimer_set(&wheelTimer, 60 * CLOCK_SECOND);
	
    while(1) {
	    PROCESS_YIELD();								//Pause the process until an event is triggered
	    if(ev == tcpip_event) {
            tcpip_handler();
        }
	
	    if(etimer_expired(&periodic)) {					//The send message event timer expired variable
    		etimer_set(&periodic, SEND_INTERVAL);		//Reset the event timer (SEND_INTERVAL may have changed due to a node reconfiguration)
	    	watchdog_periodic();						//Give the dog some food
    		printf("Prepare Packet\r");					//Debug message
				
			//Turn GPS ON
			//printf("GPS ON\r");
			//sendUBX(GPSon, sizeof(GPSon)/sizeof(uint8_t));
			
			//Wait 45 seconds for GPS to get fix
			ctimer_set(&backoff_timer, (45 * CLOCK_SECOND), send_message, NULL);	
	    	watchdog_periodic();				//Feed doge
	    }
		
		if(etimer_expired(&wheelTimer)) {
			wheelTurning = false;
		}
		if(etimer_expired(&spoolTimer))	{
			spoolTurning = false;
		}
    }
    PROCESS_END();
}

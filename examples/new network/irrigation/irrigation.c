/* irrigation Source Code for CC2538 version microcontroller
 * Uses the mesh networking functunality of the RIME stack
 * Uses ContikiMAC for radio duty cycling
 */


//Included files
#include <stdio.h>
#include <string.h>
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

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

#define MAX_PAYLOAD_LEN		150

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define false 0
#define true 1

#define SEND_INTERVAL	(MeasurementPeriod * CLOCK_SECOND)			//This is the rate at which data packets are sent
#define pulse 2

//Global Variables
float A0, A1, A2;										//Variables for storing ADC readings
float x,y,z;									//Variable for accelerometer readings
bool Above45 = false;
uint8_t i = 0;

//Used as temporary variables for printing float values as direct printing of floats is not supported
int16_t dec;
float frac;

char ThisNodeAddress[12];											//String containing this nodes address (used for checking if a trickle message is for us)

static struct etimer periodic;										//Declare the timer used for sending measurements to the network
static struct ctimer backoff_timer;									//Declare the timer for delaying a measurement (get one wire data, delay and send)

char usart_rx_buffer[500];											//Define a buffer for storing serial port strings
int usart_rx_buffer_index = 0;										//And an index for that buffer

unsigned char channel = 0x19;										//Set the RF channel to 0x19 by default			
unsigned char client[11] = {0,0,0,0,0,0,0,0,0,0,0};					//Declare a variable to store the client name (used as a reference only)
unsigned int MeasurementPeriod = 30;								//Default measurement period = 1 min

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

uint8_t GPSon[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x4C, 0x37};
uint8_t GPSoff[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B};

void sendUBX(uint8_t *MSG, uint8_t len);

char Relay1, Relay2, Relay3, Relay4;


static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

/*---------------------------------------------------------------------------*/
PROCESS(example_mesh_process, "Mesh example");
AUTOSTART_PROCESSES(&example_mesh_process);
/*---------------------------------------------------------------------------*/

// This extends the built in contiki microsecond delay routing to milliseconds
void delay_msec(int time) {
	while (time > 0) {
		clock_delay_usec(1000);
		time--;
		watchdog_periodic();
	}	
}

void UpdateRelay1()
{
	//Bring A0 low, A1 and A2 high
	printf("R1\r");
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(3));
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(1));
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(0));
	clock_delay_usec(100);
	//CS low
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
	clock_delay_usec(100);
	//CS high
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
	clock_delay_usec(100);
}

void UpdateRelay2()
{
	//Bring A0, A1 and A2 high
	printf("R2\r");
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(3));
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(1));
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(0));
	clock_delay_usec(100);
	//CS low
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
	clock_delay_usec(100);
	//CS high
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
	clock_delay_usec(100);
}

void UpdateRelay3()
{
	//Bring A1 high, A0 and A2 low
	printf("R3\r");
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(3));
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(1));
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(0));
	clock_delay_usec(100);
	//CS low
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
	clock_delay_usec(100);
	//CS high
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
	clock_delay_usec(100);
}

void UpdateRelay4()
{
	//Bring A0 and A1 high, A2 low
	printf("R4\r");
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(3));
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(1));
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(0));
	clock_delay_usec(100);
	//CS low
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
	clock_delay_usec(100);
	//CS high
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
	clock_delay_usec(100);		

}

void UpdateOutputs()
{		
	//CS high
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));
		
	printf("Updating the relay outputs\r");
	//Relay1, Relay2, Relay3, Relay4
			
	//If Relay1
	if (Relay1 == true) {
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay1();
	}
	else if (Relay1 == false) {
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay1();
	}
	else if (Relay1 == pulse) {
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay1();
		delay_msec(100);
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay1();
		delay_msec(500);
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay1();
		Relay1 = false;
	}
	

	if (Relay2 == true) 
	{
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay2();
	}
	else if (Relay2 == false) {
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay2();
	}
	else if (Relay2 == pulse)
	{
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay2();
		delay_msec(100);
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay2();
		delay_msec(500);
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay2();
		Relay2 = false;
	}

	if (Relay3 == true) 
	{
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay3();
	}
	else if (Relay3 == false) {
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay3();
	}
	else if (Relay3 == pulse)
	{
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay3();
		delay_msec(100);
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay3();
		delay_msec(500);
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay3();
		Relay3 = false;
	}
	
	if (Relay4 == true) 
	{
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay4();
	}
	else if (Relay4 == false){
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay4();
	}
	else if (Relay4 == pulse){
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay4();
		delay_msec(100);
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay4();
		delay_msec(500);
		GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));
		UpdateRelay4();
		Relay4 = false;
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
	char Input1Value, Input2Value, Input3Value, Input4Value;
    static int seq_id;

	printf("ADC Set\r");				//Print debug to UART

	lpm_set_max_pm(0);					//Disable power saving modes as this can affect the ADC / radio messages
	//leds_on(LEDS_RED);					//Turn RED leds on to signal that power saving is disabled

	UpdateOutputs();
	
	//Check inputs
	//Digital Input1
	if (GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(5)))
		Input1Value = 1;															//If it is a 1 (ON) set O1Value to 1
	else 																		//Else it is 0 (OFF)
		Input1Value = 0;		
		
	//Digital Input2
	if (GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4)))
		Input2Value = 1;															//If it is a 1 (ON) set O1Value to 1
	else 																		//Else it is 0 (OFF)
		Input2Value = 0;	
	
	//Digital Input3
	if (GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(6)))
		Input3Value = 1;															//If it is a 1 (ON) set O1Value to 1
	else 																		//Else it is 0 (OFF)
		Input3Value = 0;	
		
	//Digital Input4
	if (GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(7)))
		Input4Value = 1;															//If it is a 1 (ON) set O1Value to 1
	else 																		//Else it is 0 (OFF)
		Input4Value = 0;	
		
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
	
	value = 0;							//Reset the temporary 'value' variable
	for (i = 0; i < 20; i++) {			//Take 20 readings of Analog1
		value = value + adc_sensor.value(ADC_SENSOR_SENS1);
	}
	A1 = ((value / 20) * A0) / (2047 << 4);
		
	value = 0;							//Reset the temporary 'value' variable
	for (i = 0; i < 20; i++) {			//Take 20 readings of Analog2
		value = value + adc_sensor.value(ADC_SENSOR_SENS2);
	}
	A2 = ((value / 20) * A0) / (2047 << 4);
	A2 = (A2 - 0.6) / 0.0238;

	sprintf(StringBuffer, "GA,");															
	sprintf(StringBuffer, "%sO1=%u,",StringBuffer, Relay1);					//Add Digital Output1 value to packet buffer (or else Mango keeps sending a set command)
	sprintf(StringBuffer, "%sO2=%u,",StringBuffer, Relay2);					//Add Digital Output1 value to packet buffer (or else Mango keeps sending a set command)														
	sprintf(StringBuffer, "%sO3=%u,",StringBuffer, Relay3);					//Add Digital Output1 value to packet buffer (or else Mango keeps sending a set command)
	sprintf(StringBuffer, "%sO4=%u,",StringBuffer, Relay4);					//Add Digital Output1 value to packet buffer (or else Mango keeps sending a set command)
	
	sprintf(StringBuffer, "%sI1=%u,",StringBuffer, Input1Value);					//Add Digital Output1 value to packet buffer (or else Mango keeps sending a set command)
	sprintf(StringBuffer, "%sI2=%u,",StringBuffer, Input2Value);					//Add Digital Output1 value to packet buffer (or else Mango keeps sending a set command)
	sprintf(StringBuffer, "%sI3=%u,",StringBuffer, Input3Value);					//Add Digital Output1 value to packet buffer (or else Mango keeps sending a set command)
	sprintf(StringBuffer, "%sI4=%u,",StringBuffer, Input4Value);					//Add Digital Output1 value to packet buffer (or else Mango keeps sending a set command)
	
	watchdog_periodic();											//Feed the dog

	//CRC Check the wireless stream
//	printf("CRC Check..\r");
	//CRC Check serial string by summing each character in array into a 8 bit unsigned int
//	CRC = 0;								//Reset CRC to 0
//	for (i = 0; i < 130; i++) {
//		if (StringBuffer[i] == 0)			//If we have reached end of string i.e. a null terminator
//			break;							//Stop adding characters to CRC value
//		CRC += StringBuffer[i];				//Else we are still progressing thru the string and need to add each character value to running CRC
//	}
//	printf("\r");
	
//	watchdog_periodic();											//Feed the dog
//	sprintf(StringBuffer, "%s ~%d",StringBuffer, CRC);				//Add the CRC value to the string buffer

//	printf("%s\r",StringBuffer);			//Print the complete string buffer to the UART (debug)
//	printf("Sending message\r");			//Debug message
	
	//Send the string over the wireless network to the sink using RIME collect

//    printf("Sending\n");					//Debug message
    
//	seq_id++;
//	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
//    uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	
	
	////Turn GPS OFF
	//sendUBX(GPSoff, sizeof(GPSoff)/sizeof(uint8_t));	
	//Reset packet sending timer

	//******************SECOND MESSAGE *******************************
//	sprintf(StringBuffer, "GA,");
	if (GPS_Lat_Hemisphere > 48) 
	{
		sprintf(StringBuffer,"%sLAT=%s%c,",StringBuffer, GPS_LatString,GPS_Lat_Hemisphere);
		sprintf(StringBuffer, "%sLON=%s%c,",StringBuffer,GPS_LongString,GPS_Long_Hemisphere);	
	}
	dec = A1;
	frac = A1 - dec;	
	sprintf(StringBuffer, "%sA1=%d.%02u,", StringBuffer, dec, (unsigned int) (frac*100));				//Add CL designator and Analog1 value to packet buffer
	dec = A2;
	frac = A2 - dec;	
	sprintf(StringBuffer, "%sA2=%d.%02u,",StringBuffer, dec, (unsigned int) (frac*100));	//Add Analog2 value to packet buffer
	
	//CRC Check the wireless stream
//	printf("CRC Check..\r");
	//CRC Check serial string by summing each character in array into a 8 bit unsigned int
//	CRC = 0;								//Reset CRC to 0
//	for (i = 0; i < 130; i++) {
//		if (StringBuffer[i] == 0)			//If we have reached end of string i.e. a null terminator
//			break;							//Stop adding characters to CRC value
//		CRC += StringBuffer[i];				//Else we are still progressing thru the string and need to add each character value to running CRC
//	}
//	printf("\r");
	
	watchdog_periodic();											//Feed the dog
//	sprintf(StringBuffer, "%s ~%d",StringBuffer, CRC);				//Add the CRC value to the string buffer

	printf("%s\r",StringBuffer);			//Print the complete string buffer to the UART (debug)
	printf("Sending message\r");			//Debug message
	
	//Send the string over the wireless network to the sink using RIME collect

    printf("Sending\n");					//Debug message
     seq_id++;
	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
    uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	
}

/*
static void recv(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{  
	static char InputBuffer[100];									//Temporary buffer for storing the string received from the base node as a trickle broadcast
	packetbuf_copyto(InputBuffer);									//Copy the packet buffer to a temporary string variable (InputBuffer) as it could soon be overwritten
	printf("DATA: '%s'\r", InputBuffer);							//Debug message printing the message that was sent from the base node
	if (strstr(InputBuffer, ThisNodeAddress) > 0) {					//Look for the address of this node in the message (it is a network wide broadcast)
		printf("Got a message from server\r");						//If this node's address is in the message, continue with processing the string
		if (strstr(InputBuffer, "O1=1") > 0) {						//Search for 'O1=' in the string. This is the message to set Digital Output 1
			Relay1 = true;
			printf("RELAY1 ON\r");
		}			
		else if (strstr(InputBuffer, "O1=0") > 0) {					
			Relay1 = false;
			printf("RELAY1 OFF\r");
		}	
		else if (strstr(InputBuffer, "O1=2") > 0) {					
			Relay1 = pulse;
			printf("RELAY1 PULSE\r");
		}
		else if (strstr(InputBuffer, "O2=1") > 0) {						//Search for 'O1=' in the string. This is the message to set Digital Output 1
			Relay2 = true;
			printf("RELAY2 ON\r");
		}
		else if (strstr(InputBuffer, "O2=2") > 0) {					
			Relay2 = pulse;
			printf("RELAY1 PULSE\r");
		}		
		else if (strstr(InputBuffer, "O2=0") > 0) {					
			Relay2 = false;
			printf("RELAY2 OFF\r");
		}
		else if (strstr(InputBuffer, "O3=1") > 0) {						//Search for 'O1=' in the string. This is the message to set Digital Output 1
			Relay3 = true;
			printf("RELAY3 ON\r");
		}
		else if (strstr(InputBuffer, "O3=2") > 0) {					
			Relay3 = pulse;
			printf("RELAY3 PULSE\r");
		}
		else if (strstr(InputBuffer, "O3=0") > 0) {					
			Relay3 = false;
			printf("RELAY3 OFF\r");
		}
		else if (strstr(InputBuffer, "O4=1") > 0) {						//Search for 'O1=' in the string. This is the message to set Digital Output 1
			Relay4 = true;
			printf("RELAY4 ON\r");
		}
		else if (strstr(InputBuffer, "O4=2") > 0) {					
			Relay4 = pulse;
			printf("RELAY4 PULSE\r");
		}
		else if (strstr(InputBuffer, "O4=0") > 0) {					
			Relay4 = false;
			printf("RELAY2 OFF\r");
												//Update Mango with latest sensor readings
		}
		
		else														//We shouldn't get here but if we do, send a debug message to the UART
			printf("UNKNOWN OUTPUT\r");
			
		send_message(NULL);	
	}
}
*/
/*
static void
recv(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops)
{
  printf("Data received from %d.%d: %.*s (%d)\n",
	 from->u8[0], from->u8[1],
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());
  printf("RSSI=%d, HOPS=%u\r" , (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI),hops);

}
*/
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


void sendUBX(uint8_t *MSG, uint8_t len) {
	int i = 0;
	uart_write_byte(0, 0xff);		//Wake up device
	delay_msec(500);
	for(i=0; i<len; i++) {
		uart_write_byte(0, MSG[i]);
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
//	uart_write_byte(0,'T');
	//uart_write_byte(0,'C');
	//uart_write_byte(0,':');
	
    while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Universal Time
     {
	//	uart_write_byte(0,gps_buffer[x]);
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
	 
	// uart_write_byte(0,'\r');
	// uart_write_byte(0,'L');
	// uart_write_byte(0,'A');
	// uart_write_byte(0,'T');
	// uart_write_byte(0,':');
     while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Decimal Latitude
     {
//		uart_write_byte(0,gps_buffer[x]);
        GPS_LatString[i] = gps_buffer[x];
        x++;
        i++;
     }  
     
     x++;  //Search for next comma
     while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Latitude Hemisphere
     {
		uart_write_byte(0,gps_buffer[x]);
        GPS_Lat_Hemisphere = gps_buffer[x];
        x++;
     }
     
     x++;  //Search for next comma 
       
     i = 0;

   //  uart_write_byte(0,'\r');
	// uart_write_byte(0,'L');
	// uart_write_byte(0,'O');
//	 uart_write_byte(0,'N');
//	 uart_write_byte(0,':');
	 while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Decimal Longitude
     {
//		uart_write_byte(0,gps_buffer[x]);
        GPS_LongString[i] = gps_buffer[x];
        x++;
        i++;
     }  
     
     x++;  //Search for next comma
     
     while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the Longitude Hemisphere
     {
	//	uart_write_byte(0,gps_buffer[x]);
        GPS_Long_Hemisphere = gps_buffer[x];
        x++;
     }
     
    
     x++;  //Search for next comma
     
//	 uart_write_byte(0,'\r');
//	 uart_write_byte(0,'k');
//	 uart_write_byte(0,'M');
//	 uart_write_byte(0,'H');
//	 uart_write_byte(0,':');
    i = 0;
    GPS_SpeedString[0] = '0';
    while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the gps speed
     {
//		uart_write_byte(0,gps_buffer[x]);
        GPS_SpeedString[i] = gps_buffer[x];
        x++;
        i++;
     }  
                               
     x++;  //Search for next comma
     
    i = 0;

//	uart_write_byte(0,'\r');
//	uart_write_byte(0,'D');
//	uart_write_byte(0,'E');
//	uart_write_byte(0,'G');
//	uart_write_byte(0,':');
    GPS_BearingString[0] = '0';
    while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the bearing
     {
	//	uart_write_byte(0,gps_buffer[x]);      
		GPS_BearingString[i] = gps_buffer[x];
        x++;
        i++;
     }  
                               
     x++;  //Search for next comma   
     
      i = 0;

//	uart_write_byte(0,'\r');
//	uart_write_byte(0,'D');
//	uart_write_byte(0,'A');
//	uart_write_byte(0,'T');
//	uart_write_byte(0,':');

    while (gps_buffer[x] != ',' && x < StrLen)         //Strip out the UTC Date
     {
//		uart_write_byte(0,gps_buffer[x]);
        GPS_DateString[i] = gps_buffer[x];
        x++;
        i++;
     }  

//	uart_write_byte(0,'\r');
}

//.............................................................................

/*---------------------------------------------------------------------------*/
int gps_uart_rx_callback(unsigned char c)
{
	//This is data from the GPS
	watchdog_periodic();

    if (c == 0)   //Ignor a null character 
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
	
	static char InputBuffer[200];									//Temporary buffer for storing the string received from the base node as a trickle broadcast
	strcpy(InputBuffer, str);									//Copy the packet buffer to a temporary string variable (InputBuffer) as it could soon be overwritten
	
	printf("DATA: '%s'\r", InputBuffer);							//Debug message printing the message that was sent from the base node
	
	if (strstr(InputBuffer, ThisNodeAddress) > 0) {					//Look for the address of this node in the message (it is a network wide broadcast)
		printf("Got a message from server\r");						//If this node's address is in the message, continue with processing the string
		if (strstr(InputBuffer, "O1=1") > 0) {						//Search for 'O1=' in the string. This is the message to set Digital Output 1
			Relay1 = true;
			printf("RELAY1 ON\r");
			send_message(NULL);	
		}			
		else if (strstr(InputBuffer, "O1=0") > 0) {					
			Relay1 = false;
			printf("RELAY1 OFF\r");
			send_message(NULL);	
		}	
		else if (strstr(InputBuffer, "O1=2") > 0) {					
			Relay1 = pulse;
			printf("RELAY1 PULSE\r");
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "O2=1") > 0) {						//Search for 'O1=' in the string. This is the message to set Digital Output 1
			Relay2 = true;
			printf("RELAY2 ON\r");
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "O2=2") > 0) {					
			Relay2 = pulse;
			printf("RELAY1 PULSE\r");
			send_message(NULL);	
		}		
		else if (strstr(InputBuffer, "O2=0") > 0) {					
			Relay2 = false;
			printf("RELAY2 OFF\r");
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "O3=1") > 0) {						//Search for 'O1=' in the string. This is the message to set Digital Output 1
			Relay3 = true;
			printf("RELAY3 ON\r");
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "O3=2") > 0) {					
			Relay3 = pulse;
			printf("RELAY3 PULSE\r");
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "O3=0") > 0) {					
			Relay3 = false;
			printf("RELAY3 OFF\r");
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "O4=1") > 0) {						//Search for 'O1=' in the string. This is the message to set Digital Output 1
			Relay4 = true;
			printf("RELAY4 ON\r");
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "O4=2") > 0) {					
			Relay4 = pulse;
			printf("RELAY4 PULSE\r");
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "O4=0") > 0) {					
			Relay4 = false;
			printf("RELAY2 OFF\r");												//Update Mango with latest sensor readings
			send_message(NULL);	
		}
		else if (strstr(InputBuffer, "?") > 0) {					
			print_route_network();				//Send list of routes over network
		}
		else														//We shouldn't get here but if we do, send a debug message to the UART
			printf("UNKNOWN OUTPUT\r");
			
		
	}
	
	
	
	
	
  //  printf("DATA recv '%s'\n", str);
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

//This is the main Gate process.
//Here is where we set up I/O, timers and kick of the network processes
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_mesh_process, ev, data)
{
	PROCESS_BEGIN();
	
	//Set up relay outputs
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(0));			
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(0));
	
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(1));			
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(1));	
	
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(2));			
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(2));	
	
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(3));			
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(3));	
	
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));			
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(4));	
	
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));			
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));	
	
	//Set up digital inputs
	GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(5));		//IN1	
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(5));	
	
	GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));		//IN2	
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));	
	
	GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(6));		//IN4	
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(6));	
	
	GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(7));		//IN4	
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_A_NUM), GPIO_PIN_MASK(7));	
	
	//Turn GPS OFF
	//sendUBX(GPSoff, sizeof(GPSon)/sizeof(uint8_t));

	//Enable high gain mode on cc2592
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(2));
	
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
	//uart_set_input(0,uart_rx_callback);
	
	printf("Open GPS port\r");
	uart_set_input(1, gps_uart_rx_callback);
	
	etimer_set(&periodic, SEND_INTERVAL);											//Set up an event timer to send data back to base at a set interval
	watchdog_periodic();															//Feed the doge
	
  while(1) {
	PROCESS_YIELD();								//Pause the process until an event is triggered
	if(ev == tcpip_event) {
      tcpip_handler();
    } 
	if(etimer_expired(&periodic)) {					//The send message event timer expired variable	
		etimer_set(&periodic, SEND_INTERVAL);		//Reset the event timer (SEND_INTERVAL may have changed due to a node reconfiguration
		watchdog_periodic();						//Give the dog some food
		printf("Prepare Packet\r");					//Debug message
		
		//Turn GPS ON
		//printf("GPS ON\r");
		//sendUBX(GPSon, sizeof(GPSon)/sizeof(uint8_t));
		//Wait 45 seconds for GPS to get fix
		printf("Send message\r");
		ctimer_set(&backoff_timer, (10 * CLOCK_SECOND), send_message, NULL);	
		watchdog_periodic();				//Feed doge
	}
  }
  PROCESS_END();
}

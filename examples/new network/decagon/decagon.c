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

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

//Global Variables
float Voltage, A0, A1, A2, A3;										//Variables for storing ADC readings
float WireTemp[5] = {-99.9, -99.9, -99.9, -99.9, -99.9};			//Variables for storing one wire temperature readings
uint8_t DigitalInput1 = false;										//Value of Digital Input 1

float Soil, Soil_e, SoilTemp;

const char OneWireAddresses [5] = {0x20, 0x28, 0x30, 0x38, 0x40};	//EEPROM memory locations of stored one wire addresses (not the addresses themselves)

//Used as temporary variables for printing float values as direct printing of floats is not supported
int16_t dec;
float frac;

char ThisNodeAddress[12];											//String containing this nodes address (used for checking if a trickle message is for us)

static struct etimer periodic;										//Declare the timer used for sending measurements to the network
static struct ctimer backoff_timer;									//Declare the timer for delaying a measurement (get one wire data, delay and send)

char usart_rx_buffer[500];											//Define a buffer for storing serial port strings
int usart_rx_buffer_index = 0;										//And an index for that buffer

char usart1_rx_buffer[100];											//Define a buffer for storing serial port strings
int usart1_rx_buffer_index = 0;										//And an index for that buffer

unsigned char channel = 0x19;										//Set the RF channel to 0x19 by default			
unsigned char client[11] = {0,0,0,0,0,0,0,0,0,0,0};					//Declare a variable to store the client name (used as a reference only)
unsigned int MeasurementPeriod = 60;								//Default measurement period = 1 min

char OneWireDeviceFound;											//True if we found one wire devices on the bus after a prescence pulse
//Declare a variable to store one wire temperature sensor addresses
unsigned char TempAddr[5][8] = {{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
							   {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
							   {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
							   {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
							   {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};

//Define a couple of functions that appear out of order
void ReadFromEEPROM(void);
void WriteToEEPROM(void);

//CRC Lookup table used for DS18B20 one wire sensor
uint8_t dscrc_table[] = {
   0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
  157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220,
  35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98,
  190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
  70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7,
  219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154,
  101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36,
  248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185,
  140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
  17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
  175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
  50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
  202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139,
  87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
  233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
  116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

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

	printf("ADC Set\r");				//Print debug to UART

	lpm_set_max_pm(0);					//Disable power saving modes as this can affect the ADC / radio messages
	leds_on(LEDS_RED);					//Turn RED leds on to signal that power saving is disabled

	//Check digital input line. This is on port c, pin 5
	if (GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(5)) == 0)	//If PC.5 is low
		DigitalInput1 = 0;				//Set the DigitalInput1 variable to 0 to signify that Input1 is low
	else
		DigitalInput1 = 1;				//Else set the DigitalInput1 variable to 0 to signify that Input1 is low
		
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
	
	//Measure soil moisture sensor 1
    value = 0;
	for (i = 0; i < 20; i++) {
		value = value + adc_sensor.value(ADC_SENSOR_SENS1);
	}
	A1 = ((value / 20) * A0) / (2047 << 4);
	
	//Measure soil moisture sensor 2
	value = 0;
	for (i = 0; i < 20; i++) {
		value = value + adc_sensor.value(ADC_SENSOR_SENS2);
	}
	A2 = ((value / 20) * A0) / (2047 << 4);

	//Measure soil moisture sensor 3
//	value = 0;
//	for (i = 0; i < 50; i++) {
//		value = value + adc_sensor.value(ADC_SENSOR_SENS3);
//	}
//	A3 = ((value / 50) * A0) / (2047 << 4);
	
	//Measure soil moisture battery voltage
	value = 0;
	for (i = 0; i < 20; i++) {
		value = value + adc_sensor.value(ADC_SENSOR_SENS4);
	}
	//Voltage = (((value / 20) * A0) / (2047 << 4)) * 4.031;
	Voltage = (((value / 20) * A0) / (2047 << 4)) * 4.7212;

	//Turn off Soil Moisture Sensors (PORTB.5)
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
		
		
	//There is no float support for printf so we need to take a float value, get it's integer value and subtract the two.
	//This leaves us with the remainder. We then multiply it by 100 to turn it into an integer and print both values with a '.' in between
	//Analog1
	dec = A1;
	frac = A1 - dec;	
	sprintf(StringBuffer, "SO,A1=%d.%02u,", dec, (unsigned int) (frac*100));				//Add SO designator and Analog1 value to packet buffer
	//Analog2
	dec = A2;
	frac = A2 - dec;	
	sprintf(StringBuffer, "%sA2=%d.%02u,",StringBuffer, dec, (unsigned int) (frac*100));	//Add Analog2 value to packet buffer
	//Analog3
	dec = Soil;
	frac = Soil - dec;	
	sprintf(StringBuffer, "%sA3=%d.%02u,",StringBuffer, dec, (unsigned int) (frac*100));	//Add Analog3 value to packet buffer
	
	//Print the one wire temperature values. Soil moisture boards support 2 one wire sensors.
	for (i = 0; i < 2; i++)	{
		dec = WireTemp[i];							//Extract the integer part of the float reading
		if (WireTemp[i] > 0)						//If it is positive, extract the decimal part by subtracting original-integer and store in frac(float)
			frac = WireTemp[i] - dec;	
		else
			frac = -WireTemp[i] + dec;				////If it is negative, extract the decimal part by inverting original and adding integer and store in frac(float)
		sprintf(StringBuffer, "%sT%d=%d.%02u,",StringBuffer, (i + 1), dec, (unsigned int) (frac*100));	//Add each Temperature sensor value to packet buffer
	}
	dec = SoilTemp;
	frac = SoilTemp - dec;	
	sprintf(StringBuffer, "%sT3=%d.%02u,",StringBuffer, dec, (unsigned int) (frac*100));	//Add each Temperature sensor value to packet buffer
	
	//Digital input
	sprintf(StringBuffer, "%sI1=%d.0,",StringBuffer, DigitalInput1);						//Add Digital Input1 value to packet buffer
	//Battery voltage
	dec = Voltage;
	frac = Voltage - dec;	
	sprintf(StringBuffer, "%sBV=%d.%02u,",StringBuffer, dec, (unsigned int) (frac*100));	//Add Battery Voltage value to packet buffer
	
	watchdog_periodic();											//Feed the dog

//	//CRC Check the wireless stream
//	printf("CRC Check..\r");
//	//CRC Check serial string by summing each character in array into a 8 bit unsigned int
//	CRC = 0;								//Reset CRC to 0
//	for (i = 0; i < 130; i++) {
//		if (StringBuffer[i] == 0)			//If we have reached end of string i.e. a null terminator
//			break;							//Stop adding characters to CRC value
//		CRC += StringBuffer[i];				//Else we are still progressing thru the string and need to add each character value to running CRC
//	}
//	printf("\r");

//	watchdog_periodic();											//Feed the dog
//	sprintf(StringBuffer, "%s ~%d",StringBuffer, CRC);				//Add the CRC value to the string buffer

	printf("%s\r",StringBuffer);			//Print the complete string buffer to the UART (debug)
	printf("Sending message\r");			//Debug message
	
	//Send the string over the wireless network to the sink using RIME collect

    printf("Sending\n");					//Debug message
    
	seq_id++;
	PRINTF("DATA send to %d seq %d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
    uip_udp_packet_sendto(client_conn, StringBuffer, strlen(StringBuffer), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
	
	//Reset decagon readings in case sensor goes offline
	Soil = -99;
	SoilTemp = -99;
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
	else if (strncmp(usart_rx_buffer,"c",1) == 0) {							//Chars "c","d","e","f" and "g" are one wire address strings in ascii hex (MSB then LSB x 8)
		TempAddr[0][0] = ConvertASCIICharToHex(usart_rx_buffer[1]) << 4;
		TempAddr[0][0] += ConvertASCIICharToHex(usart_rx_buffer[2]);
		TempAddr[0][1] = ConvertASCIICharToHex(usart_rx_buffer[3]) << 4;
		TempAddr[0][1] += ConvertASCIICharToHex(usart_rx_buffer[4]);
		TempAddr[0][2] = ConvertASCIICharToHex(usart_rx_buffer[5]) << 4;
		TempAddr[0][2] += ConvertASCIICharToHex(usart_rx_buffer[6]);
		TempAddr[0][3] = ConvertASCIICharToHex(usart_rx_buffer[7]) << 4;
		TempAddr[0][3] += ConvertASCIICharToHex(usart_rx_buffer[8]);
		TempAddr[0][4] = ConvertASCIICharToHex(usart_rx_buffer[9]) << 4;
		TempAddr[0][4] += ConvertASCIICharToHex(usart_rx_buffer[10]);
		TempAddr[0][5] = ConvertASCIICharToHex(usart_rx_buffer[11]) << 4;
		TempAddr[0][5] += ConvertASCIICharToHex(usart_rx_buffer[12]);
		TempAddr[0][6] = ConvertASCIICharToHex(usart_rx_buffer[13]) << 4;
		TempAddr[0][6] += ConvertASCIICharToHex(usart_rx_buffer[14]);
		TempAddr[0][7] = ConvertASCIICharToHex(usart_rx_buffer[15]) << 4;
		TempAddr[0][7] += ConvertASCIICharToHex(usart_rx_buffer[16]);
	}
	else if (strncmp(usart_rx_buffer,"d",1) == 0) {
		TempAddr[1][0] = ConvertASCIICharToHex(usart_rx_buffer[1]) << 4;
		TempAddr[1][0] += ConvertASCIICharToHex(usart_rx_buffer[2]);
		TempAddr[1][1] = ConvertASCIICharToHex(usart_rx_buffer[3]) << 4;
		TempAddr[1][1] += ConvertASCIICharToHex(usart_rx_buffer[4]);
		TempAddr[1][2] = ConvertASCIICharToHex(usart_rx_buffer[5]) << 4;
		TempAddr[1][2] += ConvertASCIICharToHex(usart_rx_buffer[6]);
		TempAddr[1][3] = ConvertASCIICharToHex(usart_rx_buffer[7]) << 4;
		TempAddr[1][3] += ConvertASCIICharToHex(usart_rx_buffer[8]);
		TempAddr[1][4] = ConvertASCIICharToHex(usart_rx_buffer[9]) << 4;
		TempAddr[1][4] += ConvertASCIICharToHex(usart_rx_buffer[10]);
		TempAddr[1][5] = ConvertASCIICharToHex(usart_rx_buffer[11]) << 4;
		TempAddr[1][5] += ConvertASCIICharToHex(usart_rx_buffer[12]);
		TempAddr[1][6] = ConvertASCIICharToHex(usart_rx_buffer[13]) << 4;
		TempAddr[1][6] += ConvertASCIICharToHex(usart_rx_buffer[14]);
		TempAddr[1][7] = ConvertASCIICharToHex(usart_rx_buffer[15]) << 4;
		TempAddr[1][7] += ConvertASCIICharToHex(usart_rx_buffer[16]);
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
	int i, j;												//For loop counters	
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
	watchdog_periodic();
	SPI_WAITFORTx_AFTER();
	SPI_CS_SET(GPIO_B_NUM, 1);
	clock_delay_usec(10);
	
	//Refer to top example for how EEPROM SPI Read works
	//Read One Wire Addresses from EEPROM
	for (j = 0; j <2; j++)
	{
		SPI_CS_CLR(GPIO_B_NUM, 1);
		clock_delay_usec(10);
		SPI_WRITE(READ);									//Read command
		SPI_WRITE(OneWireAddresses[j]);						//Read address for 1st One Wire address
		SPI_FLUSH();
		for (i = 0; i <= 7; i++)	{						//8 bytes long
			SPI_READ(TempAddr[j][i]);
		}
		watchdog_periodic();
		SPI_WAITFORTx_AFTER();
		SPI_CS_SET(GPIO_B_NUM, 1);
		clock_delay_usec(10);
	}
	
	if (MeasurementPeriod < 10)								//If the value from EEPROM is less than 10, set to 30sec
		MeasurementPeriod = 30;
		
	//Print out everything we have read to the UART
	printf("@A=%d\r", channel);
	printf("@B=%s\r", client);
	printf("@C=%02x%02x%02x%02x%02x%02x%02x%02x\r", TempAddr[0][0], TempAddr[0][1], TempAddr[0][2], TempAddr[0][3], TempAddr[0][4], TempAddr[0][5], TempAddr[0][6], TempAddr[0][7]);
	printf("@D=%02x%02x%02x%02x%02x%02x%02x%02x\r", TempAddr[1][0], TempAddr[1][1], TempAddr[1][2], TempAddr[1][3], TempAddr[1][4], TempAddr[1][5], TempAddr[1][6], TempAddr[1][7]);
	printf("@R=%d\r", MeasurementPeriod);
	printf("@S=%u.%u\r",linkaddr_node_addr.u8[0], 	linkaddr_node_addr.u8[1]);
	printf("@T=SOIL\r");
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
		SPI_WRITE(OneWireAddresses[j]);							//Write one wire (j) address	
		SPI_WRITE(TempAddr[j][0]);
		SPI_WRITE(TempAddr[j][1]);
		SPI_WRITE(TempAddr[j][2]);
		SPI_WRITE(TempAddr[j][3]);
		SPI_WRITE(TempAddr[j][4]);
		SPI_WRITE(TempAddr[j][5]);
		SPI_WRITE(TempAddr[j][6]);
		SPI_WRITE(TempAddr[j][7]);

		SPI_CS_SET(GPIO_B_NUM, 1);								//Set CS high to start programming

		SPI_FLUSH();
		watchdog_periodic();
		delay_msec(10);
		printf("1Wire%d\r", j + 1);
		watchdog_periodic();
	}
	printf("SETTINGS SAVED TO EEPROM\r");						//Debug message
}

/*---------------------------------------------------------------------------*/
//Function to compute the one wire CRC (see datasheet)
//This was stolen from somewhere on the internet (Arduino one wire library)
//Uses a tricky polynomial function
unsigned char OWI_ComputeCRC8(unsigned char inData, unsigned char seed){
	unsigned char bitsLeft;
	unsigned char temp;

    for (bitsLeft = 8; bitsLeft > 0; bitsLeft--) {
        temp = ((seed ^ inData) & 0x01);
        if (temp == 0) {
            seed >>= 1;
        }
        else {
            seed ^= 0x18;
            seed >>= 1;
            seed |= 0x80;
        }
        inData >>= 1;
    }
    return seed;    
}

//Allow one wire data line to float
void SetOneWireToRX(void) {
	GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));		//Set PC4 to input
}

//Set the one wire data line to an output to pull the line low
void PullOneWireLow(void) {
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));		//Set PC4 to output
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));			//Set PC4 low (current sink)
}

//Send a '1' to the one wire bus
void SendOneWire1(void) {
	PullOneWireLow();					//Pull the data line low (sync pulse)
	clock_delay_usec(6);				//Wait 6 microseconds
	SetOneWireToRX();					//Float the data line to send a 1 (high)
	clock_delay_usec(64);				//Wait out the rest of the data window with the data line high
}

void SendOneWire0(void) {	
	PullOneWireLow();					//Pull data low for 10uS (sync) and keep low for the last 50uS (zero)
	clock_delay_usec(60);
	SetOneWireToRX();					//Float the data line after the data window
	clock_delay_usec(10);				//Wait settling time
}

char ReadOneWireBit(void) {
	PullOneWireLow();					//Pull the data line low (sync pulse)
	clock_delay_usec(6);				//Wait 6 microseconds
	SetOneWireToRX();					//Float data line and wait 9 seconds for slave to respond
	clock_delay_usec(9);
	if (GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4)))	{	//Check to see if slave has pulled line low (0) or left if floating (1)
		clock_delay_usec(55);			//Wait out complete data frame before
		return 1;						//Returning that a 1 was sent
	}
		clock_delay_usec(55);			//Else a zero must have been sent, wait out the frame
		return 0;
}

//Read a complete 8 bit byte from the one wire bus
unsigned char oneWireReadByte(void) {
	unsigned char i;					//Loop Counter
	unsigned char value = 0;			//Temporary result variable

	for (i = 0; i < 8; i++)	{
		if (ReadOneWireBit())
			value = value + (1 << i);	//Left shift each bit as it is read (we start with the LSB) and add it to the total
	}
	return value;
}

//Read the temperature values from the selected sensor
//Also reads the CRC value and compares the read CRC with the computed CRC
int ReadOneWireTemp(void) {
	unsigned char OneWireScratchpad[8];				//This is where the one wire scratchpad is temporarily held
	unsigned char CRC;								//This is where the CRC received form the node is stored
	unsigned char RunningCRC = 0;					//This is where the computed CRC is stored
	watchdog_periodic();							//About time we fed the Dog

	//Read each byte from the scratchpad.
	//There are 8 data bytes plus a CRC byte
	OneWireScratchpad[0] = oneWireReadByte();
	OneWireScratchpad[1] = oneWireReadByte();
 	OneWireScratchpad[2] = oneWireReadByte();
	OneWireScratchpad[3] = oneWireReadByte();
	OneWireScratchpad[4] = oneWireReadByte();
	OneWireScratchpad[5] = oneWireReadByte();
	OneWireScratchpad[6] = oneWireReadByte();
	OneWireScratchpad[7] = oneWireReadByte();	
	CRC = oneWireReadByte();

	watchdog_periodic();

	//Compute the CRC
	RunningCRC = OWI_ComputeCRC8(OneWireScratchpad[0], RunningCRC);
	RunningCRC = OWI_ComputeCRC8(OneWireScratchpad[1], RunningCRC);
	RunningCRC = OWI_ComputeCRC8(OneWireScratchpad[2], RunningCRC);
	RunningCRC = OWI_ComputeCRC8(OneWireScratchpad[3], RunningCRC);
	RunningCRC = OWI_ComputeCRC8(OneWireScratchpad[4], RunningCRC);
	RunningCRC = OWI_ComputeCRC8(OneWireScratchpad[5], RunningCRC);
	RunningCRC = OWI_ComputeCRC8(OneWireScratchpad[6], RunningCRC);
	RunningCRC = OWI_ComputeCRC8(OneWireScratchpad[7], RunningCRC);
	
	watchdog_periodic();

	//Compare the computed CRC with the read CRC
	if (CRC != RunningCRC)	{
		return -1584;				//If CRCs do not match return the equivalent of -99 - we deal with this higher up in the program
	}

	return ((int)OneWireScratchpad[1] << 8) + (long)OneWireScratchpad[0];		//If we got a CRC match return the 16 bit value for temperature (scratchpad 0 & 1)
}

//Send an 8 bit value to a one wire slave
void SendOneWireCommand(char value) {
	unsigned char i = 0;						//Loop counter

	for (i = 0; i < 8; i++)	{
		if (value & (0x01 << i))				//Set up a mask for checking if the byte at index i is a 1 or a 0
			SendOneWire1();						//If a 1, send a 1
		else
			SendOneWire0();						//And if a 0, send a 0
	}
	SetOneWireToRX();							//Back to default (data line floating) mode
}

//Send a reset to all devices on the network
void OneWireReset(void) {
	OneWireDeviceFound = false;					//Set the device found boolean value to false initially
	PullOneWireLow();							//Pull the line low for 480usec (reset pulse)
	watchdog_periodic();	
	clock_delay_usec(480);
	watchdog_periodic();	
	SetOneWireToRX();							//Float the line high
	clock_delay_usec(70);						//Wait 70microseconds for the line to settle
	watchdog_periodic();

	//Check to see if the line is low. This indicates a presence pulse from a (or many) slaves
	if (!GPIO_READ_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4))) {	   //If the line is low we found a slave
		OneWireDeviceFound = true;				//Set our device found boolean to true
		clock_delay_usec(410);					//Wait 410 microseconds
		watchdog_periodic();
	}
	else {												//If we did not detect any one wire devices
		printf("Error waiting for 1 wire device\r");	//Send out a debug message
		OneWireDeviceFound = false;						//Skip the rest of the one wire process
		return;
	}	
}

//Detect if there are any devices on the bus and if there is, instruct them to take a reading
//We read the data values from the scratchpad in the next one wire function.
void OneWireGetReading(void)
{
	//One wire devices are on port PC.4
	//We need to configure this as an open drain input, with no microcontroller pull up (it is on the PCB)
	GPIO_SOFTWARE_CONTROL(GPIO_C_BASE, 4);								//Software control of one wire data pin
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));		//Disable pull up
	GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));	//Set pin to input

	watchdog_periodic();					//Reset the watchdog timer
	OneWireReset();							//Send a reset pulse to one wire devices (and detect if there are devices on the bus)

	//Perform reading
	if (OneWireDeviceFound == true) {		//Check to see if any one wire devices were found before proceeding
		SendOneWireCommand(0xCC);			//Send skip ROM command to send a message to ALL slaves
		SendOneWireCommand(0x44);			//Send Convert T command to slaves
		delay_msec(500);					//Wait 1/2 sec for temperature reading
	}
	printf("Temperature conversion\r");		//Debug message
}

//Read temperature data from each one wire slave
//This should be sitting in the scratchpad after running OneWireGetReading and waiting 500msec
float ProcessOneWire(int SensorNo) {
	int OneWireValue;									//This is where the raw one wire temperature value is temporarily stored
	unsigned char i;									//Loop index
	if (OneWireDeviceFound == true)	{					//There is no point doing anything if we didn't detect any slaves
		OneWireReset();									//Send a reset pulse to one wire devices to sync the bus
		SendOneWireCommand(0x55);						//Send the match rom command followed by a slave address to talk to a single device

		for (i = 0; i < 8; i++)							//Send the address of SensorNo across the bus (8 bytes)
			SendOneWireCommand(TempAddr[SensorNo][i]);

		clock_delay_usec(100);							//Wait 100us before sending the
		SendOneWireCommand(0xBE);						//Read scratchpad command

		OneWireValue = ReadOneWireTemp();				//Then read the scratchpad values, compute crc, etc
		return OneWireValue * 0.0625;					//Multiply by (0.0625 per bit) and return as a float
	}
	return -99.0; 										//If there were no one wire devices found, return -99.0 (mango will ignore it)
}


char CalculateChecksum(char * Response){
	int length, sum = 0, i, crc;
	// Finding the length of the response string
	length = strlen(Response);
	// Adding characters in the response together
	for( i = 0; i < length; i++ ) {
		sum += Response[i];
	}
	//Add \rx to value
	sum += '\r';
	sum += 'x';
	// Converting checksum to a printable character
	crc = sum % 64 + 32;
	return crc;
 }
 
static void decagon_line(void) {
	char* token;
	char tokencount = 0;
	char DecagonCRC = 0;
	char CalculatedCRC = 255;
	
	if (strlen(usart1_rx_buffer) > 5)
	{
		token = strtok(usart1_rx_buffer, "\r");	
		while (token != NULL)
		{
			if (tokencount == 1) {
				printf("CRC:%c\r", token[1]);
				DecagonCRC = token[1];
			}
			token = strtok(NULL, "\r");
			tokencount++;
		}
		tokencount = 0;
		
		printf("DECAGON:%s\r", usart1_rx_buffer);
		CalculatedCRC = CalculateChecksum(usart1_rx_buffer);
		printf("CHECK:%c\r", CalculatedCRC);
		token = strtok(usart1_rx_buffer, " ");
	
		while (token != NULL)
		{
			if (tokencount == 0) {
				printf("M%s\r", token);
				Soil_e = atoi(token);
			}
			else if (tokencount == 2) {
				printf("T%s\r", token);
				SoilTemp = atoi(token);
				
			}
			token = strtok(NULL, " ");
			tokencount++;
		}
		
		if (CalculatedCRC == DecagonCRC) {
			printf("CRC Matches!!\r");
			Soil_e = Soil_e / 50;	
			Soil = 4.3 * pow(10,-6) * pow(Soil_e,3) - 5.5 * pow(10,-4) * pow(Soil_e,2) + 2.92 * pow(10,-2) * Soil_e - 5.3 * pow(10,-2);
			Soil = Soil * 100;
			dec = Soil;
			frac = Soil - dec;	
			printf("Smoist=%d.%02u\r", dec, (unsigned int) (frac*100));
				
			if (SoilTemp > 900){
				SoilTemp = 900 + 5 * (SoilTemp - 900);
			}
			SoilTemp = (SoilTemp - 400) / 10;
			dec = SoilTemp;
			frac = SoilTemp - dec;	
			printf("Tdeg=%d.%02u\r", dec, (unsigned int) (frac*100));
		}
		else {
			printf("CRC doesn't match!!\r");
			Soil = -99;
			dec = Soil;
			frac = Soil - dec;	
			printf("Smoist=%d.%02u\r", dec, (unsigned int) (frac*100));
			SoilTemp = -99;
			dec = SoilTemp;
			frac = SoilTemp - dec;	
			printf("Tdeg=%d.%02u\r", dec, (unsigned int) (frac*100));
		}
		
	}
	usart1_rx_buffer_index = 0;
	usart1_rx_buffer[0] = '\0';
	//Reset the UART buffer and return from the process line function
}

int uart1_rx_callback(unsigned char c){
	if(usart1_rx_buffer_index < 100) {						//The usart buffer string is 500 chars long so this prevents an overflow
		if ((c >= 32 && c <= 128) || c == '\n' || c == '\r'){
			usart1_rx_buffer[usart1_rx_buffer_index++] = c;		//Add a byte to the rx_buffer
			usart1_rx_buffer[usart1_rx_buffer_index] = '\0';		//Null terminate the next char in the buffer to prevent corruption
		}
	}
	else													//Reset the buffer if it has filled up with crap
		usart1_rx_buffer_index = 0;

	if(c == '\n')											//If we rx a full carriage return terminated string, process the string
		decagon_line();							

	return 1;												//We always return 1 (success)
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

//This is the main Gate process.
//Here is where we set up I/O, timers and kick of the network processes
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_mesh_process, ev, data)
{
	int j, Retry;
	float TempReading;
	PROCESS_BEGIN();
	
	//Set Digital Output1 to low
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));		//Take software control of Digital Output1 pin
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));			//Set Digital Output1 pin to an Output pin
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));				//Set Digital Output1 to low (OFF)

	spi_init();											//Initialise the SPI bus (this library is included in Contiki and is cc2538 specific)
	spi_cs_init(GPIO_B_NUM, 1);							//Configure PORTB.1 as the Chip Select (CS) line for the EEPROM chip on the SPI BUS
	SPI_CS_SET(GPIO_B_NUM, 1);							//Set the EEPROM CS line to high (chip not selected - default state)

	ReadFromEEPROM();											//Read all our saved values from SPI EEPROM
		
	//Set up one wire data pin (PINC.4) as GPIO input with no pullup
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));		//Enable software control of PINC.4
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(4));				//Make sure PINC.4 is set to no pullup

	//Set up Sleep mode control pin (PIND.5) as input with weak pullup (floating = sleep mode enabled & 0v = no sleep)
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));		//Enable software control of PIND.5
	GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));			//Set PIND.5 as input
	GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_D_NUM), GPIO_PIN_MASK(5));				//Enable weak pullup on PIND.5

	//Set analog inputs to ADC peripheral control
	ioc_set_over(GPIO_A_NUM, ADC_SENSOR_SENS1_PIN, IOC_OVERRIDE_ANA);
    ioc_set_over(GPIO_A_NUM, ADC_SENSOR_SENS2_PIN, IOC_OVERRIDE_ANA);
 //   ioc_set_over(GPIO_A_NUM, ADC_SENSOR_SENS3_PIN, IOC_OVERRIDE_ANA);
    ioc_set_over(GPIO_A_NUM, ADC_SENSOR_SENS4_PIN, IOC_OVERRIDE_ANA);
	GPIO_PERIPHERAL_CONTROL(GPIO_A_NUM, ADC_SENSOR_SENS1_PIN);
	GPIO_PERIPHERAL_CONTROL(GPIO_A_NUM, ADC_SENSOR_SENS2_PIN);
//	GPIO_PERIPHERAL_CONTROL(GPIO_A_NUM, ADC_SENSOR_SENS3_PIN);
	GPIO_PERIPHERAL_CONTROL(GPIO_A_NUM, ADC_SENSOR_SENS4_PIN);
	
	//Set up PINC.5 as an input. This is the Digital Input1 pin
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(5));			//Set PORTC.5 to software peripheral control
	GPIO_SET_INPUT(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(5));				//Set PORTC.5 as input
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_C_NUM), GPIO_PIN_MASK(5));					//No pull-up
	
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
		
		//Turn on Soil Moisture Sensors (PORTB.5)
		GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
		GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
		GPIO_SET_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
		OneWireGetReading();						//Instruct one wire devices to take a measurement (takes 1/2 sec)
			
		//Take one wire readings (Soil moisture node has up to 2 one wire sensors
		for (j = 0; j < 2; j++) {
			for (Retry = 0; Retry < 5; Retry++) {				//Sometimes the one wire function gets interrupted and the data gets corrupted, we retry up to 5 times
				TempReading = ProcessOneWire(j);				//Call ProcessOneWire on node number (j) and store the result in TempReading as a float
				dec = TempReading;								//Do our little trick for printing floats
				if (TempReading > 0)
					frac = TempReading - dec;	
				else
					frac = -TempReading + dec;	
				printf("Temp%d = %d.%02u\r", j +1, dec, (unsigned int) (frac*100));			//Debug printing to UART
				if (!(TempReading == 85.0 || TempReading == -99)) {							//If the value read is 85 (Default) or -99 (CRC Error), throw data away
					WireTemp[j] = TempReading;												//If not, store and break out of retry loop to read the next sensor
					break;
				}
			}				
		}
		
		ctimer_set(&backoff_timer, (random_rand() % (3 * CLOCK_SECOND)), send_message, NULL);		//Wait 3 seconds for soil moisture node to settle and then send data
		watchdog_periodic();				//Feed doge
	}
  }
  PROCESS_END();
}

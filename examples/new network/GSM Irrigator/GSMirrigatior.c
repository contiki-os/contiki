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
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "dev/watchdog.h"
#include "dev/serial-line.h"
#include "dev/sys-ctrl.h"
#include "lpm.h"
#include "dev/uart.h"
#include "dev/adc-sensor.h"
#include "spi-arch.h"
//#include "spi.h"
#include "cpu.h"
#include "dev/cc2538-rf.h"
#include "net/netstack.h"
#include "random.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/adc-sensor.h"
#include "dev/leds.h"

#define SAPBR 10
#define CREG 11
#define HTTPINIT 12
#define HTTPPARA 13
#define HTTPACTION 14
#define HTTPREAD 15
#define HTTPTERM 16

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

static struct ctimer GPSStartTimer;

float Voltage, A0, A1;
uint8_t DigitalInput1 = false;

int16_t rv, dec;
float frac;

char ThisNodeAddress[12];											//String containing this nodes address (used for checking if a trickle message is for us)
char UART1TXBuffer[200];

static struct etimer periodic;										//Declare the timer used for sending measurements to the network
//static struct ctimer backoff_timer;									//Declare the timer for delaying a measurement (get one wire data, delay and send)

static struct etimer spoolTimer;
static struct etimer wheelTimer;

char usart_rx_buffer[500];											//Define a buffer for storing serial port strings
int usart_rx_buffer_index = 0;										//And an index for that buffer

static char response_string[200];
static char StringBuffer[200];				//Buffer for building the string of readings sent over the wireless network

static char gsm_buffer[300];
static int gsm_buffer_index = 0;

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
unsigned int MeasurementPeriod = 20;								//Default measurement period = 1 min

uint8_t GPSon[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x4C, 0x37};
uint8_t GPSoff[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B};

bool wheelTurning, spoolTurning;

void sendUBX(uint8_t *MSG, uint8_t len);
int sendHTTPdata();

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
	watchdog_periodic();				//Feed the dog
	int32_t value;						//Temporary variable for storing ADC readings. 32bit as readings are averaged
	unsigned char i;				//Temporary variables for storing string loop index
	
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
	sprintf(StringBuffer, "__device=IRRIGATOR&IRA1=%d.%02u&", dec, (unsigned int) (frac*100));				//Add SO designator and Analog1 value to packet buffer
	
	sprintf(StringBuffer, "%sIRLAT=%s%c,",StringBuffer,GPS_LatString,GPS_Lat_Hemisphere);
	sprintf(StringBuffer, "%sIRLON=%s%c,",StringBuffer,GPS_LongString,GPS_Long_Hemisphere);	
	
	if (spoolTurning)
		sprintf(StringBuffer, "%sIRSpool=1&",StringBuffer);	
	else
		sprintf(StringBuffer, "%sIRSpool=0&",StringBuffer);	

	if (wheelTurning)
		sprintf(StringBuffer, "%sIRWheel=1",StringBuffer);	
	else
		sprintf(StringBuffer, "%sIRWheel=0",StringBuffer);	
	
	watchdog_periodic();											//Feed the dog

	printf("%s\r",StringBuffer);			//Print the complete string buffer to the UART (debug)

    printf("Sending\n");					//Debug message
	sendHTTPdata();
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
	//	ReadFromEEPROM();								//Read settings from EEPROM - the ReadFromEEPROM function will send out the data
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
	//		WriteToEEPROM();
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
unsigned int uart1_send_bytes()
{
  int i = 0;
  
  while (UART1TXBuffer[i] != 0)
  {
	  if (i > 200)
		return i;
	
	  uart_write_byte(1,UART1TXBuffer[i++]);
  }
  return i;
}

int waitForResponse(int param)
{
	int i, timeout;
	
	timeout = 300;
	
	for (i = 0; i < timeout; i++)
	{
			if (param == SAPBR)
			{
				if (strcmp(response_string, "OK") > 0)
					return 1;					
			}
			else if (param == CREG)
			{
				if (strcmp(response_string, "OK") > 0)
					return 1;
			}
			else if (param == HTTPINIT)
			{
				if (strcmp(response_string, "OK") > 0)
					return 1;
			}
			else if (param == HTTPPARA)
			{
				if (strcmp(response_string, "OK") > 0)
					return 1;
			}
			else if (param == HTTPACTION)
			{
				if (strcmp(response_string, "OK") > 0)
					return 1;
			}
			else if (param == HTTPREAD)
			{
				if (strcmp(response_string, "+HTTPREAD:") > 0)
				{
					printf(response_string);
					return 1;
				}
			}
			else if (param == HTTPTERM)
			{
				if (strcmp(response_string, "OK") > 0)
					return 1;
			}
		delay_msec(10);
	}
	
	return 0;

}

int sendHTTPdata()
{
	printf("Get Bearer Profiles\r");
	sprintf(UART1TXBuffer, "AT+SAPBR=4,1\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);
	
	//if (!waitForResponse(SAPBR))
	//	return 0;
	delay_msec(500);
	
	printf("Open Bearer 1\r");
	sprintf(UART1TXBuffer, "AT+SAPBR=1,1\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);
	
	//if (!waitForResponse(SAPBR))
	//	return 0;
	delay_msec(500);
	
	printf("Check registration status\r");
	sprintf(UART1TXBuffer, "AT+CREG?\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);

	//if (!waitForResponse(CREG))
	//	return 0;
	delay_msec(500);
	
	printf("Query bearer 1\r");
	sprintf(UART1TXBuffer, "AT+SAPBR=2,1\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);
	
	//if (!waitForResponse(SAPBR))
	//	return 0;
	delay_msec(500);
	
	printf("Initialize HTTP service\r");
	sprintf(UART1TXBuffer, "AT+HTTPINIT\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);

	//if (!waitForResponse(HTTPINIT))
	//	return 0;
	delay_msec(500);
	
	printf("Send URL\r");
	sprintf(UART1TXBuffer, "AT+HTTPPARA=\"URL\",\"http://onfarmdata.com/httpds?%s\"\r", StringBuffer);
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);

	//if (!waitForResponse(HTTPPARA))
	//	return 0;
	delay_msec(500);
	
	printf("Send CID\r");
	sprintf(UART1TXBuffer, "AT+HTTPPARA=\"CID\",1\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);

	//if (!waitForResponse(HTTPPARA))
	//	return 0;
	delay_msec(500);
	
	printf("Send HTTP Action\r");
	sprintf(UART1TXBuffer, "AT+HTTPACTION=0\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);

	//if (!waitForResponse(HTTPACTION))
	//	return 0;
	delay_msec(500);
	
	printf("Send HTTP Read command\r");
	sprintf(UART1TXBuffer, "AT+HTTPACTION=0\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);

	//if (!waitForResponse(HTTPREAD))
	//	return 0;
	delay_msec(500);
	
	printf("Terminate HTTP session\r");
	sprintf(UART1TXBuffer, "AT+HTTPTERM\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);

	//if (!waitForResponse(HTTPTERM))
	//	return 0;
	delay_msec(500);
	
	return 1;
}
 
void ExtractDataFromGPS_String(void)
{
	unsigned char x, i, StrLen;      
	
       
    //The format of the GPRMC string is:
    //"GPRMC,UTC,FIX,LAT,lATHEM,LONG,LONGHEM,SPEED,BEARING,DATE(UTC),CHECKSUM"

    x = strncmp(response_string, "32", 2);    //Compare the first 5 characters of the Rx0Buffer string with "GPRMC"
    if (x!=0) return;                    //If not matching the exit
     
	printf("Extracting GPS Data\r");
    StrLen = strlen(response_string);            //Find out the length of the GPS string
          
    //The first variable is UTC time, starting at character 3
    x = 3;         
    i = 0;     

	uart_write_byte(0, '\r');
	uart_write_byte(0, 'U');
	uart_write_byte(0,'T');
	uart_write_byte(0,'C');
	uart_write_byte(0,':');
	
    while (response_string[x] != ',' && x < StrLen)         //Strip out the Universal Time
     {
		uart_write_byte(0,response_string[x]);
        GPS_TimeString[i] = response_string[x];
        x++;
        i++;
     }
     
     x++;
     
     if (response_string[x++] == 'A')
        FoundSatellite = true;
     else
	 {
		FoundSatellite = false;
        return;
	 }
     
     x++;  //Search for next comma
     
     i = 0;
	 
	 uart_write_byte(0,'\r');
	 uart_write_byte(0,'L');
	 uart_write_byte(0,'A');
	 uart_write_byte(0,'T');
	 uart_write_byte(0,':');
     while (response_string[x] != ',' && x < StrLen)         //Strip out the Decimal Latitude
     {
		uart_write_byte(0,response_string[x]);
        GPS_LatString[i] = response_string[x];
        x++;
        i++;
     }  
     
     x++;  //Search for next comma
     while (response_string[x] != ',' && x < StrLen)         //Strip out the Latitude Hemisphere
     {
		uart_write_byte(0,response_string[x]);
        GPS_Lat_Hemisphere = response_string[x];
        x++;
     }
     
     x++;  //Search for next comma 
       
     i = 0;

     uart_write_byte(0,'\r');
	 uart_write_byte(0,'L');
	 uart_write_byte(0,'O');
	 uart_write_byte(0,'N');
	 uart_write_byte(0,':');
	 while (response_string[x] != ',' && x < StrLen)         //Strip out the Decimal Longitude
     {
		uart_write_byte(0,response_string[x]);
        GPS_LongString[i] = response_string[x];
        x++;
        i++;
     }  
     
     x++;  //Search for next comma
     
     while (response_string[x] != ',' && x < StrLen)         //Strip out the Longitude Hemisphere
     {
		uart_write_byte(0,response_string[x]);
        GPS_Long_Hemisphere = response_string[x];
        x++;
     }
     
    
     x++;  //Search for next comma
     
	 uart_write_byte(0,'\r');
	 uart_write_byte(0,'k');
	 uart_write_byte(0,'M');
	 uart_write_byte(0,'H');
	 uart_write_byte(0,':');
    i = 0;
    GPS_SpeedString[0] = '0';
    while (response_string[x] != ',' && x < StrLen)         //Strip out the gps speed
     {
		uart_write_byte(0,response_string[x]);
        GPS_SpeedString[i] = response_string[x];
        x++;
        i++;
     }  
                               
     x++;  //Search for next comma
     
    i = 0;

	uart_write_byte(0,'\r');
	uart_write_byte(0,'D');
	uart_write_byte(0,'E');
	uart_write_byte(0,'G');
	uart_write_byte(0,':');
    GPS_BearingString[0] = '0';
    while (response_string[x] != ',' && x < StrLen)         //Strip out the bearing
     {
	//	uart_write_byte(0,response_string[x]);      
		GPS_BearingString[i] = response_string[x];
        x++;
        i++;
     }  
                               
     x++;  //Search for next comma   
     
      i = 0;

	uart_write_byte(0,'\r');
	uart_write_byte(0,'D');
	uart_write_byte(0,'A');
	uart_write_byte(0,'T');
	uart_write_byte(0,':');

    while (response_string[x] != ',' && x < StrLen)         //Strip out the UTC Date
     {
		uart_write_byte(0,response_string[x]);
        GPS_DateString[i] = response_string[x];
        x++;
        i++;
     }  

	uart_write_byte(0,'\r');
}

static void start_GPS(void *ptr)
{
	printf("Disable Local Echo\r");
	sprintf(UART1TXBuffer, "ATE0\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);
	
	delay_msec(500);
			
	printf("Turn GPS ON\r");
	sprintf(UART1TXBuffer, "AT+CGPSPWR=1;+CGPSRST=1\r");
	uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);
}

//.............................................................................
/*---------------------------------------------------------------------------*/
int uart1_rx_callback(unsigned char c)
{
    if (c == 0 || c == '\n')   //Ignore a null character or linefeed
        return 0;   
       
    if (c == '\r')
    {
		printf("->");
        gsm_buffer_index = 0;  
		strncpy(response_string, gsm_buffer, 200);		
		printf(response_string);
		printf("\r");
		if (strcmp(response_string, "32,") > 0)
		{
			ExtractDataFromGPS_String();
		}
		
		if (strcmp(response_string, "GPS Ready") == 0)
		{
			ctimer_set(&GPSStartTimer, CLOCK_SECOND * 5, start_GPS, NULL);
		}
		return 1;
    }   
         
    else
    {
        gsm_buffer[gsm_buffer_index++] = c;
        gsm_buffer[gsm_buffer_index] = 0;
    } 


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
	
//	ReadFromEEPROM();											//Read all our saved values from SPI EEPROM

	timer_set(&debouncetimer, 0);
	
	config_spool_pulse();
	config_wheel_pulse();
	
	//Turn off 9V output(PORTB.5)
	GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
	GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
	GPIO_CLR_PIN(GPIO_PORT_TO_BASE(GPIO_B_NUM), GPIO_PIN_MASK(5));
			
	PROCESS_PAUSE();

	//Set up the UART for communication with the programmer
	printf("UART BEGIN\r");
	uart_set_input(0,uart_rx_callback);
	uart_set_input(1,uart1_rx_callback);

	etimer_set(&periodic, SEND_INTERVAL);											//Set up an event timer to send data back to base at a set interval
	etimer_set(&spoolTimer, 60 * CLOCK_SECOND);
	etimer_set(&wheelTimer, 60 * CLOCK_SECOND);
	
	
	delay_msec(500);
		
    while(1) {
	    PROCESS_YIELD();								//Pause the process until an event is triggered
	
	    if(etimer_expired(&periodic)) {					//The send message event timer expired variable
    		etimer_set(&periodic, SEND_INTERVAL);		//Reset the event timer (SEND_INTERVAL may have changed due to a node reconfiguration)
    		printf("Prepare Packet\r");					//Debug message
			//Get GPS Info
			printf("Get GPS Info\r");
			sprintf(UART1TXBuffer, "AT+CGPSINF=32\r");
			uart1_send_bytes((uint8_t *)UART1TXBuffer,sizeof(UART1TXBuffer)-1);
	
			
			send_message(NULL);
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

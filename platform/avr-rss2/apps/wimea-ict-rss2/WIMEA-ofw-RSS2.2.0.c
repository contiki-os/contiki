/*
 * Copyright (c) 2015, Copyright Robert Olsson / Radio Sensors AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 *
 * Some code adopted from Robert Olsson <robert@herjulf.se> and Manee
 * @Author: 
 * Nsabagwa Mary <mnsabagwa@cit.ac.ug>
 * Okello Joel <okellojoelocaye@gmail.com>,
 * Alinitwe Sandra Kamugisha <sandraalinitwe@gmail.com>
 * Byamukama Maximus <maximus.byamukama@gmail.com>
 *
 * The application reads sensor data, transmits it via broadbast, unicast .. using RIME
 */

#include "contiki.h"
#include "WIMEA-ofw-RSS2.2.0.h"
#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "dev/serial-line.h"
#include <avr/eeprom.h>
#include "sys/etimer.h"
#include "dev/adc.h"
#include "dev/i2c.h"
#include "dev/leds.h"
#include "dev/battery-sensor.h"
#include "dev/temp-sensor.h"
#include "dev/temp_mcu-sensor.h"
#include "dev/ds1307.h"
#include "dev/sht25.h"
#include "math.h"
#include "dev/mcp3424.h"
#include "dev/ms5611.h"
#include "lib/fs/fat/diskio.h"
#include "lib/fs/fat/ff.h"
#include "net/rime/rime.h"
#include "netstack.h"

#define NAME_LENGTH 10
#define TAGMASK_LENGTH 50
#define NO_SENSORS 10
#define MAX_BCAST_SIZE 99
#define DEF_TTL 0xF

//Configuration Parameters
uint16_t EEMEM eemem_transmission_interval;
uint8_t EEMEM eemem_node_name[NAME_LENGTH];
uint8_t EEMEM eemem_adc1[NAME_LENGTH];
uint8_t EEMEM eemem_adc2[NAME_LENGTH];
uint8_t EEMEM eemem_tagmask[TAGMASK_LENGTH];
/*flags to check if eeprom has valid values*/
uint16_t EEMEM eemem_interval_flag;
uint16_t EEMEM eemem_name_flag;
uint16_t EEMEM eemem_tagmask_flag;

PROCESS(serial_input_process, "Serial line input commands");
PROCESS(default_config_process, "Default configurations");
PROCESS(broadcast_data_process, "Broadcast sensor data");
PROCESS(sensor_data_process, "Read sensor data");
PROCESS(buffer_process, "Buffer sensor data");

uint16_t time_interval;
struct etimer et;
unsigned char eui64_addr[8];
static char default_sensors[]="ADC_1 ADC_2 ADC_3 ADC_4 RH V_AD1 V_AD2 V_IN ADC T_MCU V_MCU ";
uint16_t rssi, lqi; //Received Signal Strength Indicator(RSSI), Link Quality Indicator(LQI)
struct broadcast_message {
	uint8_t head;
	uint8_t seqno;
	char buf[MAX_BCAST_SIZE+20];
};
static struct broadcast_conn broadcast;
uint8_t ttl=DEF_TTL;

FATFS FatFs;
FIL *fp;
UINT bw;

/*function to time stamp files on the SD card, using date and time from RTC or hard coded incase RTC is absent. */
DWORD get_fattime (void)
{
	if( i2c_probed & I2C_DS1307 ) {
		return ((DWORD)(ds1307_sensor.value(11) - 1980) << 25)	/* RTC Year */
			| ((DWORD)ds1307_sensor.value(10) << 21)			/* RTC Month*/
			| ((DWORD)ds1307_sensor.value(9) << 16)				/* RTC day*/
			| ((DWORD)ds1307_sensor.value(8) << 11)				/* RTC Hours*/
			| ((DWORD)ds1307_sensor.value(7) << 5)				/* RTC Mins*/
			| ((DWORD)ds1307_sensor.value(6)/2 >> 1);			/* RTC Sec*/
	} else {
		return ((DWORD)(2017 - 1980) << 25)	/* Year 2017 */
			| ((DWORD)1 << 21)				/* Month 1 */
			| ((DWORD)1 << 16)				/* Mday 1 */
			| ((DWORD)0 << 11)				/* Hour 0 */
			| ((DWORD)0 << 5)				/* Min 0 */
			| ((DWORD)0 >> 1);				/* Sec 0 */
       }
}

AUTOSTART_PROCESSES(&default_config_process, &buffer_process, &sensor_data_process, &broadcast_data_process, &serial_input_process);

PROCESS_THREAD(default_config_process, ev, data)
{
	uint16_t interval_flag, tagmask_flag;
	char adc1[NAME_LENGTH]="V_AD1";
	char adc2[NAME_LENGTH]="V_AD2";
	
	PROCESS_BEGIN();
	i2c_at24mac_read((char *) &eui64_addr, 1);
	cli();
	interval_flag = eeprom_read_word(&eemem_interval_flag);
	tagmask_flag = eeprom_read_word(&eemem_tagmask_flag);

	if (interval_flag != 1) {
		time_interval = 60;
		eeprom_update_word(&eemem_transmission_interval, time_interval);
		interval_flag=1;
		eeprom_update_word(&eemem_interval_flag, interval_flag);
	}

	if (tagmask_flag != 1) {
		set_default_tagmask();
		tagmask_flag=1;
		eeprom_update_word(&eemem_tagmask_flag, tagmask_flag);
		eeprom_update_block((const void *)&adc1, (void *)&eemem_adc1, NAME_LENGTH);
		eeprom_update_block((const void *)&adc2, (void *)&eemem_adc2, NAME_LENGTH);
	}
	sei();
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(buffer_process, ev, data)
{
	int i;
	PROCESS_BEGIN();
	/*** init sdcard ****/
	f_mount(0, &FatFs);		// Give a work area to the FatFs module 
	fp = (FIL *)malloc(sizeof (FIL));
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
		i=strlen(data);
		/*write to sd card*/
		if (f_open(fp, "sensor.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {	// Open existing or create new file
			if (f_lseek(fp, f_size(fp)) == FR_OK) {
				f_write(fp, (char*)data, i+2, &bw);	// Write data to the file
			}
			if (bw == i+2) {//we wrote the entire string 
				leds_on(LEDS_YELLOW);
			} else {
				leds_on(LEDS_RED);
			}
			f_close(fp);// close the file
			} else {
				leds_on(LEDS_RED);
			}
    }
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_data_process, ev, data)
{
	PROCESS_BEGIN();
	SENSORS_ACTIVATE(temp_sensor);
	SENSORS_ACTIVATE(temp_mcu_sensor);
	SENSORS_ACTIVATE(battery_sensor);

        if( i2c_probed & I2C_SHT25) {                
               // sht25_init();
               // printf("initialized");
                SENSORS_ACTIVATE(sht25_sensor);
                strcpy(default_sensors,"T RH V_AD1 V_AD2 V_IN ADC T_MCU V_MCU");           
       
        }
         if( i2c_probed & I2C_MCP3424 ) {
		//mcp3424_init(MCP3424_ADDR,0,8,16);  
		SENSORS_ACTIVATE(mcp3424_sensor);
           strcpy(default_sensors,"ADC_1 ADC_2 ADC_3 ADC_4 V_AD1 V_AD2 V_IN ADC T_MCU V_MCU"); 
	}

	 if(i2c_probed & I2C_MS5611_ADDR){
		SENSORS_ACTIVATE(ms5611_sensor);
           strcpy(default_sensors,"P RH V_AD1 V_AD2 V_IN ADC T_MCU V_MCU");
	}

	if( i2c_probed & I2C_DS1307 ) {
			DS1307_init();
			SENSORS_ACTIVATE(ds1307_sensor);
	}
        
	while(1) {
		time_interval=eeprom_read_word(&eemem_transmission_interval);
		etimer_set(&et, CLOCK_SECOND * time_interval);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		read_sensor_values();
	}

	SENSORS_DEACTIVATE(temp_sensor);
	SENSORS_DEACTIVATE(temp_mcu_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(ds1307_sensor);
        SENSORS_DEACTIVATE(ms5611_sensor);
        SENSORS_DEACTIVATE(mcp3424_sensor);
        SENSORS_DEACTIVATE(sht25_sensor);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(serial_input_process, ev, data)
{
	PROCESS_BEGIN();
	char delimiter[]=" ";
	char *command = NULL;
	uint8_t flag;
	char * value;
	for(;;){
		PROCESS_YIELD_UNTIL(ev == serial_line_event_message);
		command = (char*)strtok((char*)data, (const char*)delimiter);
		if(!strncmp(command, "h", 1)) {
			if (strlen(command) == 1){
				print_help_command_menu();
			} else {
				printf("Invalid command %s. Try h for a list of commands\n", command);
			}
		} else if (!strncmp(command,"ri",2)) {
			if(strlen(command)>2) {
				value=(char*) malloc(6);
				strlcpy(value, command+3, 6);//(strlen(command)-3)
				change_reporting_interval(value);
				free(value);
			} else {
				display_reporting_interval();
			}
		} else if (!strncmp(command,"ss",2)) {
			display_system_information();
		} else if(!strncmp(command, "name", 4)) {
			if(strlen(command)>4){
				value=(char*) malloc(12);
				strlcpy(value, command+5, 12);//(strlen(command)-3)
				change_node_name(value);
				free(value);
			} else {
				display_node_name();
			}
		} else if (!strncmp(command,"u", 1)){
			display_system_uptime();
		} else if(!strncmp(command,"date", 4)) {//date setup
			if (strlen(command) == 4){
				printf("Date: %s\n", return_date());
			} else if (strlen(command)>4 && strlen(command)<=13) {
				value = (char*) malloc(9);
				flag = 0;
				strlcpy(value, command+5, 9);
				set_datetime(value, flag);
				free(value);
			} else {
				printf("Invalid command: %s. Try example date 01/01/17.\n", command);
			}
		} else if(!strncmp(command,"time", 4)) {//time setup
			if (strlen(command) == 4){
				printf("Time: %s\n", return_time());
			} else if (strlen(command)>4 && strlen(command)<=13) {
				value = (char*) malloc(9);
				flag = 1;
				strlcpy(value, command+5, 9);
				set_datetime(value, flag);
				free(value);
			} else{
				printf("Invalid command: %s. Try example time 13:01:56.\n", command);
			}
		} else if(!strncmp(command, "tagmask", 7)) {
			if (strlen(command) == 7){
				display_tagmask();
			} else {
				value=(char*) malloc(TAGMASK_LENGTH);
				strlcpy(value, command+8, TAGMASK_LENGTH);
				if (!strncmp(value, "auto", 4)){
					set_default_tagmask();
					display_tagmask();
				} else if(strlen(value)>0) {
					change_tagmask(value);
				}
				free(value);
			}
		} else if(!strncmp(command, "alias", 5)) {
			if (strlen(command) == 5){
				printf("V_AD1=%s\n", return_alias(1));
				printf("V_AD2=%s\n", return_alias(2));
			} else {
				value=(char*) malloc(20);
				strlcpy(value, command+6, 19);
				if (strlen(value)>0){
					change_alias(value);
				} else {
					printf("Can not set alias name: %s. Try 'alias V_AD1=soil'\n", value);
				}
				free(value);
			}
		} else {
			printf("Invalid command %s. Try h for a list of commands\n", command);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct broadcast_message *msg_recv;
	msg_recv = packetbuf_dataptr();
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
    printf("%s [ADDR=%d.%d RSSI=%u LQI=%u TTL=%u SEQ=%u]\n", (char *)msg_recv->buf, from->u8[0], from->u8[1], rssi, lqi, msg_recv->head & 0xF, msg_recv->seqno);
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
/*---------------------------------------------------------------------------*/
/*This process waits for a synchronous event and broadcasts the data received from the sender process*/
PROCESS_THREAD(broadcast_data_process, ev, data)
{
	char node[NAME_LENGTH];
	uint8_t node_name[NAME_LENGTH];
	uint8_t len, i=0;
	static uint8_t seqno;
	struct broadcast_message msg;
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
    PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
		cli();
		eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);
		sei();
		len=strlen(data);
              //  i += snprintf(msg.buf+i, 15, "&: NAME=%s ", );
		if( i2c_probed & I2C_DS1307 ) {
				i += snprintf(msg.buf+i, 25, "%s %s ", return_date(), return_time());
		}
		if (node_name>0){
			strlcpy(node, (char*)node_name, NAME_LENGTH);
			i += snprintf(msg.buf+i, 15, "&: NAME=%s ", node);
		}
		/* Read out mote 64bit MAC address */
		len+=25;
		i += snprintf(msg.buf+i, len, "E64=%02x%02x%02x%02x%02x%02x%02x%02x %s", eui64_addr[0], eui64_addr[1], eui64_addr[2], eui64_addr[3], eui64_addr[4], eui64_addr[5], eui64_addr[6], eui64_addr[7], (char*)data);
		msg.buf[i++]='\0';//null terminate report.
		printf("%s\n", msg.buf);
		msg.head = 1<<4;
		msg.head |= ttl;
		msg.seqno = seqno;
		packetbuf_copyfrom(&msg, i+2);
		broadcast_send(&broadcast);
		seqno++;
		i += snprintf(msg.buf+i, 2, "\n\r"); //append new line before data is buffered
		process_post_synch(&buffer_process, PROCESS_EVENT_CONTINUE, msg.buf);
	}
	PROCESS_END();
}

//menu 
void
print_help_command_menu()
{
	printf("\n-----------------------Menu----------------------------------\n") ;
	printf("\n Prints a list of supported commands Usage: h") ;
	printf("\n Display System Summary Usage: ss");
	printf("\n Display System Uptime Usage: u");
	printf("\n Set/Display Node Name Usage: name <node name>");
	printf("\n Set/Display reporting interval Usage: ri <period in seconds>");
	printf("\n Set the report tag mask Usage: tagmask <var1,var2>, <auto>");
	printf("\n Set alias name Usage: alias <sensor=alias_name>.\n \t\t For example alias V_AD1=soil");
	if( i2c_probed & I2C_DS1307 ) {
		printf("\n Set/Display Time Usage: time hh:mm:ss.\n \t\t For example time 13:01:56");
		printf("\n Set Date/Display Usage: date dd/mm/yy.\n \t\t For example date 01/01/17");
	}
	printf("\n");
	printf("---------------------------------------------------------------\n\n");
}
/*---------------------------------------------------------------------------*/

//Displaying reporting interval 
void
display_reporting_interval()
{
	uint16_t saved_interval;
	cli();
	saved_interval = eeprom_read_word(&eemem_transmission_interval);
	sei();
	printf("Current Reporting Interval is %d\n",saved_interval);
}

//changing reporting interval
void 
change_reporting_interval(char* value)
{
	// check if value is an integer
    int interval = atoi(value);
    if (interval == 0) {
    	printf("Invalid value %s!, Please enter an interger for period.\nUsage: ri <period in seconds>.\n ", value);
    	return;
    }
	cli();
	eeprom_update_word(&eemem_transmission_interval, interval);  
	sei();
	printf("Interval changed to %d\n",interval);
	time_interval = eeprom_read_word(&eemem_transmission_interval);
}

//Display Mote information
void
display_system_information()
{
	uint32_t time=clock_seconds();
	printf("\n-------------------------System Information----------------------------\n") ;
	printf("Alias names set:\n  V_AD1=%s. \n  V_AD2=%s. \n", return_alias(1), return_alias(2));
	printf("Active sensors: %s.\n", default_sensors);
	display_reporting_interval();
	if (f_open(fp, "sensor.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK){
		printf("Memory card mounted: Yes.\n");
	} else printf("Memory card mounted: No.\n");
	display_node_name();
	if( i2c_probed & I2C_DS1307 ) {
		printf("System date: %s.\n", return_date());
		printf("System time: %s.\n", return_time());
	}
	printf("System uptime: %ld days %ld hours %ld minutes %ld seconds.\n", (time/86400), (time/3600)%24, (time/60)%60, time%60);
	display_tagmask();//print tagmask
	printf("\n-----------------------------------------------------------------------\n");
}

//display system uptime
void
display_system_uptime()
{
	uint32_t time=clock_seconds();
    printf("System has been up for %ld days %ld hours %ld minutes %ld seconds. \n", (time/86400), (time/3600)%24, (time/60)%60, time%60);// clock_time()
}

//change node name
void
change_node_name(char *value){
	uint16_t name_flag=1;
	char new_name[NAME_LENGTH];
	strlcpy(new_name, value, NAME_LENGTH);
	cli();
    eeprom_update_block((const void *)&new_name, (void *)&eemem_node_name, NAME_LENGTH);
    eeprom_update_word(&eemem_name_flag, name_flag);
	sei();
	printf("Node name changed to %s\n", new_name);
}

//display node name
void
display_node_name(){
	uint16_t name_flag;
	uint8_t node_name[NAME_LENGTH];
	cli();
	name_flag = eeprom_read_word(&eemem_name_flag);
	if (name_flag != 1) {
		printf("Node name not set.\n");
	} else {
		eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);
		printf("Node name = %s\n", (char *)node_name);
	}
	sei();
}

/*
 * Function to return the report mask using data from various sensors
 */
void
read_sensor_values(void){
	char result[TAGMASK_LENGTH], *sensors;
	uint8_t tagmask[TAGMASK_LENGTH], i=0;
	cli();
	eeprom_read_block((void*)&tagmask, (const void*)&eemem_tagmask, TAGMASK_LENGTH);
	sei();
	sensors=strtok ((char*)tagmask, " ");
	while (sensors != NULL){
		if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f", adc_read_v_in());
		} else if (!strncmp(trim(sensors), "V_AD1", 5)) {
			i += snprintf(result+i, 15, " %s=%-4.2f", return_alias(1), adc_read_a1());
		} else if (!strncmp(trim(sensors), "V_AD2", 5)) {
			i += snprintf(result+i, 15, " %s=%-4.2f", return_alias(2), adc_read_a2());
		} 
                  else if (!strncmp(trim(sensors), "P", 1))   			
                {      if(i2c_probed & I2C_MS5611_ADDR)              
			i += snprintf(result+i, 12, " P=%d", ms5611_sensor.value(0)); 
           
		}
                else if (!strncmp(trim(sensors), "ADC", 8)) {
                        if(i2c_probed & I2C_MS5611_ADDR) 
			i += snprintf(result+i,7, " ADC=%d", mcp3424_sensor.value(0));
		}
                 else if (!strncmp(trim(sensors), "T", 7) ) {
                         if( i2c_probed & I2C_SHT25) 
			i += snprintf(result+i,11, " T=%u", sht25_sensor.value(0));
                       
		}
	        else if (!strncmp(trim(sensors), "RH", 8)) {
                        if( i2c_probed & I2C_SHT25) {
			//RH above ice(d) = RH above water(sht25_sensor.value(1)) * d2;
			double tempe = sht25_sensor.value(0) * 1.000;
			double d = exp(17.62*(tempe)/(243+(tempe)));
			double d1 = exp(22.46*tempe/(272.62+tempe));
			//double d2 = d/d1;
						i += snprintf(result+i,12, " RH=%u",sht25_sensor.value(1));
                     

			///(exp(22.46(sht25_sensor.value(0))/((272.62+sht25_sensor.value(0))));
                       }

		}
                else if (!strncmp(trim(sensors), "ADC_1", 8)) {
                 if( i2c_probed & I2C_MCP3424 )
			i += snprintf(result+i,12, " ADC_1=%.4f",mcp3424_sensor.value(0)/1000.000);
		}
                else if (!strncmp(trim(sensors), "ADC_2", 8)) {
                if( i2c_probed & I2C_MCP3424 )
			i += snprintf(result+i,12, " ADC_2=%.4f",mcp3424_sensor.value(1)/1000.000);
		}
               else if (!strncmp(trim(sensors), "ADC_3", 8)) {
               if( i2c_probed & I2C_MCP3424 )
			i += snprintf(result+i,12, " ADC_3=%.4f",mcp3424_sensor.value(2)/1000.000);
		}
               else if (!strncmp(trim(sensors), "ADC_4", 8)) {
               if( i2c_probed & I2C_MCP3424 )
			i += snprintf(result+i,12, " ADC_4=%.4f",mcp3424_sensor.value(3)/1000.000);
		}
                 
		if(i>44){//if the report is greater than 45bytes, send the current result, reset i and result
			result[i++]='\0';//null terminate result before sending
			process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready
			i=0;
			result[0]='\0';
		}
		sensors = strtok(NULL, " ");
	}
	result[i++]='\0';//null terminate result before sending
	process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready
}

//return time form rtc
char
*return_time(void){
	static char time[15];
	snprintf(time, 15, "%02d:%02d:%02d", ds1307_sensor.value(8), ds1307_sensor.value(7), ds1307_sensor.value(6));
	return time;
}

//return date from rtc
char
*return_date(void){
	static char date[15];
	snprintf(date, 15, "%02d/%02d/%02d", ds1307_sensor.value(9), ds1307_sensor.value(10), ds1307_sensor.value(11));
	return date;
}

//set time on the rtc
static void
set_time(uint8_t hours, uint8_t minutes, uint8_t seconds){
	ds1307_sensor.configure(2, hours);
	ds1307_sensor.configure(1, minutes);
	ds1307_sensor.configure(0, seconds);
}

//set the date on the rtc
static void
set_date(uint8_t day, uint8_t month, uint8_t year){
	ds1307_sensor.configure(3, day);
	ds1307_sensor.configure(4, month);
	ds1307_sensor.configure(5, year);
}

//prepare date or time data in a format expected by the set functions
void
set_datetime(char *value, uint8_t choice){
	int8_t i=0, datetime[3];
	char new_datetime[9];
	strlcpy(new_datetime, value, 9);
	char *split_datetime=NULL;
	if (choice == 0){
		split_datetime = strtok (new_datetime, "/");
		while (split_datetime != NULL && i < 3 )
		{
			datetime[i] = atoi(split_datetime);
			split_datetime = strtok (NULL, "/");
			i++;
		}
		if (datetime[0] <= 0 || datetime[0] > 31){
			printf("Invalid command: date %s!. Try example date 01/01/17.\n", value);
			return;
		}
		if (datetime[1] <= 0 || datetime[1] > 12){
			printf("Invalid command: date %s!. Try example date 01/01/17.\n", value);
			return;
		}
		if (datetime[2] > 99){
			printf("Invalid command: date %s!. Try example date 01/01/17.\n", value);
			return;
		}
		set_date(datetime[0], datetime[1], datetime[2]);
		printf("Date set: %s\n", return_date());
	} else {
		split_datetime = strtok (new_datetime, ":");
		while (split_datetime != NULL && i < 3 )
		{
			datetime[i] = atoi(split_datetime);
			split_datetime = strtok (NULL, ":");
			i++;
		}
		if (datetime[0] < 0 || datetime[0] > 24){
			printf("Invalid command: time %s!. Try example time 13:01:56.\n", value);
			return;
		}
		if (datetime[1] < 0 || datetime[1] > 60){
			printf("Invalid command: time %s!. Try example time 13:01:56.\n", value);
			return;
		}
		if (datetime[2] < 0 || datetime[2] > 60){
			printf("Invalid command: time %s!. Try example time 13:01:56.\n", value);
			return;
		}
		set_time(datetime[0], datetime[1], datetime[2]);
		printf("Time set: %s\n", return_time());
	}
}

//change tagmask
void
change_tagmask(char *value){
	int i, size, m=0;
	char *tagmask=(char*) malloc(TAGMASK_LENGTH); //store mask from the user
	char *sensors[13]={"ADC_1","ADC_2","ADC_3","ADC_4", "RH", "V_AD1", "V_AD2", "P","T","V_IN","ADC","T_MCU", "V_MCU"}; //array of available sensors
	char *split_tagmask, save_tagmask[TAGMASK_LENGTH]; //store the mask with sanitized values that we are going to write to eeprom
	strlcpy(tagmask, value, TAGMASK_LENGTH);
	split_tagmask = strtok (tagmask, ",");//split the string with commas
	while (split_tagmask != NULL)
	{
		split_tagmask=trim(split_tagmask);
		/*Compare sensors requested by the user with the array of available sensors and return a string that can be written to eeprom*/
		for (i=0; i < 13; i++){
			size=strlen(sensors[i]);
			if (!strncmp(split_tagmask, sensors[i], size)){
				if (m==0){
					strlcpy(save_tagmask, split_tagmask, size+1);
					m+=1;
				} else {
					strncat(save_tagmask, " ", 1);
					strncat(save_tagmask, split_tagmask, size);
				}
				break;
			}
		}
		split_tagmask = strtok(NULL, ",");
	}
	if (strlen(save_tagmask)>0) {//check if tagmask is not empty after validation
		cli();
		eeprom_update_block((const void *)&save_tagmask, (void *)&eemem_tagmask, TAGMASK_LENGTH);
		sei();
	}
	free(tagmask);
	display_tagmask();
}

/*set tagmask to default*/
void
set_default_tagmask(void){
	cli();
	eeprom_update_block((const void *)&default_sensors, (void *)&eemem_tagmask, TAGMASK_LENGTH);
	sei();
}

/*Display the tagmask stored in eeprom*/
void
display_tagmask(void){
	uint8_t tagmask[TAGMASK_LENGTH];
	cli();
	eeprom_read_block((void*)&tagmask, (const void*)&eemem_tagmask, TAGMASK_LENGTH);
	sei();
	printf("Tagmask = %s\n", (char*)tagmask);
}

/*Trim whitespaces before and after a string*/
char
*trim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

// set alias name
void
set_alias(uint8_t sensor, char *value){
	char alias_name[NAME_LENGTH];
	strlcpy(alias_name, value, NAME_LENGTH);
	cli();
	if (sensor == 1){
		eeprom_update_block((const void *)&alias_name, (void *)&eemem_adc1, NAME_LENGTH);
	}
	else {
		eeprom_update_block((const void *)&alias_name, (void *)&eemem_adc2, NAME_LENGTH);
	}
	sei();
}

//return alias name
char*
return_alias(uint8_t sensor){
	static uint8_t name1[NAME_LENGTH], name2[NAME_LENGTH];
	if (sensor == 1){
		cli();
		eeprom_read_block((void*)&name1, (const void*)&eemem_adc1, NAME_LENGTH);
		sei();
		return (char*)name1;
	} else {
		cli();
		eeprom_read_block((void*)&name2, (const void*)&eemem_adc2, NAME_LENGTH);
		sei();
		return (char*)name2;
	}
}

//change alias name using value from menu
void
change_alias(char * value){
	char sensor[NAME_LENGTH], alias[NAME_LENGTH];
	char *alias_value=(char*) malloc(20); //store alias value from the user
	strlcpy(alias_value, value, 19);
	strlcpy(sensor, strtok (alias_value, "="), NAME_LENGTH-1);
	strlcpy(alias, strtok(NULL, "="), NAME_LENGTH-1);
	if (!strncmp("V_AD1", sensor, 5) && strlen(alias)>0) {
		set_alias(1, alias);
		printf("Successful V_AD1 = %s\n", alias);
	} else if (!strncmp("V_AD2", sensor, 5) && strlen(alias)>0) {
		set_alias(2, alias);
		printf("Successful V_AD2 = %s\n", alias);
	} else {
		printf("Alias can not be set for %s\n.", sensor);
	}
	free(alias_value);
}

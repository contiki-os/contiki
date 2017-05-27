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
 * @Author: Nsabagwa Mary, Flavia Nshemerirwe, Osbert Mugabe, Nahabwe Brian
 * The application reads sensor data, transmits it via broadbast, unicast .. using RIME
 */

#include "contiki.h"
#include "wimea-ict-rss2.h"
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
#include "dev/light-sensor.h"
#include "dev/pulse-sensor.h"
#include "dev/bme280/bme280-sensor.h"
#include "dev/co2_sa_kxx-sensor.h"
#include "dev/ds1307.h"
#include "dev/diskio.h"
#include "dev/ff.h"
#include "net/rime/rime.h"

#define NAME_LENGTH 10
#define TAGMASK_LENGTH 100
#define NO_SENSORS 10

//Configuration Parameters
uint16_t EEMEM eemem_transmission_interval;
uint8_t EEMEM eemem_node_name[NAME_LENGTH];
uint8_t EEMEM eemem_tagmask[TAGMASK_LENGTH];
/*flags to check if eeprom has valid values*/
uint16_t EEMEM eemem_interval_flag;
uint16_t EEMEM eemem_name_flag;
uint16_t EEMEM eemem_tagmask_flag;

PROCESS(serial_input_process, "Serial line input commands");
PROCESS(default_config_process, "Default configurations");
PROCESS(broadcast_data_process, "Broadcast sensor data");
PROCESS(sensor_data_process, "Read sensor data");

uint16_t time_interval;
struct etimer et;
unsigned char eui64_addr[8];
uint16_t rssi, lqi; //Received Signal Strength Indicator(RSSI), Link Quality Indicator(LQI)
static struct broadcast_conn broadcast;
FATFS FatFs;
FIL *fp;
UINT bw;

DWORD get_fattime (void)
{
	return	  ((DWORD)(2013 - 1980) << 25)	/* Year 2013 */
				| ((DWORD)7 << 21)				/* Month 7 */
				| ((DWORD)28 << 16)				/* Mday 28 */
				| ((DWORD)0 << 11)				/* Hour 0 */
				| ((DWORD)0 << 5)				/* Min 0 */
				| ((DWORD)0 >> 1);				/* Sec 0 */
}

AUTOSTART_PROCESSES(&default_config_process, &sensor_data_process, &broadcast_data_process, &serial_input_process);

PROCESS_THREAD(default_config_process, ev, data)
{
	uint16_t interval_flag, tagmask_flag;

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
	}
	sei();
	/*** init sdcard ****/
	f_mount(0, &FatFs);		// Give a work area to the FatFs module 
	fp = (FIL *)malloc(sizeof (FIL)); 
	
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_data_process, ev, data)
{
	PROCESS_BEGIN();
	SENSORS_ACTIVATE(temp_sensor);
	SENSORS_ACTIVATE(light_sensor);
	SENSORS_ACTIVATE(temp_mcu_sensor);
	SENSORS_ACTIVATE(battery_sensor);
	SENSORS_ACTIVATE(pulse_sensor);

	if( i2c_probed & I2C_BME280 ) {
		SENSORS_ACTIVATE(bme280_sensor);
	}
	if( i2c_probed & I2C_CO2SA ) {
		SENSORS_ACTIVATE(co2_sa_kxx_sensor);
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
	SENSORS_DEACTIVATE(light_sensor);
	SENSORS_DEACTIVATE(temp_mcu_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(pulse_sensor);
	SENSORS_DEACTIVATE(bme280_sensor);
	SENSORS_DEACTIVATE(co2_sa_kxx_sensor);
	SENSORS_DEACTIVATE(ds1307_sensor);

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
			value=(char*) malloc(10);
			strlcpy(value, command+6, 10);
			if (strlen(value)>0){
				printf("%s\n", value);
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
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
    printf("%s [RSSI=%u LQI=%u]\n", (char *)packetbuf_dataptr(), rssi, lqi);
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};


/*---------------------------------------------------------------------------*/
/*This process waits for a synchronous event and broadcasts the data received from the sender process*/
PROCESS_THREAD(broadcast_data_process, ev, data)
{
	char report[150], node[NAME_LENGTH];
	uint8_t node_name[NAME_LENGTH];
	int len, i=0;
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
    PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
		cli();
		eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);
		sei();
		len=strlen(data);
		if( i2c_probed & I2C_DS1307 ) {
				i += snprintf(report+i, 30, "%s %s ", return_date(), return_time());
		}
		if (node_name>0){
			strlcpy(node, (char*)node_name, NAME_LENGTH);
			i += snprintf(report+i, NAME_LENGTH, "TXT=%s ", node);
		}
		/* Read out mote 64bit MAC address */
		len+=30;
		i += snprintf(report+i, len, "E64=%02x%02x%02x%02x%02x%02x%02x%02x %s", eui64_addr[0], eui64_addr[1], eui64_addr[2], eui64_addr[3], eui64_addr[4], eui64_addr[5], eui64_addr[6], eui64_addr[7], (char*)data);
		printf("%s\n", report);
		/*write to sd card*/
		if (f_open(fp, "data.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {	// Open existing or create new file
			if (f_lseek(fp, f_size(fp)) == FR_OK) {
					f_write(fp, report, i+2, &bw);	// Write data to the file
				} 	
			if (bw == strlen(report)) {//we wrote the entire string 
					leds_off(LEDS_RED);
					leds_on(LEDS_YELLOW);
					printf("Success.\n");
			} else {
				leds_off(LEDS_YELLOW);
				leds_on(LEDS_RED);
				printf("Error writing.\n");
			}
			f_close(fp);// close the file	
		} else {
			printf("File not opened\n");
		}
		packetbuf_copyfrom(report, i+2);
		broadcast_send(&broadcast);
	}
	PROCESS_END();
}

//menu 
void
print_help_command_menu()
{
	printf("\n------------------ Menu--------------------------------\n") ;
	printf("\n Prints a list of supported commands \t Usage: h") ;
	printf("\n Display System Summary \t Usage: ss");
	printf("\n Display System Uptime \t Usage: u");
	printf("\n Set/Display Node Name \t Usage: name <node name>");
	printf("\n Set/Display reporting interval \t  Usage: ri <period in seconds>");
	printf("\n Set the report tag mask \t Usage: tagmask <var1,var2>, <auto>");
	printf("\n Set alias name \t Usage: alias <sensor=alias_name>. For example alias T=temp");
	if( i2c_probed & I2C_DS1307 ) {
		printf("\n Set Time\t time hh:mm:ss. For example time 13:01:56");
		printf("\n Set Date\t date dd/mm/yy. For example date 01/01/17");
		printf("\n Display Time\t time");
		printf("\n Display Date\t date");
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
	printf("Current Interval is %d\n",saved_interval);
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
	printf("System Information");
}

//display system uptime
void
display_system_uptime()
{
    printf("System has been up for %ld seconds \n", clock_seconds());// clock_time()
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
 * Function to return the report mask using data frim various sensors
 */
void
read_sensor_values(void){
	char result[150], *sensors;
	uint8_t tagmask[TAGMASK_LENGTH], i=0;
	cli();
	eeprom_read_block((void*)&tagmask, (const void*)&eemem_tagmask, TAGMASK_LENGTH);
	sei();
	sensors=strtok ((char*)tagmask, " ");
	while (sensors != NULL){
		if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 15, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 15, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 15, " V_IN=%-4.2f", adc_read_v_in());
		} else if (!strncmp(trim(sensors), "V_AD1", 5)) {
			i += snprintf(result+i, 15, " V_AD1=%-4.2f", adc_read_a1());
		} else if (!strncmp(trim(sensors), "V_AD2", 5)) {
			i += snprintf(result+i, 15, " V_AD2=%-4.2f", adc_read_a2());
		} else if (!strncmp(trim(sensors), "T", 1)) {
			i += snprintf(result+i, 15, " T=%-5.2f", ((double) temp_sensor.value(0))/100.);
		} else if (!strncmp(trim(sensors), "LIGHT", 5)) {
			i += snprintf(result+i, 15, " LIGHT=%-d", light_sensor.value(0));
		} else if (!strncmp(trim(sensors), "PULSE_0", 5)) {
			i += snprintf(result+i, 15, " PULSE_0=%-d", pulse_sensor.value(0));
		} else {
			i += snprintf(result+i, 15, " PULSE_1=%-d", pulse_sensor.value(1));
		}
		if(i>45){//if the report is greater than 45bytes, send the current result, reset i and result
			process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready
			i=0;
			result[0]='\0';
		}
		sensors = strtok(NULL, " ");
	}
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
	char *sensors[9]={"T_MCU", "T", "V_MCU", "V_IN", "V_AD1", "V_AD2", "LIGHT", "PULSE_0", "PULSE_1"}; //array of available sensors
	char *split_tagmask, save_tagmask[TAGMASK_LENGTH]; //store the mask with sanitized values that we are going to write to eeprom
	strlcpy(tagmask, value, TAGMASK_LENGTH);
	split_tagmask = strtok (tagmask, ",");//split the string with commas
	while (split_tagmask != NULL)
	{
		split_tagmask=trim(split_tagmask);
		/*Compare sensors requested by the user with the array of available sensors and return a string that can be written to eeprom*/
		for (i=0; i < 9; i++){
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
	char default_sensors[]="T T_MCU V_MCU V_IN V_AD1 V_AD2 LIGHT PULSE_0 PULSE_1";
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

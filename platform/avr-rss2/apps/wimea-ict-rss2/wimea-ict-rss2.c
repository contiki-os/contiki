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
#include "net/rime/rime.h"



#define NAME_LENGTH 10
#define TAGMASK_LENGTH 100
#define NO_SENSORS 10

//Configuration Parameters
uint16_t EEMEM eemem_transmission_interval;
uint8_t EEMEM eemem_node_name[NAME_LENGTH];
uint8_t EEMEM eemem_tagmask[TAGMASK_LENGTH];

PROCESS(serial_input_process, "Serial line input commands");
PROCESS(default_config_process, "Default configurations");
PROCESS(sensor_data_process, "Read sensor data");

uint16_t time_interval;
struct etimer et;

AUTOSTART_PROCESSES(&default_config_process, &sensor_data_process, &serial_input_process);

PROCESS_THREAD(default_config_process, ev, data)
{
	uint16_t eeprom_interval;
	PROCESS_BEGIN();
	cli();
	eeprom_interval = eeprom_read_word(&eemem_transmission_interval);
	time_interval = (eeprom_interval == 0) ? 60 : eeprom_interval;
	eeprom_update_word(&eemem_transmission_interval, time_interval);  
	sei();
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
			print_help_command_menu();
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
				if (!strncmp(value, "default", 7)){
					char default_sensors[]="T T_MCU V_MCU V_IN V_AD1 V_AD2 LIGHT PULSE_0 PULSE_1";
					cli();
						eeprom_update_block((const void *)&default_sensors, (void *)&eemem_tagmask, TAGMASK_LENGTH);
						display_tagmask();
					sei();
				} else {
				change_tagmask(value);
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
		etimer_set(&et, CLOCK_SECOND * time_interval);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		printf("%s\n", read_sensor_values()); 
	}
	
	SENSORS_DEACTIVATE(temp_sensor);
	SENSORS_DEACTIVATE(light_sensor);
	SENSORS_DEACTIVATE(temp_mcu_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(pulse_sensor);
	SENSORS_DEACTIVATE(bme280_sensor);
	SENSORS_DEACTIVATE(co2_sa_kxx_sensor);
	SENSORS_ACTIVATE(ds1307_sensor);
	
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
	printf("\n Set the report tag mask \t Usage: tagmask <var1,var2>, <default>"); 
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
	cli();
	char new_name[NAME_LENGTH];
	strlcpy(new_name, value, NAME_LENGTH);
    eeprom_update_block((const void *)&new_name, (void *)&eemem_node_name, NAME_LENGTH);
	sei();
	printf("Node Name changed to %s\n", new_name);
}

//display node name
void
display_node_name(){
	cli();
	uint8_t node_name[NAME_LENGTH];
	eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);
	sei();
	printf("Node Name = %s\n", (char *)node_name);
}

/*
 * Function to return the report mask using data frim various sensors
 */
char 
*read_sensor_values(void){
	static char result[200], *sensors;
	uint8_t tagmask[TAGMASK_LENGTH], i=0;
	if( i2c_probed & I2C_DS1307 ) {
		i = snprintf(result, 30, "%s %s ", return_date(), return_time());
	}
	/* Read out mote 64bit MAC address */
	i += snprintf(result+i, 21, "E64=%02x%02x%02x%02x%02x%02x%02x%02x", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], linkaddr_node_addr.u8[2], 
			linkaddr_node_addr.u8[3], linkaddr_node_addr.u8[4], linkaddr_node_addr.u8[5], linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);
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
		sensors = strtok(NULL, " ");
	}
	/*
	if( i2c_probed & I2C_CO2SA ) {
		printf(" CO2=%-d", co2_sa_kxx_sensor.value( CO2_SA_KXX_CO2));
	}
	if( i2c_probed & I2C_BME280 ) {
		#if STD_API
	 		printf(" BME280_TEMP=%-d", bme280_sensor.value(BME280_SENSOR_TEMP));
		  	printf(" BME280_RH=%-d", bme280_sensor.value(BME280_SENSOR_HUMIDITY));
		  	printf(" BME280_P=%-d", bme280_sensor.value(BME280_SENSOR_PRESSURE));
	    #else*/ 
		    /* Trigger burst read */
		/*    bme280_sensor.value(BME280_SENSOR_TEMP);
		    printf(" T_BME280=%5.2f", (double)bme280_mea.t_overscale100 / 100.);
		    printf(" RH_BME280=%5.2f", (double)bme280_mea.h_overscale1024 / 1024.);
	  	#ifdef BME280_64BIT
		  	printf(" P_BME280=%5.2f", (double)bme280_mea.p_overscale256 / 256.);
	  	#else
		  	printf(" P_BME280=%5.2f", (double)bme280_mea.p);
	  	#endif
	    #endif
	 }*/
	return result;
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
	cli();
	eeprom_update_block((const void *)&save_tagmask, (void *)&eemem_tagmask, TAGMASK_LENGTH);
	sei();
	free(tagmask);
	display_tagmask();
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
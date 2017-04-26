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
#include "net/rime/rime.h"


#define NAME_LENGTH 10

//Configuration Parameters
uint16_t EEMEM eemem_transmission_interval;
uint8_t EEMEM eemem_node_name;

PROCESS(serial_input_process, "Serial line input commands");
PROCESS(default_config_process, "Default configurations");
PROCESS(sensor_data_process, "Read sensor data");

static process_event_t event_new_interval;
uint16_t time_interval;
struct etimer et;

AUTOSTART_PROCESSES(&default_config_process, &sensor_data_process);

PROCESS_THREAD(default_config_process, ev, data)
{
	uint16_t eeprom_interval;
	PROCESS_BEGIN();
	cli();
	eeprom_interval = eeprom_read_word(&eemem_transmission_interval);
	time_interval = (eeprom_interval == 0) ? 60 : eeprom_interval;
	eeprom_update_word(&eemem_transmission_interval, time_interval);  
	sei(); 
	process_start(&serial_input_process, NULL);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(serial_input_process, ev, data)
{
	PROCESS_BEGIN();
	char delimiter[]=" ";
	char *command = NULL;
	//uint8_t flag;
	char * value;
	event_new_interval = process_alloc_event();
   
	for(;;){
		PROCESS_YIELD_UNTIL(ev == serial_line_event_message);
		command = (char*)strtok((char*)data, (const char*)delimiter);
	   
		if(!strncmp(command, "h", 1)) {
			print_help_command_menu();
		} else if (!strncmp(command,"ri",2)) {
			value=(char*) malloc(6);
			strncpy(value, command+3, 6);//(strlen(command)-3)
			if(strlen(command)>2)
				change_reporting_interval(value);
			else
				display_reporting_interval();
			free(value);
		} else if (!strncmp(command,"ss",2)) {
			display_system_information();
		} else if(!strncmp(command, "name", 4)) {
			value=(char*) malloc(12);
			strncpy(value, command+5, 12);//(strlen(command)-3)
			if(strlen(command)>4)
				change_node_name(value);
			else 
				display_node_name();
			free(value);
		} else if (!strncmp(command,"u", 1)){
			display_system_uptime();
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
	while(1) {
		etimer_set(&et, CLOCK_SECOND * time_interval);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		read_sensor_values();
	}
	
	SENSORS_DEACTIVATE(temp_sensor);
	SENSORS_DEACTIVATE(light_sensor);
	SENSORS_DEACTIVATE(temp_mcu_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(pulse_sensor);
	
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
	printf("\n Set the report tag mask \t Usage: tagmask <var1,var2>"); 
	printf("\n Set Time\t time h:m:s");
	printf("\n Set Date\t date d/m/yy");
	printf("\n Display Time\t time");
	printf("\n Display Date\t date\n");
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
	strncpy(new_name, value, NAME_LENGTH);
    eeprom_update_block((const void *)&new_name, (void *)&eemem_node_name, NAME_LENGTH);
	sei();
	printf("Node Name changed to %s\n", new_name);
}

void
display_node_name(){
	cli();
	uint8_t node_name[NAME_LENGTH];
	eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);
	sei();
	printf("Node Name = %s\n", (char *)node_name);
}

static void
read_sensor_values(void){
	int i;
	
	/* Read out mote 64bit MAC address */
	printf("E64=");
	for(i=0; i < 8; i++)
		printf("%02x", linkaddr_node_addr.u8[i]);
	  
	printf(" T=%-5.2f", ((double) temp_sensor.value(0)/100.));
	printf(" V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
	printf(" V_IN=%-4.2f", adc_read_v_in());
	printf(" V_AD1=%-4.2f", adc_read_a1());
	printf(" V_AD2=%-4.2f", adc_read_a2());
	printf(" T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
	printf(" LIGHT=%-d", light_sensor.value(0));
	printf(" PULSE_0=%-d", pulse_sensor.value(0));
	printf(" PULSE_1=%-d", pulse_sensor.value(1));
	 
	if( i2c_probed & I2C_CO2SA ) {
		printf(" CO2=%-d", co2_sa_kxx_sensor.value( CO2_SA_KXX_CO2));
	}
	if( i2c_probed & I2C_BME280 ) {
		#if STD_API
	 		printf(" BME280_TEMP=%-d", bme280_sensor.value(BME280_SENSOR_TEMP));
		  	printf(" BME280_RH=%-d", bme280_sensor.value(BME280_SENSOR_HUMIDITY));
		  	printf(" BME280_P=%-d", bme280_sensor.value(BME280_SENSOR_PRESSURE));
	    #else 
		    /* Trigger burst read */
		    bme280_sensor.value(BME280_SENSOR_TEMP);
		    printf(" T_BME280=%5.2f", (double)bme280_mea.t_overscale100 / 100.);
		    printf(" RH_BME280=%5.2f", (double)bme280_mea.h_overscale1024 / 1024.);
	  	#ifdef BME280_64BIT
		  	printf(" P_BME280=%5.2f", (double)bme280_mea.p_overscale256 / 256.);
	  	#else
		  	printf(" P_BME280=%5.2f", (double)bme280_mea.p);
	  	#endif
	    #endif
	 }
	 printf("\n");
}


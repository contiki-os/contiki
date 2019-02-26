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
#include "contiki-lib.h"
#include "wimea-ict-rss2.h"
#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "dev/serial-line.h"
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include "sys/etimer.h"
#include "dev/adc.h"
#include "dev/i2c.h"
#include "dev/powertrace.h"
#include "dev/leds.h"
#include "dev/battery-sensor.h"
#include "dev/temp-sensor.h"
#include "dev/temp_mcu-sensor.h"
#include "dev/ds1307.h"
#include "dev/sht25.h"
#include "math.h"
#include "dev/mcp3424.h"
#include "dev/pulse-sensor.h"
#include "dev/ms5611.h"
#include "lib/fs/fat/diskio.h"
#include "lib/fs/fat/ff.h"
#include "net/rime/rime.h"
#include "netstack.h"
#include "sys/energest.h"
#include "sys/compower.h"

#define NAME_LENGTH 12
#define TAGMASK_LENGTH 100
#define report_0_length 100
#define report_1_length 100
#define report_2_length 100
#define report_3_length 100
#define error_code_length 100

#define NO_SENSORS 16
#define MAX_BCAST_SIZE 99
#define DEF_TTL 0xF

uint16_t i2c_probed1; /* i2c devices we have probed */
//Configuration Parameters
uint16_t EEMEM eemem_transmission_interval;
uint16_t EEMEM eemem_error_logs;
uint16_t EEMEM eemem_report_0_transmission_interval;
uint16_t EEMEM eemem_report_1_transmission_interval;
uint16_t EEMEM eemem_report_2_transmission_interval;
uint16_t EEMEM eemem_report_3_transmission_interval;

uint8_t EEMEM eemem_node_name[NAME_LENGTH];
uint8_t EEMEM eemem_adc1[NAME_LENGTH];
uint8_t EEMEM eemem_adc2[NAME_LENGTH];
uint8_t EEMEM eemem_tagmask[TAGMASK_LENGTH];
uint8_t EEMEM eemem_report_0[report_0_length];
uint8_t EEMEM eemem_report_1[report_1_length];
uint8_t EEMEM eemem_report_2[report_2_length];
uint8_t EEMEM eemem_report_3[report_3_length];
uint8_t EEMEM eemem_error_codes[error_code_length];


/*flags to check if eeprom has valid values*/
uint16_t EEMEM eemem_interval_flag;
uint16_t EEMEM eemem_report_0_interval_flag;
uint16_t EEMEM eemem_report_1_interval_flag;
uint16_t EEMEM eemem_report_2_interval_flag;
uint16_t EEMEM eemem_report_3_interval_flag;
uint16_t EEMEM eemem_name_flag;
uint16_t EEMEM eemem_tagmask_flag;
uint16_t EEMEM eemem_report_0_flag;
uint16_t EEMEM eemem_report_1_flag;
uint16_t EEMEM eemem_report_2_flag;
uint16_t EEMEM eemem_report_3_flag;
uint16_t EEMEM eemem_I2C_SHT25_flag;
uint16_t EEMEM eemem_I2C_MCP3424_flag;
uint16_t EEMEM eemem_I2C_MS5611_flag;


PROCESS(serial_input_process, "Serial line input commands");
PROCESS(default_config_process, "Default configurations");
PROCESS(broadcast_data_process, "Broadcast sensor data");
PROCESS(sensor_data_process_report0, "Read sensor data for report 0");
PROCESS(sensor_data_process_report1, "Read sensor data for report 1");
PROCESS(sensor_data_process_report2, "Read sensor data for report 2");
PROCESS(sensor_data_process_report3, "Read sensor data for repport 3");
PROCESS(buffer_process, "Buffer sensor data");
PROCESS(mcu_sleep_process, "Sleeps the mcu");
PROCESS(powertrace_process, "Periodic power output");

uint16_t time_interval;
struct etimer et;
struct etimer et0;
struct etimer et1;
struct etimer et2;
struct etimer et3;
struct etimer periodic;
unsigned char eui64_addr[8];
static int power_save = 0;      /* Power-save false */
static int error_status = 0;  
static int I2C_SHT25_flag;
static int I2C_MCP3424_flag;
static int I2C_MS5611_flag;
static char default_sensors[50]=" V_IN T_MCU V_MCU V_A1 V_A2 ";
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

AUTOSTART_PROCESSES(&default_config_process, &buffer_process, &broadcast_data_process,&mcu_sleep_process, &serial_input_process, &sensor_data_process_report0,&sensor_data_process_report1,&sensor_data_process_report2,&sensor_data_process_report3, &powertrace_process);

PROCESS_THREAD(powertrace_process, ev, data)
{
   
   static struct etimer periodic_timer;
   PROCESS_BEGIN();
    
    power_save = 1;
    int interval =  60;
    
  etimer_set(&periodic_timer, CLOCK_SECOND * interval);
   while(1) {
      power_save = 0;
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
     
     float voltage = adc_read_v_in();
     power_save = 1;
    powertrace_print1("",interval,voltage);
    etimer_reset(&periodic_timer);
  }
  power_save = 0;
  PROCESS_END();
}
PROCESS_THREAD(default_config_process, ev, data)
{
        power_save = 1;
	uint16_t interval_flag, tagmask_flag,report_0_flag,report_1_flag,report_2_flag,report_3_flag;
        uint16_t report_0_interval_flag,report_1_interval_flag,report_2_interval_flag,report_3_interval_flag;
	char adc1[NAME_LENGTH]="V_A1";
	char adc2[NAME_LENGTH]="V_A2";
	 I2C_SHT25_flag = 0;
	 I2C_MCP3424_flag = 0 ;
         I2C_MS5611_flag=0;
        power_save = 0;
	PROCESS_BEGIN();
                        
	i2c_at24mac_read((char *) &eui64_addr, 1);
         
	cli();
         power_save = 1;
	interval_flag = eeprom_read_word(&eemem_interval_flag);
        eeprom_update_word(&eemem_I2C_SHT25_flag, I2C_SHT25_flag);
        eeprom_update_word(&eemem_I2C_MCP3424_flag, I2C_MCP3424_flag);
        eeprom_update_word(&eemem_I2C_MS5611_flag, I2C_MS5611_flag);
        report_0_interval_flag = eeprom_read_word(&eemem_report_0_interval_flag);
        report_1_interval_flag = eeprom_read_word(&eemem_report_1_interval_flag); 
        report_2_interval_flag = eeprom_read_word(&eemem_report_2_interval_flag);
        report_3_interval_flag = eeprom_read_word(&eemem_report_3_interval_flag); 
	tagmask_flag = eeprom_read_word(&eemem_tagmask_flag);
        report_0_flag = eeprom_read_word(&eemem_report_0_flag);
        report_1_flag = eeprom_read_word(&eemem_report_1_flag);
        report_2_flag = eeprom_read_word(&eemem_report_2_flag);
        report_3_flag = eeprom_read_word(&eemem_report_3_flag);
       
	if (interval_flag != 1) {
		time_interval = 60;
		eeprom_update_word(&eemem_transmission_interval, time_interval);
		interval_flag=1;
		eeprom_update_word(&eemem_interval_flag, interval_flag);
	}
        if (report_0_interval_flag != 1) {
		time_interval = 60;
		eeprom_update_word(&eemem_report_0_transmission_interval, time_interval);
		report_0_interval_flag=1;
		eeprom_update_word(&eemem_report_0_interval_flag, report_0_interval_flag);
	}if (report_1_interval_flag != 1) {
		time_interval = 0;
		eeprom_update_word(&eemem_report_1_transmission_interval, time_interval);
		report_1_interval_flag=1;
		eeprom_update_word(&eemem_report_1_interval_flag, report_1_interval_flag);
	}if (report_2_interval_flag != 1) {
		time_interval = 0;
		eeprom_update_word(&eemem_report_2_transmission_interval, time_interval);
		report_2_interval_flag=1;
		eeprom_update_word(&eemem_report_2_interval_flag, report_2_interval_flag);
	}if (report_3_interval_flag != 1) {
		time_interval = 0;
		eeprom_update_word(&eemem_report_3_transmission_interval, time_interval);
		report_3_interval_flag=1;
		eeprom_update_word(&eemem_report_3_interval_flag, report_3_interval_flag);
	}

	if (tagmask_flag != 1) {
		//set_default_tagmask();
		tagmask_flag=1;
		eeprom_update_word(&eemem_tagmask_flag, tagmask_flag);
		eeprom_update_block((const void *)&adc1, (void *)&eemem_adc1, NAME_LENGTH);
		eeprom_update_block((const void *)&adc2, (void *)&eemem_adc2, NAME_LENGTH);
	}
        if (report_0_flag != 1) {
		set_default_report0mask();
		report_0_flag=1;
		eeprom_update_word(&eemem_report_0_flag, report_0_flag);
		eeprom_update_block((const void *)&adc1, (void *)&eemem_adc1, NAME_LENGTH);
		eeprom_update_block((const void *)&adc2, (void *)&eemem_adc2, NAME_LENGTH);
	}
        if (report_1_flag != 1) {
		set_default_report1mask();
		report_1_flag=1;
		eeprom_update_word(&eemem_report_1_flag, report_1_flag);
		eeprom_update_block((const void *)&adc1, (void *)&eemem_adc1, NAME_LENGTH);
		eeprom_update_block((const void *)&adc2, (void *)&eemem_adc2, NAME_LENGTH);
	}
        if (report_2_flag != 1) {
		set_default_report2mask();
		report_2_flag=1;
		eeprom_update_word(&eemem_report_2_flag, report_2_flag);
		eeprom_update_block((const void *)&adc1, (void *)&eemem_adc1, NAME_LENGTH);
		eeprom_update_block((const void *)&adc2, (void *)&eemem_adc2, NAME_LENGTH);
	}
        if (report_3_flag != 1) {
		set_default_report3mask();
		report_3_flag=1;
		eeprom_update_word(&eemem_report_3_flag, report_3_flag);
		eeprom_update_block((const void *)&adc1, (void *)&eemem_adc1, NAME_LENGTH);
		eeprom_update_block((const void *)&adc2, (void *)&eemem_adc2, NAME_LENGTH);
	}
       
       power_save = 0;
	sei();
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(buffer_process, ev, data)
{
	int i;
	PROCESS_BEGIN();
	/*** init sdcard ***
	f_mount(0, &FatFs);		// Give a work area to the FatFs module 
	fp = (FIL *)malloc(sizeof (FIL));
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
		i=strlen(data);
		/*write to sd card
		if (f_open(fp, "sensor.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {	// Open existing or create new file
			if (f_lseek(fp, f_size(fp)) == FR_OK) {
				f_write(fp, (char*)data, i+2, &bw);	// Write data to the file
			}
			if (bw == i+2) {//we wrote the entire string 
				leds_on(LEDS_YELLOW);
			} else {
				//leds_on(LEDS_RED);
			}
			f_close(fp);// close the file
			} else {
				//leds_on(LEDS_RED);
			}
    }*/
	PROCESS_END();
}
/*--------------------------------------------------------------------------- */

/*--------------------------------------------------------------------------- */
PROCESS_THREAD(sensor_data_process_report0, ev, data)
{
	PROCESS_BEGIN();
	SENSORS_ACTIVATE(temp_sensor);
	SENSORS_ACTIVATE(temp_mcu_sensor);
	SENSORS_ACTIVATE(battery_sensor);
        SENSORS_ACTIVATE(pulse_sensor);
       
       static uint16_t time_interval_0 =0 ;
       static int i, j;
	while(1) {
                // strcpy(default_sensors," INTR V_AD1 V_AD2 V_IN V_MCU"); 
              // set_default_report2mask(); 
               
            power_save = 1;
              for(i=0; i < 2; i++) {   /* Loop over min and max rpc settings  */

	    NETSTACK_RADIO.off(); /* Radio off for rpc change */
	    //NETSTACK_RADIO.off();
	   
	    if(i == 0){
	      rf230_set_rpc(0x0); /* Disbable all RPC features */
            }
	    else{ 
	      rf230_set_rpc(0xFF); /* Enable all RPC features. Only XRFR2 radios */
            }
	    

	    /*  Loop over the different TX power settings 0-15  */
	     
	    for(j=15; j >= 0; j--) {
	      NETSTACK_RADIO.on();
	      rf230_set_txpower(j);
	      
           
              //
        }
             NETSTACK_RADIO.off();
        if(i==1){
               // 
		time_interval_0 = eeprom_read_word(&eemem_report_0_transmission_interval);
		power_save = 0;
               // printf("TX with PWR=%.2f\n",adc_read_v_in());
                if(time_interval_0 != 0 && adc_read_v_in() < 2.89){
                power_save = 1;
                time_interval_0 = 5;
                }else{
		time_interval_0 = eeprom_read_word(&eemem_report_0_transmission_interval);
                 }
		etimer_set(&et0, CLOCK_SECOND * time_interval_0);
		//printf("TX with PWR=%d RPC=0x%02x\n", rf230_get_txpower(), rf230_get_rpc());
		power_save = 0;
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et0));
                power_save = 1;
		if(time_interval_0 != 0){
                check_sensor_connection("0");
                 power_save = 0;
		NETSTACK_RADIO.on();
		read_sensor_values_report0();
		NETSTACK_RADIO.off();
		}
                
        }
        }
	}

	SENSORS_DEACTIVATE(temp_sensor);
	SENSORS_DEACTIVATE(temp_mcu_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(ds1307_sensor);
        SENSORS_DEACTIVATE(ms5611_sensor);
        SENSORS_DEACTIVATE(mcp3424_sensor);
        SENSORS_DEACTIVATE(sht25_sensor);
        SENSORS_DEACTIVATE(pulse_sensor);
	PROCESS_END();
}
/*--------------------------------------------------------------------------- */
PROCESS_THREAD(sensor_data_process_report1, ev, data)
{
	PROCESS_BEGIN();
	SENSORS_ACTIVATE(temp_sensor);
	SENSORS_ACTIVATE(temp_mcu_sensor);
	SENSORS_ACTIVATE(battery_sensor);
        SENSORS_ACTIVATE(pulse_sensor);
           
         
       
        uint16_t time_interval_1 = 0;
	while(1) {
        
               time_interval_1=eeprom_read_word(&eemem_report_1_transmission_interval);
                if(time_interval_1 != 0 && adc_read_v_in() < 2.89){
                time_interval_1 = 1800;
                }else{
		time_interval_1=eeprom_read_word(&eemem_report_1_transmission_interval);
                 }
		power_save = 1;
                etimer_set(&et1, CLOCK_SECOND * time_interval_1);
                power_save = 0;
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));
               power_save = 1;
		
		//read_sensor_values();
                 if(time_interval_1 != 0){
                  
                 check_sensor_connection("1");
                 power_save = 0;
                NETSTACK_RADIO.on();
		NETSTACK_RADIO.on();
		read_sensor_values_report1();
                NETSTACK_RADIO.off();
		NETSTACK_RADIO.off();
		}
		
                
		
	}

	SENSORS_DEACTIVATE(temp_sensor);
	SENSORS_DEACTIVATE(temp_mcu_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(ds1307_sensor);
        SENSORS_DEACTIVATE(ms5611_sensor);
        SENSORS_DEACTIVATE(mcp3424_sensor);
        SENSORS_DEACTIVATE(sht25_sensor);
        SENSORS_DEACTIVATE(pulse_sensor);
	PROCESS_END();
}
/*--------------------------------------------------------------------------- */
PROCESS_THREAD(sensor_data_process_report2, ev, data)
{
	PROCESS_BEGIN();
        
	SENSORS_ACTIVATE(temp_sensor);
	SENSORS_ACTIVATE(temp_mcu_sensor);
	SENSORS_ACTIVATE(battery_sensor);
        SENSORS_ACTIVATE(pulse_sensor);
        
       
       static uint16_t time_interval_2 = 0;
	while(1) {

                time_interval_2=eeprom_read_word(&eemem_report_2_transmission_interval);  
		if(time_interval_2 != 0 && adc_read_v_in() < 2.89){
                time_interval_2 = 1800;
                }else{
		time_interval_2=eeprom_read_word(&eemem_report_2_transmission_interval);
                 }
                 power_save = 1;
		
                etimer_set(&et2, CLOCK_SECOND * time_interval_2);
                power_save = 0;
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et2));
                power_save = 1;
		//read_sensor_values();
                 if(time_interval_2 != 0){

                 check_sensor_connection("2");
                power_save = 0;
                NETSTACK_RADIO.on();
		NETSTACK_RADIO.on();
		read_sensor_values_report2();
                NETSTACK_RADIO.off();
		NETSTACK_RADIO.off();
		   }
		
              
		//reset_count();
	}

	SENSORS_DEACTIVATE(temp_sensor);
	SENSORS_DEACTIVATE(temp_mcu_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(ds1307_sensor);
        SENSORS_DEACTIVATE(ms5611_sensor);
        SENSORS_DEACTIVATE(mcp3424_sensor);
        SENSORS_DEACTIVATE(sht25_sensor);
        SENSORS_DEACTIVATE(pulse_sensor);
	PROCESS_END();
}

/*--------------------------------------------------------------------------- */
PROCESS_THREAD(sensor_data_process_report3, ev, data)
{
	PROCESS_BEGIN();
	SENSORS_ACTIVATE(temp_sensor);
	SENSORS_ACTIVATE(temp_mcu_sensor);
	SENSORS_ACTIVATE(battery_sensor);
        SENSORS_ACTIVATE(pulse_sensor);
       
        
       static uint16_t time_interval_3 = 0;
	while(1) {
                time_interval_3 = eeprom_read_word(&eemem_report_3_transmission_interval);
                 if(time_interval_3 != 0 && adc_read_v_in() < 2.89){
                time_interval_3 = 1800;
                }else{
		time_interval_3 = eeprom_read_word(&eemem_report_3_transmission_interval);
                 }
                 power_save =1;
		time_interval_3 = eeprom_read_word(&eemem_report_3_transmission_interval);
                 etimer_set(&et3, CLOCK_SECOND * time_interval_3);
                 power_save =0;
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et3));
                power_save =1;
		 
		//read_sensor_values();
                 if(time_interval_3 != 0){
                check_sensor_connection("3");
                 power_save =0;
                NETSTACK_RADIO.on();
		NETSTACK_RADIO.on();
		read_sensor_values_report3();
                NETSTACK_RADIO.off();
		NETSTACK_RADIO.off();
		}
		
                
		//reset_count();
	}

	SENSORS_DEACTIVATE(temp_sensor);
	SENSORS_DEACTIVATE(temp_mcu_sensor);
	SENSORS_DEACTIVATE(battery_sensor);
	SENSORS_DEACTIVATE(ds1307_sensor);
        SENSORS_DEACTIVATE(ms5611_sensor);
        SENSORS_DEACTIVATE(mcp3424_sensor);
        SENSORS_DEACTIVATE(sht25_sensor);
        SENSORS_DEACTIVATE(pulse_sensor);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(serial_input_process, ev, data)
{
	PROCESS_BEGIN();
        power_save = 1; 
	char delimiter[]=" ";
	char *command = NULL;
	uint8_t flag;
	char * value;
        char * reportNo;
	char * parameter;
	for(;;){ 
                power_save = 0; 

		PROCESS_YIELD_UNTIL(ev == serial_line_event_message);
		command = (char*)strtok((char*)data, (const char*)delimiter);
                power_save = 1;
		if(!strncmp(command, "h", 1)) {
			if (strlen(command) == 1){
				print_help_command_menu();
			} else {
                                //power_save = 1; 
				printf("Invalid command %s. Try h for a list of commands\n", command);
                                //power_save = 0; 
			}
		} else if (!strncmp(command,"ri",2)) {
                     	if(strlen(command)==2){
                       
                       display_reporting_interval();
			}else{
                         if(!strncmp(command+3,"#",1)){
  
                       if(!strncmp(command+4, "0", 1)||!strncmp(command+4, "1", 1) ||!strncmp(command+4, "2", 1) ||!strncmp(command+4, "3", 1)){                     reportNo=(char*) malloc(6);
				strlcpy(reportNo, command+4, 6);
		                 
                       if(strlen(command+5)>0) {
				value=(char*) malloc(6);
				strlcpy(value, command+5, 6);//(strlen(command)-3)
				change_reporting_interval(value,reportNo);
				free(value);
          			free(reportNo);
			} else {
				 show_report_interval(reportNo);//displays individual resport interval
                                 free(reportNo);
			}
			}else{
                          //power_save = 1; 
			printf("Invalid report number %s. Try ri #<report number> command\n", command);
                        //power_save = 0; 
          		}
			}else{
                          //power_save = 1; 
			printf("Invalid command %s. Try ri #<report number> command\n", command);
                        //power_save = 0; 
          		}
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
                                //power_save = 1; 
				printf("Date: %s\n", return_date());
                                //power_save = 0; 
			} else if (strlen(command)>4 && strlen(command)<=13) {
				value = (char*) malloc(9);
				flag = 0;
				strlcpy(value, command+5, 9);
				set_datetime(value, flag);
				free(value);
			} else {
                                //power_save = 1; 
				printf("Invalid command: %s. Try example date 01/01/17.\n", command);
                                //power_save = 0; 
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
                                //power_save = 1; 
				printf("Invalid command: %s. Try example time 13:01:56.\n", command);
                                //power_save = 0; 
			}
		} else if(!strncmp(command, "re", 2)) {
			if (strlen(command) == 2){
				display_tagmask();
			} else {
				
                                reportNo = (char*) malloc(TAGMASK_LENGTH);
				strlcpy(reportNo, command+3, TAGMASK_LENGTH);
                 if(!strncmp(reportNo, "0", 1)||!strncmp(reportNo, "1", 1) ||!strncmp(reportNo, "2", 1) ||!strncmp(reportNo, "3", 1) ||!strncmp(reportNo, "4", 1) ||!strncmp(reportNo, "5", 1) ){
                                value=(char*) malloc(TAGMASK_LENGTH);
				strlcpy(value, command+4, TAGMASK_LENGTH);
                                if (!strncmp(value, "auto", 4)){
					//set_default_tagmask();
					set_default_report0mask();
					set_default_report1mask(); 
					set_default_report2mask();
					set_default_report3mask();
					display_tagmask();
				} else if(strlen(value)>0) {
                                         parameter = (char*) malloc(TAGMASK_LENGTH);
				         strlcpy(parameter, command+5, TAGMASK_LENGTH);
                                         if(!strncmp(value, "+", 1)){
                                         add2reportmask(parameter,reportNo);
					printf("ready to add aparameeter on a mask \n");
					 display_tagmask();
                                        }else if(!strncmp(value, "-", 1)){
                                         removefromreportmask(parameter,reportNo);
					printf("ready to remove aparameeter on a mask %s\n",value);
					 display_tagmask();
                                       }else{
					//change_reportmask(value,reportNo);
                                        display_tagmask();
                                        free(reportNo);
				        free(value);
                                       }
                                        free(parameter);
				}else{
                                       //power_save = 1; 
                                       printf("no mask specified \n");
                                       //power_save = 0; 
                                       display_tagmask();
                                       free(reportNo);
					free(value);
                                            }
					}else{
                                    //power_save = 1; 
                                    printf("please select a valid report number \n");
                                    //power_save = 0; 
					}
                                
				
                                free(reportNo);
				free(value);
			}
		} else if(!strncmp(command, "alias", 5)) {
			if (strlen(command) == 5){
                                //power_save =1; 
				printf("V_A1=%s\n", return_alias(1));
				printf("V_A2=%s\n", return_alias(2));
                                //power_save = 0; 
			} else {
				value=(char*) malloc(20);
				strlcpy(value, command+6, 19);
				if (strlen(value)>0){
					change_alias(value);
				} else {
                                        //power_save = 1; 
					printf("Can not set alias name: %s. Try 'alias V_A1=soil'\n", value);
                                        //power_save = 0; 
				}
				free(value);
			}
		} else {
                        //power_save = 1; 
			printf("Invalid command %s. Try h for a list of commands\n", command);
			//power_save = 0; 
		} 
              power_save = 0; 
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
        power_save = 1; 
    printf("%s [ADDR=%d.%d RSSI=%u LQI=%u TTL=%u SEQ=%u]\n", (char *)msg_recv->buf, from->u8[0], from->u8[1], rssi, lqi, msg_recv->head & 0xF, msg_recv->seqno);
       power_save = 0; 
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
       //NETSTACK_MAC.off(0);
       //NETSTACK_MAC.off(0);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
		cli();
		eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);
		sei();
		len=strlen(data);
              //  i += snprintf(msg.buf+i, 15, "&: NAME=%s ", );
		/*if( i2c_probed & I2C_DS1307 ) {
				//i += snprintf(msg.buf+i, 25, "%s %s ", return_date(), return_time());
		}*/
		if (node_name>0){
			strlcpy(node, (char*)node_name, NAME_LENGTH);
			i += snprintf(msg.buf+i, 30, "&: NAME=%s ", node);
		}
		/* Read out mote 64bit MAC address */
		len+=25;

		i += snprintf(msg.buf+i, len, "E64=%02x%02x%02x%02x%02x%02x%02x%02x %s", eui64_addr[0], eui64_addr[1], eui64_addr[2], eui64_addr[3], eui64_addr[4], eui64_addr[5], eui64_addr[6], eui64_addr[7], (char*)data);
		msg.buf[i++]='\0';//null terminate report.
		printf("%s\n", msg.buf);
		msg.head = 1<<4;
		msg.head |= ttl;
		msg.seqno = seqno;
                  NETSTACK_RADIO.off();
                 NETSTACK_RADIO.off();

		packetbuf_copyfrom(&msg, i+2);
		broadcast_send(&broadcast);
                   NETSTACK_RADIO.off();
                   NETSTACK_RADIO.off();
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
	//power_save = 1; 
	printf("\n-----------------------Menu----------------------------------\n") ;
	printf("\n Version Number: WIMEA RSS2 Node Application Version 2.0\n") ;
	printf("\n Prints a list of supported commands Usage: h") ;
	printf("\n Display System Summary Usage: ss");
	printf("\n Display System Uptime Usage: u");
	printf("\n Set/Display Node Name Usage: name <node name>");
        printf("\n Display all reporting intervals : ri ");
        printf("\n Display specific reporting interval : ri #<report number> ");
	printf("\n Set reporting interval : ri #<report number> <period in seconds>");
        printf("\n Display all report masks : re ");
	printf("\n Set the report tag mask Usage: re <report number> <var1,var2> or <auto>");
        printf("\n add a parameter to a report tag mask : re <report number><+/-> <var1>");
	printf("\n Set alias name Usage: alias <sensor=alias_name>.\n \t\t For example alias V_AD1=soil");
        //power_save = 0; 
	if( i2c_probed & I2C_DS1307 ) {
                power_save = 1; 
		printf("\n Set/Display Time Usage: time hh:mm:ss.\n \t\t For example time 13:01:56");
		printf("\n Set Date/Display Usage: date dd/mm/yy.\n \t\t For example date 01/01/17");
 		power_save = 0; 
	}
       // power_save = 1; 
	printf("\n");
	printf("---------------------------------------------------------------\n\n");
        //power_save = 0; 
}
/*---------------------------------------------------------------------------*/

//Displaying reporting interval 
void
display_reporting_interval()
{
	uint16_t saved_interval,report_0_interval,report_1_interval,report_2_interval,report_3_interval;
	cli();
	saved_interval = eeprom_read_word(&eemem_transmission_interval);
	report_0_interval = eeprom_read_word(&eemem_report_0_transmission_interval);
	report_1_interval = eeprom_read_word(&eemem_report_1_transmission_interval);
	report_2_interval = eeprom_read_word(&eemem_report_2_transmission_interval);
	report_3_interval = eeprom_read_word(&eemem_report_3_transmission_interval);
	sei();
        //power_save = 1; 
	printf("Current Reporting Interval is \n");
	printf("Report 0 \t\t%d\n",report_0_interval);
	printf("Report 1 \t\t%d\n",report_1_interval);
	printf("Report 2 \t\t%d\n",report_2_interval);
	printf("Report 3 \t\t%d\n",report_3_interval);
       // power_save = 0; 
}
//Displaying reporting interval 
void
show_report_interval(char* report_no)
{
	uint16_t saved_interval,report_0_interval,report_1_interval,report_2_interval,report_3_interval;
	cli();
	saved_interval = eeprom_read_word(&eemem_transmission_interval);
	report_0_interval = eeprom_read_word(&eemem_report_0_transmission_interval);
	report_1_interval = eeprom_read_word(&eemem_report_1_transmission_interval);
	report_2_interval = eeprom_read_word(&eemem_report_2_transmission_interval);
	report_3_interval = eeprom_read_word(&eemem_report_3_transmission_interval);
	sei();
        
	
		if(!strncmp(report_no, "0", 1)){
		//power_save = 1; 
		printf("Current Report 0 Interval is \t\t%d\n",report_0_interval); 
		//power_save = 0;
		}else if(!strncmp(report_no, "1", 1)){
		//power_save = 1; 
		printf("Current Report 1 Interval is \t\t%d\n",report_1_interval); 
		//power_save = 0;
		}else if(!strncmp(report_no, "2", 1)){
		//power_save = 1; 
		printf("Current Report 2 Interval is \t\t%d\n",report_2_interval); 
		//power_save = 0;
		}else {
		//power_save = 1; 
		printf("Current Report 3 Interval is \t\t%d\n",report_3_interval); 
		//power_save = 0;
		}
	
     
}

//changing reporting interval
void 
change_reporting_interval(char* value, char* number)
{
	// check if value is an integer
    int interval = atoi(value);
    if (interval == 0 && !strncmp(value, "0", 1)) {
        //power_save = 1; 
    	printf("Invalid value %s!, Please enter an interger for period.\nUsage: ri <period in seconds>.\n ", value);
        //power_save = 0; 
    	return;
    }
	

		if(!strncmp(number, "0", 1)){
		cli();
		eeprom_update_word(&eemem_report_0_transmission_interval, interval);  
		sei();
		//power_save = 1; 
		printf("Report 0 Interval changed to %d\n",interval);
		//power_save = 0; 
		}else if(!strncmp(number, "1", 1)){
		cli();
		eeprom_update_word(&eemem_report_1_transmission_interval, interval);  
		sei();
		//power_save = 1; 
		printf("Report 1 Interval changed to %d\n",interval);
		//power_save = 0; 
		}else if(!strncmp(number, "2", 1)){
		cli();
		eeprom_update_word(&eemem_report_2_transmission_interval, interval); 
		sei(); 
		//power_save = 1; 
		printf("Report 2 Interval changed to %d\n",interval);
		//power_save = 0; 
		}else {
		cli();
		eeprom_update_word(&eemem_report_3_transmission_interval, interval);  
		sei();
		//power_save = 1; 
		printf("Report 3 Interval changed to %d\n",interval);
		//power_save = 0; 
		}
	
      
	//time_interval = eeprom_read_word(&eemem_transmission_interval);
}

//Display Mote information
void
display_system_information()
{
	uint32_t time=clock_seconds();
        //power_save = 1; 
	printf("\n-------------------------System Information----------------------------\n") ;
	printf("Alias names set:\n  V_A1=%s. \n  V_A2=%s. \n", return_alias(1), return_alias(2));
	printf("Active sensors: %s.\n", default_sensors);
        //power_save = 0; 
	display_reporting_interval();
   
	if (f_open(fp, "sensor.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK){
                //power_save = 1; 
		printf("Memory card mounted: Yes.\n");
               // power_save = 0; 
	} else
              //power_save = 1; 
              printf("Memory card mounted: No.\n");
              //power_save = 0; 
	display_node_name();
	if( i2c_probed & I2C_DS1307 ) 
	{
                //power_save = 1; 
		printf("System date: %s.\n", return_date());
		printf("System time: %s.\n", return_time());
                //power_save = 0; 
	}
        //power_save = 1; 
	printf("System uptime: %ld days %ld hours %ld minutes %ld seconds.\n", (time/86400), (time/3600)%24, (time/60)%60, time%60);
        //power_save = 0; 
	display_tagmask();//print tagmask
       // power_save = 1; 
	printf("\n-----------------------------------------------------------------------\n");
       // power_save = 0; 
}

//display system uptime
void
display_system_uptime()
{
	uint32_t time=clock_seconds();
    //power_save = 1; 
    printf("System has been up for %ld days %ld hours %ld minutes %ld seconds. \n", (time/86400), (time/3600)%24, (time/60)%60, time%60);// clock_time()
    //power_save = 0; 
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
        //power_save = 1; 
	printf("Node name changed to %s\n", new_name);
        //power_save = 0; 
}

//display node name
void
display_node_name(){
	uint16_t name_flag;
	uint8_t node_name[NAME_LENGTH];
	cli();
	name_flag = eeprom_read_word(&eemem_name_flag);
	if (name_flag != 1) {
                //power_save = 1; 
		printf("Node name not set.\n");
                //power_save = 0; 
	} else {
		eeprom_read_block((void*)&node_name, (const void*)&eemem_node_name, NAME_LENGTH);
                //power_save = 1; 
		printf("Node name = %s\n", (char *)node_name);
                //power_save = 0; 
	}
	sei();
}

/*
 * Function to return the report mask using data from various sensors
 */

void
read_sensor_values_report0(void){
//SMCR = 0x01;
//sleep_disable();
        static int adc1 = 0,adc2 = 0,adc3 = 0,adc4 = 0;
	char result[TAGMASK_LENGTH], *sensors;
	uint8_t tagmask0[TAGMASK_LENGTH],error[error_code_length], i=0;
	cli();
	eeprom_read_block((void*)&tagmask0, (const void*)&eemem_report_0, TAGMASK_LENGTH);
	sei();
        static int adc01 = 0,adc02 = 0,adc03 = 0,adc04 = 0;
	sensors=strtok ((char*)tagmask0, " ");
	//i += snprintf(result+i, 5, "R0");
	while (sensors != NULL){
                if(adc_read_v_in() < 2.71){
                  if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f",adc_read_v_in());
                         error_log("L-PWR");
		} 
                }else{

                
                if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f",adc_read_v_in());
		} 
 		   else if (!strncmp(trim(sensors), "V_A1", 5)) {
			i += snprintf(result+i, 15, " V_A1=%.2f",adc_read_a1());
		}  else if (!strncmp(trim(sensors), "V_A2", 5)) {
			i += snprintf(result+i, 13, " V_A2=%.2f",adc_read_a2());
                  
		} else if (!strncmp(trim(sensors), "T1", 2)) {
			i += snprintf(result+i, 10, " T1=%f", (float) temp_sensor.value(0)/100.0);
		}
               else if (!strncmp(trim(sensors), "P", 1)) {   //pressure   
			if(i2c_probed1 & I2C_MS5611_ADDR){
                        if(ms5611_sensor.value(0) && missing_p_value() == 0){              
			i += snprintf(result+i, 12, " P=%d", ms5611_sensor.value(0)); 
                          }else{
			error_log("#E_mv(ms5611)");
                         }}
                   
		} else if (!strncmp(trim(sensors), "T", 1) ) {//temperature
                         if( i2c_probed1 & I2C_SHT25){
                         
                        if(sht25_sensor.value(0) && missing_t_value() == 0){
			i += snprintf(result+i,9, " T=%.2f",(float) sht25_sensor.value(0)/10.0);
                       }else{
                        error_log("#E_mv(sht25)");
                         }}
		} else if (!strncmp(trim(sensors), "RH", 2)) {//humidity
                        if( i2c_probed1 & I2C_SHT25) {
			if(sht25_sensor.value(1) && missing_rh_value() == 0){
			i += snprintf(result+i,10, " RH=%.2f",(float) sht25_sensor.value(1)/10.0);
                       }else{
                        error_log("#E_mv(sht25)");
                         }}

		} else if (!strncmp(trim(sensors), "INTR", 4) ) {//int pin eg. rain gauge and anenometer 
     			i += snprintf(result+i,15," P0_LST60=%d ", pulse_sensor.value(0));

		} else if (!strncmp(trim(sensors), "ADC_1", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(0) && missing_adc_value() == 0){
		i += snprintf(result+i,14, " ADC_1=%.3f",mcp3424_sensor.value(0)/1000.000);
                      adc1 =1;
		}else{
                        adc1 = 0;
                      
                       // error_log("#E_mv(mcp3424)");
                }}
		} else if (!strncmp(trim(sensors), "ADC_2", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                   if(mcp3424_sensor.value(1) && missing_adc_value() == 0 && adc1 == 0){
                    
		i += snprintf(result+i,14, " ADC_2=%.3f",mcp3424_sensor.value(1)/1000.000);
                        adc2 = 1;
		}else{
                         adc2 =0;
                       
                       // error_log("#E_mv(mcp3424)");
                }}

		} else if (!strncmp(trim(sensors), "ADC_3", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(2) && missing_adc_value() == 0 && adc1 == 0 && adc2 == 0){
		i += snprintf(result+i,14, " ADC_3=%.3f",mcp3424_sensor.value(2)/1000.000);
                      adc3 =1;
		}else{
                     
                      adc3 = 0;
                      //  error_log("#E_mv(mcp3424)");
                }}

		} else if (!strncmp(trim(sensors), "ADC_4", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(3) && missing_adc_value() == 0 && adc1 == 0 && adc2 == 0 && adc3 == 0){
		i += snprintf(result+i,14, " ADC_4=%.3f",mcp3424_sensor.value(3)/1000.000);
		 adc4 =1;
		
		}else{
                   adc4 =0;
                     
                         if(adc1==0 && adc2 == 0 && adc3 == 0 && adc4 == 0){
                        error_log("#E_mv(mcp3424)");
                        }
                }}}
                }
                if(error_status==1){
               
                cli();
		eeprom_read_block((void*)&error, (const void*)&eemem_error_codes, error_code_length);
                sei();
               
		i += snprintf(result+i,16," %s",error);
		 error_status=0;
          }

		if(i>80){//if the report is greater than 45bytes, send the current result, reset i and result
			result[i++]='\0';//null terminate result before sending
			process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready
			i=0;
			result[0]='\0';
		}
		sensors = strtok(NULL, " ");
	}
	result[i++]='\0';//null terminate result before sending
	process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready

//sleep_enable();
 // go to deep sleep
//SMCR = 0x01;
//SMCR = 0x03;
//sleep_cpu();
//PRR1 |= (1<<6);


}
void
read_sensor_values_report1(void){
//SMCR = 0x01;
//sleep_disable();
	char result[TAGMASK_LENGTH], *sensors;
	uint8_t tagmask1[TAGMASK_LENGTH],error[error_code_length],i=0;
	cli();
	eeprom_read_block((void*)&tagmask1, (const void*)&eemem_report_1, TAGMASK_LENGTH);
	sei();
         static int adc1 = 0,adc2 = 0,adc3 = 0,adc4 = 0;
	sensors=strtok ((char*)tagmask1, " ");
	//i += snprintf(result+i, 5, "R1");
while (sensors != NULL){
                if(adc_read_v_in() < 2.71){
                  if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f",adc_read_v_in());
                         error_log("L-PWR");
		} 
                }else{

		if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f",adc_read_v_in());
		} else if (!strncmp(trim(sensors), "V_A2", 5)) {
			i += snprintf(result+i, 13, " V_A2=%-4.2f",adc_read_a2());
                  
		} 
 		   else if (!strncmp(trim(sensors), "V_A1", 5)) {
			i += snprintf(result+i, 15, " V_A1=%-4.2f",adc_read_a1());
		}
               else if (!strncmp(trim(sensors), "P", 1)) {   //pressure   
			if(i2c_probed1 & I2C_MS5611_ADDR){
                        if(ms5611_sensor.value(0) && missing_p_value() == 0){              
			i += snprintf(result+i, 12, " P=%d", ms5611_sensor.value(0)); 
                          }else{
			error_log("#E_mv(ms5611)");
                         }}
                   
		} else if (!strncmp(trim(sensors), "T", 1) ) {//temperature
                         if( i2c_probed1 & I2C_SHT25){
                         
                        if(sht25_sensor.value(0) && missing_t_value() == 0){
			i += snprintf(result+i,9, " T=%.2f",(float) sht25_sensor.value(0)/10.0);
                       }else{
                        error_log("#E_mv(sht25)");
                         }}
		} else if (!strncmp(trim(sensors), "RH", 2)) {//humidity
                        if( i2c_probed1 & I2C_SHT25) {
			if(sht25_sensor.value(1) && missing_rh_value() == 0){
			i += snprintf(result+i,10, " RH=%.2f",(float) sht25_sensor.value(1)/10.0);
                       }else{
                        error_log("#E_mv(sht25)");
                         }}

		} else if (!strncmp(trim(sensors), "INTR", 4) ) {//int pin eg. rain gauge and anenometer 
     			i += snprintf(result+i,15," P0_LST60=%d ", pulse_sensor.value(0));

		} else if (!strncmp(trim(sensors), "ADC_1", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(0) && missing_adc_value() == 0){
		i += snprintf(result+i,14, " ADC_1=%.3f",mcp3424_sensor.value(0)/1000.000);
                      adc1 =1;
		}else{
                        adc1 = 0;
                      
                       // error_log("#E_mv(mcp3424)");
                }}
		} else if (!strncmp(trim(sensors), "ADC_2", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                   if(mcp3424_sensor.value(1) && missing_adc_value() == 0 && adc1 == 0){
                    
		i += snprintf(result+i,14, " ADC_2=%.3f",mcp3424_sensor.value(1)/1000.000);
                        adc2 = 1;
		}else{
                         adc2 =0;
                       
                       // error_log("#E_mv(mcp3424)");
                }}

		} else if (!strncmp(trim(sensors), "ADC_3", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(2) && missing_adc_value() == 0 && adc1 == 0 && adc2 == 0){
		i += snprintf(result+i,14, " ADC_3=%.3f",mcp3424_sensor.value(2)/1000.000);
                      adc3 =1;
		}else{
                     
                      adc3 = 0;
                      //  error_log("#E_mv(mcp3424)");
                }}

		} else if (!strncmp(trim(sensors), "ADC_4", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(3) && missing_adc_value() == 0 && adc1 == 0 && adc2 == 0 && adc3 == 0){
		i += snprintf(result+i,14, " ADC_4=%.3f",mcp3424_sensor.value(3)/1000.000);
		 adc4 =1;
		
		}else{
                   adc4 =0;
                     
                         if(adc1==0 && adc2 == 0 && adc3 == 0 && adc4 == 0){
                        error_log("#E_mv(mcp3424)");
                        }
                }}}
                }
                if(error_status==1){
               
                cli();
		eeprom_read_block((void*)&error, (const void*)&eemem_error_codes, error_code_length);
                sei();
               
		i += snprintf(result+i,16," %s",error);
		 error_status=0;
          }
		if(i>100){//if the report is greater than 45bytes, send the current result, reset i and result
			result[i++]='\0';//null terminate result before sending
			process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready
			i=0;
			result[0]='\0';
		}
		sensors = strtok(NULL, " ");
	}
	result[i++]='\0';//null terminate result before sending
	process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready

//sleep_enable();
 // go to deep sleep
//SMCR = 0x01;
//SMCR = 0x03;
//sleep_cpu();
//PRR1 |= (1<<6);


}
void
read_sensor_values_report2(void){
//SMCR = 0x01;
//sleep_disable();
        // char errors[error_code_length];
	char result[TAGMASK_LENGTH], *sensors;
	uint8_t tagmask2[TAGMASK_LENGTH],error[error_code_length], i=0;

	cli();
	eeprom_read_block((void*)&tagmask2, (const void*)&eemem_report_2, TAGMASK_LENGTH);
	sei();
         static int adc1 = 0,adc2 = 0,adc3 = 0,adc4 = 0;
	sensors=strtok ((char*)tagmask2, " ");
	//i += snprintf(result+i, 5, "R2");
       
	while (sensors != NULL){
                if(adc_read_v_in() < 2.71){
                  if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f",adc_read_v_in());
                         error_log("L-PWR");
		} 
                }else{

		if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f",adc_read_v_in());
		} else if (!strncmp(trim(sensors), "V_A2", 5)) {
			i += snprintf(result+i, 13, " V_A2=%-4.2f",adc_read_a2());
                  
		} 
 		   else if (!strncmp(trim(sensors), "V_A1", 5)) {
			i += snprintf(result+i, 15, " V_A1=%-4.2f",adc_read_a1());
		}
               else if (!strncmp(trim(sensors), "P", 1)) {   //pressure   
			if(i2c_probed1 & I2C_MS5611_ADDR){
                        if(ms5611_sensor.value(0) && missing_p_value() == 0){              
			i += snprintf(result+i, 12, " P=%d", ms5611_sensor.value(0)); 
                          }else{
			error_log("#E_mv(ms5611)");
                         }}
                   
		} else if (!strncmp(trim(sensors), "T", 1) ) {//temperature
                         if( i2c_probed1 & I2C_SHT25){
                         
                        if(sht25_sensor.value(0) && missing_t_value() == 0){
			i += snprintf(result+i,9, " T=%.2f", (float) sht25_sensor.value(0)/10.0);
                       }else{
                        error_log("#E_mv(sht25)");
                         }}
		} else if (!strncmp(trim(sensors), "RH", 2)) {//humidity
                        if( i2c_probed1 & I2C_SHT25) {
			if(sht25_sensor.value(1) && missing_rh_value() == 0){
			i += snprintf(result+i,10, " RH=%.2f",(float) sht25_sensor.value(1)/10.0);
                       }else{
                        error_log("#E_mv(sht25)");
                         }}

		} else if (!strncmp(trim(sensors), "INTR", 4) ) {//int pin eg. rain gauge and anenometer 
     			i += snprintf(result+i,18," P0_LST60=%d IS=%d", pulse_sensor.value(0),pulse_sensor.value(2));

		} else if (!strncmp(trim(sensors), "ADC_1", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(0) && missing_adc_value() == 0){
		i += snprintf(result+i,14, " ADC_1=%.3f",mcp3424_sensor.value(0)/1000.000);
                      adc1 =1;
		}else{
                        adc1 = 0;
                      
                       // error_log("#E_mv(mcp3424)");
                }}
		} else if (!strncmp(trim(sensors), "ADC_2", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                   if(mcp3424_sensor.value(1) && missing_adc_value() == 0 && adc1 == 0){
                    
		i += snprintf(result+i,14, " ADC_2=%.3f",mcp3424_sensor.value(1)/1000.000);
                        adc2 = 1;
		}else{
                         adc2 =0;
                       
                       // error_log("#E_mv(mcp3424)");
                }}

		} else if (!strncmp(trim(sensors), "ADC_3", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(2) && missing_adc_value() == 0 && adc1 == 0 && adc2 == 0){
		i += snprintf(result+i,14, " ADC_3=%.3f",mcp3424_sensor.value(2)/1000.000);
                      adc3 =1;
		}else{
                     
                      adc3 = 0;
                      //  error_log("#E_mv(mcp3424)");
                }}

		} else if (!strncmp(trim(sensors), "ADC_4", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(3) && missing_adc_value() == 0 && adc1 == 0 && adc2 == 0 && adc3 == 0){
		i += snprintf(result+i,14, " ADC_4=%.3f",mcp3424_sensor.value(3)/1000.000);
		 adc4 =1;
		
		}else{
                   adc4 =0;
                     
                         if(adc1==0 && adc2 == 0 && adc3 == 0 && adc4 == 0){
                        error_log("#E_mv(mcp3424)");
                        }
                }}}
                }
                if(error_status==1){
               
                cli();
		eeprom_read_block((void*)&error, (const void*)&eemem_error_codes, error_code_length);
                sei();
               
		i += snprintf(result+i,16," %s",error);
		 error_status=0;
          }
		if(i>60){//if the report is greater than 45bytes, send the current result, reset i and result
			result[i++]='\0';//null terminate result before sending
			process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready
			i=0;
			result[0]='\0';
		}
		sensors = strtok(NULL, " ");
	}
	result[i++]='\0';//null terminate result before sending
	process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready

//sleep_enable();
 // go to deep sleep
//SMCR = 0x01;
//SMCR = 0x03;
//sleep_cpu();
//PRR1 |= (1<<6);


}
void
read_sensor_values_report3(void){
//SMCR = 0x01;
//sleep_disable();
	char result[TAGMASK_LENGTH], *sensors;
	uint8_t tagmask3[TAGMASK_LENGTH],error[error_code_length],i=0;
	cli();
	eeprom_read_block((void*)&tagmask3, (const void*)&eemem_report_3, TAGMASK_LENGTH);
	sei();
       static int adc1 = 0,adc2 = 0,adc3 = 0,adc4 = 0;
      
	sensors=strtok ((char*)tagmask3, " ");
      //i += snprintf(result+i, 5, "R3");
	while (sensors != NULL){
                if(adc_read_v_in() < 2.71){
                  if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f",adc_read_v_in());
                         error_log("L-PWR");
		} 
                }else{

		if (!strncmp(trim(sensors), "T_MCU", 5)) {
			i += snprintf(result+i, 12, " T_MCU=%-4.1f", ((double) temp_mcu_sensor.value(0)/10.));
		} else if (!strncmp(trim(sensors), "V_MCU", 5)) {
			i += snprintf(result+i, 11, " V_MCU=%-3.1f", ((double) battery_sensor.value(0))/1000.);
		} else if (!strncmp(trim(sensors), "V_IN", 4)) {
			i += snprintf(result+i, 11, " V_IN=%-4.2f",adc_read_v_in());
		} else if (!strncmp(trim(sensors), "V_A2", 5)) {
			i += snprintf(result+i, 13, " V_A2=%-4.2f",adc_read_a2());
                  
		}else if (!strncmp(trim(sensors), "V_A1", 5)) {
			i += snprintf(result+i, 15, " V_A1=%-4.2f",adc_read_a1());
		}else if (!strncmp(trim(sensors), "T1", 2)) {
			i += snprintf(result+i, 9, " T1=%d",temp_sensor.value(0));
		}
               else if (!strncmp(trim(sensors), "P", 1)) {   //pressure   
			if(i2c_probed1 & I2C_MS5611_ADDR){
                        if(ms5611_sensor.value(0) && missing_p_value() == 0){              
			i += snprintf(result+i, 12, " P=%d", ms5611_sensor.value(0)); 
                           
                          }else{
			     error_log("#E_mv(sht25)");
                         }}
                   
		} else if (!strncmp(trim(sensors), "T", 1) ) {//temperature
                         if( i2c_probed1 & I2C_SHT25){
                         
                        if(sht25_sensor.value(0) && missing_t_value() == 0){
			i += snprintf(result+i,9, " T=%.2f", sht25_sensor.value(0)/10.0);
                       }else{
                        error_log("#E_mv(sht25)");
                         }}
		} else if (!strncmp(trim(sensors), "RH", 2)) {//humidity
                        if( i2c_probed1 & I2C_SHT25){ 
			if(sht25_sensor.value(1) && missing_rh_value() == 0){
			i += snprintf(result+i,10, " RH=%.2f",sht25_sensor.value(1)/10.0);
                       }else{
                        error_log("#E_mv(sht25)");
                         }}

		} else if (!strncmp(trim(sensors), "INTR", 4) ) {//int pin eg. rain gauge and anenometer 
     			i += snprintf(result+i,18," P0_LST60=%d IS=%d", pulse_sensor.value(0),pulse_sensor.value(2));

		} else if (!strncmp(trim(sensors), "ADC_1", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(0) && missing_adc_value() == 0){
		i += snprintf(result+i,14, " ADC_1=%.3f",mcp3424_sensor.value(0)/1000.000);
                      adc1 =1;
		}else{
                        adc1 = 0;
                      
                       // error_log("#E_mv(mcp3424)");
                }}
		} else if (!strncmp(trim(sensors), "ADC_2", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                   if(mcp3424_sensor.value(1) && missing_adc_value() == 0 && adc1 == 0){
                    
		i += snprintf(result+i,14, " ADC_2=%.3f",mcp3424_sensor.value(1)/1000.000);
                        adc2 = 1;
		}else{
                         adc2 =0;
                       
                       // error_log("#E_mv(mcp3424)");
                }}

		} else if (!strncmp(trim(sensors), "ADC_3", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(2) && missing_adc_value() == 0 && adc1 == 0 && adc2 == 0){
		i += snprintf(result+i,14, " ADC_3=%.3f",mcp3424_sensor.value(2)/1000.000);
                      adc3 =1;
		}else{
                     
                      adc3 = 0;
                      //  error_log("#E_mv(mcp3424)");
                }}

		} else if (!strncmp(trim(sensors), "ADC_4", 5)) {
                 if( i2c_probed1 & I2C_MCP3424 ){
                if(mcp3424_sensor.value(3) && missing_adc_value() == 0 && adc1 == 0 && adc2 == 0 && adc3 == 0){
		i += snprintf(result+i,14, " ADC_4=%.3f",mcp3424_sensor.value(3)/1000.000);
		 adc4 =1;
		
		}else{
                   adc4 =0;
                     
                         if(adc1==0 && adc2 == 0 && adc3 == 0 && adc4 == 0){
                        error_log("#E_mv(mcp3424)");
                        }
                }}}
                }
                if(error_status==1){
               
                cli();
		eeprom_read_block((void*)&error, (const void*)&eemem_error_codes, error_code_length);
                sei();
               
		i += snprintf(result+i,16," %s",error);
		 error_status=0;
          }
		if(i>60){//if the report is greater than 45bytes, send the current result, reset i and result
			result[i++]='\0';//null terminate result before sending
			process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready
			i=0;
			result[0]='\0';
		}
		sensors = strtok(NULL, " ");
	}
	result[i++]='\0';//null terminate result before sending
	process_post_synch(&broadcast_data_process, PROCESS_EVENT_CONTINUE, result);//send an event to broadcast process once data is ready

//sleep_enable();
 // go to deep sleep
//SMCR = 0x01;
//SMCR = 0x03;
//sleep_cpu();
//PRR1 |= (1<<6);


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
                       // power_save = 1; 
			printf("Invalid command: date %s!. Try example date 01/01/17.\n", value);
                        //power_save = 0; 
			return;
		}
		if (datetime[1] <= 0 || datetime[1] > 12){
                        //power_save = 1; 
			printf("Invalid command: date %s!. Try example date 01/01/17.\n", value);
                        //power_save = 0; 
			return;
		}
		if (datetime[2] > 99){
                       // power_save = 1; 
			printf("Invalid command: date %s!. Try example date 01/01/17.\n", value);
                        //power_save = 0; 
			return;
		}
		set_date(datetime[0], datetime[1], datetime[2]);
               // power_save = 1; 
		printf("Date set: %s\n", return_date());
                //power_save = 0; 
	} else {
		split_datetime = strtok (new_datetime, ":");
		while (split_datetime != NULL && i < 3 )
		{
			datetime[i] = atoi(split_datetime);
			split_datetime = strtok (NULL, ":");
			i++;
		}
		if (datetime[0] < 0 || datetime[0] > 24){
                        //power_save = 1; 
			printf("Invalid command: time %s!. Try example time 13:01:56.\n", value);
			//power_save = 0; 
			return;
		}
		if (datetime[1] < 0 || datetime[1] > 60){
                        //power_save = 1; 
			printf("Invalid command: time %s!. Try example time 13:01:56.\n", value);
                        //power_save = 0; 
			return;
		}
		if (datetime[2] < 0 || datetime[2] > 60){
                        //power_save = 1; 
			printf("Invalid command: time %s!. Try example time 13:01:56.\n", value);
                        //power_save = 0; 
			return;
		}
		set_time(datetime[0], datetime[1], datetime[2]);
                //power_save = 1; 
		printf("Time set: %s\n", return_time());
                //power_save = 0; 
	}
}

//change tagmask
void
change_tagmask(char *value){
	int i, size, m=0;
	char *tagmask=(char*) malloc(TAGMASK_LENGTH); //store mask from the user
	char *sensors[15]={"ADC_1","ADC_2","ADC_3","ADC_4","RH","P","T","T1","V_A1","V_A2","V_IN","T_MCU","V_MCU","INTR","P0_LST60"}; //array of available sens3
	char *split_tagmask, save_tagmask[TAGMASK_LENGTH]; //store the mask with sanitized values that we are going to write to eeprom
	strlcpy(tagmask, value, TAGMASK_LENGTH);
	split_tagmask = strtok (tagmask, ",");//split the string with commas
	while (split_tagmask != NULL)
	{
		split_tagmask=trim(split_tagmask);
		/*Compare sensors requested by the user with the array of available sensors and return a string that can be written to eeprom*/
		for (i=0; i < 15; i++){
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
//change tagmask
void
change_reportmask(char *value, char *reportNo){
	int i, size, m=0;
	char *tagmask=(char*) malloc(TAGMASK_LENGTH); //store mask from the user
	char *sensors[15]={"ADC_1","ADC_2","ADC_3","ADC_4","RH","P","T","T1","V_A1","V_A2","V_IN","T_MCU","V_MCU","INTR","P0_LST60"}; //array of available sens3
	char *split_tagmask, save_tagmask[TAGMASK_LENGTH]; //store the mask with sanitized values that we are going to write to eeprom
	strlcpy(tagmask, value, TAGMASK_LENGTH);
	split_tagmask = strtok (tagmask, ",");//split the string with commas
	while (split_tagmask != NULL)
	{
		split_tagmask=trim(split_tagmask);
		/*Compare sensors requested by the user with the array of available sensors and return a string that can be written to eeprom*/
		for (i=0; i < 15; i++){
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
                 if(!strncmp(reportNo, "0", 1)){
		eeprom_update_block((const void *)&save_tagmask, (void *)&eemem_report_0, report_0_length);
                      }else if(!strncmp(reportNo, "1", 1)){
		eeprom_update_block((const void *)&save_tagmask, (void *)&eemem_report_1, report_1_length);
                      }else if(!strncmp(reportNo, "2", 1)){
		eeprom_update_block((const void *)&save_tagmask, (void *)&eemem_report_2, report_2_length);
                      }else if(!strncmp(reportNo, "3", 1)){
		eeprom_update_block((const void *)&save_tagmask, (void *)&eemem_report_3, report_3_length);
                      
                      }else {
                       // power_save = 1; 
                        printf("please specify report number\n");  
                        //power_save = 0;    
                             }
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
void
set_default_report0mask(void){
	cli();
	eeprom_update_block((const void *)&default_sensors, (void *)&eemem_report_0, report_0_length);
	sei();
}
void
set_default_report1mask(void){
	cli();
	eeprom_update_block((const void *)&default_sensors, (void *)&eemem_report_1, report_1_length);
	sei();
}
void
set_default_report2mask(void){
	cli();
	eeprom_update_block((const void *)&default_sensors, (void *)&eemem_report_2, report_2_length);
	sei();
}
void
set_default_report3mask(void){
	cli();
	eeprom_update_block((const void *)&default_sensors, (void *)&eemem_report_3, report_3_length);
	sei();
}


/*Display the tagmask stored in eeprom*/
void
display_tagmask(void){
//char *sensorsl = "ADC_1 ADC_2 ADC_3 ADC_4 RH P T V_A1 V_A2 V_IN T_MCU V_MCU INTR";
	uint8_t tagmask[TAGMASK_LENGTH];
	uint8_t report_0[report_0_length];
	uint8_t report_1[report_1_length];
	uint8_t report_2[report_2_length];
	uint8_t report_3[report_3_length];
	
	cli();
	//eeprom_read_block((void*)&tagmask, (const void*)&eemem_tagmask, TAGMASK_LENGTH);
	eeprom_read_block((void*)&report_0, (const void*)&eemem_report_0, report_0_length);
	eeprom_read_block((void*)&report_1, (const void*)&eemem_report_1, report_1_length);
	eeprom_read_block((void*)&report_2, (const void*)&eemem_report_2, report_2_length);
	eeprom_read_block((void*)&report_3, (const void*)&eemem_report_3, report_3_length);
	
	
	sei();
	//power_save = 1; 
	//printf("Tagmask = %s\n", (char*)tagmask);
	printf("Report 0 = %s\n", (char*)report_0);
	printf("Report 1 = %s\n", (char*)report_1);
	printf("Report 2 = %s\n", (char*)report_2);
	printf("Report 3 = %s\n", (char*)report_3);
	printf("Possible report mask parameters = ADC_1 ADC_2 ADC_3 ADC_4 RH P T T1 V_A1 V_A2 V_IN T_MCU V_MCU INTR P0_LST60\n");
	//power_save = 0; 
       

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
	if (!strncmp("V_A1", sensor, 5) && strlen(alias)>0) {
		set_alias(1, alias);
                //power_save = 1; 
		printf("Successful V_A1 = %s\n", alias);
                //power_save = 0; 
	} else if (!strncmp("V_A2", sensor, 5) && strlen(alias)>0) {
		set_alias(2, alias);
                //power_save = 1; 
		printf("Successful V_A2 = %s\n", alias);
                //power_save = 0; 
	} else {
                //power_save = 1; 
		printf("Alias can not be set for %s\n.", sensor);
                //power_save = 0; 
	}
	free(alias_value);
}
/* Sleep process is just put MCU in sleep so we minimiza MCU impact */
PROCESS_THREAD(mcu_sleep_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    watchdog_periodic();
    if(power_save) 
      set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    else
      set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    sleep_enable();
    sei();  
    sleep_cpu();
    sleep_disable();
    sei();
    PROCESS_PAUSE();
  }
  PROCESS_END();
}


void auto_set_report_mask(char *value2[],int size1, int sensor_status,char *reportNo){

     int i, size,m=0,d=1,k=0,j=0; 
	char *tagmask2=(char*) malloc(TAGMASK_LENGTH); //store mask from the user
         char *sensors[15]={"ADC_1","ADC_2","ADC_3","ADC_4","RH","P","T","T1","V_A1","V_A2","V_IN","T_MCU","V_MCU","INTR","P0_LST60"}; //array of available sens3
        char *sensors3[15];
	 uint8_t report_a2[report_2_length];
         cli();
         if(!strncmp(reportNo, "0", 1)){
		 eeprom_read_block((void*)&report_a2, (const void*)&eemem_report_0, report_2_length);
                      }else if(!strncmp(reportNo, "1", 1)){
		 eeprom_read_block((void*)&report_a2, (const void*)&eemem_report_1, report_2_length);
                      }else if(!strncmp(reportNo, "2", 1)){
		 eeprom_read_block((void*)&report_a2, (const void*)&eemem_report_2, report_2_length);
                      }else if(!strncmp(reportNo, "3", 1)){
		 eeprom_read_block((void*)&report_a2, (const void*)&eemem_report_3, report_2_length);
                      }
         sei();
	char *split_tagmask2, save_tagmask2[TAGMASK_LENGTH] = {}; //store the mask with sanitized values that we are going to write to eeprom
	strlcpy(tagmask2, (char*)report_a2, report_2_length);
	split_tagmask2 = strtok (tagmask2, " ");//split the string with commas
        while( split_tagmask2 != NULL ) {
          split_tagmask2 = trim(split_tagmask2);
          sensors3[m] = split_tagmask2;
	    m++;
      split_tagmask2 = strtok(NULL, " ");
   }
      if(sensor_status == 1){
      for (i=0; i < size1; i++){
                       d=1;
                         for (int v=0; v < m; v++){
                             size=strlen(value2[i]);
			if (!strncmp(sensors3[v], value2[i], size)){
                                 d=0;
				}
                                }
                            if(d==1 && sensor_status==1){
                               size=strlen(value2[i]);
                               strncat((char*)report_a2, " ", 1);
			       strncat((char*)report_a2, value2[i], size);  
                              }
                              
      }
	if (strlen((char*)report_a2)>0) {//check if tagmask is not empty after validation
		cli();
                if(!strncmp(reportNo, "0", 1)){
		eeprom_update_block((const void *)&report_a2, (void *)&eemem_report_0, report_0_length);
                      }else if(!strncmp(reportNo, "1", 1)){
		eeprom_update_block((const void *)&report_a2, (void *)&eemem_report_1, report_1_length);
                      }else if(!strncmp(reportNo, "2", 1)){
		eeprom_update_block((const void *)&report_a2, (void *)&eemem_report_2, report_2_length);
                      }else if(!strncmp(reportNo, "3", 1)){
		eeprom_update_block((const void *)&report_a2, (void *)&eemem_report_3, report_3_length);
                      
                      }
                      sei();
	}
}
                    if(sensor_status==0){
                     for (i=0; i < m; i++){
                        d=1; j=0;
                         for (int v=0; v < size1; v++){
                             size=strlen(sensors3[i]);
			 if (!strncmp(sensors3[i], value2[v], size)){
                                 d=0;
                         if(!strncmp(sensors3[i],"T", 1)&& !strncmp(value2[v], "T_MCU", 5)){
                          d=1;
			      }	
                              }
                              }
                         for (int h=0; h < 16; h++){
                              size=strlen(sensors[j]);
                           if (!strncmp(sensors3[i], sensors[h], size)){
                            j=1;
                         }
                        }
                        if(d==1 && sensor_status==0 && j==1 ){
                         
                         if(k==0){
                          size=strlen(sensors3[i]);
                         strlcpy((char*)save_tagmask2,sensors3[i] , size+1);
                          k++;
                         }  else{
			size=strlen(sensors3[i]);
                        strncat((char*)save_tagmask2, " ", 1);
			strncat((char*)save_tagmask2, sensors3[i], size);		
			}
                        k++;
			}                        
                              
                         }
                     if (strlen((char*)save_tagmask2)>0) {
                       cli();
                       if(!strncmp(reportNo, "0", 1)){
		eeprom_update_block((const void *)&save_tagmask2, (void *)&eemem_report_0, report_0_length);
                      }else if(!strncmp(reportNo, "1", 1)){
		eeprom_update_block((const void *)&save_tagmask2, (void *)&eemem_report_1, report_1_length);
                      }else if(!strncmp(reportNo, "2", 1)){
		eeprom_update_block((const void *)&save_tagmask2, (void *)&eemem_report_2, report_2_length);
                      }else if(!strncmp(reportNo, "3", 1)){
		eeprom_update_block((const void *)&save_tagmask2, (void *)&eemem_report_3, report_3_length);
                      
                      }
                      sei();
                      }
      }
	free(tagmask2);
}
void removefromreportmask(char* parameter,char* reportNo){

     int  size1,size,m=0,d=1,correct =0,k=0; 
     
	//size = (int)sizeof(value2);
	//printf("%d\n",size);
	char *tagmaskremove = (char*) malloc(TAGMASK_LENGTH); 
	char *parametervalue = (char*) malloc(TAGMASK_LENGTH);//store mask from the user
        char *sensors[15]={"ADC_1","ADC_2","ADC_3","ADC_4","RH","P","T","T1","V_A1","V_A2","V_IN","T_MCU","V_MCU","INTR","P0_LST60"}; //array of available sens3
        char *sensorsremove[15];
	 uint8_t report_remove[report_2_length];
                    cli();
               if(!strncmp(reportNo, "0", 1)){
		 eeprom_read_block((void*)&report_remove, (const void*)&eemem_report_0, report_2_length);
                      }else if(!strncmp(reportNo, "1", 1)){
		 eeprom_read_block((void*)&report_remove, (const void*)&eemem_report_1, report_2_length);
                      }else if(!strncmp(reportNo, "2", 1)){
		 eeprom_read_block((void*)&report_remove, (const void*)&eemem_report_2, report_2_length);
                      }else if(!strncmp(reportNo, "3", 1)){
		 eeprom_read_block((void*)&report_remove, (const void*)&eemem_report_3, report_2_length);
                      }
                      sei();
        
       
	char *split_tagmaskremove, *split_parameterremove,save_tagmaskremove[TAGMASK_LENGTH] = {}; //store the mask with sanitized values that we are going to write to eeprom
	strlcpy(parametervalue, (char*)parameter, report_2_length);
	split_parameterremove = strtok (parametervalue, " ");//split the string with space
	    while( split_parameterremove != NULL ) {
	   // d=1;correct=0;
              
           split_parameterremove = trim(split_parameterremove);
           sensorsremove[m] =  split_parameterremove;
           m++;
          
      split_parameterremove = strtok(NULL, " ");
}
	strlcpy(tagmaskremove, (char*)report_remove, report_2_length);
	split_tagmaskremove = strtok (tagmaskremove, " ");//split the string with commas
        
        while( split_tagmaskremove != NULL ) {
            d=1;correct=0;
          split_tagmaskremove = trim(split_tagmaskremove);
              
                size1=strlen(split_tagmaskremove);
	     for (int v=0; v < m; v++){
          if (!strncmp(sensorsremove[v], split_tagmaskremove, size1)){
			//printf( " \t\t\t%s\n",(char*)split_tagmaskremove  );	        
                                 d=0;
	  }
	  }
	  for(int x =0; x<16; x++){
	  if (!strncmp(sensors[x], split_tagmaskremove, size1)){
	  correct = 1;
	  }
	  }           if(d==1 && correct==1){
	              if(k==0){
                         size=strlen(split_tagmaskremove);
                         strlcpy((char*)save_tagmaskremove,split_tagmaskremove, size+1);
                          k++;
                          }  else{
			size=strlen(split_tagmaskremove);
                        strncat((char*)save_tagmaskremove, " ", 1);
			strncat((char*)save_tagmaskremove, split_tagmaskremove, size);
			k++;
                       	
			}}
	                   
         split_tagmaskremove = strtok(NULL, " ");
   }  
                   cli();
            if (strlen((char*)save_tagmaskremove)>0) {
                       if(!strncmp(reportNo, "0", 1)){
                 eeprom_update_block((const void *)&save_tagmaskremove, (void *)&eemem_report_0, report_0_length);
                      }else if(!strncmp(reportNo, "1", 1)){
                     // printf( " %s\t\t\t%d\n",(char*)save_tagmaskremove,k  );	
		eeprom_update_block((const void *)&save_tagmaskremove, (void *)&eemem_report_1, report_1_length);
                      }else if(!strncmp(reportNo, "2", 1)){
		eeprom_update_block((const void *)&save_tagmaskremove, (void *)&eemem_report_2, report_2_length);
                      }else if(!strncmp(reportNo, "3", 1)){
		eeprom_update_block((const void *)&save_tagmaskremove, (void *)&eemem_report_3, report_3_length);
                      }
                 printf("%s removed successfully from report mask\n",parameter);
                     }
                      sei();
                      
 free(tagmaskremove);
 free(parametervalue);
}
void add2reportmask(char* parameter, char* reportNo){
int  size1,size,m=0,d=1,correct =0; 
     
	//size = (int)sizeof(value2);
	//printf("%d\n",size);
	char *tagmaskadd = (char*) malloc(TAGMASK_LENGTH); 
	char *parametervalue = (char*) malloc(TAGMASK_LENGTH);//store mask from the user
         char *sensors[15]={"ADC_1","ADC_2","ADC_3","ADC_4","RH","P","T","T1","V_A1","V_A2","V_IN","T_MCU","V_MCU","INTR","P0_LST60"}; //array of available sens3
        char *sensorsadd[15];
	 uint8_t report_add[report_2_length];
                    cli();
               if(!strncmp(reportNo, "0", 1)){
		 eeprom_read_block((void*)&report_add, (const void*)&eemem_report_0, report_2_length);
                      }else if(!strncmp(reportNo, "1", 1)){
		 eeprom_read_block((void*)&report_add, (const void*)&eemem_report_1, report_2_length);
                      }else if(!strncmp(reportNo, "2", 1)){
		 eeprom_read_block((void*)&report_add, (const void*)&eemem_report_2, report_2_length);
                      }else if(!strncmp(reportNo, "3", 1)){
		 eeprom_read_block((void*)&report_add, (const void*)&eemem_report_3, report_2_length);
                      }
                      sei();
        
        
	char *split_tagmaskadd, *split_parameteradd,save_tagmask2[TAGMASK_LENGTH] = {}; //store the mask with sanitized values that we are going to write to eeprom
	strlcpy(tagmaskadd, (char*)report_add, report_2_length);
	split_tagmaskadd = strtok (tagmaskadd, " ");//split the string with commas
       
        while( split_tagmaskadd != NULL ) {
         // size1=strlen(sensors2);
          split_tagmaskadd = trim(split_tagmaskadd);
          sensorsadd[m] = split_tagmaskadd;
	    m++;
         split_tagmaskadd = strtok(NULL, " ");
   }
   
        strlcpy(parametervalue, (char*)parameter, report_2_length);
	split_parameteradd = strtok (parametervalue, " ");//split the string with space
            
	    while( split_parameteradd != NULL ) {
	    d=1;correct=0;
         
           split_parameteradd = trim(split_parameteradd);
           size1=strlen(split_parameteradd);
           for (int v=0; v < m; v++){
          if (!strncmp(sensorsadd[v], split_parameteradd, size1)){
				        
                                 d=0;
	  }
	  }
	  for(int x =0; x<16; x++){
	  if (!strncmp(sensors[x], split_parameteradd, size1)){
	  correct = 1;
	  }
	  }
	                    if(d==1 && correct==1){
                             // printf( " \t\t\t%s\n",(char*)split_parameteradd );
                               size=strlen(split_parameteradd);
                               strncat((char*)report_add, " ", 1);
			       strncat((char*)report_add, split_parameteradd, size);  
                              }
         // sensorsadd[m] = split_tagmaskadd;
	   // m++;
      split_parameteradd = strtok(NULL, " ");
   }
	if (strlen((char*)report_add)>0) {//check if tagmask is not empty after validation
		cli();
                if(!strncmp(reportNo, "0", 1)){
		eeprom_update_block((const void *)&report_add, (void *)&eemem_report_0, report_0_length);
                      }else if(!strncmp(reportNo, "1", 1)){
		eeprom_update_block((const void *)&report_add, (void *)&eemem_report_1, report_1_length);
                      }else if(!strncmp(reportNo, "2", 1)){
		eeprom_update_block((const void *)&report_add, (void *)&eemem_report_2, report_2_length);
                      }else if(!strncmp(reportNo, "3", 1)){
		eeprom_update_block((const void *)&report_add, (void *)&eemem_report_3, report_3_length);
                      }
                      sei();
		 printf("%s added successfully to report mask\n",(char*)parameter);
                      }
                      free(parametervalue);
                      free(tagmaskadd);
}
//plug and play
void check_sensor_connection(char *reportNo){
    i2c_probed1 = i2c_probe();
    I2C_SHT25_flag = 0;
    I2C_MCP3424_flag = 0;
    I2C_MS5611_flag = 0;
        if( i2c_probed1 & I2C_SHT25) {                
               // sht25_init();
                SENSORS_ACTIVATE(sht25_sensor);
                char *mask[]= {"RH","T"};
		auto_set_report_mask(mask,2,1,reportNo);
               I2C_SHT25_flag = 1;
       
        }else if(!(i2c_probed1 & I2C_SHT25)){
         char *mask[]= {"RH","T"};
		auto_set_report_mask(mask,2,0,reportNo);
                I2C_SHT25_flag = 0;
              }
              
         if( i2c_probed1 & I2C_MCP3424 ) {
		//mcp3424_init(MCP3424_ADDR,0,8,16);  
          	SENSORS_ACTIVATE(mcp3424_sensor);
           	char *mask[]= {"ADC_1","ADC_2","ADC_3","ADC_4"};
		auto_set_report_mask(mask,4,1,reportNo);
                I2C_MCP3424_flag = 1;
	}else if(!(i2c_probed1 & I2C_MCP3424)){
         char *mask[]= {"ADC_1","ADC_2","ADC_3","ADC_4"};
		auto_set_report_mask(mask,4,0,reportNo);
                I2C_MCP3424_flag = 0;
              }
	 if(i2c_probed1 & I2C_MS5611_ADDR){
		SENSORS_ACTIVATE(ms5611_sensor);
		 char *mask[]= {"P"};
		auto_set_report_mask(mask,1,1,reportNo);
               I2C_MS5611_flag = 1;
	}else if(!(i2c_probed1 & I2C_MS5611_ADDR)){
         char *mask[]= {"P"};
		auto_set_report_mask(mask,1,0,reportNo);
                I2C_MS5611_flag = 0;
              }
	if( i2c_probed1 & I2C_DS1307 ) {
			DS1307_init();
			SENSORS_ACTIVATE(ds1307_sensor);
	}
         cli();
       if((eeprom_read_word(&eemem_I2C_SHT25_flag) != I2C_SHT25_flag) && (I2C_SHT25_flag == 0)){
        printf( "\nAlert \t******** I2C_SHT25 SENSOR Disconnected ***********\n\n" );
        eeprom_update_word(&eemem_I2C_SHT25_flag, I2C_SHT25_flag);
        error_log("#E_Dc(SHT25)");
       	}else if((eeprom_read_word(&eemem_I2C_SHT25_flag) != I2C_SHT25_flag )&& (I2C_SHT25_flag == 1)){
        printf( "\nAlert \t******** I2C_SHT25 SENSOR Connected ***********\n\n" );
         eeprom_update_word(&eemem_I2C_SHT25_flag, I2C_SHT25_flag);
        
        }
       if((eeprom_read_word(&eemem_I2C_MS5611_flag) != I2C_MS5611_flag )&& (I2C_MS5611_flag == 0)){
        printf( "\nAlert \t******** I2C_MS5611 SENSOR Disconnected ***********\n\n" );
        eeprom_update_word(&eemem_I2C_MS5611_flag, I2C_MS5611_flag);
        error_log("#E_Dc(MS5611)");
         
	}else if((eeprom_read_word(&eemem_I2C_MS5611_flag) != I2C_MS5611_flag )&& (I2C_MS5611_flag == 1)){
        printf( "\nAlert \t******** I2C_MS5611 SENSOR Connected ***********\n\n" );
        eeprom_update_word(&eemem_I2C_MS5611_flag, I2C_MS5611_flag);
         
	}
       if((eeprom_read_word(&eemem_I2C_MCP3424_flag) != I2C_MCP3424_flag) && (I2C_MCP3424_flag ==0)){
        printf( "\nAlert \t******** I2C_MCP3424 SENSOR Disconnected ***********\n\n" );
         eeprom_update_word(&eemem_I2C_MCP3424_flag, I2C_MCP3424_flag);
         error_log("#E_Dc(MCP3424)");
	}else if((eeprom_read_word(&eemem_I2C_MCP3424_flag) != I2C_MCP3424_flag) && (I2C_MCP3424_flag ==1)){
        printf( "\nAlert \t******** I2C_MCP3424 SENSOR Connected ***********\n\n" );
         eeprom_update_word(&eemem_I2C_MCP3424_flag, I2C_MCP3424_flag);
	}
        sei();

}
// error logging
void error_log(char *message){
   char* savelog[error_code_length];
   strlcpy(savelog, message, error_code_length);
	cli();
	eeprom_update_block((const void *)&savelog, (void *)&eemem_error_codes, error_code_length);
	sei();
        error_status =1;

}
/*static unsigned long
to_second(uint64_t time)
{
  return (unsigned long)(time / ENERGEST_SECOND);
}*/




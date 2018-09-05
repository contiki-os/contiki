/**
* \file
*         WIMEA-ICT Gen3 AWS Gateway
* \details
*   ATMEGA256RFR2 RSS2 MOTE with RTC, SD card and Electron 3G uplink + Asynchronous scheduling
* \author
*         Maximus Byamukama <maximus.byamukama@cedat.mak.ac.ug>
*/
#include <stdio.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "contiki.h"
#include "rss2.h"
#include "netstack.h"
#include "rf230bb.h"
#include "net/rime/rime.h"
#include "net/packetbuf.h"
#include "dev/leds.h"
#include "dev/pwrpin.h"
#include "dev/adc.h" 
#include "dev/i2c.h"
#include "dev/ds3231.h"
#include "dev/temp-sensor.h"

#define SYS_VLOW 3.30
#define ARR_SZ 15


char report [200],sinkrep[150];
float time=0, arr_t[ARR_SZ];
static int get_schedule=1, i=0, j=0, count=0,mcu_sleeping=0;

double v_in=0, v_mcu=0;
uint8_t rssi,lqi,v_low=1,secs=0,len=0;
long int rep_count=0;

static struct timer t;
static double up_time=0;

datetime_t datetime;

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[120];  
};


PROCESS(radio_process, "Radio Process");
PROCESS(time_process, "Time Process");
AUTOSTART_PROCESSES(&radio_process,&time_process);


static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct broadcast_message *msg; 
	len=0;
		
	msg = packetbuf_dataptr();
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
	
	//ds3231_get_datetime(&datetime); /* use if RTC is connected*/
	datetime.year=18;datetime.month=9;datetime.day=5;datetime.hours=12;datetime.mins=0;datetime.secs=0;
	secs=datetime.secs; //struct seems to lose this value!
	
	/*remove for time keeper node*/
	if(get_schedule){
		arr_t[j]=time;
		j++; 
		if(i>ARR_SZ)i=0;
	}
	
	len+=sprintf(report, "TIME = %4.2f RTC_T=20%d-%02d-%02d,%02d:%02d:%02d %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",
	time, datetime.year,datetime.month,datetime.day, datetime.hours,datetime.mins,secs,msg->buf,
	from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi);

	report[len++] = '\0'; 
	pwr_pin_on();
	timer_set(&t, CLOCK_SECOND/100); //10ms pulse to wake up electron from sleep
	while(!timer_expired(&t));
	pwr_pin_off();
	
	leds_on(LEDS_RED);
	printf("%s", report);
	leds_off(LEDS_RED);
	++rep_count;
	
	if(rep_count%10==0)
	{
		len=0;  
		len += sprintf(&sinkrep[len],"RTC_T=20%d-%02d-%02d,%02d:%02d:%02d TXT=makg3-sink",
		datetime.year,datetime.month, datetime.day, datetime.hours,datetime.mins,secs);
		
		v_in=adc_read_v_in();
		len += sprintf(&sinkrep[len]," V_IN=%-4.2f", v_in); 
		
		SENSORS_ACTIVATE(temp_sensor);
		len += sprintf(&sinkrep[len]," T=%-5.2f", (double)(temp_sensor.value(0)*1.0/100));
		SENSORS_DEACTIVATE(temp_sensor);
		
		if(v_in < SYS_VLOW){
			len += sprintf(&sinkrep[len]," V_LOW=1");
		}	
		v_mcu = adc_read_v_mcu();
		
		len += sprintf(&sinkrep[len]," V_MCU=%-4.2f REPS=%ld UP_TIME=%.2f\n",v_mcu,rep_count,up_time); 
		
		sinkrep[len++] = '\0';  
		pwr_pin_on();
	    timer_set(&t, CLOCK_SECOND/100); //10ms pulse to wake up electron from sleep
	    while(!timer_expired(&t));
	    pwr_pin_off();
	
		leds_on(LEDS_YELLOW);
		printf("%s", sinkrep);
		leds_off(LEDS_YELLOW);
		++rep_count;	
	}
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

PROCESS_THREAD(radio_process, ev, data)
{
	static struct etimer et;
	
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
	PROCESS_BEGIN();
	printf("\nradio process has started. time =%4.2f\n",time);
	pwr_pin_init();
    ds3231_init();
	broadcast_open(&broadcast, 129, &broadcast_call);	
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	/*for rpc change*/
	NETSTACK_RADIO.off(); 
	rf230_set_rpc(0xFF); 
	for(i=0; i<ARR_SZ; i++)
	{
		arr_t[i]=0.0;
	}
	NETSTACK_RADIO.on();
	get_schedule=1;
	etimer_set(&et, CLOCK_SECOND*60); 
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	NETSTACK_RADIO.off();
	get_schedule=0;
	printf("expected report times: ");
	for(i=0; i<ARR_SZ; i++)
	{
		printf("[%4.2f]",arr_t[i]);
	}
	
	while(1) {	  	
		
		for(i=0; i<ARR_SZ; i++)
		{
			if((arr_t[i] - time) >= -0.25 && (arr_t[i] - time) <= 0.25)
			{
				NETSTACK_RADIO.on();
				if(mcu_sleeping) //wake up!
				{
					mcu_sleeping=0;
					sleep_disable();
					sei();
				}
				printf("radio on. report ETA=%4.2f, time now=%4.2f\n",arr_t[i],time);
				break;
			}
			else 
			{
				NETSTACK_RADIO.off();
				if(!mcu_sleeping){  
				set_sleep_mode(SLEEP_MODE_IDLE);  //we need lower sleep mode but issue with timing!!!
				mcu_sleeping=1;
				cli();
				sleep_enable();
				sei(); 
				sleep_cpu();
				}
			}
		}
		etimer_set(&et, CLOCK_SECOND/7); //142 ms
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}
PROCESS_THREAD(time_process, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	while(1) {	
		time+=0.25;
		if(time > 59.75) 
		{
			time=0;	
			printf("one minute has elapsed\n");
			++count;
			if(count==5)  //5 minutes elapsed. re-establish schedule
			{
				count=0;
				process_exit(&radio_process);
				printf("restarting radio process. time =%4.2f\n",time);
				process_start(&radio_process, NULL);
			}
		}
		etimer_set(&et, CLOCK_SECOND/4); //250 ms
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}
	PROCESS_END();
}

/**
* \file
*         WIMEA-ICT Gen3 AWS Gateway
* \details
*   ATMEGA256RFR2 RSS2 MOTE with RTC, SD card and Electron 3G uplink
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
#include <dev/watchdog.h>

#define SYS_VLOW 3.30

double v_in=0, v_mcu=0;
uint8_t rssi,lqi,seqno=0,v_low=1,secs=0,len=0;
long int rep_count=0;
char report[200],sinkrep[150];
static struct timer t;
static int ps = 0; /* Power-save false */
static double up_time=0;

datetime_t datetime;

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[120]; 
};

/*---------------------------------------------------------------------------*/
PROCESS(init_process, "Init Process");
PROCESS(radio_process, "Radio Process");
PROCESS(sleep_process, "Sleep Process");
AUTOSTART_PROCESSES(&init_process, &radio_process,&sleep_process);

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct broadcast_message *msg; 
	len=0;
		
	msg = packetbuf_dataptr();
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
	
	ds3231_get_datetime(&datetime);
	secs=datetime.secs;
	
	len+=sprintf(report, "RTC_T=20%d-%02d-%02d,%02d:%02d:%02d %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",
	datetime.year,datetime.month,datetime.day, datetime.hours,datetime.mins,secs,msg->buf,
	from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi);

	report[len++] = '\0'; 
	pwr_pin_on();
	timer_set(&t, CLOCK_SECOND/100); //5ms pulse to wake up electron from sleep
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
	    timer_set(&t, CLOCK_SECOND/100); //5ms pulse to wake up electron from sleep
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
/*---------------------------------------------------------------------------*/
/* sinknode_process handles sink own reports and initializes RTC */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(init_process, ev, data)
{
	static struct etimer et;
	
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);   
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	/*for rpc change*/
	NETSTACK_RADIO.off(); 
	rf230_set_rpc(0xFF); 
	NETSTACK_RADIO.on();
	pwr_pin_init();
    ds3231_init();
	etimer_set(&et, CLOCK_SECOND * 60);
	while(1) {  
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);
		up_time+=(1.0/1440);     
	}
	PROCESS_END();
}

/*This handles application level radio duty cycling*/
PROCESS_THREAD(radio_process, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	while(1) {  
		NETSTACK_RADIO.on();
		ps=0;
		etimer_set(&et, CLOCK_SECOND * 17);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		NETSTACK_RADIO.off();
		ps=1;
		etimer_set(&et, CLOCK_SECOND * 43);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));	
	}
	PROCESS_END();
}
/* This handles MCU sleep */
PROCESS_THREAD(sleep_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    watchdog_periodic();
    if(ps) 
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

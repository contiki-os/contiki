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
#include "dev/adc.h"
#include "dev/i2c.h"
#include "dev/ds3231.h"
#include "dev/temp-sensor.h"

#define SYS_VLOW 2.80

double v_in, v_mcu,t_box;
uint8_t rssi,lqi,seqno=0,v_low=1,secs;
int len=0,rep_count=0,ps=1;
long int up_time=0; 
static int id_count=0;
char report[220],sinkrep[200], rep_id[17];
static struct timer t;


datetime_t datetime;

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[120]; 
};


/*---------------------------------------------------------------------------*/
PROCESS(sinknode_process, "Sink Node Process");
AUTOSTART_PROCESSES(&sinknode_process);


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
	
	
	++id_count; if(id_count>99) id_count=0;
	sprintf(rep_id,"mak%02d%02d%02d%02d%02d%02d%02d",datetime.year,datetime.month,datetime.day, 
	datetime.hours,datetime.mins,secs,id_count); //-mak17120415475601
	
	len+=sprintf(report, "ID=%s RTC_T=20%d-%02d-%02d,%02d:%02d:%02d %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",
	rep_id, datetime.year,datetime.month,datetime.day, datetime.hours,datetime.mins,secs,msg->buf,
	from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi);

	report[len++] = '\0'; 
	PORTE &= ~ (1 << PWR_1); //high
	timer_set(&t, CLOCK_SECOND/100); //10ms pulse to wake up electron from sleep
	while(!timer_expired(&t));
	PORTE |= (1 << PWR_1); //low
	
	leds_on(LEDS_RED);
	printf("%s", report);
	leds_off(LEDS_RED);
	++rep_count;
}


static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
/* sinknode_process handles sink own reports and initializes RTC and SD card */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sinknode_process, ev, data)
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
	
	/*initialize rtc, leds and watchdog*/
	ds3231_init();
	leds_init();
	
	DDRE |= (1 << PWR_1); //init power pin (uplink switch)
	PORTE |= (1 << PWR_1); //and make sure its off -- pulled high
	
	etimer_set(&et, CLOCK_SECOND * 60);
	while(1) {  
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);
		++up_time;       
		len=0;  
		/*get RTC datetime --this is part of the report*/
		ds3231_get_datetime(&datetime);
		secs=datetime.secs;
		
		++id_count; if(id_count>99) id_count=0;
		sprintf(rep_id,"mak%02d%02d%02d%02d%02d%02d%02d",datetime.year,datetime.month,datetime.day, 
		datetime.hours,datetime.mins,secs,id_count); //-mak17112915475600001
		
		len += sprintf(&sinkrep[len],"ID=%s RTC_T=20%d-%02d-%02d,%02d:%02d:%02d TXT=mak-gen3",rep_id,
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
		
		len += sprintf(&sinkrep[len]," V_MCU=%-4.2f REPS=%d UP_TIME=%ld",v_mcu,rep_count,up_time); 
		
		sinkrep[len++] = '\0';  
		PORTE &= ~ (1 << PWR_1); //high
		timer_set(&t, CLOCK_SECOND/100); //10ms pulse to wake up electron from sleep
	    while(!timer_expired(&t));
		PORTE |= (1 << PWR_1); //low
		
		leds_on(LEDS_YELLOW);
		printf("%s\n", sinkrep);
		leds_off(LEDS_YELLOW);
		++rep_count;
	}
	PROCESS_END();
}

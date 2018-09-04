#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contiki.h"
#include "netstack.h"
#include "rf230bb.h"
#include "net/rime/rime.h"
#include "net/packetbuf.h"
#include "dev/temp-sensor.h"
#include "dev/pwrpin.h"
#include "dev/leds.h"
#include "dev/adc.h"
#include "dev/watchdog.h"


#define DEF_TTL 0xF
#define TRANSMIT_INTERVAL 1

char report [200];
float v_in, v_mcu;
uint8_t rssi,lqi;
static int time=0,i=0,diff=0,j=0;
static int arr_t[5];
int ps=0,min=500;

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[120];  
};

uint8_t ttl = DEF_TTL;

/*---------------------------------------------------------------------------*/
PROCESS(radio_process, "Radio Process");
PROCESS(time_process, "Time Process");
AUTOSTART_PROCESSES(&radio_process,&time_process);


static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct broadcast_message *msg;
	leds_on(LEDS_RED);
	msg = packetbuf_dataptr();
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
   
	 /*remove for time keeper node*/
	 arr_t[i]=time;
	 i++; if(i>7)i=0;
	sprintf(report, "TIME=%d %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",time,msg->buf,
	from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi); //datetime string
	leds_off(LEDS_RED);
	printf("%s",report);
}


static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

PROCESS_THREAD(radio_process, ev, data)
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
	etimer_set(&et, CLOCK_SECOND*60); 
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	NETSTACK_RADIO.off();
	
	while(1) {	  	
		if(ps) 
		{
			NETSTACK_RADIO.on();
			printf("time = %d, diff=% incoming", time, diff);
			etimer_set(&et, CLOCK_SECOND); //check every 125ms
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		}
		else NETSTACK_RADIO.off();
	
	etimer_set(&et, CLOCK_SECOND/5); //check every 200ms
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}
PROCESS_THREAD(time_process, ev, data)
{
   static struct etimer et;
   	PROCESS_BEGIN();
	while(1) {	
	++time;
	if(time > 239) time=0;
	for(j=0;j<5;j++)
	{
		diff = arr_t[j]-time;
		if(diff < min && diff > 0)
		{
			min=diff;
		}
		
	}

	if(min < 3 && min >= 0)
	{
		ps=0;
	}else
	{
		ps=1;
	}	
	etimer_set(&et, CLOCK_SECOND/4); //250 ms
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}
	PROCESS_END();
}


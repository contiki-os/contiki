/**
* \file
*         WIMEA-ICT Gen3 Sink Node with RTC and SD card
* \author
*         Maximus Byamukama <maximus.byamukama@cedat.mak.ac.ug>
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include "contiki.h"
#include "netstack.h"
#include "rf230bb.h"
#include "net/rime/rime.h"
#include "net/packetbuf.h"
#include "dev/leds.h"
#include "dev/adc.h"
#include "dev/ff.h"
#include "dev/integer.h"
#include <dev/watchdog.h>

#define MAX_BCAST_SIZE 99
#define DEF_TTL 0xF
#define TRANSMIT_INTERVAL 20

char str_t[127], report [200];
float v_in, v_mcu;
int p1,p2,lines=0; //temporary upper and lower parts of floating point variables
uint8_t rssi,lqi,v_low=0; 

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[MAX_BCAST_SIZE+20];  /* Check for max payload 20 extra to be able to test */

};

uint8_t ttl = DEF_TTL;

/*---------------------------------------------------------------------------*/
PROCESS(sinknode_process, "Sink Node Process");
PROCESS(sleep_process,   "Sleep process");
AUTOSTART_PROCESSES(&sinknode_process, &sleep_process);


static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct broadcast_message *msg;
	
	msg = packetbuf_dataptr();
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

	sprintf(report, "%s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",msg->buf,
	from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi); //datetime string
	printf("%s",report);
}


static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sinknode_process, ev, data)
{
	static uint8_t seqno;
	struct broadcast_message msg;
	static struct etimer et;
	
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);	
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	NETSTACK_RADIO.off(); //for rpc change
	rf230_set_rpc(0xFF); 
	NETSTACK_RADIO.on();

	while(1) {
		/* Delay 4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	
		int len=0;	
		len += sprintf(&msg.buf[len],"&: TXT=mak-gen3");
	
		v_in=adc_read_v_in();
		p1 = (int)v_in; //e.g. 4.93 gives 4
		p2 = (v_in*100)-(p1*100); // =93	
		len += sprintf(&msg.buf[len]," V_IN=%d.%02d",p1,p2);
		
		v_mcu = adc_read_v_mcu();
		p1 = (int)v_mcu; //e.g. 4.93 gives 4
		p2 = (v_mcu*100)-(p1*100); // =93	
		len += sprintf(&msg.buf[len]," V_MCU=%d.%02d ",p1,p2);	
		
		if(v_in < 2.80){
			v_low=1;
			len += sprintf(&msg.buf[len]," V_LOW=1");
		}
		
		msg.buf[len++] = '\0';		/*null terminate the string*/
		packetbuf_copyfrom(&msg, len+2);		
		
		msg.head = 1<<4; 
		msg.head |= ttl;
		msg.seqno = seqno;
		
		broadcast_send(&broadcast);
		seqno++;
		printf("&: %s \n\r", msg.buf);
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sleep_process, ev, data)
{
	PROCESS_BEGIN();
	while(1) {
		watchdog_periodic(); 
		set_sleep_mode(SLEEP_MODE_PWR_SAVE);
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
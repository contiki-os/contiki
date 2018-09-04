#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contiki.h"
#include "netstack.h"
#include "net/rime/rime.h"
#include "net/packetbuf.h"
#include "dev/temp-sensor.h"
#include "dev/pwrpin.h"
#include "dev/leds.h"
#include "dev/adc.h"
#include "dev/watchdog.h"


#define DEF_TTL 0xF

char report [200];
float v_in, v_mcu;
uint8_t rssi,lqi;
static int ntime=0, ltime=0, count=0;

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[120];  
};

uint8_t ttl = DEF_TTL;

/*---------------------------------------------------------------------------*/
PROCESS(transmit_process, "Transmit Process");
PROCESS(time_process, "Time Process");
AUTOSTART_PROCESSES(&transmit_process,&time_process);


static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct broadcast_message *msg;
	leds_on(LEDS_RED);
	msg = packetbuf_dataptr();
	rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
   
   /*remove for time keeper node*/
    char* substr = (char*) malloc(3);
	if(strstr(msg->buf,"NET_TIME")!=NULL) //time set instruction
	{
		strncpy(substr, msg->buf+9, 2);
		ntime=atoi(substr);
		ltime=ntime;
		printf("new time received t=%d",ntime);
	}
	 /*remove for time keeper node*/
	sprintf(report, "LOCAL_TIME=%d %s [ADDR=%-d.%-d SEQ=%-d TTL=%-u RSSI=%-u LQI=%-u]\n",ltime,msg->buf,
	from->u8[0], from->u8[1],msg->seqno, msg->head & 0xF, rssi,lqi); //datetime string
	printf("%s",report);
}


static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(transmit_process, ev, data)
{
	static uint8_t seqno;
	struct broadcast_message msg;
	static struct etimer et;
	
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);	
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);

	etimer_set(&et, CLOCK_SECOND*60);
	PROCESS_YIELD_UNTIL(etimer_expired(&et));
	while(1) {	
		PROCESS_YIELD_UNTIL(ev==0x10);
		leds_on(LEDS_YELLOW);
		int len=0;	
		len += sprintf(&msg.buf[len],"LOCAL_TIME=%d TXT=node2",ltime);	
		v_in=adc_read_v_in();
		len += sprintf(&msg.buf[len]," V_IN=%-4.2f", v_in); 
		
		SENSORS_ACTIVATE(temp_sensor);
		len += sprintf(&msg.buf[len]," T=%-5.2f", (double)(temp_sensor.value(0)*1.0/100));
		SENSORS_DEACTIVATE(temp_sensor);
		v_mcu = adc_read_v_mcu();	
		len += sprintf(&msg.buf[len]," V_MCU=%-4.2f\n",v_mcu); 		
		
		msg.buf[len++] = '\0';		/*null terminate the string*/
		packetbuf_copyfrom(&msg, len+2);			
		msg.head = 1<<4; 
		msg.head |= ttl;
		msg.seqno = seqno;	
		NETSTACK_RADIO.on();
		broadcast_send(&broadcast);
		NETSTACK_RADIO.off();
		seqno++;
		printf("%s", msg.buf);
		leds_off(LEDS_YELLOW);
	}

	PROCESS_END();
}
PROCESS_THREAD(time_process, ev, data)
{
   static struct etimer et;
   	PROCESS_BEGIN();
	while(1) {	
	++ltime;
	if(ltime>59) ltime=0;
	if(ltime==ntime)
	{
		process_post(&transmit_process, 0x10, NULL);
		++count;
	}
	if(count>=60) //1 hour has elapsed
	{
		count=0;
		watchdog_reboot(); //restart the node to re-establish sync
	}
	printf("time=%d\n",ltime);
	etimer_set(&et, CLOCK_SECOND);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}
	PROCESS_END();
}


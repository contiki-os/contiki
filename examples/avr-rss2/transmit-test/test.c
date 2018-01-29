#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contiki.h"
#include "netstack.h"
#include "net/rime/rime.h"
#include "net/packetbuf.h"

struct broadcast_message {
	uint8_t head; 
	uint8_t seqno;
	char buf[200]; 
};

char sinkrep[200];
int len=0,seqno;

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
}

PROCESS(transmit_process, "Transmit Process");
AUTOSTART_PROCESSES(&transmit_process);

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

PROCESS_THREAD(transmit_process, ev, data)
{
	static struct etimer et;
	struct broadcast_message msg;  //to be sent out
	
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);   
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	etimer_set(&et, CLOCK_SECOND * 2);
	while(1) {  
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);			
		len += sprintf(&sinkrep[len],"ID=mak17121313450942 RTC_T=2017-12-13,13:45:09 TXT=mak-gen3 T=31.56 V_MCU=3.00 V_IN=4.12 SD.ERR=0 REPS=350 UP_T=100000");
			/*transmit diagnositic report to nearby nodes */
			snprintf(msg.buf, len, "%s", (char*)sinkrep);
			msg.buf[len++]='\0';//null terminate report.
			msg.head = (1<<4) | 0xF;
			msg.seqno = ++seqno;
			packetbuf_copyfrom(&msg, len+2);
			broadcast_send(&broadcast);
			printf("Transmission sent\n");
		}
	PROCESS_END();
}
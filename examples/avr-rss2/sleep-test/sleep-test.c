#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include "dev/leds.h"
#include "rf230bb.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"
#include <avr/sleep.h>
#include <dev/watchdog.h>


static int ps = 1,t=1;     /* Power-save false */

PROCESS(sleep_process,   "Sleep process");
PROCESS(check_process,   "Check process");
AUTOSTART_PROCESSES(&sleep_process, &check_process);

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	printf("\nbroadcast message received from %d.%d: '%s'\n",
	from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

PROCESS_THREAD(sleep_process, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	while(1) {
		etimer_set(&et, CLOCK_SECOND/100);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
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

PROCESS_THREAD(check_process, ev, data)
{
	static struct etimer et;
	PROCESS_EXITHANDLER(broadcast_close(&broadcast));
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);   
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26);
	NETSTACK_RADIO.on();
	while(1) {
		etimer_set(&et, CLOCK_SECOND);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		printf("\nt=%d\n",t);
		++t; if(t>59) t=0;
		
		if(t>20 && t < 40) NETSTACK_RADIO.off();
		else NETSTACK_RADIO.on();
	}
	PROCESS_END();
}

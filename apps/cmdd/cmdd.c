#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

struct cmdd_state {
  struct psock s;
  char inputbuf[8];
  struct timer timer;
  int i;
  char command;
};

#define COMMAND_NONE 0
#define COMMAND_PS   1

MEMB(conns, struct cmdd_state, 1);

PROCESS(cmdd_process, "Command server");

static struct uip_udp_conn *udpconn;
static char send_udp = 0;
static const char *prompt = "contiki> ";

/*---------------------------------------------------------------------------*/
static char * CC_FASTCALL
n(uint16_t num, char *ptr)
{
  uint16_t d;
  uint8_t a, f;

  if(num == 0) {
    *ptr = '0';
    return ptr + 1;
  } else {
    f = 0;
    for(d = 10000; d >= 1; d /= 10) {
      a = (num / d) % 10;
      if(f == 1 || a > 0) {
	*ptr = a + '0';
	++ptr;
	f = 1;
      }
    }
  }
  return ptr;
}
/*---------------------------------------------------------------------------*/
/*static unsigned short
ps_generate(void *state)
{
  struct ek_proc *p = (struct ek_proc *)state;
  char *ptr = (char *)uip_appdata;

  ptr = n(EK_PROC_ID(p), ptr);

  *ptr++ = ' ';
  strncpy(ptr, p->name, 40);
  ptr += strlen(p->name);
  *ptr++ = '\n';

  return ptr - (char *)uip_appdata;
}*/
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_connection(struct cmdd_state *s))
{
  PSOCK_BEGIN(&s->s);

  while(1) {
    PSOCK_SEND(&s->s, prompt, strlen(prompt));
    PSOCK_WAIT_UNTIL(&s->s, PSOCK_NEWDATA(&s->s) ||
		             s->command != COMMAND_NONE);

    if(PSOCK_NEWDATA(&s->s)) {
      PSOCK_READTO(&s->s, '\n');
      if(strncmp(s->inputbuf, "quit", 4) == 0) {
	PSOCK_CLOSE_EXIT(&s->s);
	memb_free(&conns, s);
	tcp_markconn(uip_conn, NULL);
	/*      } else if(strncmp(s->inputbuf, "ps", 2) == 0) {
	PSOCK_GENERATOR_SEND(&s->s, ps_generate, ek_procs);
	
	for(s->i = 0; s->i < 40; s->i++) {
	  if(ek_process(s->i) != NULL) {
	    PSOCK_GENERATOR_SEND(&s->s, ps_generate, ek_process(s->i));
	  }
	  }*/
      } else if(strncmp(s->inputbuf, "send", 4) == 0) {
	send_udp = 1;
	PSOCK_WAIT_UNTIL(&s->s, send_udp == 0);
      } else {
	PSOCK_SEND(&s->s, "?\n", 2);
      }
    } else {
      switch(s->command) {
      }
    }

  }
  PSOCK_END(&s->s);
}
/*---------------------------------------------------------------------------*/
static void
appcall(void *state)
{
  struct cmdd_state *s = (struct cmdd_state *)state;

  if(uip_udpconnection()) {
    if(send_udp) {
      uip_udp_send(8);
      send_udp = 0;
    }
  } else {
    if(uip_closed() || uip_timedout() || uip_aborted()) {
      if(state != NULL) {
	memb_free(&conns, state);
      }
    } else if(uip_connected()) {
      s = (struct cmdd_state *)memb_alloc(&conns);
      if(s == NULL) {
	uip_abort();
      } else {
	tcp_markconn(uip_conn, s);
	PSOCK_INIT(&s->s, s->inputbuf, sizeof(s->inputbuf) - 1);
	timer_set(&s->timer, CLOCK_SECOND * 60);
	handle_connection(s);
      }
    } else if(s != NULL) {
      if(uip_poll()) {
	if(timer_expired(&s->timer)) {
	  memb_free(&conns, state);
	  uip_abort();
	  return;
	}
      }
      timer_reset(&s->timer);
      handle_connection(s);
    } else {
      uip_abort();
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cmdd_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  
  PROCESS_BEGIN();
  
  tcp_listen(UIP_HTONS(6581));
  memb_init(&conns);
  uip_ipaddr(&ipaddr, 255,255,255,255);
  udpconn = udp_new(&ipaddr, UIP_HTONS(6712), NULL);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {
      appcall(data);
    } else if(ev == PROCESS_EVENT_EXIT) {
      process_exit(&cmdd_process);
      LOADER_UNLOAD();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

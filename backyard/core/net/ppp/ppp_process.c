/*
 * $Id: ppp_process.c,v 1.1 2007/05/26 07:14:40 oliverschmidt Exp $
 */

#include "contiki.h"
#include "contiki-net.h"
#include "net/ppp/ppp.h"

PROCESS(ppp_process, "PPP");

//-----------------------------------------------------------------------------
// Note, that the UART RX-FIFO must be polled often enough so we don't get an
// overrun at the used baudrate.
//-----------------------------------------------------------------------------
#define	PPP_POLL_INTERVAL	(CLOCK_SECOND / 256)
#define	AT_TIMEOUT		(2 * CLOCK_SECOND / PPP_POLL_INTERVAL)
#define	AT_RX_BUF		64

enum {
  AT_CONNECT_DIRECT,
  AT_CONNECT_AUTO,
  AT_CONNECT_MANUAL
} At_connect;

enum {
  AT_IDLE,
  AT_CONNECT,
  AT_CONNECT1,
  AT_CONNECT2,
  AT_CONNECTED,
  AT_RESET,
  AT_RESET1,
  AT_RESET2,
  AT_RESET3,
  AT_RESET4,
  AT_RESET5
};

static struct etimer pppTimer;
static	u8_t	at_state;
static	u8_t	at_connect_mode = AT_CONNECT_MODE;
static	int	at_timeout = -1;
static	u8_t	at_rxBuf[ AT_RX_BUF ];


static void
at_puts(char *s) {
  while(*s) {
    ppp_arch_putchar(*s);
    s++;
  }
}


static u8_t
*at_gets() {
  static u8_t consumed;
  int	len;
  int	i;

  if (consumed) {
    consumed = 0;
    at_rxBuf[0] = 0;
  }
  len = strlen(at_rxBuf);

  for (i=len; i<AT_RX_BUF-1; i++) {
    if ( ppp_arch_getchar(&at_rxBuf[i])) {
      at_rxBuf[i+1] = 0;
      if (at_rxBuf[i] == '\n') {
//printf("at_gets=(%s)\n", at_rxBuf);
	consumed = 1;
        return at_rxBuf;
      }
    } else {
      return NULL;
    }
  }
  // overflow -> reset the buf
  at_rxBuf[0] = 0;
  return NULL;
}


static void
at_poll() {

  static u8_t loopCnt = 0;
  u8_t	*s;

  if (at_timeout > 0) {
    at_timeout--;
  }

  if (at_state == AT_CONNECTED) {
    ppp_poll();
    if(uip_len > 0) {
      tcpip_input();
    }

    if (loopCnt++ > 16) {
      int	i;
      loopCnt = 0;
      for (i=0; i<UIP_CONNS; i++) {
	uip_periodic(i);
	if(uip_len > 0) {
	  tcpip_output();
	}
      }
    }
  } else {

    switch (at_state) {

    case AT_IDLE:
	switch (at_connect_mode) {

	case AT_CONNECT_DIRECT:
	  at_state = AT_CONNECTED;
	  ppp_connect();
	  break;

	case AT_CONNECT_AUTO:
	  at_state = AT_CONNECT;
	  break;
	}
	break;

    case AT_CONNECT:
	at_gets();	// make the input buffer empty
	at_puts("ATE1\r\n");
	at_state = AT_CONNECT1;
	at_timeout = AT_TIMEOUT;
        break;
    
    case AT_CONNECT1:
	if (at_timeout == 0) {
	  at_state = AT_RESET;
	} else if ((s=at_gets())) {
	  if (strcmp(s,"OK\r\n") == 0) {
	    at_puts("AT+CGDCONT=1,\"IP\",\"online.telia.se\",\"\",0,0\r\n");
	    at_timeout = AT_TIMEOUT;
	    at_state = AT_CONNECT2;
	  }
	}
        break;

    case AT_CONNECT2:
	if (at_timeout == 0) {
	  at_state = AT_RESET;
	} else if ((s=at_gets())) {
	  if (strcmp(s,"OK\r\n") == 0) {
	    at_puts("ATD*99***1#\r\n");
	    at_state = AT_CONNECTED;
	    ppp_connect();
	  }
	}
        break;

    case AT_RESET:
//printf("AT_RESET\n");
	at_timeout = AT_TIMEOUT;
	at_state = AT_RESET1;
        break;

    case AT_RESET1:
	if (at_timeout == 0) {
	  lcp_disconnect(random_rand() & 0xff);
	  at_timeout = AT_TIMEOUT;
	  at_state = AT_RESET2;
	}
        break;

    case AT_RESET2:
	if (at_timeout == 0) {
	  lcp_disconnect(random_rand() & 0xff);
	  at_timeout = AT_TIMEOUT;
	  at_state = AT_RESET3;
	}
        break;

    case AT_RESET3:
	if (at_timeout == 0) {
	  at_puts("+++");
	  at_timeout = AT_TIMEOUT;
	  at_state = AT_RESET4;
	}
        break;

    case AT_RESET4:
	if (at_timeout == 0) {
	  at_puts("ATZ\r\n");
	  at_timeout = AT_TIMEOUT;
	  at_state = AT_RESET5;
	}
        break;

    case AT_RESET5:
	at_gets();	// make the input buffer empty
	if (at_timeout == 0) {
	  at_state = AT_IDLE;
	}
        break;
    }
  }
}

void ppp_reconnect() {
  at_state = AT_RESET;
  uip_fw_unregister(&pppif);
}


PROCESS_THREAD(ppp_process, ev, data)
{
  PROCESS_BEGIN();
  
  at_state = AT_IDLE;
  at_rxBuf[0] = 0;

  ppp_init();
  etimer_set(&pppTimer, PPP_POLL_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    
    if(etimer_expired(&pppTimer)) {
      etimer_set(&pppTimer, PPP_POLL_INTERVAL);
      at_poll();
    }
  }
  
  PROCESS_END();
}

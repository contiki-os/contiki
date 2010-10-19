
#include "contiki-net.h"

static struct psock ps;
static char buffer[100];

PROCESS(example_psock_client_process, "Example protosocket client");

/*---------------------------------------------------------------------------*/
static int
handle_connection(struct psock *p)
{
  PSOCK_BEGIN(p);

  PSOCK_SEND_STR(p, "GET / HTTP/1.0\r\n");
  PSOCK_SEND_STR(p, "Server: Contiki example protosocket client\r\n");
  PSOCK_SEND_STR(p, "\r\n");

  while(1) {
    PSOCK_READTO(p, '\n');
    printf("Got: %s", buffer);
  }
  
  PSOCK_END(p);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_psock_client_process, ev, data)
{
  uip_ipaddr_t addr;

  printf("%d\n", TEST);
  
  PROCESS_BEGIN();

  uip_ipaddr(addr, 192,168,2,1);
  tcp_connect(addr, UIP_HTONS(80), NULL);

  printf("Connecting...\n");
  PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

  if(uip_aborted() || uip_timedout() || uip_closed()) {
    printf("Could not establish connection\n");
  } else if(uip_connected()) {
    printf("Connected\n");
    
    PSOCK_INIT(&ps, buffer, sizeof(buffer));

    do {
      handle_connection(&ps);
      PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    } while(!(uip_closed() || uip_aborted() || uip_timedout()));

    printf("\nConnection closed.\n");
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

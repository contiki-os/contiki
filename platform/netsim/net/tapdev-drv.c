
#include "net/tapdev.h"
#include "net/uip_arp.h"

static struct uip_eth_addr addr =
  {{0x08, 0x12, 0x23, 0x89, 0xa3, 0x94}};
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

PROCESS(tapdev_drv_process, "TAP driver");

void tapdev_drv_request_poll(void);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tapdev_drv_process, ev, data)
     
{
  PROCESS_BEGIN();

  tapdev_init();
  uip_setethaddr(addr);
  tapdev_drv_request_poll();

  while(1) {
    PROCESS_WAIT_EVENT();
    tapdev_drv_request_poll();
    
    tapdev_drv_request_poll();
    uip_len = tapdev_poll();
    
    if(uip_len > 0) {
      if(BUF->type == htons(UIP_ETHTYPE_IP)) {
	uip_arp_ipin();
	uip_len -= sizeof(struct uip_eth_hdr);
	/*    uip_input();*/
	tcpip_input();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
      } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
	uip_arp_arpin();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */	
	if(uip_len > 0) {
	  tapdev_send_raw();
	}
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
tapdev_drv_request_poll(void)
{
  process_poll(&tapdev_drv_process);
}

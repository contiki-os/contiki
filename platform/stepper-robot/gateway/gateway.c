#include <stdio.h>
#include <sys/etimer.h>
#include <sys/process.h>
#include <sys/autostart.h>
#include <usb/usb-api.h>
#include <usb/cdc-acm.h>
#include <dev/leds.h>
#include <debug-uart.h>

#include <net/uip-fw-drv.h>
#include <net/uip-over-mesh.h>
#include <dev/slip.h>

#include "contiki-main.h"

/* SLIP interface */

extern struct uip_fw_netif cc2420if;

rimeaddr_t node_addr = {{0,129}};

static struct uip_fw_netif slipif =
{UIP_FW_NETIF(0,0,0,0, 255,255,255,255, slip_send)};

/* USB buffers */
static unsigned char input_buffer[128];
static unsigned char output_buffer[128];
static unsigned char interrupt_buffer[16];

#define DEV_TO_HOST 0x81
#define HOST_TO_DEV 0x02

#define GATEWAY_TRICKLE_CHANNEL 8
void
slip_arch_init(unsigned long ubr)
{
}

void
slip_arch_writeb(unsigned char c)
{
  while(usb_send_data(DEV_TO_HOST, &c, 1) != 1);
}

#if WITH_UIP

static void
set_gateway(void)
{
  struct gateway_msg msg;
  /* Make this node the gateway node, unless it already is the
     gateway. */
  if(!is_gateway) {
    leds_on(LEDS_RED);
    printf("%d.%d: making myself the gateway\n",
	   rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
    uip_over_mesh_set_gateway(&rimeaddr_node_addr);
    rimeaddr_copy(&(msg.gateway), &rimeaddr_node_addr);
    rimebuf_copyfrom(&msg, sizeof(struct gateway_msg));
    trickle_send(&gateway_trickle);
    is_gateway = 1;
  }
}
#endif /* WITH_UIP */

PROCESS(gateway_process, "Gateway process");


PROCESS_THREAD(gateway_process, ev , data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
  usb_set_user_process(process_current);
  usb_setup();
  usb_cdc_acm_setup();

  uip_fw_default(&slipif);
  uip_over_mesh_set_gateway_netif(&slipif);

  process_start(&slip_process, NULL);

  set_gateway();
  
  while(ev != PROCESS_EVENT_EXIT) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
      leds_toggle(LEDS_YELLOW);
      /* printf("FIFOP: %d\n", FIFOP_IS_1); */
      etimer_restart(&timer);
    } else if (ev == PROCESS_EVENT_MSG) {
      const struct usb_user_msg * const msg = data;
      switch(msg->type) {
      case USB_USER_MSG_TYPE_CONFIG:
	printf("User config\n");
	if (msg->data.config != 0) {
	  usb_setup_bulk_endpoint(DEV_TO_HOST,
				  input_buffer, sizeof(input_buffer));
	  usb_setup_bulk_endpoint(HOST_TO_DEV,
				  output_buffer, sizeof(output_buffer));
	  usb_setup_interrupt_endpoint(0x83,interrupt_buffer,
				       sizeof(interrupt_buffer));
	  etimer_set(&timer, CLOCK_SECOND);
	} else {
	  etimer_stop(&timer);
	  usb_disable_endpoint(DEV_TO_HOST);
	  usb_disable_endpoint(HOST_TO_DEV);
	  usb_disable_endpoint(0x83);

	}
	break;
      case USB_USER_MSG_TYPE_EP_OUT(2):
	{
	  /*unsigned int len = msg->data.length; 
	    printf("Received %d:\n", len);  */
	  {
	    unsigned char ch;
	    unsigned int xfer;
	    while((xfer = usb_recv_data(HOST_TO_DEV, &ch, 1)) > 0) {
	      /* printf(" %02x",ch); */
	      if (slip_input_byte(ch)) break;
	    }
	    /* printf("\n"); */
	  }
	}
	break;
      }
      
    }
  }
  printf("USB test process exited\n");
  PROCESS_END();
}

AUTOSTART_PROCESSES(&gateway_process);

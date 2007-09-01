#include <stdio.h>
#include <sys/etimer.h>
#include <sys/process.h>
#include <sys/autostart.h>
#include <usb/usb-api.h>
#include <usb/cdc-acm.h>
#include <dev/leds.h>
#include <debug-uart.h>

#include <net/uip-fw-drv.h>
#include <dev/slip.h>

/* SLIP interface */

extern struct uip_fw_netif cc2420if;

static struct uip_fw_netif slipif =
{UIP_FW_NETIF(0,0,0,0, 255,255,255,255, slip_send)};

/* USB buffers */
static unsigned char input_buffer[128];
static unsigned char output_buffer[128];
static unsigned char interrupt_buffer[16];

#define DEV_TO_HOST 0x81
#define HOST_TO_DEV 0x02

void
slip_arch_init(unsigned long ubr)
{
}

void
slip_arch_writeb(unsigned char c)
{
  while(usb_send_data(DEV_TO_HOST, &c, 1) != 1);
}


PROCESS(gateway_process, "Gateway process");


PROCESS_THREAD(gateway_process, ev , data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
  usb_set_user_process(process_current);
  usb_setup();
  usb_cdc_acm_setup();

  uip_fw_default(&slipif);
  uip_fw_register(&cc2420if);

  process_start(&slip_process, NULL);
  
  while(ev != PROCESS_EVENT_EXIT) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_TIMER) {
      leds_toggle(LEDS_YELLOW);
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
	  unsigned int len = msg->data.length; 
	  /* printf("Received %d:\n", len);  */
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

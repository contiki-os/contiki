#include <cdc-eth.h>
#include <usb-api.h>
#include <uip_arp.h>
#include <stdio.h>
#include <string.h>
#include <net/uip-fw.h>

#define DATA_IN 0x81
#define DATA_OUT 0x02
#define INTERRUPT_IN 0x83


struct uip_eth_addr default_uip_ethaddr = {{0x02,0x00,0x00,0x00,0x00,0x02}};
 
static unsigned int
handle_cdc_eth_requests()
{
  return 0;
}

static const struct USBRequestHandler cdc_eth_request_handler =
  {
    0x21, 0x7f,
    0x00, 0x00,
    handle_cdc_eth_requests
  };

static struct USBRequestHandlerHook cdc_eth_request_hook =
  {
    NULL,
    &cdc_eth_request_handler
  };

static USBBuffer recv_buffer;
static uint8_t recv_data[UIP_BUFSIZE];

static USBBuffer xmit_buffer[3];
static uint8_t xmit_data[UIP_BUFSIZE];

static void
init_recv_buffer()
{
  recv_buffer.next = NULL;
  recv_buffer.data = recv_data;
  recv_buffer.left = UIP_BUFSIZE;
  recv_buffer.flags = USB_BUFFER_SHORT_END | USB_BUFFER_NOTIFY;
}

uint8_t
usbeth_send(void)
{
  if ((xmit_buffer[0].flags & USB_BUFFER_SUBMITTED)) return UIP_FW_DROPPED;
  uip_arp_out();
  memcpy(xmit_data, uip_buf, uip_len);
  xmit_buffer[0].next = NULL;
  xmit_buffer[0].left = uip_len;
  xmit_buffer[0].flags = USB_BUFFER_NOTIFY | USB_BUFFER_SHORT_END;
  xmit_buffer[0].data = xmit_data;
  
  /* printf("usbeth_send: %d\n", uip_len);  */
  usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
  return UIP_FW_OK;
}

static struct uip_fw_netif usbethif =
  {UIP_FW_NETIF(172,16,0,1, 255,255,255,255, usbeth_send)};

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

PROCESS(usb_eth_process, "USB ethernet");

PROCESS_THREAD(usb_eth_process, ev , data)
{
  PROCESS_BEGIN();
  usb_register_request_handler(&cdc_eth_request_hook);
  usb_setup();
  usb_set_ep_event_process(DATA_OUT, process_current);
  usb_set_global_event_process(process_current);
  uip_fw_default(&usbethif);
  uip_setethaddr(default_uip_ethaddr);
  uip_arp_init();
  
  while(1) {
    PROCESS_WAIT_EVENT();
    if (ev == PROCESS_EVENT_EXIT) break;
    if (ev == PROCESS_EVENT_POLL) {
      unsigned int events = usb_get_global_events();
      if (events) {
	if (events & USB_EVENT_CONFIG) {
	  if (usb_get_current_configuration() != 0) {
	    printf("Configured\n");
	    usb_setup_bulk_endpoint(DATA_IN);
	    usb_setup_bulk_endpoint(DATA_OUT);
	    usb_setup_interrupt_endpoint(INTERRUPT_IN);
	    init_recv_buffer();
	    usb_submit_recv_buffer(DATA_OUT, &recv_buffer);
#if 0
	    {
	      static const uint8_t foo[4] = {0x12,0x34,0x56,0x78};
	      xmit_buffer[0].next = NULL;
	      xmit_buffer[0].left = sizeof(foo);
	      xmit_buffer[0].flags = USB_BUFFER_SHORT_END;
	      xmit_buffer[0].data = &foo;
	      
	      usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
	    }
#endif
	  } else {
	    usb_disable_endpoint(DATA_IN);
	    usb_disable_endpoint(DATA_OUT);
	    usb_disable_endpoint(INTERRUPT_IN);
	  }
	}
      }
      events = usb_get_ep_events(DATA_OUT);
      if (events & USB_EP_EVENT_NOTIFICATION) {
	uip_len = sizeof(recv_data) - recv_buffer.left;
	/* printf("Received: %d bytes\n", uip_len);  */
	memcpy(uip_buf, recv_data, uip_len);
#if UIP_CONF_IPV6
	if(BUF->type == uip_htons(UIP_ETHTYPE_IPV6)) {
	  uip_neighbor_add(&IPBUF->srcipaddr, &BUF->src);
	    tcpip_input();
	} else 
#endif /* UIP_CONF_IPV6 */
	  if(BUF->type == uip_htons(UIP_ETHTYPE_IP)) {
	    uip_len -= sizeof(struct uip_eth_hdr);
	    tcpip_input();
	  } else if(BUF->type == uip_htons(UIP_ETHTYPE_ARP)) {
	    uip_arp_arpin();
	    /* If the above function invocation resulted in data that
	       should be sent out on the network, the global variable
	       uip_len is set to a value > 0. */
	    if (uip_len > 0) {
	      memcpy(xmit_data, uip_buf, uip_len);
	      xmit_buffer[0].next = NULL;
	      xmit_buffer[0].data = xmit_data;
	      xmit_buffer[0].left = uip_len;
	      xmit_buffer[0].flags = USB_BUFFER_SHORT_END;
	      
	      usb_submit_xmit_buffer(DATA_IN, &xmit_buffer[0]);
	      /* printf("Sent: %d bytes\n", uip_len); */
	    }
	  }
	
	init_recv_buffer();
	usb_submit_recv_buffer(DATA_OUT, &recv_buffer);
      }
    }
  }
  PROCESS_END();
}

void
usb_cdc_eth_setup()
{
  process_start(&usb_eth_process, NULL);
}

void
usb_cdc_eth_set_ifaddr(uip_ipaddr_t *addr)
{
  usbethif.ipaddr = *addr;
}
  
void
dummy(uip_ipaddr_t *addr1,  uip_ipaddr_t *addr2)
{
  *addr1 = *addr2;
}

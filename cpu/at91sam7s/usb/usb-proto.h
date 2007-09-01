#ifndef __USB_PROTO_H__RVJQ2JAGM4__
#define __USB_PROTO_H__RVJQ2JAGM4__

#include <usb.h>
#include <usb-config.h>

#ifndef NULL
#define NULL 0
#endif

typedef struct _USBEndpoint USBEndpoint;
struct _USBEndpoint
{
  unsigned char addr;
  unsigned char buf_size_mask; /* mask for the buffer index.  This implies
			     that the buffer size must be a power of
			     2 */
  unsigned char buf_len;
  unsigned char buf_pos;
  unsigned char* buffer;
  unsigned short status;
  unsigned short flags;
};

#define USB_EP_FLAGS_BANK_1_RECV_NEXT 0x01 /* The next packet received
					      should be read from bank 1
					      if possible */
					    
#define USB_EP_FLAGS_RECV_BLOCKED 0x02	/* Recetpion is blocked.
					   Interrupt turned off */

#define USB_EP_FLAGS_TRANSMITTING 0x04 /* Waiting for TXCOMP before sending
					  more data */

/* Read only */
struct USBRequestHandler
{
  unsigned char request_type;
  unsigned char request_type_mask;
  unsigned char request;
  unsigned char request_mask;
  /* Returns true if it handled the request, if false let another handler try*/
  unsigned int (*handler_func)();
};
  
/* Must be writeable */
struct USBRequestHandlerHook
{
  struct USBRequestHandlerHook *next;
  const struct USBRequestHandler * const handler;
};

void
usb_register_request_handler(struct USBRequestHandlerHook *hook);

void
usb_reset();

void
usb_ep0_int();

void
usb_epx_int();

void
usb_clear_events(unsigned events);

void
usb_clear_ep_events(unsigned int ep, unsigned int events);

void
usb_error_stall();

void
usb_send_ctrl_response(const unsigned char *buffer, unsigned short len);

void 
usb_send_ctrl_status();

void
usb_set_address();


void
usb_set_configuration_value(unsigned char c);

unsigned char
usb_get_configuration_value();

void
usb_init_endpoints();

volatile USBEndpoint*
usb_find_endpoint(unsigned char epaddr);

void
usb_halt_endpoint(unsigned char ep_addr, unsigned int halt);

extern volatile unsigned char usb_events;
extern struct process *usb_handler_process;
extern struct USB_request_st usb_setup_buffer;

extern unsigned char usb_ctrl_data_buffer[];
extern unsigned short usb_ctrl_data_len;

#define USB_EVENT_EP(ep) (0x01<<(ep))
#define USB_EVENT_RESET 0x10
#define USB_EVENT_SUPEND 0x20
#define USB_EVENT_RESUME 0x40
#define USB_EVENT_CONFIG 0x08

extern  volatile unsigned char usb_endpoint_events[];

#define USB_EP_EVENT_SETUP 0x01
#define USB_EP_EVENT_OUT 0x02
#define USB_EP_EVENT_IN 0x04

#endif /* __USB_PROTO_H__RVJQ2JAGM4__ */

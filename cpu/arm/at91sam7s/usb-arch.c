#include <usb-arch.h>
#include <usb-interrupt.h>
#include <AT91SAM7S64.h>
#include <stdio.h>
#include <debug-uart.h>


/* #define DEBUG   */
#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define USB_PULLUP_PIN AT91C_PIO_PA16

#ifndef AT91C_UDP_STALLSENT
#define AT91C_UDP_STALLSENT AT91C_UDP_ISOERROR
#endif

/* Bits that won't effect the state if they're written at a specific level.
 */
/* Bits that should be written as 1 */
#define NO_EFFECT_BITS (AT91C_UDP_TXCOMP | AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RXSETUP \
			| AT91C_UDP_ISOERROR | AT91C_UDP_RX_DATA_BK1)
/* Also includes bits that should be written as 0 */
#define NO_EFFECT_MASK (NO_EFFECT_BITS | AT91C_UDP_TXPKTRDY)

#define RXBYTECNT(s) (((s)>>16)&0x7ff)


static inline void
udp_set_ep_ctrl_flags(AT91_REG *reg, unsigned int flags,
		      unsigned int write_mask, unsigned int check_mask)
{
  while ( (*reg & check_mask) != (flags & check_mask)) {
    *reg = (*reg & ~write_mask) | flags; 
  }
}

#define UDP_SET_EP_CTRL_FLAGS(reg, flags, mask) \
udp_set_ep_ctrl_flags((reg), \
  (NO_EFFECT_BITS & ~(mask)) | ((flags) & (mask)), (mask) | NO_EFFECT_MASK,\
  (mask))


#define USB_DISABLE_INT *AT91C_AIC_IDCR = (1 << AT91C_ID_UDP)
#define USB_ENABLE_INT *AT91C_AIC_IECR = (1 << AT91C_ID_UDP)

#define USB_DISABLE_EP_INT(hw_ep) *AT91C_UDP_IDR = (1 << (hw_ep))
#define USB_ENABLE_EP_INT(hw_ep) *AT91C_UDP_IER = (1 << (hw_ep))

#if CTRL_EP_SIZE > 8
#error Control endpoint size too big
#endif

#if USB_EP1_SIZE > 64
#error Endpoint 1 size too big
#endif

#if USB_EP2_SIZE > 64
#error Endpoint 2 size too big
#endif

#if USB_EP3_SIZE > 64
#error Endpoint 3 size too big
#endif

static const uint16_t ep_xfer_size[8] =
  {
    CTRL_EP_SIZE,
    USB_EP1_SIZE,
    USB_EP2_SIZE,
    USB_EP3_SIZE
  };

#define USB_EP_XFER_SIZE(ep) ep_xfer_size[ep]

typedef struct _USBEndpoint USBEndpoint;
struct _USBEndpoint
{
  uint16_t status;
  uint8_t addr;
  uint8_t flags;
  USBBuffer *buffer;	/* NULL if no current buffer */
  struct process *event_process;
  unsigned int events;
  uint16_t xfer_size;
};

#define USB_EP_FLAGS_TYPE_MASK 0x03
#define USB_EP_FLAGS_TYPE_BULK 0x00
#define USB_EP_FLAGS_TYPE_CONTROL 0x01
#define USB_EP_FLAGS_TYPE_ISO 0x02
#define USB_EP_FLAGS_TYPE_INTERRUPT 0x03

#define EP_TYPE(ep) ((ep)->flags & USB_EP_FLAGS_TYPE_MASK)
#define IS_EP_TYPE(ep, type) (EP_TYPE(ep) == (type))
#define IS_CONTROL_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_CONTROL)
#define IS_BULK_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_BULK)

#define USB_EP_FLAGS_ENABLED 0x04

/* A packet has been received but the data is still in hardware buffer */
#define USB_EP_FLAGS_RECV_PENDING 0x08
/* The pending packet is a SETUP packet */
#define USB_EP_FLAGS_SETUP_PENDING 0x10

/* The data in the hardware buffer is being transmitted */
#define USB_EP_FLAGS_TRANSMITTING 0x20

/* The  receiver is waiting for a packet */
#define USB_EP_FLAGS_RECEIVING 0x40

/* For bulk endpoints. Both buffers are busy are in use, either by
   hardware or software. */
#define USB_EP_FLAGS_DOUBLE 0x80

/* The next packet received should be read from bank 1 if possible */
#define USB_EP_FLAGS_BANK_1_RECV_NEXT 0x10

/* States for double buffered reception:

Packets being received	0	1	2	1	0	0
Packets pending		0	0	0	1	2	1

RECVING			0	1	1	1	0	0
RECV_PENDING		0	0	0	1	1	1
DOUBLE			0	0	1	0	1	0
*/

/* States for double buffered transmission:
   
Packets being transmitted	0	1	2

TRANSMITTING			0	1	1
DOUBLE				0	0	1
*/

/* Index in endpoint array */
#define EP_INDEX(addr) ((addr) & 0x7f)

/* Get address of endpoint struct */
#define EP_STRUCT(addr) &usb_endpoints[EP_INDEX(addr)];

/* Number of hardware endpoint */
#define EP_HW_NUM(addr) ((addr) & 0x7f)


static USBEndpoint usb_endpoints[USB_MAX_ENDPOINTS];
struct process *event_process = 0;
volatile unsigned int events = 0;

static void
notify_process(unsigned int e)
{
  events |= e;
  if (event_process) {
    process_poll(event_process);
  }
}

static void
notify_ep_process(USBEndpoint *ep, unsigned int e)
{
  ep->events |= e;
  if (ep->event_process) {
    process_poll(ep->event_process);
  }
}


static void
usb_arch_reset(void)
{
  unsigned int e;
  for (e = 0; e < USB_MAX_ENDPOINTS; e++) {
    if (usb_endpoints[e].flags &USB_EP_FLAGS_ENABLED) {
      USBBuffer *buffer = usb_endpoints[e].buffer;
      usb_endpoints[e].flags = 0;
      usb_disable_endpoint(e);
      while(buffer) {
	buffer->flags &= ~USB_BUFFER_SUBMITTED;
	buffer = buffer->next;
      }
    }
  }
  usb_arch_setup_control_endpoint(0);  

}

void
usb_arch_setup(void)
{
  unsigned int i;
  /* Assume 96MHz PLL frequency */
  *AT91C_CKGR_PLLR = ((*AT91C_CKGR_PLLR & ~AT91C_CKGR_USBDIV)
			  | AT91C_CKGR_USBDIV_1);
  /* Enable 48MHz USB clock */
  *AT91C_PMC_SCER = AT91C_PMC_UDP;
  /* Enable USB main clock */
  *AT91C_PMC_PCER = (1 << AT91C_ID_UDP);
  
  /* Enable pullup */
  *AT91C_PIOA_PER = USB_PULLUP_PIN;
  *AT91C_PIOA_OER = USB_PULLUP_PIN;
  *AT91C_PIOA_CODR = USB_PULLUP_PIN;

  for(i = 0; i < USB_MAX_ENDPOINTS; i++) {
    usb_endpoints[i].flags = 0;
    usb_endpoints[i].event_process = 0;
  }
  
  usb_arch_reset();
  /* Enable usb_interrupt */
  AT91C_AIC_SMR[AT91C_ID_UDP] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | 4;
  AT91C_AIC_SVR[AT91C_ID_UDP] = (unsigned long) usb_int;
  *AT91C_AIC_IECR = (1 << AT91C_ID_UDP);
}


static void
usb_arch_setup_endpoint(unsigned char addr, unsigned int hw_type)
{
  unsigned int ei = EP_HW_NUM(addr);
  USBEndpoint *ep = EP_STRUCT(addr);
  ep->status = 0;
  ep->flags = USB_EP_FLAGS_ENABLED;
  ep->buffer = 0;
  ep->addr = addr;
  ep->events = 0;
  ep->xfer_size = 0;

  *AT91C_UDP_IDR = 1<<ei;

  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[ei], hw_type | AT91C_UDP_EPEDS,
			AT91C_UDP_EPTYPE | AT91C_UDP_EPEDS);
    
  *AT91C_UDP_IER = 1<<ei;
};

void
usb_arch_setup_control_endpoint(unsigned char addr)
{
  unsigned int ei = EP_HW_NUM(addr);
  USBEndpoint *ep = EP_STRUCT(addr);
  usb_arch_setup_endpoint(addr, AT91C_UDP_EPTYPE_CTRL);
  ep->flags |= USB_EP_FLAGS_TYPE_CONTROL;
  ep->xfer_size = ep_xfer_size[ei];

}

void
usb_arch_setup_bulk_endpoint(unsigned char addr)
{
  unsigned int ei = EP_HW_NUM(addr);
  USBEndpoint *ep = EP_STRUCT(addr);
  usb_arch_setup_endpoint(addr, ((addr & 0x80)
				 ? AT91C_UDP_EPTYPE_BULK_IN
				 : AT91C_UDP_EPTYPE_BULK_OUT));
  ep->flags |= USB_EP_FLAGS_TYPE_BULK;
  ep->xfer_size = ep_xfer_size[ei];
}

void
usb_arch_setup_interrupt_endpoint(unsigned char addr)
{
  unsigned int ei = EP_HW_NUM(addr);
  USBEndpoint *ep = EP_STRUCT(addr);
  usb_arch_setup_endpoint(addr, ((addr & 0x80)
				 ? AT91C_UDP_EPTYPE_INT_IN
				 : AT91C_UDP_EPTYPE_INT_OUT));
  ep->flags |= USB_EP_FLAGS_TYPE_BULK;
  ep->xfer_size = ep_xfer_size[ei];
}

void
usb_arch_disable_endpoint(uint8_t addr)
{
  USBEndpoint *ep = EP_STRUCT(addr);
  ep->flags &= ~USB_EP_FLAGS_ENABLED;
  
  *AT91C_UDP_IDR = 1<<EP_HW_NUM(addr);
  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[EP_HW_NUM(addr)], 0, AT91C_UDP_EPEDS);
}
     

#define USB_READ_BLOCK 0x01	/* The currently submitted buffers
				   can't hold the received data, wait
				   for more buffers. No data was read
				   from the hardware buffer */
#define USB_READ_NOTIFY 0x02	/* Some buffers that had the
				   USB_BUFFER_NOTIFY flags set were
				   released */
#define USB_READ_FAIL 0x04	/* The received data doesn't match the
				   submitted buffers. The hardware
				   buffer is discarded. */

/* Skip buffers until mask and flags matches*/
static USBBuffer *
skip_buffers_until(USBBuffer *buffer, unsigned int mask, unsigned int flags,
		   unsigned int *resp)
{
  while(buffer && !((buffer->flags & mask) == flags)) {
    USBBuffer *next = buffer->next;
    buffer->flags &= ~USB_BUFFER_SUBMITTED ;
    buffer->flags |= USB_BUFFER_FAILED;
    if (buffer->flags & USB_BUFFER_NOTIFY) *resp |= USB_READ_NOTIFY;
    buffer = next;
  }
  return buffer;
}

static void
read_hw_buffer(uint8_t *data, unsigned int hw_ep, unsigned int len)
{
  AT91_REG *fdr;
  fdr = &AT91C_UDP_FDR[hw_ep];
  while(len-- > 0) {
    *data++ = *fdr;
  }
}


#define USB_WRITE_BLOCK 0x01
#define USB_WRITE_NOTIFY 0x02

void
write_hw_buffer(const uint8_t *data, unsigned int hw_ep, unsigned int len)
{
  AT91_REG *fdr;
  fdr = &AT91C_UDP_FDR[hw_ep];
  /* PRINTF("Write %d\n", len); */
  while(len-- > 0) {
    *fdr = *data++;
  }
}

static unsigned int
get_receive_capacity(USBBuffer *buffer)
{
  unsigned int capacity = 0;
  while(buffer && !(buffer->flags & (USB_BUFFER_IN| USB_BUFFER_SETUP|USB_BUFFER_HALT))) {
    capacity += buffer->left;
    buffer = buffer->next;
  }
  return capacity;
}

static int
handle_pending_receive(USBEndpoint *ep)
{
  int short_packet;
  unsigned int len;
  unsigned int copy;
  unsigned int res = 0;
  unsigned int hw_ep = EP_HW_NUM(ep->addr);
  USBBuffer *buffer = ep->buffer;
  uint8_t *setup_data = NULL;
  unsigned int flags = ep->flags;
  if (!(flags & USB_EP_FLAGS_ENABLED) || !buffer) return USB_READ_BLOCK;
  len = RXBYTECNT(AT91C_UDP_CSR[hw_ep]);
  PRINTF("handle_pending_receive: %d\n", len);
  switch(flags & USB_EP_FLAGS_TYPE_MASK) {
  case USB_EP_FLAGS_TYPE_CONTROL:
    if (flags & USB_EP_FLAGS_SETUP_PENDING) {
      /* Discard buffers until we find a SETUP buffer */
      buffer =
	skip_buffers_until(buffer, USB_BUFFER_SETUP, USB_BUFFER_SETUP, &res);
      ep->buffer = buffer;
      if (!buffer || buffer->left < len) {
	res |= USB_READ_BLOCK;
	return res;
      }
      /* SETUP packet must fit in a single buffer */
      if (buffer->left < len) {
	buffer->flags |= USB_BUFFER_FAILED;
	buffer->flags &= ~USB_BUFFER_SUBMITTED ;
	if (buffer->flags & USB_BUFFER_NOTIFY) res |= USB_READ_NOTIFY;
	ep->buffer = buffer->next;
	res |= USB_READ_FAIL;
	return res;
      }
      setup_data = buffer->data;
    } else {
      if (buffer->flags & (USB_BUFFER_SETUP|USB_BUFFER_IN)) {
	buffer->flags |= USB_BUFFER_FAILED;
	
	buffer->flags &= ~USB_BUFFER_SUBMITTED ;
	if (buffer->flags & USB_BUFFER_NOTIFY) res |= USB_READ_NOTIFY;
	ep->buffer = buffer->next;
	res |= USB_READ_FAIL;
	return res;
      }

      if (len == 0) {
	/* Status OUT */
	if (buffer->left > 0) {
	  buffer->flags |= USB_BUFFER_FAILED;
	  res |= USB_READ_FAIL;
	}
	buffer->flags &= ~USB_BUFFER_SUBMITTED ;
	if (buffer->flags & USB_BUFFER_NOTIFY) res |= USB_READ_NOTIFY;
	ep->buffer = buffer->next;
	return res;
      }
      if (get_receive_capacity(buffer) <  len) return USB_READ_BLOCK;
    }
    break;
  case USB_EP_FLAGS_TYPE_INTERRUPT:
  case USB_EP_FLAGS_TYPE_BULK:
  case USB_EP_FLAGS_TYPE_ISO:
    if (get_receive_capacity(buffer) <  len) {
      return USB_READ_BLOCK;
    }
    break;
  }

  short_packet = len < ep->xfer_size;

  do {
    if (buffer->left < len) {
      copy = buffer->left;
    } else {
      copy = len;
    }
    len -= copy;
    buffer->left -= copy;
    read_hw_buffer(buffer->data, hw_ep, copy);
    buffer->data += copy;

    if (len == 0) break;

    /* Release buffer */
    buffer->flags &= ~(USB_BUFFER_SUBMITTED | USB_BUFFER_SHORT_PACKET);
    if (buffer->flags & USB_BUFFER_NOTIFY) res |= USB_READ_NOTIFY;
    /* Use next buffer. */
    buffer = buffer->next;
  } while(1);
  
  if (short_packet) {
    buffer->flags |= USB_BUFFER_SHORT_PACKET;
  }
  
  if ((buffer->left == 0)
      || (buffer->flags & USB_BUFFER_PACKET_END)
      || (short_packet && (buffer->flags & USB_BUFFER_SHORT_END))) {
    /* Release buffer */
    buffer->flags &= ~USB_BUFFER_SUBMITTED;
    if (buffer->flags & USB_BUFFER_NOTIFY) res |= USB_READ_NOTIFY;
    /* Use next buffer. */
    buffer = buffer->next;
  }
  
  ep->buffer = buffer;
  if (setup_data) {
    /* Set direction according to request */
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],
			  ((setup_data[0] & 0x80)
			   ? AT91C_UDP_DIR : 0), AT91C_UDP_DIR);
  }
  return res;
}


static void
start_receive(USBEndpoint *ep)
{
  ep->flags |= USB_EP_FLAGS_RECEIVING;
}

#if 0
static unsigned int
get_transmit_length(USBBuffer *buffer)
{
  unsigned int length = 0;
  while(buffer && (buffer->flags & USB_BUFFER_IN)) {
    length += buffer->left;
    buffer = buffer->next;
  }
  return length;
}
#endif

static int
start_transmit(USBEndpoint *ep)
{
  unsigned int res = 0;
  USBBuffer *buffer = ep->buffer;
  unsigned int len;
  unsigned int hw_ep = EP_HW_NUM(ep->addr);
  unsigned int ep_flags = ep->flags;
  len = ep->xfer_size;
  if (!(ep_flags & USB_EP_FLAGS_ENABLED) || !buffer) return USB_WRITE_BLOCK;
  switch(ep_flags & USB_EP_FLAGS_TYPE_MASK) {
  case USB_EP_FLAGS_TYPE_BULK:
    if (buffer->flags & USB_BUFFER_HALT) {
      if (ep->status & 0x01) return USB_WRITE_BLOCK;
      ep->status |= 0x01;
      if (!(ep->flags & USB_EP_FLAGS_TRANSMITTING)) {
	UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],
			      AT91C_UDP_FORCESTALL, AT91C_UDP_FORCESTALL);
	PRINTF("HALT IN\n");
      }
      return USB_WRITE_BLOCK;
    }
  case USB_EP_FLAGS_TYPE_ISO:
    if (!(ep->flags & USB_EP_FLAGS_TRANSMITTING)) {
      if (AT91C_UDP_CSR[hw_ep] & AT91C_UDP_TXPKTRDY) return USB_WRITE_BLOCK;
    }
    break;
  default:
    if (AT91C_UDP_CSR[hw_ep] & AT91C_UDP_TXPKTRDY) return USB_WRITE_BLOCK;
  }
  

  while (buffer) {
    unsigned int copy;
    if (buffer->left < len) {
      copy = buffer->left;
    } else {
      copy = len;
    }
    len -= copy;
    buffer->left -= copy;
    write_hw_buffer(buffer->data, hw_ep, copy);
    buffer->data += copy;
    if (buffer->left == 0) {
      if (buffer->flags & USB_BUFFER_SHORT_END) {
	if (len == 0) {
	  /* Send zero length packet. */
	  break;
	} else {
	  len = 0;
	}
      }
      /* Release buffer */
      buffer->flags &= ~USB_BUFFER_SUBMITTED;
      if (buffer->flags & USB_BUFFER_NOTIFY) res = USB_WRITE_NOTIFY;
	   /* Use next buffer. */
      buffer = buffer->next;
    }
    if (len == 0) break;
  }
  ep->buffer = buffer;
  if (ep->flags & USB_EP_FLAGS_TRANSMITTING) {
    ep->flags |= USB_EP_FLAGS_DOUBLE;
  } else {
    ep->flags |= USB_EP_FLAGS_TRANSMITTING;
  }
 
  PRINTF("start_transmit: sent %08x\n",AT91C_UDP_CSR[hw_ep]);
  /* Start transmission */
  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],
			AT91C_UDP_TXPKTRDY, AT91C_UDP_TXPKTRDY);

  return res;
}

static void
start_transfer(USBEndpoint *ep)
{
  unsigned int hw_ep = EP_HW_NUM(ep->addr);
  int res;
  while (1) {
    if (!(ep->addr & 0x80)) {
      if (ep->buffer && (ep->buffer->flags & USB_BUFFER_HALT)) {
	if (ep->status & 0x01) return ;
	ep->status |= 0x01;
	UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[EP_HW_NUM(ep->addr)],
			      AT91C_UDP_FORCESTALL, AT91C_UDP_FORCESTALL);
	PRINTF("HALT OUT\n");
	*AT91C_UDP_IDR = 1<<hw_ep;
	return;
      }
    }
    if (!(ep->flags & USB_EP_FLAGS_RECV_PENDING)) break;
    res = handle_pending_receive(ep);
    if (res & USB_READ_NOTIFY) {
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
    }
    PRINTF("received res = %d\n", res);
    if (res & USB_READ_BLOCK) {
      *AT91C_UDP_IDR = 1<<hw_ep;
      return;
    }
    if (AT91C_UDP_CSR[hw_ep] & AT91C_UDP_RXSETUP) {
      /* Acknowledge SETUP */
      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],0, AT91C_UDP_RXSETUP);
    } else if (AT91C_UDP_CSR[hw_ep] & (AT91C_UDP_RX_DATA_BK1)) {
       /* Ping-pong */
      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],0,
			    (ep->flags & USB_EP_FLAGS_BANK_1_RECV_NEXT)
			    ? AT91C_UDP_RX_DATA_BK1
			    : AT91C_UDP_RX_DATA_BK0);
      ep->flags ^= USB_EP_FLAGS_BANK_1_RECV_NEXT;
    } else {
      /* Ping-pong or single buffer */
      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],0,
			    AT91C_UDP_RX_DATA_BK0);
      ep->flags |= USB_EP_FLAGS_BANK_1_RECV_NEXT;
    }

    if (ep->flags & USB_EP_FLAGS_DOUBLE) {
      ep->flags &= ~USB_EP_FLAGS_DOUBLE;
    } else if IS_CONTROL_EP(ep) {
      ep->flags &= ~(USB_EP_FLAGS_RECV_PENDING|USB_EP_FLAGS_SETUP_PENDING);
    } else {
      ep->flags &= ~USB_EP_FLAGS_RECV_PENDING;
    }
    if (res & USB_READ_FAIL) {
      /* Only fails for control endpoints */
      usb_arch_control_stall(ep->addr);
      return;
    }
    *AT91C_UDP_IER = 1<<hw_ep;
  }
  if (ep->flags & (USB_EP_FLAGS_TRANSMITTING | USB_EP_FLAGS_RECEIVING)) {
#if 0
    if (!IS_BULK_EP(ep) || (ep->flags & USB_EP_FLAGS_DOUBLE)) {
#else
      if(1) {
#endif
      PRINTF("Busy\n");
      return;
    }
  }
  if (ep->status & 0x01) return; /* Don't start transfer if halted */
  if (ep->buffer) {
    if (ep->buffer->flags & USB_BUFFER_IN) {
      res = start_transmit(ep);
      if (res & USB_WRITE_NOTIFY) {
	notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
      }
    } else {
      start_receive(ep);
    }
  }
}


void
usb_arch_transfer_complete(unsigned int hw_ep)
{
  unsigned int status = AT91C_UDP_CSR[hw_ep];
  USBEndpoint *ep = &usb_endpoints[hw_ep];
  PRINTF("transfer_complete: %d\n", hw_ep);
  if (status & AT91C_UDP_STALLSENT) {
    /* Acknowledge */
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],0, AT91C_UDP_STALLSENT);
  }
  if (status & (AT91C_UDP_RXSETUP
		| AT91C_UDP_RX_DATA_BK1 | AT91C_UDP_RX_DATA_BK0)) {
    if (status & AT91C_UDP_RXSETUP) {
      PRINTF("SETUP\n"); 
      ep->flags |= USB_EP_FLAGS_SETUP_PENDING;
    }
    if (ep->flags & USB_EP_FLAGS_DOUBLE) {
      ep->flags &= ~USB_EP_FLAGS_DOUBLE;
    } else {
      ep->flags &= ~USB_EP_FLAGS_RECEIVING;
    }
    if ( ep->flags & USB_EP_FLAGS_RECV_PENDING) {
      ep->flags |= USB_EP_FLAGS_DOUBLE;
    } else {
      ep->flags |= USB_EP_FLAGS_RECV_PENDING;
    }
    start_transfer(ep);
  }
  if (status & AT91C_UDP_TXCOMP) {
     PRINTF("Sent packet\n");
    if (ep->flags & USB_EP_FLAGS_DOUBLE) {
      ep->flags &= ~USB_EP_FLAGS_DOUBLE;
    } else {
      ep->flags &= ~USB_EP_FLAGS_TRANSMITTING;
    }
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],0, AT91C_UDP_TXCOMP);
    if (ep->status & 0x01) {
      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],
			    AT91C_UDP_FORCESTALL, AT91C_UDP_FORCESTALL);
      PRINTF("HALT IN\n");
    } else {
      start_transfer(ep);
    }
  }
  
}


void
usb_set_ep_event_process(unsigned char addr, struct process *p)
{
  USBEndpoint *ep = &usb_endpoints[EP_INDEX(addr)];
  ep->event_process = p;
}

/* Select what process should be polled when a global event occurs */
void
usb_arch_set_global_event_process(struct process *p)
{
  event_process = p;
}

unsigned int
usb_arch_get_global_events(void)
{
  unsigned int e;
  USB_DISABLE_INT;
  e = events;
  events = 0;
  USB_ENABLE_INT;
  return e;
}

unsigned int
usb_get_ep_events(unsigned char addr)
{
  unsigned int e;
  unsigned int ei = EP_HW_NUM(addr);
  USB_DISABLE_INT;
  e = usb_endpoints[ei].events;
  usb_endpoints[ei].events = 0;
  USB_ENABLE_INT;
  return e;
}


void
usb_submit_recv_buffer(unsigned char ep_addr, USBBuffer *buffer)
{
  USBBuffer **tailp;
  USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
  if (!(ep->flags & USB_EP_FLAGS_ENABLED)) return;
  /* PRINTF("buffer: %p\n", ep->buffer); */
  /* dbg_drain(); */
  USB_DISABLE_INT;
  tailp = (USBBuffer**)&ep->buffer;
  while(*tailp) {
    tailp = &(*tailp)->next;
  }
  *tailp = buffer;
  while(buffer) {
    buffer->flags |= USB_BUFFER_SUBMITTED;
    buffer = buffer->next;
  }
  start_transfer(ep);
  
  USB_ENABLE_INT;
}

void
usb_submit_xmit_buffer(unsigned char ep_addr, USBBuffer *buffer)
{
  USBBuffer **tailp;
  USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
  if (!(ep->flags & USB_EP_FLAGS_ENABLED)) return;
  /* PRINTF("usb_submit_xmit_buffer %d\n", buffer->left); */
  USB_DISABLE_INT;
  tailp = (USBBuffer**)&ep->buffer;
  while(*tailp) {
    tailp = &(*tailp)->next;
  }
  *tailp = buffer;
  while(buffer) {
    buffer->flags |= USB_BUFFER_SUBMITTED | USB_BUFFER_IN;
    buffer = buffer->next;
  }
  start_transfer(ep);
  USB_ENABLE_INT;
}

void
usb_arch_discard_all_buffers(unsigned char ep_addr)
{
  USBBuffer *buffer;
  volatile USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
  USB_DISABLE_EP_INT(EP_HW_NUM(ep_addr));
  buffer = ep->buffer;
  ep->buffer = NULL;

  USB_ENABLE_EP_INT(EP_HW_NUM(ep_addr));
  while(buffer) {
    buffer->flags &= ~USB_BUFFER_SUBMITTED;
    buffer = buffer->next;
  }
}

uint16_t
usb_arch_get_ep_status(uint8_t addr)
{
  if (EP_INDEX(addr) > USB_MAX_ENDPOINTS) return 0;
  return usb_endpoints[EP_INDEX(addr)].status;
}

void
usb_arch_set_configuration(uint8_t usb_configuration_value)
{
  /* Nothing needs to be done */
}

void
usb_arch_control_stall(unsigned char addr)
{
  if (EP_INDEX(addr) > USB_MAX_ENDPOINTS) return;
  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[EP_HW_NUM(addr)],
			AT91C_UDP_FORCESTALL, AT91C_UDP_FORCESTALL);
}

/* Not for control endpoints */
void
usb_arch_halt_endpoint(unsigned char ep_addr, int halt)
{
  if (EP_INDEX(ep_addr) > USB_MAX_ENDPOINTS) return;
  if (!usb_endpoints[EP_INDEX(ep_addr)].flags & USB_EP_FLAGS_ENABLED) return;
  *AT91C_UDP_IDR = 1<<EP_HW_NUM(ep_addr);
  if (halt) {
    usb_endpoints[EP_INDEX(ep_addr)].status |= 0x01;
    /* Delay stall if a transmission is i progress */
    if (!(usb_endpoints[EP_INDEX(ep_addr)].flags & USB_EP_FLAGS_TRANSMITTING)){
      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[EP_HW_NUM(ep_addr)],
			    AT91C_UDP_FORCESTALL, AT91C_UDP_FORCESTALL);
    }
  } else {
    USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
    ep->status &= ~0x01;
    *AT91C_UDP_IDR = 1<<EP_HW_NUM(ep_addr);
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[EP_HW_NUM(ep_addr)],
			  0, AT91C_UDP_FORCESTALL);
    *AT91C_UDP_RSTEP = 1<<EP_HW_NUM(ep_addr);
    *AT91C_UDP_RSTEP = 0;
    
    /* Release HALT buffer */
    if (ep->buffer && (ep->buffer->flags & USB_BUFFER_HALT)) {
      ep->buffer->flags &= ~USB_BUFFER_SUBMITTED;
      if (ep->buffer->flags & USB_BUFFER_NOTIFY) {
	notify_ep_process(ep,USB_EP_EVENT_NOTIFICATION);
      }
      ep->buffer = ep->buffer->next;
    }
    
    /* Restart transmission */
    start_transfer(&usb_endpoints[EP_INDEX(ep_addr)]);
  }
  *AT91C_UDP_IER = 1<<EP_HW_NUM(ep_addr);
}

int 
usb_arch_send_pending(uint8_t ep_addr)
{
  return usb_endpoints[EP_INDEX(ep_addr)].flags & USB_EP_FLAGS_TRANSMITTING;
}

void
usb_arch_set_address(unsigned char addr)
{
  *AT91C_UDP_FADDR = AT91C_UDP_FEN | addr;
  *AT91C_UDP_GLBSTATE |= AT91C_UDP_FADDEN;
}

void
usb_arch_reset_int()
{
  usb_arch_reset();
  notify_process(USB_EVENT_RESET);
}

void
usb_arch_suspend_int()
{
  notify_process(USB_EVENT_SUSPEND);
}

void
usb_arch_resume_int()
{
  notify_process(USB_EVENT_RESUME);
}


#include <AT91SAM7S64.h>
#include <usb-api.h>
#include <usb-proto.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/process.h>
#include<interrupt-utils.h>

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

/* Index in endpoint array */
#define EP_INDEX(addr) (((addr) & 0x7f) - 1)
/* Number of hardware endpoint */
#define EP_HW_NUM(addr) ((addr) & 0x7f)

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


void
usb_error_stall()
{
  /* Disable all USB events */
  *AT91C_AIC_IDCR = (1 << AT91C_ID_UDP);
  /* Set stall state */
  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],
			AT91C_UDP_FORCESTALL, AT91C_UDP_FORCESTALL);
  /* Reenable interrupt */
  *AT91C_AIC_IECR = (1 << AT91C_ID_UDP);
  puts("Stalled");
}

volatile unsigned char usb_events = 0;
struct process *usb_handler_process = NULL;

#define NUM_EP 4
volatile unsigned char usb_endpoint_events[NUM_EP] = {0,0,0,0};
static volatile USBEndpoint usb_endpoints[NUM_EP - 1];

volatile unsigned char usb_flags = 0;
#define USB_FLAG_ADDRESS_PENDING 0x01
#define USB_FLAG_RECEIVING_CTRL 0x04
#define USB_FLAG_SEND_ZLP 0x08 /* If the last packet has max length,
				  then it needs to be followed by a
				  zero length packet to mark the
				  end. */

static unsigned short usb_ctrl_send_len = 0;
static const unsigned char *usb_ctrl_send_pos = NULL;

unsigned char usb_ctrl_data_buffer[MAX_CTRL_DATA];
unsigned short usb_ctrl_data_len = 0;

static void
write_endpoint(unsigned int hw_ep,
	       const unsigned char *buffer, unsigned short len)
{
  AT91_REG *fdr = &AT91C_UDP_FDR[hw_ep];
  {
#if 0
    unsigned int i;
    printf("Sending: ");
    for (i = 0; i< len; i++) printf(" %02x", buffer[i]);
    putchar('\n');
#endif
  } 

  while(len > 0) {
    *fdr = *buffer++;
    len--;
  }
  /* Start transmission */
  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[hw_ep],
			AT91C_UDP_TXPKTRDY, AT91C_UDP_TXPKTRDY);
 
}
     
static void
write_ctrl()
{
  if (usb_ctrl_send_pos) {
    unsigned int xfer_len = usb_ctrl_send_len;
    /* Check if FIFO is ready */
    if (AT91C_UDP_CSR[0] & AT91C_UDP_TXPKTRDY) return;
    if (xfer_len > CTRL_EP_SIZE) xfer_len = CTRL_EP_SIZE;
    write_endpoint(0, usb_ctrl_send_pos, xfer_len);
    if (xfer_len < CTRL_EP_SIZE) {
      /* Last packet, stop sending */
      usb_ctrl_send_pos = NULL;
    } else {
      usb_ctrl_send_pos += xfer_len;
      usb_ctrl_send_len -= xfer_len;
      if (usb_ctrl_send_len == 0 && !(usb_flags & USB_FLAG_SEND_ZLP)) {
	usb_ctrl_send_pos = NULL;
      }
    }
  }
}

static unsigned char
read_buffered_endpoint(volatile USBEndpoint *ep)
{
  unsigned char len;
  unsigned char mask = ep->buf_size_mask;
  unsigned char *buffer = ep->buffer;
  unsigned char pos = (ep->buf_pos + ep->buf_len) & mask;
  AT91_REG *fdr = &AT91C_UDP_FDR[EP_HW_NUM(ep->addr)]; 
  len = RXBYTECNT(AT91C_UDP_CSR[EP_HW_NUM(ep->addr)]);
  if (mask + 1 - ep->buf_len < len) return 0;
  ep->buf_len += len;
  while(len-- > 0) {
    buffer[pos] = *fdr;
    pos = (pos + 1) & mask;
  }
  return 1;
}

unsigned int
usb_recv_data(unsigned char ep_addr, unsigned char *dat, unsigned int len)
{
  volatile USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
  unsigned char mask = ep->buf_size_mask;
  *AT91C_UDP_IDR = 1<<EP_HW_NUM(ep_addr);
  {
    unsigned char l;
    unsigned char *to = ep->buffer;
    unsigned char pos = ep->buf_pos;
    if (ep->buf_len < len) len = ep->buf_len;
    ep->buf_len -= len;
    l = len;
    while(l-- > 0) {
      *dat++ = to[pos]; 
      pos = (pos + 1) & mask;
    }
    ep->buf_pos = pos;
  }
  ep->flags &= ~USB_EP_FLAGS_RECV_BLOCKED;
  *AT91C_UDP_IER = 1<<EP_HW_NUM(ep_addr);
  return len;
}

void
write_buffered_endpoint(volatile USBEndpoint *ep)
{
  unsigned int irq = disableIRQ();
  {
    unsigned int pos = ep->buf_pos;
    unsigned int xfer_len = ep->buf_len;
    unsigned int mask = ep->buf_size_mask;
    const unsigned char *buf_tmp = ep->buffer;
    AT91_REG *fdr = &AT91C_UDP_FDR[EP_HW_NUM(ep->addr)]; 
    if (!(ep->flags & USB_EP_FLAGS_TRANSMITTING)
	&& !(AT91C_UDP_CSR[EP_HW_NUM(ep->addr)] & AT91C_UDP_TXPKTRDY)) {
      if (xfer_len > NON_CTRL_XFER_SIZE)  xfer_len = NON_CTRL_XFER_SIZE;
      ep->buf_len -= xfer_len;
      /* printf("Writing %d to 0x%02x\n", xfer_len, ep->addr); */
      while(xfer_len > 0) {
	*fdr = buf_tmp[pos];
	pos = (pos + 1) & mask;
	xfer_len--;
      }

      ep->flags |= USB_EP_FLAGS_TRANSMITTING;
      /* Start transmission */
      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[EP_HW_NUM(ep->addr)],
			    AT91C_UDP_TXPKTRDY, AT91C_UDP_TXPKTRDY);
      ep->buf_pos = pos;
    }
  }
  restoreIRQ(irq);
}

static void
write_send_buffer(unsigned char *buffer, const unsigned char *dat,
		  unsigned int len)
{
  while(len-- > 0) {
    *buffer++ = *dat++;
  }
}

void
usb_send_buffer_get(unsigned char ep_addr, unsigned int offset,
		    unsigned char **dat, unsigned int *lenp)
{
  unsigned int pos;
  volatile USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
  unsigned int size = ep->buf_size_mask + 1;
  unsigned int len;
  *AT91C_UDP_IDR = 1<<EP_HW_NUM(ep_addr);
  len = size - ep->buf_len;
  pos = (ep->buf_pos + offset + ep->buf_len) & (size - 1);
  *AT91C_UDP_IER = 1<<EP_HW_NUM(ep_addr);
  if (offset >= len) {
    len = 0;
  } else {
    len -= offset;
  }
  if (pos + len > size) len = size - pos;
  *dat = &ep->buffer[pos];
  *lenp = len;
}
       
void
usb_send_buffer_commit(unsigned char ep_addr, unsigned int len)
{
  volatile USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
  *AT91C_UDP_IDR = 1<<EP_HW_NUM(ep_addr);
  ep->buf_len += len;
  write_buffered_endpoint(ep);
  *AT91C_UDP_IER = 1<<EP_HW_NUM(ep_addr);
}

unsigned int
usb_send_data(unsigned char ep_addr, const unsigned char *dat, unsigned int len)
{
  unsigned char *write_pos;
  unsigned int write_len;
  unsigned int full_len = len;
  /* printf("usb_send_data %02x: IMR=%08x\n",ep_addr, *AT91C_UDP_IMR); */
  while(len > 0) {
    usb_send_buffer_get(ep_addr, 0, &write_pos, &write_len);
    if (write_len == 0) break;
    if (write_len > len) write_len = len;
    write_send_buffer(write_pos, dat, write_len);
    /* printf("Pos: %p, len %d\n", write_pos, write_len); */
    usb_send_buffer_commit(ep_addr, write_len);
    dat += write_len;
    len -= write_len;
  }
  return full_len - len;
}


void
usb_send_ctrl_response(const unsigned char *buffer, unsigned short len)
{
  if (AT91C_UDP_CSR[0] & AT91C_UDP_TXPKTRDY) return;
  *AT91C_UDP_IDR = AT91C_UDP_EP0;
  if (len >= usb_setup_buffer.wLength) {
    len = usb_setup_buffer.wLength;	/* Truncate if too long */
    usb_flags &= ~USB_FLAG_SEND_ZLP;
  } else {
    /* Send ZLP if the response is shorter than requested */
    usb_flags |= USB_FLAG_SEND_ZLP;
  }
  usb_ctrl_send_pos = buffer;
  usb_ctrl_send_len = len;
  write_ctrl();
  *AT91C_UDP_IER = AT91C_UDP_EP0;
}

void 
usb_send_ctrl_status()
{
  *AT91C_UDP_IDR = AT91C_UDP_EP0;
  /* Start transmission */
  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],
			AT91C_UDP_TXPKTRDY, AT91C_UDP_TXPKTRDY);
  *AT91C_UDP_IER = AT91C_UDP_EP0;
}

static void
notify_process(unsigned char events)
{
  usb_events |= events;
  if (usb_handler_process) {
    process_poll(usb_handler_process);
  }
}

void
usb_reset()
{
  /* Setup endpoint 0 */
  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],
			AT91C_UDP_EPTYPE_CTRL | AT91C_UDP_EPEDS,
			AT91C_UDP_EPTYPE | AT91C_UDP_EPEDS);

  /* Enable interrupt for control endpoint */
  *AT91C_UDP_IER = AT91C_UDP_EP0;
  notify_process(USB_EVENT_RESET);
}

struct USB_request_st usb_setup_buffer;



static void
read_fifo0(unsigned char *buffer, unsigned int length)
{
  unsigned int r;
  for (r = 0; r < length; r++) {
    *buffer++ = AT91C_UDP_FDR[0];
  }
}

void
usb_ep0_int()
{
  unsigned int status;
  status = AT91C_UDP_CSR[0];
#if 0
  printf("status: %08x\n", status);
#endif
  if (status & AT91C_UDP_STALLSENT) {
    /* Acknowledge */
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],0, AT91C_UDP_STALLSENT);
  }
  if (status & AT91C_UDP_RXSETUP) {
    usb_ctrl_send_pos = NULL; /* Cancel any pending control data
				 transmission */
    if (RXBYTECNT(status) == 8) {
      read_fifo0((unsigned char*)&usb_setup_buffer, 8);
    
      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],
			    ((usb_setup_buffer.bmRequestType & 0x80)
			     ? AT91C_UDP_DIR : 0),
			    AT91C_UDP_DIR);
      usb_ctrl_data_len = 0;
      if ((usb_setup_buffer.bmRequestType & 0x80) != 0
	  || usb_setup_buffer.wLength == 0) {
	usb_endpoint_events[0] |= USB_EP_EVENT_SETUP;
	notify_process(USB_EVENT_EP(0));
      } else {
	if (usb_setup_buffer.wLength > MAX_CTRL_DATA) {
	  /* stall */
	  usb_error_stall();
	} else {
	  usb_flags |= USB_FLAG_RECEIVING_CTRL;
	}
      }
    } else {
      usb_error_stall();
    }
    /* Acknowledge SETUP */
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],0, AT91C_UDP_RXSETUP);
  } else if (status & (AT91C_UDP_RX_DATA_BK1 | AT91C_UDP_RX_DATA_BK0)) {
    puts("IN");
    if (usb_flags & USB_FLAG_RECEIVING_CTRL) {
      unsigned int len;
      unsigned int left = MAX_CTRL_DATA - usb_ctrl_data_len;
      len = RXBYTECNT(status);
      if (len > left) {
	/* stall */
	usb_error_stall();
      } else {
	unsigned char *buf_tmp = usb_ctrl_data_buffer + usb_ctrl_data_len;
	usb_ctrl_data_len += len;
	if (usb_ctrl_data_len == usb_setup_buffer.wLength
	    || len < CTRL_EP_SIZE) {
	  usb_flags &= ~USB_FLAG_RECEIVING_CTRL;
	  usb_endpoint_events[0] |= USB_EP_EVENT_SETUP;
	  notify_process(USB_EVENT_EP(0));
	}
	while(len-- > 0) *buf_tmp++ = AT91C_UDP_FDR[0];
      }
    } else {
      if (RXBYTECNT(status) > 0) {
	puts("Discarded input");
      }
    }
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],0,
			  AT91C_UDP_RX_DATA_BK1 | AT91C_UDP_RX_DATA_BK0);
  }
  if (status & AT91C_UDP_TXCOMP) {
    /* puts("TX complete"); */
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[0],0, AT91C_UDP_TXCOMP);
    if (usb_flags & USB_FLAG_ADDRESS_PENDING) {
      *AT91C_UDP_FADDR = AT91C_UDP_FEN | LOW_BYTE(usb_setup_buffer.wValue);
      *AT91C_UDP_GLBSTATE |= AT91C_UDP_FADDEN;
      usb_flags &= ~USB_FLAG_ADDRESS_PENDING;
      printf("Address changed: %d\n", *AT91C_UDP_FADDR & 0x7f);
    } else {
      if(usb_ctrl_send_pos) {
	write_ctrl();
      }
    }
  }

}

void
usb_epx_int()
{
  unsigned int ep_index;
  /* Handle enabled interrupts */
  unsigned int int_status = *AT91C_UDP_ISR & *AT91C_UDP_IMR;
  for (ep_index = 0; ep_index < NUM_EP-1; ep_index++) {
    volatile USBEndpoint *ep = &usb_endpoints[ep_index];
    unsigned int ep_num = EP_HW_NUM(ep->addr);
    unsigned int ep_mask;
    if (ep->addr != 0) { /* skip if not configured */
      ep_mask = 1<<ep_num;
      if (int_status & ep_mask) {
	unsigned int status;
	status = AT91C_UDP_CSR[ep_num];
#if 0
	printf("EP %d status: %08x\n", ep->addr, status);
#endif
	if (status & AT91C_UDP_STALLSENT) {
	  /* Acknowledge */
	  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[ep_num],0, AT91C_UDP_STALLSENT);
	}
	if (status & AT91C_UDP_TXCOMP) {
	  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[ep_num],0, AT91C_UDP_TXCOMP);
	  ep->flags &= ~USB_EP_FLAGS_TRANSMITTING;
	  if (ep->buf_len > 0) {
	    write_buffered_endpoint(ep);
	    /* Tell the application that there's more room in the buffer */
	    usb_endpoint_events[ep_num] |= USB_EP_EVENT_IN;
	    notify_process(USB_EVENT_EP(ep_num));
	  }
	}
	if (status & (AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1)) {
	  unsigned char read_cnt;
	  read_cnt = read_buffered_endpoint(ep);
	  if (read_cnt == 0) {
	    *AT91C_UDP_IDR = 1<<ep_num;
	    ep->flags |= USB_EP_FLAGS_RECV_BLOCKED;
	  } else {
	    if (status & AT91C_UDP_RX_DATA_BK1) {
	      /* Ping-pong */
	      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[ep_num],0,
				    (ep->flags & USB_EP_FLAGS_BANK_1_RECV_NEXT)
				    ? AT91C_UDP_RX_DATA_BK1
				    : AT91C_UDP_RX_DATA_BK0);
	      ep->flags ^= USB_EP_FLAGS_BANK_1_RECV_NEXT;
	    } else {
	      /* Ping-pong or single buffer */
	      UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[ep_num],0,
				  AT91C_UDP_RX_DATA_BK0);
	      ep->flags |= USB_EP_FLAGS_BANK_1_RECV_NEXT;
	    }
	  }
	  usb_endpoint_events[ep_num] |= USB_EP_EVENT_OUT;
	  notify_process(ep_mask);
	}
      }
    }
  }
}


/* Clear usb events from non-interrupt code */
void
usb_clear_events(unsigned events)
{
  /* Disable allUSB events */
  *AT91C_AIC_IDCR = (1 << AT91C_ID_UDP);
  usb_events &= ~events;
  /* Reenable interrupt */
  *AT91C_AIC_IECR = (1 << AT91C_ID_UDP);
}

void
usb_clear_ep_events(unsigned int ep, unsigned int events)
{
  /* Disable all USB events */
  *AT91C_AIC_IDCR = (1 << AT91C_ID_UDP);
  usb_endpoint_events[ep] &= ~events;
  /* Reenable interrupt */
  *AT91C_AIC_IECR = (1 << AT91C_ID_UDP);
}

void
usb_set_address()
{
  usb_flags |= USB_FLAG_ADDRESS_PENDING;
  /* The actual setting of the address is done when the status packet
     is sent. */
}


static void
setup_endpoint(unsigned char addr,
	       unsigned char *buffer, unsigned int buf_size,
	       unsigned int type) 
{
  volatile USBEndpoint *ep;
  /* Check if the address points to an existing endpoint */
  if (EP_INDEX(addr) >= (sizeof(usb_endpoints)/sizeof(usb_endpoints[0]))) {
    return;
  }
  ep = &usb_endpoints[EP_INDEX(addr)];
  ep->addr = addr;
  ep->buf_size_mask = buf_size - 1;
  ep->buffer = buffer;
  ep->buf_len = 0;
  ep->buf_pos = 0;
  ep->status = 0;
  *AT91C_UDP_IDR = 1<<EP_HW_NUM(addr);
  {
    unsigned int ep_num = EP_HW_NUM(addr);
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[ep_num],
			  type | AT91C_UDP_EPEDS,
			  AT91C_UDP_EPTYPE | AT91C_UDP_EPEDS);
    
    
  }
  *AT91C_UDP_IER = 1<<EP_HW_NUM(addr);
}

void
usb_setup_bulk_endpoint(unsigned char addr,
			unsigned char *buffer, unsigned int buf_size)
{
  setup_endpoint(addr, buffer, buf_size,
		 (addr & 0x80) ? AT91C_UDP_EPTYPE_BULK_IN
		 :AT91C_UDP_EPTYPE_BULK_OUT);
}

void
usb_setup_interrupt_endpoint(unsigned char addr,
			     unsigned char *buffer, unsigned int buf_size)
{
  setup_endpoint(addr, buffer, buf_size,
		 (addr & 0x80) ? AT91C_UDP_EPTYPE_INT_IN
		 :AT91C_UDP_EPTYPE_INT_OUT);
}



void
usb_disable_endpoint(unsigned char addr)
{
  /* Check if the address points to an existing endpoint */
  if (EP_INDEX(addr) >= (sizeof(usb_endpoints)/sizeof(usb_endpoints[0]))) {
    return;
  }
  *AT91C_UDP_IDR = 1<<EP_HW_NUM(addr);
  UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[EP_HW_NUM(addr)], 0, AT91C_UDP_EPEDS);
  usb_endpoints[EP_INDEX(addr)].addr = 0;
}

static void
init_ep(volatile USBEndpoint *ctxt)
{
  ctxt->addr = 0;
  ctxt->buf_size_mask = 0;
  ctxt->buf_len = 0;
  ctxt->buf_pos = 0;
  ctxt->buffer = 0;
  ctxt->status = 0;
  ctxt->flags = 0;
}

void
usb_init_endpoints()
{
  unsigned int i;
  for (i = 0; i < NUM_EP-1; i++) {
    init_ep(&usb_endpoints[i]);
  }
}

volatile USBEndpoint*
usb_find_endpoint(unsigned char epaddr)
{
  if (EP_INDEX(epaddr) >= NUM_EP - 1) return 0;
  return &usb_endpoints[EP_INDEX(epaddr)];
}

void
usb_halt_endpoint(unsigned char ep_addr, unsigned int halt)
{
  *AT91C_UDP_IDR = 1<<EP_HW_NUM(ep_addr);
  if (halt) {
    UDP_SET_EP_CTRL_FLAGS(&AT91C_UDP_CSR[EP_HW_NUM(ep_addr)],
			  AT91C_UDP_FORCESTALL, AT91C_UDP_FORCESTALL);
    usb_endpoints[EP_INDEX(ep_addr)].status |= 0x01;
  } else {
    *AT91C_UDP_RSTEP = 1<<EP_HW_NUM(ep_addr);
    usb_endpoints[EP_INDEX(ep_addr)].status &= ~0x01;
  }
  *AT91C_UDP_IER = 1<<EP_HW_NUM(ep_addr);
}

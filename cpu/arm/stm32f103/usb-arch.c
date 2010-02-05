#include <usb-arch.h>
#include <gpio.h>
#include <nvic.h>
#include <stdio.h>
#include <debug-uart.h>
#include <usb-stm32f103.h>

/* #define DEBUG     */
#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
typedef struct
{
  vu32 EPR[8];
  u32  RESERVED[8];
  vu32 CNTR;
  vu32 ISTR;
  vu32 FNR;
  vu32 DADDR;
  vu32 BTABLE;
} USB_TypeDef;

#define USB_BASE             (APB1PERIPH_BASE + 0x5c00)
#define USB_MEM_BASE	     (APB1PERIPH_BASE + 0x6000)
#define USB_MEM_SIZE	     (512)
#define USB ((volatile USB_TypeDef *) USB_BASE)

typedef struct {
  vu32 ADDR_TX;
  vu32 COUNT_TX;
  vu32 ADDR_RX;
  vu32 COUNT_RX;
} USB_HW_Buffer;

#define USB_EP0_BUF_SIZE (2*CTRL_EP_SIZE)

#define USB_EP1_BUF_SIZE (2*USB_EP1_SIZE)

#define USB_EP2_BUF_SIZE (2*USB_EP2_SIZE)

#define USB_EP3_BUF_SIZE (2*USB_EP3_SIZE)

#define USB_EP4_BUF_SIZE (2*USB_EP4_SIZE)

#define USB_EP5_BUF_SIZE (2*USB_EP5_SIZE)

#define USB_EP6_BUF_SIZE (2*USB_EP6_SIZE)

#define USB_EP7_BUF_SIZE (2*USB_EP7_SIZE)



#ifndef MAX_CTRL_DATA
#define MAX_CTRL_DATA 128
#endif
/* Double buffered IN endpoint */
#define ADDR_TX_0 ADDR_TX
#define ADDR_TX_1 ADDR_RX
#define COUNT_TX_0 COUNT_TX
#define COUNT_TX_1 COUNT_RX

/* Double buffered OUT endpoint */
#define ADDR_RX_0 ADDR_TX
#define ADDR_RX_1 ADDR_RX
#define COUNT_RX_0 COUNT_TX
#define COUNT_RX_1 COUNT_RX

#define USB_EPxR_EP_TYPE_BULK 0
#define USB_EPxR_EP_TYPE_CONTROL USB_EP0R_EP_TYPE_0
#define USB_EPxR_EP_TYPE_ISO USB_EP0R_EP_TYPE_1
#define USB_EPxR_EP_TYPE_INTERRUPT (USB_EP0R_EP_TYPE_1|USB_EP0R_EP_TYPE_0)

#define USB_EPxR_EP_DBL_BUF USB_EP0R_EP_KIND
#define USB_EPxR_EP_STATUS_OUT USB_EP0R_EP_KIND

#define USB_EPxR_STAT_RX_DISABLED 0
#define USB_EPxR_STAT_RX_STALL USB_EP0R_STAT_RX_0
#define USB_EPxR_STAT_RX_NAK USB_EP0R_STAT_RX_1
#define USB_EPxR_STAT_RX_VALID (USB_EP0R_STAT_RX_1|USB_EP0R_STAT_RX_0)

#define USB_EPxR_STAT_TX_DISABLED 0
#define USB_EPxR_STAT_TX_STALL USB_EP0R_STAT_TX_0
#define USB_EPxR_STAT_TX_NAK USB_EP0R_STAT_TX_1
#define USB_EPxR_STAT_TX_VALID (USB_EP0R_STAT_TX_1|USB_EP0R_STAT_TX_0)

#define USB_EPxR_SW_BUF_TX USB_EP0R_DTOG_RX
#define USB_EPxR_SW_BUF_RX USB_EP0R_DTOG_TX

static const uint16_t ep_buffer_size[8] =
  {
    USB_EP0_BUF_SIZE,
    USB_EP1_BUF_SIZE,
    USB_EP2_BUF_SIZE,
    USB_EP3_BUF_SIZE,
    USB_EP4_BUF_SIZE,
    USB_EP5_BUF_SIZE,
    USB_EP6_BUF_SIZE,
    USB_EP7_BUF_SIZE
  };

#define USB_EP_BUF_SIZE(ep) ep_buffer_size[ep]
#define USB_EP_BUF_OFFSET(ep) ep_buffer_offset[ep]
#define USB_EP_BUF_ADDR(ep) (u32*)(USB_MEM_BASE + ep_buffer_offset[ep]*2);
#define USB_EP_BUF_DESC(ep) ((USB_HW_Buffer*)(USB_MEM_BASE + 16 * (ep)))

#define USB_EP0_OFFSET (8*USB_MAX_ENDPOINTS)
#define USB_EP1_OFFSET (USB_EP0_OFFSET + USB_EP0_BUF_SIZE)
#define USB_EP2_OFFSET (USB_EP1_OFFSET + USB_EP1_BUF_SIZE)
#define USB_EP3_OFFSET (USB_EP2_OFFSET + USB_EP2_BUF_SIZE)
#define USB_EP4_OFFSET (USB_EP3_OFFSET + USB_EP3_BUF_SIZE)
#define USB_EP5_OFFSET (USB_EP4_OFFSET + USB_EP4_BUF_SIZE)
#define USB_EP6_OFFSET (USB_EP5_OFFSET + USB_EP5_BUF_SIZE)
#define USB_EP7_OFFSET (USB_EP6_OFFSET + USB_EP6_BUF_SIZE)

#if (USB_EP7_OFFSET+USB_EP7_BUF_SIZE) > USB_MEM_SIZE
#error USB endpoints buffers does not fit in USB memory
#endif
static const uint16_t ep_buffer_offset[8] =
  {
    USB_EP0_OFFSET,
    USB_EP1_OFFSET,
    USB_EP2_OFFSET,
    USB_EP3_OFFSET,
    USB_EP4_OFFSET,
    USB_EP5_OFFSET,
    USB_EP6_OFFSET,
    USB_EP7_OFFSET
  };

#define USB_EP_BUF_CAPACITY(s) ((((s) <64)?((s)/2):(0x20 | ((s)/64)))<<10)

typedef struct _USBEndpoint USBEndpoint;
struct _USBEndpoint
{
  uint16_t status;
  uint8_t addr;
  uint8_t flags;
  USBBuffer *buffer;	/* NULL if no current buffer */
  struct process *event_process;
  uint16_t events;
  uint16_t xfer_size;
};

#define USB_EP_FLAGS_TYPE_MASK 0x03
#define USB_EP_FLAGS_TYPE_BULK 0x00
#define USB_EP_FLAGS_TYPE_CONTROL 0x01
#define USB_EP_FLAGS_TYPE_ISO 0x02
#define USB_EP_FLAGS_TYPE_INTERRUPT 0x03

#define IS_EP_TYPE(ep, type) (((ep)->flags & USB_EP_FLAGS_TYPE_MASK) == (type))
#define IS_CONTROL_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_CONTROL)
#define IS_BULK_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_BULK)
#define IS_INTERRUPT_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_INTERRUPT)

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
#define EP_STRUCT(addr) (&usb_endpoints[EP_INDEX(addr)])

/* Number of hardware endpoint */
#define EP_HW_NUM(addr) ((addr) & 0x7f)

#define USB_DISABLE_INT \
  NVIC_DISABLE_INT(USB_LP_CAN_RX0_IRQChannel);\
  NVIC_DISABLE_INT(USB_HP_CAN_TX_IRQChannel)

#define USB_ENABLE_INT \
  NVIC_ENABLE_INT(USB_LP_CAN_RX0_IRQChannel);\
  NVIC_ENABLE_INT(USB_HP_CAN_TX_IRQChannel)

static inline uint32_t
usb_save_disable_int()
{
  uint32_t v = NVIC->ISER[0];
  NVIC->ICER[0] = (1<<USB_HP_CAN_TX_IRQChannel | 1<<USB_LP_CAN_RX0_IRQChannel);
  return v;
}

static inline void
usb_restore_int(uint32_t v)
{
  NVIC->ISER[0] =
    v & (1<<USB_HP_CAN_TX_IRQChannel | 1<<USB_LP_CAN_RX0_IRQChannel);
}

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
      while(buffer) {
	buffer->flags &= ~USB_BUFFER_SUBMITTED;
	buffer = buffer->next;
      }
    }
  }
  usb_arch_setup_control_endpoint(0);  
  USB->DADDR = 0x80;
}

void
usb_arch_setup(void)
{
  unsigned int i;
  RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
  RCC->APB2ENR |=  (RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN);
  RCC->APB1ENR |= (RCC_APB1ENR_USBEN);
  RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;

  GPIO_CONF_OUTPUT_PORT(A,11,ALT_PUSH_PULL,50);
  GPIO_CONF_OUTPUT_PORT(A,12,ALT_PUSH_PULL,50);
  GPIO_CONF_OUTPUT_PORT(A,10, PUSH_PULL, 2);
  GPIOA->BSRR = GPIO_BSRR_BR10;

  /* Turn on analog part */
  USB->CNTR &= ~USB_CNTR_PDWN;
  
  for (i = 0; i < 24; i++) asm("nop"::); /* Wait at least 1us */
  /* Release reset */
  USB->CNTR &= ~USB_CNTR_FRES;
  /* Clear any interrupts */
  USB->ISTR = ~(USB_ISTR_PMAOVR |USB_ISTR_ERR | USB_ISTR_WKUP | USB_ISTR_SUSP
		| USB_ISTR_RESET);

  for(i = 0; i < USB_MAX_ENDPOINTS; i++) {
    usb_endpoints[i].flags = 0;
    usb_endpoints[i].event_process = 0;
  }
  /* Put buffer table at beginning of buffer memory */
  USB->BTABLE = 0;
  usb_arch_reset();
  GPIOA->BSRR = GPIO_BSRR_BS10;
  USB->CNTR |= (USB_CNTR_CTRM | USB_CNTR_PMAOVRM | USB_CNTR_ERRM
		| USB_CNTR_WKUPM| USB_CNTR_SUSPM | USB_CNTR_RESETM);
  NVIC_SET_PRIORITY(USB_LP_CAN_RX0_IRQChannel, 4);
  NVIC_ENABLE_INT(USB_LP_CAN_RX0_IRQChannel);
}

#define EPR_RW (USB_EP0R_EP_TYPE|USB_EP0R_EP_KIND|USB_EP0R_EA)
#define EPR_W0 (USB_EP0R_CTR_RX|USB_EP0R_CTR_TX)
#define EPR_TOGGLE (USB_EP0R_DTOG_RX | USB_EP0R_STAT_RX \
		    | USB_EP0R_DTOG_TX | USB_EP0R_STAT_TX)

#define EPR_INVARIANT(epr) ((epr & (EPR_RW)) | EPR_W0)

#define EPR_TOGGLE_SET(epr, mask, set) \
((((epr) & (EPR_RW | (mask))) | EPR_W0) ^ (set))

static void
usb_arch_setup_endpoint(unsigned char addr)
{
  USBEndpoint *ep = EP_STRUCT(addr);
  ep->status = 0;
  ep->flags = USB_EP_FLAGS_ENABLED;
  ep->buffer = 0;
  ep->addr = addr;
  ep->events = 0;
  ep->xfer_size = 0;
};

void
usb_arch_setup_control_endpoint(unsigned char addr)
{
  USB_HW_Buffer *buf_desc;
  unsigned int ei = EP_HW_NUM(addr);
  unsigned int epr;
  USBEndpoint *ep = EP_STRUCT(addr);
  usb_arch_setup_endpoint(addr);
  ep->flags |= USB_EP_FLAGS_TYPE_CONTROL;

  buf_desc = USB_EP_BUF_DESC(ei);
  buf_desc->ADDR_TX = USB_EP_BUF_OFFSET(ei);
  buf_desc->COUNT_TX = USB_EP_BUF_SIZE(ei)/2;
  buf_desc->ADDR_RX = USB_EP_BUF_OFFSET(ei) + USB_EP_BUF_SIZE(ei)/2;
  buf_desc->COUNT_RX = USB_EP_BUF_CAPACITY(USB_EP_BUF_SIZE(ei)/2);
  ep->xfer_size = USB_EP_BUF_SIZE(ei)/2;
  epr = USB->EPR[EP_HW_NUM(addr)];
  /* Clear interrupt flags */
  epr &= ~(USB_EP0R_CTR_RX | USB_EP0R_CTR_TX);
  /* NACK both directions */
  epr ^= USB_EPxR_STAT_RX_NAK | USB_EPxR_STAT_TX_NAK;
  /* Set control type */
  epr = ((epr & ~(USB_EP0R_EP_TYPE | USB_EP0R_EP_KIND))
	 | USB_EPxR_EP_TYPE_CONTROL);
  /* Set address */
  epr = ((epr & ~USB_EP0R_EA) | addr);
  USB->EPR[EP_HW_NUM(addr)] = epr; 
}

void
usb_arch_setup_bulk_endpoint(unsigned char addr)
{
  USB_HW_Buffer *buf_desc;
  unsigned int ei = EP_HW_NUM(addr);
  unsigned int epr;
  USBEndpoint *ep = EP_STRUCT(addr);
  usb_arch_setup_endpoint(addr);
  ep->flags |= USB_EP_FLAGS_TYPE_BULK;

  buf_desc = USB_EP_BUF_DESC(ei);
  buf_desc->ADDR_TX = USB_EP_BUF_OFFSET(ei);
  buf_desc->ADDR_RX = USB_EP_BUF_OFFSET(ei) + USB_EP_BUF_SIZE(ei)/2;
  epr = USB->EPR[ei];
  if (addr & 0x80) {
    /* IN */
    buf_desc->COUNT_TX_0 = 0;
    buf_desc->COUNT_TX_1 = 0;
    /* VALID transmission */
    epr ^= USB_EPxR_STAT_TX_VALID;
  } else {
    /* OUT */
    buf_desc->COUNT_RX_0 = USB_EP_BUF_CAPACITY(USB_EP_BUF_SIZE(ei)/2);
    buf_desc->COUNT_RX_1 = USB_EP_BUF_CAPACITY(USB_EP_BUF_SIZE(ei)/2);
    
    /* VALID reception  */
    epr ^= USB_EPxR_STAT_RX_VALID;
   }
  ep->xfer_size = USB_EP_BUF_SIZE(ei)/2;
  /* Clear interrupt flags */
  epr &= ~(USB_EP0R_CTR_RX | USB_EP0R_CTR_TX);
  /* Set bulk type */
  epr = ((epr & ~(USB_EP0R_EP_TYPE | USB_EP0R_EP_KIND))
	 | USB_EPxR_EP_TYPE_BULK | USB_EPxR_EP_DBL_BUF);
  /* Set address */
  epr = ((epr & ~USB_EP0R_EA) | addr);
  USB->EPR[ei] = epr;

}

void
usb_arch_setup_interrupt_endpoint(unsigned char addr)
{
  USB_HW_Buffer *buf_desc;
  unsigned int ei = EP_HW_NUM(addr);
  unsigned int epr;
  USBEndpoint *ep = EP_STRUCT(addr);
  usb_arch_setup_endpoint(addr);
  ep->flags |= USB_EP_FLAGS_TYPE_INTERRUPT;
  
  epr = USB->EPR[EP_HW_NUM(addr)];

  buf_desc = USB_EP_BUF_DESC(ei);
  if (addr & 0x80) {
    /* IN */
    buf_desc->ADDR_TX = USB_EP_BUF_OFFSET(ei);
    buf_desc->COUNT_TX = USB_EP_BUF_SIZE(ei);
    epr ^= USB_EPxR_STAT_TX_NAK;
  } else {
    /* OUT */
    buf_desc->ADDR_RX = USB_EP_BUF_OFFSET(ei);
    buf_desc->COUNT_RX = USB_EP_BUF_CAPACITY(USB_EP_BUF_SIZE(ei));
    epr ^= USB_EPxR_STAT_RX_NAK;
  }
  ep->xfer_size = USB_EP_BUF_SIZE(ei);
  /* Clear interrupt flags */
  epr &= ~(USB_EP0R_CTR_RX | USB_EP0R_CTR_TX);
  /* Set control type */
  epr = ((epr & ~(USB_EP0R_EP_TYPE | USB_EP0R_EP_KIND))
	 | USB_EPxR_EP_TYPE_INTERRUPT);
  /* Set address */
  epr = ((epr & ~USB_EP0R_EA) | addr);
  USB->EPR[EP_HW_NUM(addr)] = epr; 
}

void
usb_arch_disable_endpoint(uint8_t addr)
{
  unsigned int epr;
  USBEndpoint *ep = EP_STRUCT(addr);
  ep->flags &= ~USB_EP_FLAGS_ENABLED;
  
  epr = USB->EPR[EP_HW_NUM(addr)];

  epr ^= USB_EPxR_STAT_TX_DISABLED | USB_EPxR_STAT_RX_DISABLED;
  /* Clear interrupt flags */
  epr &= ~(USB_EP0R_CTR_RX | USB_EP0R_CTR_TX);
  USB->EPR[EP_HW_NUM(addr)] = epr; 
}

inline void
stall_bulk_in(unsigned int hw_ep)
{
  volatile uint32_t *eprp = &USB->EPR[hw_ep];
  *eprp = (*eprp & (EPR_RW | USB_EP0R_STAT_TX_1)) | EPR_W0;
  PRINTF("HALT IN\n");
}

inline void
stall_bulk_out(unsigned int hw_ep)
{
  volatile uint32_t *eprp = &USB->EPR[hw_ep];
  *eprp = (*eprp & ((EPR_RW | USB_EP0R_STAT_RX_1) & ~USB_EP0R_CTR_RX)) |EPR_W0;
  PRINTF("HALT OUT\n");
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

inline unsigned int
ep_capacity(unsigned int count)
{
  return (((count & USB_COUNT0_RX_NUM_BLOCK)>>10)
	  * ((count & USB_COUNT0_RX_BLSIZE) ? 32 : 2));
}

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
read_hw_buffer(USBBuffer *buffer, unsigned int offset, unsigned int len)
{
#ifdef USB_STM32F103_ENABLE_ALT_COPY
  if (buffer->flags & USB_BUFFER_ARCH_ALT_COPY) {
    copy_from_hw_buffer(buffer, offset, len);
  } else
#endif
    {
      uint8_t *data = buffer->data;
      const uint32_t *hw_data = ((u32*)USB_MEM_BASE) + offset/2;
      buffer->data += len;
      if (offset & 1) {
	*data++ = *hw_data++ >> 8;
	len--;
      }
      while(len >= 2) {
	*((uint16_t*)data) = *hw_data++;
	data += 2;
	len -= 2;
      }
      if (len == 1) {
	*data++ = *hw_data;
      }
    }
}


#define USB_WRITE_BLOCK 0x01
#define USB_WRITE_NOTIFY 0x02

void
write_hw_buffer(USBBuffer *buffer,unsigned int offset, unsigned int len)
{
#ifdef USB_STM32F103_ENABLE_ALT_COPY
  if (buffer->flags & USB_BUFFER_ARCH_ALT_COPY) {
    copy_to_hw_buffer(buffer, offset, len);
  } else
#endif    
    {
      const uint8_t *data;
      uint32_t *hw_data;
      if (len == 0) return;
      data = buffer->data;
      hw_data = ((u32*)USB_MEM_BASE) + offset/2;
      buffer->data += len;
      if (offset & 1) {
	*hw_data = (*hw_data & 0xff) | (*data++ << 8);
	hw_data++;
	len--;
      }
      while(len >= 2) {
	*hw_data++ = *((uint16_t*)data) ;
	data += 2;
	len -= 2;
      }
      if (len == 1) {
	*hw_data = *data++;
      }
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
  unsigned int hw_offset;
  unsigned int hw_ep = EP_HW_NUM(ep->addr);
  USBBuffer *buffer = ep->buffer;
  unsigned int flags = ep->flags;
  USB_HW_Buffer *buf_desc = USB_EP_BUF_DESC(hw_ep);
  PRINTF("handle_pending_receive:\n"); 
  if (!(flags & USB_EP_FLAGS_ENABLED) || !buffer) return USB_READ_BLOCK;
  switch(flags & USB_EP_FLAGS_TYPE_MASK) {
  case USB_EP_FLAGS_TYPE_CONTROL:
    len = buf_desc->COUNT_RX & USB_COUNT0_RX_COUNT0_RX;
    if (flags & USB_EP_FLAGS_SETUP_PENDING) {
      /* Discard buffers until we find a SETUP buffer */
      buffer =
	skip_buffers_until(buffer, USB_BUFFER_SETUP, USB_BUFFER_SETUP, &res);
      ep->buffer = buffer;
      if (!buffer || buffer->left < len) {
	res |= USB_READ_BLOCK;
	return res;
      }
      /* SETUP buffer must fit in a single buffer */
      if (buffer->left < len) {
	buffer->flags |= USB_BUFFER_FAILED;
	buffer->flags &= ~USB_BUFFER_SUBMITTED ;
	if (buffer->flags & USB_BUFFER_NOTIFY) res |= USB_READ_NOTIFY;
	ep->buffer = buffer->next;
	res |= USB_READ_FAIL;
	return res;
      }
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
    hw_offset =  buf_desc->ADDR_RX;
    break;
  case USB_EP_FLAGS_TYPE_INTERRUPT:
    len = buf_desc->COUNT_RX & USB_COUNT0_RX_COUNT0_RX;
    if (get_receive_capacity(buffer) <  len) return USB_READ_BLOCK;
    hw_offset =  buf_desc->ADDR_RX;
    break;
  case USB_EP_FLAGS_TYPE_BULK:
     if (USB->EPR[hw_ep] & USB_EPxR_SW_BUF_RX) {
      len = buf_desc->COUNT_RX_1 & USB_COUNT0_RX_COUNT0_RX;
      hw_offset =  buf_desc->ADDR_RX_1;
    } else {
      len = buf_desc->COUNT_RX_0 & USB_COUNT0_RX_COUNT0_RX;
      hw_offset =  buf_desc->ADDR_RX_0;
    }
    if (get_receive_capacity(buffer) <  len) return USB_READ_BLOCK;
    break;
  case USB_EP_FLAGS_TYPE_ISO:
    len = buf_desc->COUNT_RX & USB_COUNT0_RX_COUNT0_RX;
    if (get_receive_capacity(buffer) <  len) return USB_READ_BLOCK;
    hw_offset =  buf_desc->ADDR_RX;
  }
  /* printf("handle_pending_receive: %d %04x\n", len, ep->flags);   */
  short_packet = len < ep->xfer_size;

  do {
    if (buffer->left < len) {
      copy = buffer->left;
    } else {
      copy = len;
    }
    len -= copy;
    buffer->left -= copy;
    read_hw_buffer(buffer, hw_offset, copy);
    hw_offset += copy;

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
  if (IS_BULK_EP(ep)) {
    USB->EPR[hw_ep] = EPR_INVARIANT(USB->EPR[hw_ep]) | USB_EPxR_SW_BUF_RX;
  }

  /* PRINTF("read_endpoint %d %d\n", (int)hw_offset-buf_desc->ADDR_RX, (int)buf_desc->ADDR_RX); */
  return res;
}


static void
start_receive(USBEndpoint *ep)
{
  unsigned int hw_ep = EP_HW_NUM(ep->addr);
  uint32_t epr =  (USB->EPR[hw_ep] | EPR_W0);
  uint32_t epr_mask = EPR_RW | EPR_W0;
  switch(ep->flags & USB_EP_FLAGS_TYPE_MASK) {
  case USB_EP_FLAGS_TYPE_CONTROL:
  case USB_EP_FLAGS_TYPE_INTERRUPT:
    {
      unsigned int capacity = get_receive_capacity(ep->buffer);
      if (capacity <= ep->xfer_size) {
	/* This is the last OUT packet of the data stage */
	epr ^= USB_EPxR_STAT_TX_NAK;
      } else {
	epr ^= USB_EPxR_STAT_TX_STALL;
      }
      epr ^= USB_EPxR_STAT_RX_VALID;
      epr_mask |= USB_EP0R_STAT_TX | USB_EP0R_STAT_RX;
    }
    break;
  case USB_EP_FLAGS_TYPE_BULK:
  case USB_EP_FLAGS_TYPE_ISO: 
   break;
  }
  ep->flags |= USB_EP_FLAGS_RECEIVING;
  USB->EPR[hw_ep] = epr & epr_mask;
}

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

static int
start_transmit(USBEndpoint *ep)
{
  unsigned int hw_start;
  unsigned int res = 0;
  USBBuffer *buffer = ep->buffer;
  unsigned int len;
  unsigned int hw_offset;
  volatile uint32_t *hw_countp;
  unsigned int hw_ep = EP_HW_NUM(ep->addr);
  uint32_t epr =  USB->EPR[hw_ep];
  unsigned int ep_flags = ep->flags;
  USB_HW_Buffer *buf_desc = USB_EP_BUF_DESC(hw_ep);
  len = ep->xfer_size;
  if (!(ep_flags & USB_EP_FLAGS_ENABLED) || !buffer) return USB_WRITE_BLOCK;
  /* PRINTF("start_transmit: %02x\n", ep->addr); */
  switch(ep_flags & USB_EP_FLAGS_TYPE_MASK) {
  case USB_EP_FLAGS_TYPE_CONTROL:
    
    if (get_transmit_length(ep->buffer) <= len) {
      /* This is the last IN packet of the data stage */
      USB->EPR[hw_ep] = USB_EPxR_EP_STATUS_OUT 
	| EPR_TOGGLE_SET(epr, USB_EP0R_STAT_RX, USB_EPxR_STAT_RX_NAK);
    } else {
      USB->EPR[hw_ep] = USB_EPxR_EP_STATUS_OUT
	| EPR_TOGGLE_SET(epr, USB_EP0R_STAT_RX, USB_EPxR_STAT_RX_STALL);
    }
    hw_offset =  buf_desc->ADDR_TX;
    hw_countp = &buf_desc->COUNT_TX;
    break;
  case USB_EP_FLAGS_TYPE_BULK:
    if (buffer->flags & USB_BUFFER_HALT) {
      if (ep->status & 0x01) return USB_WRITE_BLOCK;
      ep->status |= 0x01;
      stall_bulk_in(hw_ep);
      return USB_WRITE_BLOCK;
    }
    if (USB->EPR[hw_ep] & USB_EPxR_SW_BUF_TX) {
      hw_offset =  buf_desc->ADDR_TX_1;
      hw_countp = &buf_desc->COUNT_TX_1;
    } else {
      hw_offset =  buf_desc->ADDR_TX_0;
      hw_countp = &buf_desc->COUNT_TX_0;
    }
    break;
  }
  hw_start = hw_offset;
  while (buffer) {
    unsigned int copy;
    if (buffer->left < len) {
      copy = buffer->left;
    } else {
      copy = len;
    }
    len -= copy;
    buffer->left -= copy;
    write_hw_buffer(buffer, hw_offset, copy);
    hw_offset += copy;
    if (buffer->left == 0) {
      if (buffer->flags & USB_BUFFER_SHORT_END) {
	if (len == 0) {
	  /* Send zero length packet. */
	  break; /* Leave without moving to next buffer */
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
  *hw_countp = hw_offset - hw_start;
  /* printf("start_transmit: %02x %d %04lx\n", ep->addr, hw_offset - hw_start, USB->EPR[hw_ep]); */
  switch(ep->flags & USB_EP_FLAGS_TYPE_MASK) {
  case USB_EP_FLAGS_TYPE_CONTROL:
  case USB_EP_FLAGS_TYPE_INTERRUPT:
    USB->EPR[hw_ep] =
      EPR_TOGGLE_SET(epr, USB_EP0R_STAT_TX, USB_EPxR_STAT_TX_VALID);
    break;
  case USB_EP_FLAGS_TYPE_BULK:
    USB->EPR[hw_ep] = EPR_INVARIANT(USB->EPR[hw_ep]) | USB_EPxR_SW_BUF_TX;
    break;
  case USB_EP_FLAGS_TYPE_ISO:
    break;
  }
/*   printf("start_transmit: %04x\n", USB->EPR[hw_ep]); */
  return res;
}

static void
start_transfer(USBEndpoint *ep)
{
  int res;
  while (1) {
    if (!(ep->addr & 0x80) && (IS_BULK_EP(ep) || IS_INTERRUPT_EP(ep))) {
      if (ep->buffer && (ep->buffer->flags & USB_BUFFER_HALT)) {
	if (ep->status & 0x01) return ;
	ep->status |= 0x01;
	stall_bulk_out(EP_HW_NUM(ep->addr));
	return;
      }
    }
    if (!(ep->flags & USB_EP_FLAGS_RECV_PENDING)) break;
    res = handle_pending_receive(ep);
    if (res & USB_READ_NOTIFY) {
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
    }
    if (res & USB_READ_BLOCK) return;
    if (ep->flags & USB_EP_FLAGS_DOUBLE) {
      ep->flags &= ~USB_EP_FLAGS_DOUBLE;
    } else {
      ep->flags &= ~(USB_EP_FLAGS_RECV_PENDING|USB_EP_FLAGS_SETUP_PENDING);
    }
    if (res & USB_READ_FAIL) {
      /* Only fails for control endpoints */
      usb_arch_control_stall(ep->addr);
      return;
    }
  }
  if (ep->addr == 0x02)
    PRINTF("start EPR: %04x ep->flags: %02x\n",
	   (unsigned int)USB->EPR[EP_HW_NUM(ep->addr)],
	   (unsigned int)ep->flags);
  if (ep->flags & (USB_EP_FLAGS_TRANSMITTING | USB_EP_FLAGS_RECEIVING)) {
    if (!IS_BULK_EP(ep) || (ep->flags & USB_EP_FLAGS_DOUBLE)) {
      PRINTF("Busy\n");
      return;
    }
  }
  if (ep->status & 0x01) return; /* Don't start transfer if halted */
  if (ep->buffer) {
    if (ep->buffer->flags & USB_BUFFER_IN) {
      res = start_transmit(ep);
      if (res & USB_READ_NOTIFY) {
	notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
      }
    } else {
      start_receive(ep);
    }
  }
}


static void
transfer_complete(unsigned int hw_ep) {
  uint32_t epr =  USB->EPR[hw_ep];
  USBEndpoint *ep = &usb_endpoints[hw_ep];
  if (epr &USB_EP0R_CTR_RX) {
    PRINTF("Received packet %lx %04x\n", USB_EP_BUF_DESC(hw_ep)->COUNT_RX, (int)USB->EPR[hw_ep]);
    if (epr & USB_EP0R_SETUP) {
      PRINTF("SETUP\n"); 
      ep->flags |= USB_EP_FLAGS_SETUP_PENDING;
    }

    if (IS_BULK_EP(ep)) {
      if ((epr ^ (epr >> 8)) & USB_EP0R_DTOG_TX) {
	ep->flags &= ~USB_EP_FLAGS_DOUBLE;
      } else {
	ep->flags |= USB_EP_FLAGS_DOUBLE;
	ep->flags &= ~USB_EP_FLAGS_RECEIVING;
      }
    } else {
      ep->flags &= ~USB_EP_FLAGS_RECEIVING;
    }
    ep->flags |= USB_EP_FLAGS_RECV_PENDING;
    if (IS_CONTROL_EP(ep)) epr &= ~USB_EPxR_EP_STATUS_OUT;
    USB->EPR[hw_ep] = EPR_INVARIANT(epr) & ~USB_EP0R_CTR_RX;
#if 0
    if (ep->flags & USB_EP_FLAGS_DOUBLE) {
      printf("Double\n");
    }
#endif

    start_transfer(ep);
  }
  if (epr &USB_EP0R_CTR_TX) {
     PRINTF("Sent packet\n"); 
    if (ep->flags & USB_EP_FLAGS_DOUBLE) {
      ep->flags &= ~USB_EP_FLAGS_DOUBLE;
    } else {
      ep->flags &= ~USB_EP_FLAGS_TRANSMITTING;
    }
    USB->EPR[hw_ep] = EPR_INVARIANT(USB->EPR[hw_ep]) & ~USB_EP0R_CTR_TX;
    start_transfer(ep);
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
  USB_DISABLE_INT;
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
  uint32_t ints;
  USBBuffer *buffer;
  volatile USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
  ints = usb_save_disable_int();
  buffer = ep->buffer;
  ep->buffer = NULL;
#if 0
  /* Set both directions to NAK */
  USB->EPR[EP_HW_NUM(ep_addr)] =
    EPR_TOGGLE_SET(USB->EPR[EP_HW_NUM(ep_addr)],
		   USB_EP0R_STAT_RX|USB_EP0R_STAT_TX,
		   USB_EPxR_STAT_TX_NAK| USB_EPxR_STAT_RX_NAK);
  ep->flags &= ~(USB_EP_FLAGS_RECV_PENDING|USB_EP_FLAGS_SETUP_PENDING);
#endif
  ep->flags &= ~(USB_EP_FLAGS_RECV_PENDING | USB_EP_FLAGS_SETUP_PENDING
		 | USB_EP_FLAGS_DOUBLE);
  usb_restore_int(ints);
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
  uint32_t epr =  USB->EPR[EP_HW_NUM(addr)];
  USB->EPR[EP_HW_NUM(addr)] = EPR_TOGGLE_SET(epr,USB_EP0R_STAT_RX|USB_EP0R_STAT_TX, USB_EPxR_STAT_RX_STALL | USB_EPxR_STAT_TX_STALL);
}

/* Not for control endpoints */
void
usb_arch_halt_endpoint(unsigned char ep_addr, int halt)
{
  if (EP_INDEX(ep_addr) > USB_MAX_ENDPOINTS) return;
  if (!usb_endpoints[EP_INDEX(ep_addr)].flags & USB_EP_FLAGS_ENABLED) return;
  USB_DISABLE_INT;
  if (halt) {
    if (!(usb_endpoints[EP_INDEX(ep_addr)].status & 0x01)) {
      usb_endpoints[EP_INDEX(ep_addr)].status |= 0x01;
      if (ep_addr & 0x80) {
	stall_bulk_in(EP_HW_NUM(ep_addr));
      } else {
	stall_bulk_out(EP_HW_NUM(ep_addr));
      }
    }
  } else {
    USBEndpoint *ep = &usb_endpoints[EP_INDEX(ep_addr)];
    if (ep->status & 0x01) {
      ep->status &= ~0x01;
      PRINTF("HALT clear restart EPR: %04x %p %p\n",
	     (unsigned int)USB->EPR[EP_HW_NUM(ep_addr)],
	     ep->buffer, ep->buffer->next);
      /* Restore toggle state for double buffered endpoint */
      if (IS_BULK_EP(ep)) {
	volatile uint32_t *eprp = &USB->EPR[EP_HW_NUM(ep_addr)];
	if (ep_addr & 0x80) {
	  ep->flags &= ~(USB_EP_FLAGS_DOUBLE |USB_EP_FLAGS_TRANSMITTING);
	  
 	  *eprp =(EPR_TOGGLE_SET(*eprp,(USB_EP0R_STAT_TX | USB_EP0R_DTOG_TX 
					| USB_EPxR_SW_BUF_TX),
				 USB_EPxR_STAT_TX_VALID)); 
	} else {
	  ep->flags &= ~(USB_EP_FLAGS_DOUBLE | USB_EP_FLAGS_RECEIVING
			 | USB_EP_FLAGS_RECV_PENDING);

 	  *eprp =(EPR_TOGGLE_SET(*eprp,(USB_EP0R_STAT_RX | USB_EP0R_DTOG_RX 
					| USB_EPxR_SW_BUF_RX),
				 USB_EPxR_STAT_RX_VALID|USB_EPxR_SW_BUF_RX));
	  *eprp = EPR_INVARIANT(*eprp) | USB_EPxR_SW_BUF_RX;
	  
	}
      }
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
      PRINTF("HALT clear restart EPR: %04x %p %p\n",
	     (unsigned int)USB->EPR[EP_HW_NUM(ep_addr)],
	     ep->buffer, ep->buffer->next);
      
    }
  }
  USB_ENABLE_INT;
}

void
usb_arch_set_address(unsigned char addr)
{
  USB->DADDR = 0x80 | addr;  
}

void
USB_HP_CAN_TX_handler(void) __attribute__((interrupt));

void
USB_HP_CAN_TX_handler(void)
{
  uint32_t status = USB->ISTR;
  if (status & USB_ISTR_CTR) {
    transfer_complete(status & USB_ISTR_EP_ID);
  }
}

void
USB_LP_CAN_RX0_handler(void) __attribute__((interrupt));
void
USB_LP_CAN_RX0_handler(void)
{
  uint32_t status = USB->ISTR;
  if (status & USB_ISTR_CTR) {
    transfer_complete(status & USB_ISTR_EP_ID);
    /* PRINTF("Transfer complete ep %ld\n", status & USB_ISTR_EP_ID); */
  } else if (status & USB_ISTR_PMAOVR) {
    PRINTF("PMAOVR\n");
    USB->ISTR &= ~USB_ISTR_PMAOVR;
  } else if (status & USB_ISTR_ERR) {
    PRINTF("ERR\n");
    USB->ISTR &= ~USB_ISTR_ERR;
  } else if (status & USB_ISTR_WKUP) {
    PRINTF("WKUP\n");
    USB->ISTR &= ~USB_ISTR_WKUP;
    USB->CNTR &= ~USB_CNTR_FSUSP;
    notify_process(USB_EVENT_RESUME);
  } else if (status & USB_ISTR_SUSP) {
    PRINTF("SUSP\n");
    USB->ISTR &= ~USB_ISTR_SUSP;
    USB->CNTR |= USB_CNTR_FSUSP;
    notify_process(USB_EVENT_SUSPEND);
  } else if (status & USB_ISTR_RESET) {
    PRINTF("RESET\n");
    USB->ISTR &= ~USB_ISTR_RESET;
    usb_arch_reset();
    notify_process(USB_EVENT_RESET);
  }
}

void
usb_arch_toggle_SW_BUF_RX()
{
  USB->EPR[2] = EPR_INVARIANT(USB->EPR[2]) | USB_EPxR_SW_BUF_RX;
}

int 
usb_arch_send_pending(uint8_t ep_addr)
{
  return usb_endpoints[EP_INDEX(ep_addr)].flags & USB_EP_FLAGS_TRANSMITTING;
}

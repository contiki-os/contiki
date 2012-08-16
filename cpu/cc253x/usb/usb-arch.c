/*
Copyright (c) 2012, Philippe Retornaz
Copyright (c) 2012, EPFL STI IMT LSRO1 -- Mobots group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote
   products derived from this software without specific prior
   written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "contiki.h"
#include "usb-arch.h"
#include "port2.h"

#include "cc253x.h"
#include "sfr-bits.h"
#include "port.h"

#include "dma.h"

// P1_0
#define USB_PULLUP_PORT 	1
#define USB_PULLUP_PIN		0

#define USBCTRL_USB_EN 		(1 << 0)
#define USBCTRL_PLL_EN 		(1 << 1)
#define USBCTRL_PLL_LOCKED	(1 << 7)

#define USBIIE_EP0IE		(1 << 0)
#define USBIIE_INEPxIE(x)	(1 << x)

#define USBOIE_OUEPxIE(x)	(1 << x)

#define USBCSxH_ISO		(1 << 6)

#define USBCS0_CLR_SETUP_END	(1 << 7)
#define USBCS0_CLR_OUTPKT_RDY	(1 << 6)
#define USBCS0_SEND_STALL	(1 << 5)
#define USBCS0_SETUP_END	(1 << 4)
#define USBCS0_DATA_END		(1 << 3)
#define USBCS0_SENT_STALL	(1 << 2)
#define USBCS0_INPKT_RDY	(1 << 1)
#define USBCS0_OUTPKT_RDY	(1 << 0)

#define USBCSIL_SENT_STALL	(1 << 5)
#define USBCSIL_SEND_STALL	(1 << 4)
#define USBCSIL_FLUSH_PACKET	(1 << 3)
#define USBCSIL_UNDERRUN	(1 << 2)
#define USBCSIL_PKT_PRESENT	(1 << 1)
#define USBCSIL_INPKT_RDY	(1 << 0)

#define USBCSOL_SENT_STALL	(1 << 6)
#define USBCSOL_SEND_STALL	(1 << 5)
#define USBCSOL_OVERRUN		(1 << 2)
#define USBCSOL_OUTPKT_RDY	(1 << 0)

// Double buffering not used
// We assume only the IN or the OUT of
// each endpoint is enabled and not both.
#if CTRL_EP_SIZE > 32
#error Control endpoint size too big
#endif

#if USB_EP1_SIZE > 32
#error Endpoint 1 size too big
#endif

#if USB_EP2_SIZE > 64
#error Endpoint 2 size too big
#endif

#if USB_EP3_SIZE > 128
#error Endpoint 3 size too big
#endif

#if USB_EP4_SIZE > 256
#error Endpoint 4 size too big
#endif

#if USB_EP5_SIZE > 512
#error Endpoint 5 size too big
#endif


static const uint16_t ep_xfer_size[] = {
  CTRL_EP_SIZE,
  USB_EP1_SIZE,
  USB_EP2_SIZE,
  USB_EP3_SIZE,
  USB_EP4_SIZE,
  USB_EP5_SIZE,
};


#define EP_IDLE 0
#define EP_RX 1
#define EP_TX 2
static uint8_t ep0status;

typedef struct _USBEndpoint USBEndpoint;
struct _USBEndpoint {
  uint8_t halted;
  uint8_t addr;
  uint8_t flags;
  USBBuffer *buffer;            /* NULL if no current buffer */
  struct process *event_process;
  unsigned int events;
  uint16_t xfer_size;
};

#define USB_EP_FLAGS_TYPE_MASK 0x03
#define USB_EP_FLAGS_TYPE_BULK 0x00
#define USB_EP_FLAGS_TYPE_CONTROL 0x01
#define USB_EP_FLAGS_TYPE_ISO 0x02
#define USB_EP_FLAGS_TYPE_INTERRUPT 0x03

#define USB_EP_FLAGS_ENABLED 	0x04

#define EP_TYPE(ep) ((ep)->flags & USB_EP_FLAGS_TYPE_MASK)
#define IS_EP_TYPE(ep, type) (EP_TYPE(ep) == (type))
#define IS_CONTROL_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_CONTROL)
#define IS_BULK_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_BULK)
#define IS_INTERRUPT_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_INTERRUPT)
#define IS_ISO_EP(ep) IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_ISO)


#define USB_WRITE_BLOCK 0x01
#define USB_READ_BLOCK 0x01     /* The currently submitted buffers
                                   can't hold the received data, wait
                                   for more buffers. No data was read
                                   from the hardware buffer */
#define USB_WRITE_NOTIFY 0x02
#define USB_READ_NOTIFY 0x02    /* Some buffers that had the
                                   USB_BUFFER_NOTIFY flags set were
                                   released */
#define USB_READ_FAIL	0x4


/* Index in endpoint array */
#define EP_INDEX(addr) ((addr) & 0x7f)

/* Get address of endpoint struct */
#define EP_STRUCT(addr) &usb_endpoints[EP_INDEX(addr)];

/* Number of hardware endpoint */
#define EP_HW_NUM(addr) ((addr) & 0x7f)

#define usb_irq_disable(flag) cc253x_p2_irq_disable(flag)
#define usb_irq_enable(flag) cc253x_p2_irq_enable(flag)

static uint8_t usb_irq(void);
static void usb_arch_epin_irq(uint8_t ep_hw);
static void usb_arch_epout_irq(uint8_t ep_hw);
static void usb_arch_ep0_irq(void);

static struct cc253x_p2_handler usb_irq_handler = { NULL, usb_irq };

static USBEndpoint usb_endpoints[USB_MAX_ENDPOINTS];
struct process *event_process = 0;
volatile static unsigned int events = 0;

static uint8_t ep0_tx(void);
static uint8_t ep_tx(uint8_t ep_hw);

static void
notify_process(unsigned int e)
{
  events |= e;
  if(event_process) {
    process_poll(event_process);
  }
}

static void
notify_ep_process(USBEndpoint * ep, unsigned int e)
{
  ep->events |= e;
  if(ep->event_process) {
    process_poll(ep->event_process);
  }
}


void
usb_set_ep_event_process(unsigned char addr, struct process *p)
{
  USBEndpoint *ep = EP_STRUCT(addr);

  ep->event_process = p;
}


void
usb_arch_set_global_event_process(struct process *p)
{
  event_process = p;
}

unsigned int
usb_arch_get_global_events(void)
{
  uint8_t flag;
  volatile unsigned int e;

  usb_irq_disable(flag);
  e = events;
  events = 0;
  usb_irq_enable(flag);

  return e;
}

unsigned int
usb_get_ep_events(uint8_t addr)
{
  volatile unsigned int e;
  uint8_t flag;
  USBEndpoint *ep = EP_STRUCT(addr);

  usb_irq_disable(flag);
  e = ep->events;
  ep->events = 0;
  usb_irq_enable(flag);

  return e;
}
#if DMA_ON
#ifndef DMA_USB_CHANNEL
#error You must set DMA_USB_CHANNEL to a valid dma channel.
#endif
static void
read_hw_buffer_dma(uint8_t tl, uint8_t th, uint8_t __xdata * xptr,
                   unsigned int len)
{
  dma_conf[DMA_USB_CHANNEL].src_h = ((uint16_t) xptr) >> 8;
  dma_conf[DMA_USB_CHANNEL].src_l = ((uint16_t) xptr) & 0xFF;
  dma_conf[DMA_USB_CHANNEL].dst_h = th;
  dma_conf[DMA_USB_CHANNEL].dst_l = tl;

  // Maximum USB len transfert is 512bytes, maximum DMA len: 4096, we are safe.
  dma_conf[DMA_USB_CHANNEL].len_h = len >> 8;
  dma_conf[DMA_USB_CHANNEL].len_l = len & 0xFF;
  dma_conf[DMA_USB_CHANNEL].wtt = DMA_T_NONE | DMA_BLOCK;
  // Maximum prio, we will be polling until the transfert is done.
  dma_conf[DMA_USB_CHANNEL].inc_prio =
    DMA_SRC_INC_NO | DMA_DST_INC_1 | DMA_PRIO_HIGH;

  DMA_ARM(DMA_USB_CHANNEL);
  // Wait until the channel is armed
  while(!(DMAARM & (1 << DMA_USB_CHANNEL)));

  DMA_TRIGGER(DMA_USB_CHANNEL);
  // Wait until the transfert is done.
  // For some unknown reason, the DMA channel do not set the IRQ flag
  // sometimes, so use the DMAARM bit to check if transfert is done
  while(DMAARM & (1 << DMA_USB_CHANNEL));
  // Clear interrupt flag
  DMAIRQ = ~(1 << DMA_USB_CHANNEL);
}
static void
write_hw_buffer_dma(uint8_t __xdata * xptr, uint8_t fl, uint8_t fh,
                    unsigned int len)
{
  dma_conf[DMA_USB_CHANNEL].src_h = fh;
  dma_conf[DMA_USB_CHANNEL].src_l = fl;
  dma_conf[DMA_USB_CHANNEL].dst_h = ((uint16_t) xptr) >> 8;
  dma_conf[DMA_USB_CHANNEL].dst_l = ((uint16_t) xptr) & 0xFF;

  // Maximum USB len transfert is 512bytes, maximum DMA len: 4096, we are safe.
  dma_conf[DMA_USB_CHANNEL].len_h = len >> 8;
  dma_conf[DMA_USB_CHANNEL].len_l = len & 0xFF;
  dma_conf[DMA_USB_CHANNEL].wtt = DMA_T_NONE | DMA_BLOCK;
  // Maximum prio, we will be polling until the transfert is done.
  dma_conf[DMA_USB_CHANNEL].inc_prio =
    DMA_SRC_INC_1 | DMA_DST_INC_NO | DMA_PRIO_HIGH;

  DMA_ARM(DMA_USB_CHANNEL);
  // Wait until the channel is armed
  while(!(DMAARM & (1 << DMA_USB_CHANNEL)));

  DMA_TRIGGER(DMA_USB_CHANNEL);
  // Wait until the transfert is done.
  while(DMAARM & (1 << DMA_USB_CHANNEL));

  // Clear interrupt flag
  DMAIRQ = ~(1 << DMA_USB_CHANNEL);
}

#endif
static void
read_hw_buffer(uint8_t * to, uint8_t hw_ep, unsigned int len)
{
  __xdata uint8_t *from = &USBF0 + (hw_ep << 1);

#if DMA_ON
  // For small transfers we use PIO
  // This check is specific to SDCC and large/huge memory model
  if(len > 8 && ((uint8_t *) & to)[2] == 0x0 /* x data pointer */ ) {
    read_hw_buffer_dma(((uint8_t *) & to)[0], ((uint8_t *) & to)[1], from,
                       len);
    return;
  }
#endif
  while(len--) {
    *to++ = *from;
  }
}

static void
write_hw_buffer(uint8_t hw_ep, uint8_t * from, unsigned int len)
{
  __xdata uint8_t *to = &USBF0 + (hw_ep << 1);

#if DMA_ON
  // For small transfers we use PIO
  if(len > 8 && ((uint8_t *) & from)[2] == 0x0 /* x data pointer */ ) {
    write_hw_buffer_dma(to, ((uint8_t *) & from)[0], ((uint8_t *) & from)[1],
                        len);
    return;
  }
#endif
  while(len--) {
    *to = *from++;
  }
}

static void
usb_arch_reset(void)
{
  uint8_t e;

  for(e = 0; e < USB_MAX_ENDPOINTS; e++) {
    if(usb_endpoints[e].flags & USB_EP_FLAGS_ENABLED) {
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

/* Init USB */
void
usb_arch_setup(void)
{
  uint8_t i;

  /* Switch on USB PLL & USB module */
  USBCTRL = USBCTRL_USB_EN | USBCTRL_PLL_EN;

  /* Wait until USB PLL is stable */
  while(!(USBCTRL & USBCTRL_PLL_LOCKED));

  /* Enable pull-up on usb port */
  PORT_SET(USB_PULLUP_PORT, USB_PULLUP_PIN);
  PORT_DIR_OUTPUT(USB_PULLUP_PORT, USB_PULLUP_PIN);

  for(i = 0; i < USB_MAX_ENDPOINTS; i++) {
    usb_endpoints[i].flags = 0;
    usb_endpoints[i].event_process = 0;
  }

  usb_arch_reset();

  // Disable all endpoints interrupts
  // Enpoint 0 interrupt will be enabled later
  USBIIE = 0;
  USBOIE = 0;

  cc253x_p2_register_handler(&usb_irq_handler);
  // We have to force IRQ enable as we might be the first user
  cc253x_p2_irq_force_enable();
}

void
usb_submit_recv_buffer(uint8_t addr, USBBuffer * buffer)
{
  USBBuffer **tailp;
  uint8_t flag;
  USBEndpoint *ep = EP_STRUCT(addr);

  if(!(ep->flags & USB_EP_FLAGS_ENABLED)) {
    return;
  }

  if(buffer->data == NULL && EP_HW_NUM(addr) == 0) {
    // the usb-core is trying to get the STATUS packet (ZLP in this case)
    // but the USB hardware is catching them, thus ignore this.
    // FIXME: Use the interrupt to release this buffer when the packet is sent
    // at least the timing would be better ...
    if(buffer->flags & USB_BUFFER_NOTIFY) {
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
    }
    return;
  }

  usb_irq_disable(flag);

  tailp = &ep->buffer;
  while(*tailp) {
    tailp = &(*tailp)->next;
  }
  *tailp = buffer;
  while(buffer) {
    buffer->flags |= USB_BUFFER_SUBMITTED;
    buffer = buffer->next;
  }

  USBINDEX = EP_HW_NUM(addr);
  if(!EP_HW_NUM(ep->addr)) {
    if(USBCS0 & USBCS0_OUTPKT_RDY) {
      usb_arch_ep0_irq();
    }
  } else {
    if(USBCSOL & USBCSOL_OUTPKT_RDY) {
      usb_arch_epout_irq(EP_HW_NUM(ep->addr));
    }
  }
  usb_irq_enable(flag);
}

void
usb_submit_xmit_buffer(uint8_t addr, USBBuffer * buffer)
{
  USBBuffer **tailp;
  uint8_t flag;
  uint8_t res;
  USBEndpoint *ep = EP_STRUCT(addr);

  if(!(ep->flags & USB_EP_FLAGS_ENABLED)) {
    return;
  }

  usb_irq_disable(flag);


  if(EP_HW_NUM(addr) == 0) {
    if(buffer->data == NULL) {
      // We are asked to send a STATUS packet.
      // But the USB hardware is doing this automatically
      // as soon as we release hw FIFO.
      USBINDEX = 0;
      USBCS0 = USBCS0_CLR_OUTPKT_RDY | USBCS0_DATA_END;
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
      usb_irq_enable(flag);
      return;
    } else {
      // Release the hw FIFO ...
      USBINDEX = 0;
      USBCS0 = USBCS0_CLR_OUTPKT_RDY;
    }
  }

  tailp = &ep->buffer;
  while(*tailp) {
    tailp = &(*tailp)->next;
  }
  *tailp = buffer;
  while(buffer) {
    buffer->flags |= USB_BUFFER_SUBMITTED | USB_BUFFER_IN;
    buffer = buffer->next;
  }

  USBINDEX = EP_HW_NUM(ep->addr);
  if(EP_HW_NUM(ep->addr)) {
    res = ep_tx(EP_HW_NUM(ep->addr));
  } else {
    res = ep0_tx();
  }

  usb_irq_enable(flag);

  if(res & USB_WRITE_NOTIFY) {
    notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
  }

}


static void
usb_arch_setup_endpoint0(void)
{
  USBIIE |= USBIIE_EP0IE;
}

static void
usb_arch_setup_in_endpoint(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  USBEndpoint *ep = EP_STRUCT(addr);

  // Enable interrupt
  USBIIE |= USBIIE_INEPxIE(ei);

  // Set internal FIFO size
  USBMAXI = ep->xfer_size / 8;

  if(IS_ISO_EP(ep)) {
    USBCSIH |= USBCSxH_ISO;
  } else {
    USBCSIH &= ~USBCSxH_ISO;
  }
}

static void
usb_arch_setup_out_endpoint(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  USBEndpoint *ep = EP_STRUCT(addr);

  // Enable interrupt
  USBOIE |= USBOIE_OUEPxIE(ei);

  // Set internal FIFO size
  USBMAXO = ep->xfer_size / 8;

  if(IS_ISO_EP(ep)) {
    USBCSOH |= USBCSxH_ISO;
  } else {
    USBCSOH &= ~USBCSxH_ISO;
  }
}

static void
usb_arch_setup_endpoint(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  uint8_t flag;
  USBEndpoint *ep = EP_STRUCT(addr);

  ep->halted = 0;
  ep->flags |= USB_EP_FLAGS_ENABLED;
  ep->buffer = 0;
  ep->addr = addr;
  ep->events = 0;
  ep->xfer_size = ep_xfer_size[ei];

  usb_irq_disable(flag);
  // Select endpoint banked register
  USBINDEX = ei;

  // special case for ep 0
  if(ei == 0) {
    usb_arch_setup_endpoint0();
  } else {
    if(addr & 0x80) {
      usb_arch_setup_in_endpoint(addr);
    } else {
      usb_arch_setup_out_endpoint(addr);
    }
  }
  usb_irq_enable(flag);
}

void
usb_arch_setup_iso_endpoint(uint8_t addr)
{
  USBEndpoint *ep = EP_STRUCT(addr);

  ep->flags = USB_EP_FLAGS_TYPE_ISO;

  usb_arch_setup_endpoint(addr);
}

void
usb_arch_setup_control_endpoint(uint8_t addr)
{
  USBEndpoint *ep = EP_STRUCT(addr);

  ep->flags = USB_EP_FLAGS_TYPE_CONTROL;

  usb_arch_setup_endpoint(addr);
}

void
usb_arch_setup_bulk_endpoint(uint8_t addr)
{
  USBEndpoint *ep = EP_STRUCT(addr);

  ep->flags = USB_EP_FLAGS_TYPE_BULK;

  usb_arch_setup_endpoint(addr);
}

void
usb_arch_setup_interrupt_endpoint(uint8_t addr)
{
  USBEndpoint *ep = EP_STRUCT(addr);

  ep->flags = USB_EP_FLAGS_TYPE_INTERRUPT;

  usb_arch_setup_endpoint(addr);
}

static void
usb_arch_disable_ep0(void)
{
  USBIIE &= ~USBIIE_EP0IE;
  USBCS0 = 0xC0;                // Clear any pending status flags
}

static void
usb_arch_disable_in_endpoint(uint8_t addr)
{
  USBMAXI = 0;
  USBIIE &= ~USBIIE_INEPxIE(EP_HW_NUM(addr));

  // Flush any pending packets
  USBCSIL = 0x08;               // Double-buffering not used. Flush only once
}

static void
usb_arch_disable_out_endpoint(uint8_t addr)
{
  USBMAXO = 0;
  USBOIE &= ~USBOIE_OUEPxIE(EP_HW_NUM(addr));

  // Flush any pending packets
  USBCSOL = 0x08;               // Double buffering not used, flush only once
}


void
usb_arch_disable_endpoint(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  uint8_t flag;
  USBEndpoint *ep = EP_STRUCT(addr);

  ep->flags &= ~USB_EP_FLAGS_ENABLED;

  usb_irq_disable(flag);
  USBINDEX = ei;
  if(ei == 0) {
    usb_arch_disable_ep0();
  } else {
    if(addr & 0x80) {
      usb_arch_disable_in_endpoint(addr);
    } else {
      usb_arch_disable_out_endpoint(addr);
    }
  }
  usb_irq_enable(flag);
}

void
usb_arch_discard_all_buffers(uint8_t addr)
{
  USBBuffer *buffer;
  uint8_t flag;
  volatile USBEndpoint *ep = EP_STRUCT(addr);

  usb_irq_disable(flag);
  buffer = ep->buffer;
  ep->buffer = NULL;
  usb_irq_enable(flag);

  while(buffer) {
    buffer->flags &= ~USB_BUFFER_SUBMITTED;
    buffer = buffer->next;
  }
}

static void
set_stall(uint8_t addr, uint8_t stall)
{
  uint8_t ei = EP_HW_NUM(addr);

  USBINDEX = ei;
  if(ei == 0) {
    // Stall is automatically deasserted on EP0
    if(stall) {
      ep0status = EP_IDLE;
      USBCS0 |= USBCS0_SEND_STALL | USBCS0_OUTPKT_RDY;
    }
  } else {
    if(addr & 0x80) {
      if(stall) {
        USBCSIL |= USBCSIL_SEND_STALL;
      } else {
        USBCSIL &= ~USBCSIL_SEND_STALL;
      }
    } else {
      if(stall) {
        USBCSOL |= USBCSOL_SEND_STALL;
      } else {
        USBCSOL &= ~USBCSOL_SEND_STALL;
      }
    }
  }
}

void
usb_arch_control_stall(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  uint8_t flag;

  if(ei > USB_MAX_ENDPOINTS) {
    return;
  }

  usb_irq_disable(flag);

  set_stall(addr, 1);

  usb_irq_enable(flag);
}

void
usb_arch_halt_endpoint(uint8_t addr, int halt)
{
  uint8_t ei = EP_HW_NUM(addr);
  uint8_t flag;
  USBEndpoint *ep = EP_STRUCT(addr);


  if(ei > USB_MAX_ENDPOINTS) {
    return;
  }

  if(!(ep->flags & USB_EP_FLAGS_ENABLED)) {
    return;
  }

  usb_irq_disable(flag);

  if(halt) {
    ep->halted = 0x1;
    set_stall(addr, 1);
  } else {
    ep->halted = 0;
    set_stall(addr, 0);

    if(ep->buffer && (ep->buffer->flags & USB_BUFFER_HALT)) {
      ep->buffer->flags &= ~USB_BUFFER_SUBMITTED;
      if(ep->buffer->flags & USB_BUFFER_NOTIFY) {
        notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
      }
      ep->buffer = ep->buffer->next;
    }
    if(ei) {
      usb_arch_epout_irq(EP_HW_NUM(addr));
    }
  }

  usb_irq_enable(flag);
}

void
usb_arch_set_configuration(uint8_t usb_configuration_value)
{
  // Nothing to do ?
}

uint16_t
usb_arch_get_ep_status(uint8_t addr)
{
  uint8_t ei = EP_INDEX(addr);
  USBEndpoint *ep = EP_STRUCT(addr);

  if(ei > USB_MAX_ENDPOINTS) {
    return 0;
  }

  return ep->halted;
}

void
usb_arch_set_address(uint8_t addr)
{
  USBADDR = addr;
}

int
usb_arch_send_pending(uint8_t addr)
{
  uint8_t flag;
  uint8_t ret;
  uint8_t ei = EP_INDEX(addr);

  usb_irq_disable(flag);
  USBINDEX = ei;
  if(ei == 0) {
    ret = USBCS0 & USBCS0_INPKT_RDY;
  } else {
    ret = USBCSIL & USBCSIL_INPKT_RDY;
  }
  usb_irq_enable(flag);

  return ret;
}

static unsigned int
get_receive_capacity(USBBuffer * buffer)
{
  unsigned int capacity = 0;

  while(buffer &&
        !(buffer->flags & (USB_BUFFER_IN | USB_BUFFER_SETUP | USB_BUFFER_HALT))) {
    capacity += buffer->left;
    buffer = buffer->next;
  }
  return capacity;
}

static USBBuffer *
skip_buffers_until(USBBuffer * buffer, unsigned int mask, unsigned int flags,
                   uint8_t * resp)
{
  while(buffer && !((buffer->flags & mask) == flags)) {
    buffer->flags &= ~USB_BUFFER_SUBMITTED;
    buffer->flags |= USB_BUFFER_FAILED;
    if(buffer->flags & USB_BUFFER_NOTIFY) {
      *resp |= USB_READ_NOTIFY;
    }
    buffer = buffer->next;
  }
  return buffer;
}

static uint8_t
fill_buffers(USBBuffer * buffer, uint8_t hw_ep, unsigned int len,
             uint8_t short_packet)
{
  unsigned int t;
  uint8_t res = 0;

  do {
    if(buffer->left < len) {
      t = buffer->left;
    } else {
      t = len;
    }
    len -= t;
    buffer->left -= t;

    read_hw_buffer(buffer->data, hw_ep, t);

    buffer->data += t;

    if(len == 0) {
      break;
    }

    buffer->flags &= ~(USB_BUFFER_SUBMITTED | USB_BUFFER_SHORT_PACKET);
    if(buffer->flags & USB_BUFFER_NOTIFY) {
      res |= USB_READ_NOTIFY;
    }
    buffer = buffer->next;
  } while(1);

  if(short_packet) {
    buffer->flags |= USB_BUFFER_SHORT_PACKET;
  }

  if((buffer->left == 0) || (buffer->flags & USB_BUFFER_PACKET_END)) {
    buffer->flags &= ~USB_BUFFER_SUBMITTED;
    if(buffer->flags & USB_BUFFER_NOTIFY) {
      res |= USB_READ_NOTIFY;
    }
    buffer = buffer->next;
  } else {
    if(short_packet) {
      if(buffer->left && !(buffer->flags & USB_BUFFER_SHORT_END)) {
        buffer->flags |= USB_BUFFER_FAILED;
        res |= USB_READ_FAIL;
      }
      buffer->flags &= ~USB_BUFFER_SUBMITTED;
      if(buffer->flags & USB_BUFFER_NOTIFY) {
        res |= USB_READ_NOTIFY;
      }
      buffer = buffer->next;
    }
  }

  usb_endpoints[hw_ep].buffer = buffer;

  return res;
}


static uint8_t
ep0_get_setup_pkt(void)
{
  // The USB controller check that the packet size is == 8
  // First get a valid setup buffer
  uint8_t res;
  USBBuffer *buffer =
    skip_buffers_until(usb_endpoints[0].buffer, USB_BUFFER_SETUP,
                       USB_BUFFER_SETUP, &res);

  usb_endpoints[0].buffer = buffer;

  if(!buffer || buffer->left < 8) {
    return USB_READ_BLOCK;
  }

  read_hw_buffer(buffer->data, 0, 8);
  buffer->left -= 8;

  buffer->flags &= ~USB_BUFFER_SUBMITTED;
  if(buffer->flags & USB_BUFFER_NOTIFY) {
    res |= USB_READ_NOTIFY;
  }

  if(buffer->data[6] || buffer->data[7]) {
    // DATA stage ...
    USBCS0 |= USBCS0_CLR_OUTPKT_RDY;
    ep0status = buffer->data[0] & 0x80 ? EP_TX : EP_RX;
  } else {
    // The usb-core will submit an empty data packet
    // This will trigger the clr (see the buffer submission routine)
//              USBCS0 |= USBCS0_CLR_OUTPKT_RDY | USBCS0_DATA_END;
  }

  buffer->data += 8;

  usb_endpoints[0].buffer = buffer->next;

  return res;
}

static uint8_t
ep0_get_data_pkt(void)
{
  uint8_t res = 0;
  uint8_t short_packet = 0;
  USBBuffer *buffer = usb_endpoints[0].buffer;
  uint8_t len = USBCNT0;

  if(!buffer) {
    return USB_READ_BLOCK;
  }

  if(buffer->flags & (USB_BUFFER_SETUP | USB_BUFFER_IN)) {
    uint8_t temp;

    buffer->flags |= USB_BUFFER_FAILED;
    buffer->flags &= ~USB_BUFFER_SUBMITTED;
    if(buffer->flags & USB_BUFFER_NOTIFY) {
      res |= USB_READ_NOTIFY;
    }
    // Flush the fifo
    while(len--) {
      temp = USBF0;
    }
    usb_endpoints[0].buffer = buffer->next;
    // Force an end to the data stage
    USBCS0 |= USBCS0_CLR_OUTPKT_RDY | USBCS0_DATA_END;

    ep0status = EP_IDLE;
    return res;
  }

  if(get_receive_capacity(buffer) < len) {
    // Wait until we queue more buffers.
    return USB_READ_BLOCK;
  }

  if(len < usb_endpoints[0].xfer_size) {
    short_packet = 1;
  }

  res = fill_buffers(buffer, 0, len, short_packet);

  if(short_packet) {
    /* The usb-core will send a status packet, we will release the fifo at this stage */
    ep0status = EP_IDLE;
  } else {
    // More data to come
    USBCS0 |= USBCS0_CLR_OUTPKT_RDY;
  }
  return res;
}

static uint8_t
ep0_tx(void)
{
  USBBuffer *buffer = usb_endpoints[0].buffer;
  unsigned int len = usb_endpoints[0].xfer_size;
  uint8_t data_end = 0;
  uint8_t res = 0;

  // If TX Fifo still busy or ep0 not in TX data stage don't do anything
  if((USBCS0 & USBCS0_INPKT_RDY) || (ep0status != EP_TX)) {
    return 0;
  }

  if(!buffer) {
    return 0;
  }

  if(!(buffer->flags & USB_BUFFER_IN)) {
    return 0;                   // Huh .. problem ... we should TX but queued buffer is in RX ...
  }

  while(buffer) {
    unsigned int copy;

    if(buffer->left < len) {
      copy = buffer->left;
    } else {
      copy = len;
    }

    len -= copy;
    buffer->left -= copy;
    write_hw_buffer(0, buffer->data, copy);
    buffer->data += copy;
    if(buffer->left == 0) {
      if(buffer->flags & USB_BUFFER_SHORT_END) {
        if(len == 0) {
          break;                // We keep the buffer in queue so we will send a ZLP next time.
        } else {
          data_end = 1;
          len = 0;              // Stop looking for more data to send
        }
      }
      buffer->flags &= ~USB_BUFFER_SUBMITTED;
      if(buffer->flags & USB_BUFFER_NOTIFY) {
        res |= USB_WRITE_NOTIFY;
      }
      buffer = buffer->next;
    }
    if(len == 0) {
      break;                    // FIFO is full, send packet.
    }
  }
  if(len) {
    // Short packet will be sent.
    data_end = 1;
  }
  usb_endpoints[0].buffer = buffer;

  // Workaround the fact that the usb controller do not like to have DATA_END set after INPKT_RDY
  // for the last packet. Thus if no more is in the queue set DATA_END.
  if(data_end || !buffer) {
    ep0status = EP_IDLE;
    USBCS0 |= USBCS0_INPKT_RDY | USBCS0_DATA_END;
  } else {
    USBCS0 |= USBCS0_INPKT_RDY;
  }

  return res;
}

static void
usb_arch_ep0_irq(void)
{
  uint8_t cs0;
  uint8_t res;

  USBINDEX = 0;
  cs0 = USBCS0;
  if(cs0 & USBCS0_SENT_STALL) {
    // Ack the stall
    USBCS0 = 0;
    ep0status = EP_IDLE;
  }
  if(cs0 & USBCS0_SETUP_END) {
    // Clear it
    USBCS0 = USBCS0_CLR_SETUP_END;
    ep0status = EP_IDLE;
  }

  if(cs0 & USBCS0_OUTPKT_RDY) {
    if(ep0status == EP_IDLE) {
      res = ep0_get_setup_pkt();
    } else {
      res = ep0_get_data_pkt();
    }

    if(res & USB_READ_NOTIFY) {
      notify_ep_process(&usb_endpoints[0], USB_EP_EVENT_NOTIFICATION);
    }
    if(res & USB_READ_BLOCK) {
      return;
    }
  }
  // Trigger the TX path
  res = ep0_tx();

  if(res & USB_WRITE_NOTIFY) {
    notify_ep_process(&usb_endpoints[0], USB_EP_EVENT_NOTIFICATION);
  }
}


static uint8_t
ep_tx(uint8_t ep_hw)
{
  unsigned int len;
  uint8_t res = 0;
  USBEndpoint *ep = EP_STRUCT(ep_hw);

  len = ep->xfer_size;

  if(ep->halted) {
    return 0;
  }

  if(!ep->buffer || !(ep->buffer->flags & USB_BUFFER_IN)) {
    return 0;
  }

  while(ep->buffer) {
    unsigned int copy;

    if(ep->buffer->left < len) {
      copy = ep->buffer->left;
    } else {
      copy = len;
    }

    len -= copy;
    ep->buffer->left -= copy;
    write_hw_buffer(EP_INDEX(ep_hw), ep->buffer->data, copy);
    ep->buffer->data += copy;

    if(ep->buffer->left == 0) {
      if(ep->buffer->flags & USB_BUFFER_SHORT_END) {
        if(len == 0) {
          break;                // We keep the buffer in queue so we will send a ZLP next time.
        } else {
          len = 0;              // Stop looking for more data to send
        }
      }
      ep->buffer->flags &= ~USB_BUFFER_SUBMITTED;
      if(ep->buffer->flags & USB_BUFFER_NOTIFY) {
        res |= USB_WRITE_NOTIFY;
      }
      ep->buffer = ep->buffer->next;
    }
    if(len == 0)
      break;                    // FIFO is full, send packet.
  }

  USBCSIL |= USBCSIL_INPKT_RDY;

  return res;

}

static uint8_t
ep_get_data_pkt(uint8_t ep_hw)
{
  uint16_t pkt_len;
  uint8_t res;
  uint8_t short_packet = 0;
  USBEndpoint *ep = EP_STRUCT(ep_hw);

  if(!ep->buffer) {
    return USB_READ_BLOCK;
  }

  if(ep->buffer->flags & USB_BUFFER_HALT) {
    ep->halted = 1;
    if(!(USBCSOL & USBCSOL_SEND_STALL)) {
      USBCSOL |= USBCSOL_SEND_STALL;
    }
    return 0;
  }

  pkt_len = USBCNTL | (USBCNTH << 8);
  if(get_receive_capacity(ep->buffer) < pkt_len) {
    return USB_READ_BLOCK;
  }

  if(pkt_len < ep->xfer_size) {
    short_packet = 1;
  }

  res = fill_buffers(ep->buffer, ep_hw, pkt_len, short_packet);

  USBCSOL &= ~USBCSOL_OUTPKT_RDY;

  return res;
}

static void
usb_arch_epout_irq(uint8_t ep_hw)
{
  // RX interrupt
  uint8_t csl;
  uint8_t res;
  USBEndpoint *ep = EP_STRUCT(ep_hw);


  USBINDEX = ep_hw;
  csl = USBCSOL;

  if(csl & USBCSOL_SENT_STALL) {
    USBCSOL &= ~USBCSOL_SENT_STALL;
  }

  if(csl & USBCSOL_OVERRUN) {
    // We lost one isochronous packet ...
    USBCSOL &= ~USBCSOL_OVERRUN;
  }

  if(csl & USBCSOL_OUTPKT_RDY) {
    res = ep_get_data_pkt(ep_hw);

    if(res & USB_READ_NOTIFY) {
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
    }
  }
}

static void
usb_arch_epin_irq(uint8_t ep_hw)
{
  uint8_t csl;
  uint8_t res;
  USBEndpoint *ep = EP_STRUCT(ep_hw);

  // TX interrupt

  USBINDEX = ep_hw;
  csl = USBCSIL;

  if(csl & USBCSIL_SENT_STALL) {
    USBCSIL &= ~USBCSIL_SENT_STALL;
  }

  if(csl & USBCSIL_UNDERRUN) {
    USBCSIL &= ~USBCSIL_UNDERRUN;
  }

  if(!(csl & USBCSIL_INPKT_RDY)) {
    res = ep_tx(ep_hw);
    if(res & USB_WRITE_NOTIFY) {
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
    }
  }
}

#define EPxIF(x)        (1 << x)
#define RSTIF           (1 << 2)
#define RESUMEIF        (1 << 1)
#define SUSPENDIF       (1 << 0)

uint8_t
usb_irq(void)
{
  uint8_t ep_in_if = USBIIF & USBIIE;
  uint8_t ep_out_if = USBOIF & USBOIE;
  uint8_t common_if = USBCIF & USBCIE;
  uint8_t i;

  if(ep_in_if) {
    /* EP0 is routed on IN irq flags */
    if(ep_in_if & 0x1) {
      usb_arch_ep0_irq();
    }
    for(i = 1; i < 6; i++) {
      if(ep_in_if & EPxIF(i)) {
        usb_arch_epin_irq(i);
      }
    }
  }
  if(ep_out_if) {
    for(i = 1; i < 6; i++) {
      if(ep_out_if & EPxIF(i)) {
        usb_arch_epout_irq(i);
      }
    }
  }
  if(common_if & RSTIF) {
    usb_arch_reset();
    notify_process(USB_EVENT_RESET);
  }
  if(common_if & RESUMEIF) {
    notify_process(USB_EVENT_RESUME);
  }
  if(common_if & SUSPENDIF) {
    notify_process(USB_EVENT_SUSPEND);
  }

  return ep_in_if || ep_out_if || common_if ? CC253x_P2_ACK : CC253x_P2_NACK;
}

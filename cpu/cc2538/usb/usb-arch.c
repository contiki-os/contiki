/*
 * Copyright (c) 2012, Philippe Retornaz
 * Copyright (c) 2012, EPFL STI IMT LSRO1 -- Mobots group
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-usb
 * @{
 *
 * \file
 *     Arch-specific routines for the cc2538 USB controller. Heavily based on
 *     the cc2530 driver written by Philippe Retornaz
 */
#include "contiki.h"
#include "energest.h"
#include "usb-arch.h"
#include "usb-api.h"
#include "dev/usb-regs.h"
#include "dev/nvic.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
#include "dev/udma.h"
#include "sys/clock.h"
#include "reg.h"

#include "dev/watchdog.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* EP max FIFO sizes without double buffering */
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
/*---------------------------------------------------------------------------*/
/* uDMA transfer threshold. Use DMA only for data size higher than this */
#define UDMA_SIZE_THRESHOLD 8

/* uDMA channel control persistent flags */
#define UDMA_TX_FLAGS (UDMA_CHCTL_ARBSIZE_128 | UDMA_CHCTL_XFERMODE_AUTO \
    | UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_DSTSIZE_8 \
    | UDMA_CHCTL_SRCINC_8 | UDMA_CHCTL_DSTINC_NONE)

#define UDMA_RX_FLAGS (UDMA_CHCTL_ARBSIZE_128 | UDMA_CHCTL_XFERMODE_AUTO \
    | UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_DSTSIZE_8 \
    | UDMA_CHCTL_SRCINC_NONE | UDMA_CHCTL_DSTINC_8)
/*---------------------------------------------------------------------------*/
static const uint16_t ep_xfer_size[] = {
  CTRL_EP_SIZE,
  USB_EP1_SIZE,
  USB_EP2_SIZE,
  USB_EP3_SIZE,
  USB_EP4_SIZE,
  USB_EP5_SIZE,
};
/*---------------------------------------------------------------------------*/
typedef struct _USBBuffer usb_buffer;
/*---------------------------------------------------------------------------*/
struct usb_endpoint {
  uint8_t halted;
  uint8_t addr;
  uint8_t flags;
  usb_buffer *buffer;
  struct process *event_process;
  unsigned int events;
  uint16_t xfer_size;
};
typedef struct usb_endpoint usb_endpoint_t;
/*---------------------------------------------------------------------------*/
#define EP_STATUS_IDLE                 0
#define EP_STATUS_RX                   1
#define EP_STATUS_TX                   2

#define USB_EP_FLAGS_TYPE_MASK      0x03
#define USB_EP_FLAGS_TYPE_BULK      0x00
#define USB_EP_FLAGS_TYPE_CONTROL   0x01
#define USB_EP_FLAGS_TYPE_ISO       0x02
#define USB_EP_FLAGS_TYPE_INTERRUPT 0x03
#define USB_EP_FLAGS_ENABLED 	      0x04

#define EP_TYPE(ep)          ((ep)->flags & USB_EP_FLAGS_TYPE_MASK)
#define IS_EP_TYPE(ep, type) (EP_TYPE(ep) == (type))
#define IS_CONTROL_EP(ep)    IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_CONTROL)
#define IS_BULK_EP(ep)       IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_BULK)
#define IS_INTERRUPT_EP(ep)  IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_INTERRUPT)
#define IS_ISO_EP(ep)        IS_EP_TYPE(ep, USB_EP_FLAGS_TYPE_ISO)

#define USBIIE_INEPxIE(x)    (1 << x)
#define USBOIE_OUEPxIE(x)    (1 << x)
#define EPxIF(x)             (1 << x)

#define USB_READ_BLOCK              0x01
#define USB_WRITE_NOTIFY            0x02
#define USB_READ_NOTIFY             0x02
#define USB_READ_FAIL	              0x04

/* Index in endpoint array */
#define EP_INDEX(addr) ((addr) & 0x7f)

/* Get address of endpoint struct */
#define EP_STRUCT(addr) &usb_endpoints[EP_INDEX(addr)];

/* Number of hardware endpoint */
#define EP_HW_NUM(addr) ((addr) & 0x7f)
/*---------------------------------------------------------------------------*/
static usb_endpoint_t usb_endpoints[USB_MAX_ENDPOINTS];
struct process *event_process = 0;
volatile static unsigned int events = 0;
static uint8_t ep0status;
/*---------------------------------------------------------------------------*/
static uint8_t ep0_tx(void);
static uint8_t ep_tx(uint8_t ep_hw);
static void in_ep_interrupt_handler(uint8_t ep_hw);
static void out_ep_interrupt_handler(uint8_t ep_hw);
static void ep0_interrupt_handler(void);
/*---------------------------------------------------------------------------*/
static void
notify_process(unsigned int e)
{
  events |= e;
  if(event_process) {
    process_poll(event_process);
  }
}
/*---------------------------------------------------------------------------*/
static void
notify_ep_process(usb_endpoint_t *ep, unsigned int e)
{
  ep->events |= e;
  if(ep->event_process) {
    process_poll(ep->event_process);
  }
}
/*---------------------------------------------------------------------------*/
void
usb_set_ep_event_process(unsigned char addr, struct process *p)
{
  usb_endpoint_t *ep = EP_STRUCT(addr);

  ep->event_process = p;
}
/*---------------------------------------------------------------------------*/
void
usb_arch_set_global_event_process(struct process *p)
{
  event_process = p;
}
/*---------------------------------------------------------------------------*/
unsigned int
usb_arch_get_global_events(void)
{
  uint8_t flag;
  volatile unsigned int e;

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  e = events;
  events = 0;

  nvic_interrupt_en_restore(NVIC_INT_USB, flag);

  return e;
}
/*---------------------------------------------------------------------------*/
unsigned int
usb_get_ep_events(uint8_t addr)
{
  volatile unsigned int e;
  uint8_t flag;
  usb_endpoint_t *ep = EP_STRUCT(addr);

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  e = ep->events;
  ep->events = 0;

  nvic_interrupt_en_restore(NVIC_INT_USB, flag);

  return e;
}
/*---------------------------------------------------------------------------*/
static void
read_hw_buffer(uint8_t *to, uint8_t hw_ep, unsigned int len)
{
  uint32_t fifo_addr = USB_F0 + (hw_ep << 3);

  if(USB_ARCH_CONF_DMA && len > UDMA_SIZE_THRESHOLD) {
    /* Set the transfer source and destination addresses */
    udma_set_channel_src(USB_ARCH_CONF_RX_DMA_CHAN, fifo_addr);
    udma_set_channel_dst(USB_ARCH_CONF_RX_DMA_CHAN,
                         (uint32_t)(to) + len - 1);

    /* Configure the control word */
    udma_set_channel_control_word(USB_ARCH_CONF_RX_DMA_CHAN,
                                  UDMA_RX_FLAGS | udma_xfer_size(len));

    /* Enabled the RF RX uDMA channel */
    udma_channel_enable(USB_ARCH_CONF_RX_DMA_CHAN);

    /* Trigger the uDMA transfer */
    udma_channel_sw_request(USB_ARCH_CONF_RX_DMA_CHAN);

    /* Wait for the transfer to complete. */
    while(udma_channel_get_mode(USB_ARCH_CONF_RX_DMA_CHAN));
  } else {
    while(len--) {
      *to++ = REG(fifo_addr);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
write_hw_buffer(uint8_t hw_ep, uint8_t *from, unsigned int len)
{
  uint32_t fifo_addr = USB_F0 + (hw_ep << 3);

  if(USB_ARCH_CONF_DMA && len > UDMA_SIZE_THRESHOLD) {
    /* Set the transfer source and destination addresses */
    udma_set_channel_src(USB_ARCH_CONF_TX_DMA_CHAN,
                         (uint32_t)(from) + len - 1);
    udma_set_channel_dst(USB_ARCH_CONF_TX_DMA_CHAN, fifo_addr);

    /* Configure the control word */
    udma_set_channel_control_word(USB_ARCH_CONF_TX_DMA_CHAN,
                                  UDMA_TX_FLAGS | udma_xfer_size(len));

    /* Enabled the RF RX uDMA channel */
    udma_channel_enable(USB_ARCH_CONF_TX_DMA_CHAN);

    /* Trigger the uDMA transfer */
    udma_channel_sw_request(USB_ARCH_CONF_TX_DMA_CHAN);

    /* Wait for the transfer to complete. */
    while(udma_channel_get_mode(USB_ARCH_CONF_TX_DMA_CHAN));
  } else {
    while(len--) {
      REG(fifo_addr) = *from++;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
reset(void)
{
  uint8_t e;

  for(e = 0; e < USB_MAX_ENDPOINTS; e++) {
    if(usb_endpoints[e].flags & USB_EP_FLAGS_ENABLED) {
      usb_buffer *buffer = usb_endpoints[e].buffer;

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
/*---------------------------------------------------------------------------*/
/* Init USB */
void
usb_arch_setup(void)
{
  uint8_t i;

  /* Switch on USB PLL & USB module */
  REG(USB_CTRL) = USB_CTRL_USB_EN | USB_CTRL_PLL_EN;

  /* Wait until USB PLL is stable */
  while(!(REG(USB_CTRL) & USB_CTRL_PLL_LOCKED));

  /* Enable pull-up on usb port */
  GPIO_SET_OUTPUT(USB_PULLUP_PORT, USB_PULLUP_PIN_MASK);
  REG((USB_PULLUP_PORT | GPIO_DATA) + (USB_PULLUP_PIN_MASK << 2)) = 1;

  for(i = 0; i < USB_MAX_ENDPOINTS; i++) {
    usb_endpoints[i].flags = 0;
    usb_endpoints[i].event_process = 0;
  }

  reset();

  /* Disable all EP interrupts, EP0 interrupt will be enabled later */
  REG(USB_IIE) = 0;
  REG(USB_OIE) = 0;

  /* Initialise the USB control structures */
  if(USB_ARCH_CONF_DMA) {
    /* Disable peripheral triggers for our channels */
    udma_channel_mask_set(USB_ARCH_CONF_RX_DMA_CHAN);
    udma_channel_mask_set(USB_ARCH_CONF_TX_DMA_CHAN);
  }

  nvic_interrupt_enable(NVIC_INT_USB);
}
/*---------------------------------------------------------------------------*/
void
usb_submit_recv_buffer(uint8_t addr, usb_buffer *buffer)
{
  usb_buffer **tailp;
  uint8_t flag;
  usb_endpoint_t *ep = EP_STRUCT(addr);

  if(!(ep->flags & USB_EP_FLAGS_ENABLED)) {
    return;
  }

  if(buffer->data == NULL && EP_HW_NUM(addr) == 0) {
    if(buffer->flags & USB_BUFFER_NOTIFY) {
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
    }
    return;
  }

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  tailp = &ep->buffer;
  while(*tailp) {
    tailp = &(*tailp)->next;
  }
  *tailp = buffer;
  while(buffer) {
    buffer->flags |= USB_BUFFER_SUBMITTED;
    buffer = buffer->next;
  }

  REG(USB_INDEX) = EP_HW_NUM(addr);
  if(!EP_HW_NUM(ep->addr)) {
    if(REG(USB_CS0) & USB_CS0_OUTPKT_RDY) {
      ep0_interrupt_handler();
    }
  } else {
    if(REG(USB_CSOL) & USB_CSOL_OUTPKT_RDY) {
      out_ep_interrupt_handler(EP_HW_NUM(ep->addr));
    }
  }

  nvic_interrupt_en_restore(NVIC_INT_USB, flag);
}
/*---------------------------------------------------------------------------*/
void
usb_submit_xmit_buffer(uint8_t addr, usb_buffer *buffer)
{
  usb_buffer **tailp;
  uint8_t flag;
  uint8_t res;
  usb_endpoint_t *ep = EP_STRUCT(addr);

  if(!(ep->flags & USB_EP_FLAGS_ENABLED)) {
    return;
  }

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  if(EP_HW_NUM(addr) == 0) {
    if(buffer->data == NULL) {
      /* We are asked to send a STATUS packet.
       * But the USB hardware will do this automatically
       * as soon as we release the HW FIFO. */
      REG(USB_INDEX) = 0;
      REG(USB_CS0) = USB_CS0_CLR_OUTPKT_RDY | USB_CS0_DATA_END;
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
      nvic_interrupt_en_restore(NVIC_INT_USB, flag);
      return;
    } else {
      /* Release the HW FIFO */
      REG(USB_INDEX) = 0;
      REG(USB_CS0) = USB_CS0_CLR_OUTPKT_RDY;
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

  REG(USB_INDEX) = EP_HW_NUM(ep->addr);
  if(EP_HW_NUM(ep->addr)) {
    res = ep_tx(EP_HW_NUM(ep->addr));
  } else {
    res = ep0_tx();
  }

  nvic_interrupt_en_restore(NVIC_INT_USB, flag);

  if(res & USB_WRITE_NOTIFY) {
    notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
  }
}
/*---------------------------------------------------------------------------*/
static void
ep0_setup(void)
{
  REG(USB_IIE) |= USB_IIE_EP0IE;
}
/*---------------------------------------------------------------------------*/
static void
in_ep_setup(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  usb_endpoint_t *ep = EP_STRUCT(addr);

  /* Enable IN EP interrupt */
  REG(USB_IIE) |= USBIIE_INEPxIE(ei);

  /* Set internal FIFO size */
  REG(USB_MAXI) = ep->xfer_size / 8;

  if(IS_ISO_EP(ep)) {
    REG(USB_CSIH) |= USB_CSOH_ISO;
  } else {
    REG(USB_CSIH) &= ~USB_CSOH_ISO;
  }
}
/*---------------------------------------------------------------------------*/
static void
out_ep_setup(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  usb_endpoint_t *ep = EP_STRUCT(addr);

  /* Enable OUT EP interrupt */
  REG(USB_OIE) |= USBOIE_OUEPxIE(ei);

  /* Set internal FIFO size */
  REG(USB_MAXO) = ep->xfer_size / 8;

  if(IS_ISO_EP(ep)) {
    REG(USB_CSOH) |= USB_CSOH_ISO;
  } else {
    REG(USB_CSOH) &= ~USB_CSOH_ISO;
  }
}
/*---------------------------------------------------------------------------*/
static void
ep_setup(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  uint8_t flag;
  usb_endpoint_t *ep = EP_STRUCT(addr);

  ep->halted = 0;
  ep->flags |= USB_EP_FLAGS_ENABLED;
  ep->buffer = 0;
  ep->addr = addr;
  ep->events = 0;
  ep->xfer_size = ep_xfer_size[ei];

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  /* Select endpoint register */
  REG(USB_INDEX) = ei;

  /* EP0 requires special handing */
  if(ei == 0) {
    ep0_setup();
  } else {
    if(addr & 0x80) {
      in_ep_setup(addr);
    } else {
      out_ep_setup(addr);
    }
  }

  nvic_interrupt_en_restore(NVIC_INT_USB, flag);
}
/*---------------------------------------------------------------------------*/
void
usb_arch_setup_iso_endpoint(uint8_t addr)
{
  usb_endpoint_t *ep = EP_STRUCT(addr);

  ep->flags = USB_EP_FLAGS_TYPE_ISO;

  ep_setup(addr);
}
/*---------------------------------------------------------------------------*/
void
usb_arch_setup_control_endpoint(uint8_t addr)
{
  usb_endpoint_t *ep = EP_STRUCT(addr);

  ep->flags = USB_EP_FLAGS_TYPE_CONTROL;

  ep_setup(addr);
}
/*---------------------------------------------------------------------------*/
void
usb_arch_setup_bulk_endpoint(uint8_t addr)
{
  usb_endpoint_t *ep = EP_STRUCT(addr);

  ep->flags = USB_EP_FLAGS_TYPE_BULK;

  ep_setup(addr);
}
/*---------------------------------------------------------------------------*/
void
usb_arch_setup_interrupt_endpoint(uint8_t addr)
{
  usb_endpoint_t *ep = EP_STRUCT(addr);

  ep->flags = USB_EP_FLAGS_TYPE_INTERRUPT;

  ep_setup(addr);
}
/*---------------------------------------------------------------------------*/
static void
ep0_dis(void)
{
  REG(USB_IIE) &= ~USB_IIE_EP0IE;
  /* Clear any pending status flags */
  REG(USB_CS0) = 0xC0;
}
/*---------------------------------------------------------------------------*/
static void
in_ep_dis(uint8_t addr)
{
  REG(USB_MAXI) = 0;
  REG(USB_IIE) &= ~USBIIE_INEPxIE(EP_HW_NUM(addr));

  /* Flush pending */
  REG(USB_CSIL) = USB_CSIL_FLUSH_PACKET;
}
/*---------------------------------------------------------------------------*/
static void
out_ep_dis(uint8_t addr)
{
  REG(USB_MAXO) = 0;
  REG(USB_OIE) &= ~USBOIE_OUEPxIE(EP_HW_NUM(addr));

  /* Flush pending */
  REG(USB_CSOL) = USB_CSIL_FLUSH_PACKET;
}
/*---------------------------------------------------------------------------*/
void
usb_arch_disable_endpoint(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  uint8_t flag;
  usb_endpoint_t *ep = EP_STRUCT(addr);

  ep->flags &= ~USB_EP_FLAGS_ENABLED;

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  REG(USB_INDEX) = ei;
  if(ei == 0) {
    ep0_dis();
  } else {
    if(addr & 0x80) {
      in_ep_dis(addr);
    } else {
      out_ep_dis(addr);
    }
  }
  nvic_interrupt_en_restore(NVIC_INT_USB, flag);
}
/*---------------------------------------------------------------------------*/
void
usb_arch_discard_all_buffers(uint8_t addr)
{
  usb_buffer *buffer;
  uint8_t flag;
  volatile usb_endpoint_t *ep = EP_STRUCT(addr);

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  buffer = ep->buffer;
  ep->buffer = NULL;
  nvic_interrupt_en_restore(NVIC_INT_USB, flag);

  while(buffer) {
    buffer->flags &= ~USB_BUFFER_SUBMITTED;
    buffer = buffer->next;
  }
}
/*---------------------------------------------------------------------------*/
static void
set_stall(uint8_t addr, uint8_t stall)
{
  uint8_t ei = EP_HW_NUM(addr);

  REG(USB_INDEX) = ei;
  if(ei == 0) {
    /* Stall is automatically deasserted on EP0 */
    if(stall) {
      ep0status = EP_STATUS_IDLE;
      REG(USB_CS0) |= USB_CS0_SEND_STALL | USB_CS0_OUTPKT_RDY;
    }
  } else {
    if(addr & 0x80) {
      if(stall) {
        REG(USB_CSIL) |= USB_CSIL_SEND_STALL;
      } else {
        REG(USB_CSIL) &= ~USB_CSIL_SEND_STALL;
      }
    } else {
      if(stall) {
        REG(USB_CSOL) |= USB_CSOL_SEND_STALL;
      } else {
        REG(USB_CSOL) &= ~USB_CSOL_SEND_STALL;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void
usb_arch_control_stall(uint8_t addr)
{
  uint8_t ei = EP_HW_NUM(addr);
  uint8_t flag;

  if(ei > USB_MAX_ENDPOINTS) {
    return;
  }

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  set_stall(addr, 1);

  nvic_interrupt_en_restore(NVIC_INT_USB, flag);
}
/*---------------------------------------------------------------------------*/
void
usb_arch_halt_endpoint(uint8_t addr, int halt)
{
  uint8_t ei = EP_HW_NUM(addr);
  uint8_t flag;
  usb_endpoint_t *ep = EP_STRUCT(addr);

  if(ei > USB_MAX_ENDPOINTS) {
    return;
  }

  if(!(ep->flags & USB_EP_FLAGS_ENABLED)) {
    return;
  }

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

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
      out_ep_interrupt_handler(EP_HW_NUM(addr));
    }
  }

  nvic_interrupt_en_restore(NVIC_INT_USB, flag);
}
/*---------------------------------------------------------------------------*/
void
usb_arch_set_configuration(uint8_t usb_configuration_value)
{
  return;
}
/*---------------------------------------------------------------------------*/
uint16_t
usb_arch_get_ep_status(uint8_t addr)
{
  uint8_t ei = EP_INDEX(addr);
  usb_endpoint_t *ep = EP_STRUCT(addr);

  if(ei > USB_MAX_ENDPOINTS) {
    return 0;
  }

  return ep->halted;
}
/*---------------------------------------------------------------------------*/
void
usb_arch_set_address(uint8_t addr)
{
  REG(USB_ADDR) = addr;
}
/*---------------------------------------------------------------------------*/
int
usb_arch_send_pending(uint8_t addr)
{
  uint8_t flag;
  uint8_t ret;
  uint8_t ei = EP_INDEX(addr);

  flag = nvic_interrupt_en_save(NVIC_INT_USB);

  REG(USB_INDEX) = ei;
  if(ei == 0) {
    ret = REG(USB_CS0) & USB_CS0_INPKT_RDY;
  } else {
    ret = REG(USB_CSIL) & USB_CSIL_INPKT_RDY;
  }

  nvic_interrupt_en_restore(NVIC_INT_USB, flag);

  return ret;
}
/*---------------------------------------------------------------------------*/
static unsigned int
get_receive_capacity(usb_buffer *buffer)
{
  unsigned int capacity = 0;

  while(buffer &&
        !(buffer->flags & (USB_BUFFER_IN | USB_BUFFER_SETUP | USB_BUFFER_HALT))) {
    capacity += buffer->left;
    buffer = buffer->next;
  }
  return capacity;
}
/*---------------------------------------------------------------------------*/
static usb_buffer *
skip_buffers_until(usb_buffer *buffer, unsigned int mask, unsigned int flags,
                   uint8_t *resp)
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
/*---------------------------------------------------------------------------*/
static uint8_t
fill_buffers(usb_buffer *buffer, uint8_t hw_ep, unsigned int len,
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
/*---------------------------------------------------------------------------*/
static uint8_t
ep0_get_setup_pkt(void)
{
  uint8_t res;
  usb_buffer *buffer =
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
    REG(USB_CS0) |= USB_CS0_CLR_OUTPKT_RDY;
    ep0status = buffer->data[0] & 0x80 ? EP_STATUS_TX : EP_STATUS_RX;
  }

  buffer->data += 8;

  usb_endpoints[0].buffer = buffer->next;

  return res;
}
/*---------------------------------------------------------------------------*/
static uint8_t
ep0_get_data_pkt(void)
{
  uint8_t res = 0;
  uint8_t short_packet = 0;
  usb_buffer *buffer = usb_endpoints[0].buffer;
  uint8_t len = REG(USB_CNT0);

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
    /* Flush the fifo */
    while(len--) {
      temp = REG(USB_F0);
    }
    usb_endpoints[0].buffer = buffer->next;
    /* Force data stage end */
    REG(USB_CS0) |= USB_CS0_CLR_OUTPKT_RDY | USB_CS0_DATA_END;

    ep0status = EP_STATUS_IDLE;
    return res;
  }

  if(get_receive_capacity(buffer) < len) {
    /* Wait until we queue more buffers */
    return USB_READ_BLOCK;
  }

  if(len < usb_endpoints[0].xfer_size) {
    short_packet = 1;
  }

  res = fill_buffers(buffer, 0, len, short_packet);

  if(short_packet) {
    /* The usb-core will send a status packet, we will release the fifo at this stage */
    ep0status = EP_STATUS_IDLE;
  } else {
    REG(USB_CS0) |= USB_CS0_CLR_OUTPKT_RDY;
  }
  return res;
}
/*---------------------------------------------------------------------------*/
static uint8_t
ep0_tx(void)
{
  usb_buffer *buffer = usb_endpoints[0].buffer;
  unsigned int len = usb_endpoints[0].xfer_size;
  uint8_t data_end = 0;
  uint8_t res = 0;

  /* If TX Fifo still busy or ep0 not in TX data stage don't do anything */
  if((REG(USB_CS0) & USB_CS0_INPKT_RDY) || (ep0status != EP_STATUS_TX)) {
    return 0;
  }

  if(!buffer) {
    return 0;
  }

  if(!(buffer->flags & USB_BUFFER_IN)) {
    /* We should TX but queued buffer is in RX */
    return 0;
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
    data_end = 1;
  }
  usb_endpoints[0].buffer = buffer;

  /*
   * Workaround the fact that the usb controller do not like to have DATA_END
   * set after INPKT_RDY for the last packet. Thus if no more is in the queue
   * set DATA_END
   */
  if(data_end || !buffer) {
    ep0status = EP_STATUS_IDLE;
    REG(USB_CS0) |= USB_CS0_INPKT_RDY | USB_CS0_DATA_END;
  } else {
    REG(USB_CS0) |= USB_CS0_INPKT_RDY;
  }

  return res;
}
/*---------------------------------------------------------------------------*/
static void
ep0_interrupt_handler(void)
{
  uint8_t cs0;
  uint8_t res;

  REG(USB_INDEX) = 0;
  cs0 = REG(USB_CS0);
  if(cs0 & USB_CS0_SENT_STALL) {
    /* Ack the stall */
    REG(USB_CS0) = 0;
    ep0status = EP_STATUS_IDLE;
  }
  if(cs0 & USB_CS0_SETUP_END) {
    /* Clear it */
    REG(USB_CS0) = USB_CS0_CLR_SETUP_END;
    ep0status = EP_STATUS_IDLE;
  }

  if(cs0 & USB_CS0_OUTPKT_RDY) {
    if(ep0status == EP_STATUS_IDLE) {
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

  res = ep0_tx();

  if(res & USB_WRITE_NOTIFY) {
    notify_ep_process(&usb_endpoints[0], USB_EP_EVENT_NOTIFICATION);
  }
}
/*---------------------------------------------------------------------------*/
static uint8_t
ep_tx(uint8_t ep_hw)
{
  unsigned int len;
  uint8_t res = 0;
  usb_endpoint_t *ep = EP_STRUCT(ep_hw);

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

    /*
     * Delay somewhat if the previous packet has not yet left the IN FIFO,
     * making sure the dog doesn't bark while we're waiting
     */
    while(REG(USB_CSIL) & USB_CSIL_INPKT_RDY) {
      watchdog_periodic();
    }

    write_hw_buffer(EP_INDEX(ep_hw), ep->buffer->data, copy);
    ep->buffer->data += copy;

    if(ep->buffer->left == 0) {
      if(ep->buffer->flags & USB_BUFFER_SHORT_END) {
        if(len == 0) {
          /* We keep the buffer in queue so we will send a ZLP next */
          break;
        } else {
          /* Stop looking for more data to send */
          len = 0;
        }
      }
      ep->buffer->flags &= ~USB_BUFFER_SUBMITTED;
      if(ep->buffer->flags & USB_BUFFER_NOTIFY) {
        res |= USB_WRITE_NOTIFY;
      }
      ep->buffer = ep->buffer->next;
    }
    if(len == 0) {
      /* FIFO full, send */
      break;
    }
  }

  REG(USB_CSIL) |= USB_CSIL_INPKT_RDY;

  return res;
}
/*---------------------------------------------------------------------------*/
static uint8_t
ep_get_data_pkt(uint8_t ep_hw)
{
  uint16_t pkt_len;
  uint8_t res;
  uint8_t short_packet = 0;
  usb_endpoint_t *ep = EP_STRUCT(ep_hw);

  if(!ep->buffer) {
    return USB_READ_BLOCK;
  }

  if(ep->buffer->flags & USB_BUFFER_HALT) {
    ep->halted = 1;
    if(!(REG(USB_CSOL) & USB_CSOL_SEND_STALL)) {
      REG(USB_CSOL) |= USB_CSOL_SEND_STALL;
    }
    return 0;
  }

  /* Disambiguate UG CNTL bits */
  pkt_len = REG(USB_CNTL) | (REG(USB_CNTH) << 8);
  if(get_receive_capacity(ep->buffer) < pkt_len) {
    return USB_READ_BLOCK;
  }

  if(pkt_len < ep->xfer_size) {
    short_packet = 1;
  }

  res = fill_buffers(ep->buffer, ep_hw, pkt_len, short_packet);

  REG(USB_CSOL) &= ~USB_CSOL_OUTPKT_RDY;

  return res;
}
/*---------------------------------------------------------------------------*/
static void
out_ep_interrupt_handler(uint8_t ep_hw)
{
  uint8_t csl;
  uint8_t res;
  usb_endpoint_t *ep = EP_STRUCT(ep_hw);


  REG(USB_INDEX) = ep_hw;
  csl = REG(USB_CSOL);

  if(csl & USB_CSOL_SENT_STALL) {
    REG(USB_CSOL) &= ~USB_CSOL_SENT_STALL;
  }

  if(csl & USB_CSOL_OVERRUN) {
    /* We lost one isochronous packet */
    REG(USB_CSOL) &= ~USB_CSOL_OVERRUN;
  }

  if(csl & USB_CSOL_OUTPKT_RDY) {
    res = ep_get_data_pkt(ep_hw);

    if(res & USB_READ_NOTIFY) {
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
in_ep_interrupt_handler(uint8_t ep_hw)
{
  uint8_t csl;
#if USB_ARCH_WRITE_NOTIFY
  uint8_t res;
  usb_endpoint_t *ep = EP_STRUCT(ep_hw);
#endif

  REG(USB_INDEX) = ep_hw;
  csl = REG(USB_CSIL);

  if(csl & USB_CSIL_SENT_STALL) {
    REG(USB_CSIL) &= ~USB_CSIL_SENT_STALL;
  }

  if(csl & USB_CSIL_UNDERRUN) {
    REG(USB_CSIL) &= ~USB_CSIL_UNDERRUN;
  }

#if USB_ARCH_WRITE_NOTIFY
  if(!(csl & USB_CSIL_INPKT_RDY)) {
    res = ep_tx(ep_hw);
    if(res & USB_WRITE_NOTIFY) {
      notify_ep_process(ep, USB_EP_EVENT_NOTIFICATION);
    }
  }
#endif
}
/*---------------------------------------------------------------------------*/
void
usb_isr(void)
{
  uint8_t ep_in_if = REG(USB_IIF) & REG(USB_IIE);
  uint8_t ep_out_if = REG(USB_OIF) & REG(USB_OIE);
  uint8_t common_if = REG(USB_CIF) & REG(USB_CIE);
  uint8_t i;

  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  if(ep_in_if) {
    /* EP0 flag is in the IN Interrupt Flags register */
    if(ep_in_if & USB_IIF_EP0IF) {
      ep0_interrupt_handler();
    }
    for(i = 1; i < 6; i++) {
      if(ep_in_if & EPxIF(i)) {
        in_ep_interrupt_handler(i);
      }
    }
  }
  if(ep_out_if) {
    for(i = 1; i < 6; i++) {
      if(ep_out_if & EPxIF(i)) {
        out_ep_interrupt_handler(i);
      }
    }
  }
  if(common_if & USB_CIF_RSTIF) {
    reset();
    notify_process(USB_EVENT_RESET);
  }
  if(common_if & USB_CIF_RESUMEIF) {
    notify_process(USB_EVENT_RESUME);
  }
  if(common_if & USB_CIF_SUSPENDIF) {
    notify_process(USB_EVENT_SUSPEND);
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/

/** @} */

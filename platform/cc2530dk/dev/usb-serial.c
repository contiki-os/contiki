/*
 * Copyright (c) 2012, Philippe Retornaz
 * Copyright (c) 2012, EPFL STI IMT LSRO1 -- Mobots group
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *         Platform process which implements a UART-like functionality over
 *         the cc2531 dongle's USB hardware.
 *
 *         With it in place, putchar can be redirected to the USB and USB
 *         incoming traffic can be handled as input
 *
 * \author
 *         Philippe Retornaz (EPFL) - Original code
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 *           Turned this to a 'serial over USB' platform process
 */
#include "contiki.h"
#include "sys/process.h"
#include "usb/common/usb-api.h"
#include "usb/common/cdc-acm/cdc-acm.h"
#include "usb/common/usb.h"
#include "usb/common/usb-arch.h"
/*---------------------------------------------------------------------------*/
static const struct {
  uint8_t size;
  uint8_t type;
  uint16_t langid;
} lang_id = { sizeof(lang_id), 3, 0x0409 };

static struct {
  uint8_t size;
  uint8_t type;
  uint16_t string[16];
} string_serial_nr = {
  sizeof(string_serial_nr),
  3, {
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'
  }
};

static const struct {
  uint8_t size;
  uint8_t type;
  uint16_t string[10];
} string_manufacturer = {
  sizeof(string_manufacturer),
  3,
  { 'E', 'P', 'F', 'L', '-', 'L', 'S', 'R', 'O', '1' }
};

static const struct {
  uint8_t size;
  uint8_t type;
  uint16_t string[18];
} string_product = {
  sizeof(string_product),
  3, {
    'C', 'C', '2', '5', '3', '1', ' ',
    'D', 'e', 'v', 'e', 'l', '-', 'B', 'o', 'a', 'r', 'd'
  }
};
/*---------------------------------------------------------------------------*/
#define EPIN  0x82
#define EPOUT 0x03

#define BUFFER_SIZE USB_EP2_SIZE

static USBBuffer data_rx_urb;
static USBBuffer data_tx_urb;
static uint8_t usb_rx_data[BUFFER_SIZE];
static uint8_t enabled = 0;

#if USB_SERIAL_CONF_BUFFERED
#define SLIP_END 0300
static uint8_t usb_tx_data[BUFFER_SIZE];
static uint8_t buffered_data = 0;
#endif

/* Callback to the input handler */
static int (*input_handler)(unsigned char c);
/*---------------------------------------------------------------------------*/
uint8_t *
usb_class_get_string_descriptor(uint16_t lang, uint8_t string)
{
  switch (string) {
  case 0:
    return (uint8_t *) &lang_id;
  case 1:
    return (uint8_t *) &string_manufacturer;
  case 2:
    return (uint8_t *) &string_product;
  case 3:
    return (uint8_t *) &string_serial_nr;
  default:
    return NULL;
  }
}
/*---------------------------------------------------------------------------*/
static void
set_serial_number(void)
{
  uint8_t i;
  uint8_t *ieee = &X_IEEE_ADDR;
  uint8_t lown, highn;
  uint8_t c;

  for(i = 0; i < 8; i++) {
    lown = ieee[i] & 0x0F;
    highn = ieee[i] >> 4;
    c = lown > 9 ? 'A' + lown - 0xA : lown + '0';
    string_serial_nr.string[14 - i * 2 + 1] = c;
    c = highn > 9 ? 'A' + highn - 0xA : highn + '0';
    string_serial_nr.string[14 - i * 2] = c;
  }
}
/*---------------------------------------------------------------------------*/
static void
queue_rx_urb(void)
{
  data_rx_urb.flags = USB_BUFFER_PACKET_END;    /* Make sure we are getting immediately the packet. */
  data_rx_urb.flags |= USB_BUFFER_NOTIFY;
  data_rx_urb.data = usb_rx_data;
  data_rx_urb.left = BUFFER_SIZE;
  data_rx_urb.next = NULL;
  usb_submit_recv_buffer(EPOUT, &data_rx_urb);
}
/*---------------------------------------------------------------------------*/
static void
do_work(void)
{
  unsigned int events;

  events = usb_get_global_events();
  if(events & USB_EVENT_CONFIG) {
    /* Enable endpoints */
    enabled = 1;
    usb_setup_bulk_endpoint(EPIN);
    usb_setup_bulk_endpoint(EPOUT);

    queue_rx_urb();
  }
  if(events & USB_EVENT_RESET) {
    enabled = 0;
  }

  if(!enabled) {
    return;
  }

  events = usb_get_ep_events(EPOUT);
  if((events & USB_EP_EVENT_NOTIFICATION)
     && !(data_rx_urb.flags & USB_BUFFER_SUBMITTED)) {
    if(!(data_rx_urb.flags & USB_BUFFER_FAILED)) {
      if(input_handler) {
        uint8_t len;
        uint8_t i;

        len = BUFFER_SIZE - data_rx_urb.left;
        for(i = 0; i < len; i++) {
          input_handler(usb_rx_data[i]);
        }
      }
    }
    queue_rx_urb();
  }
}
/*---------------------------------------------------------------------------*/
#if USB_SERIAL_CONF_BUFFERED
void
usb_serial_flush()
{
  if(buffered_data == BUFFER_SIZE) {
    data_tx_urb.flags = 0;
  } else {
    data_tx_urb.flags = USB_BUFFER_SHORT_END;
  }
  data_tx_urb.flags |= USB_BUFFER_NOTIFY;
  data_tx_urb.next = NULL;
  data_tx_urb.data = usb_tx_data;
  data_tx_urb.left = buffered_data;
  buffered_data = 0;
  usb_submit_xmit_buffer(EPIN, &data_tx_urb);
}
/*---------------------------------------------------------------------------*/
void
usb_serial_writeb(uint8_t b)
{
  if(!enabled) {
    buffered_data = 0;
    return;
  }

  usb_tx_data[buffered_data] = b;
  buffered_data++;

  if(buffered_data == BUFFER_SIZE || b == SLIP_END || b == '\n') {
    usb_serial_flush();
  }
}
/*---------------------------------------------------------------------------*/
#else
void
usb_serial_writeb(uint8_t b)
{
  if(!enabled) {
    return;
  }

  data_tx_urb.flags = USB_BUFFER_SHORT_END;
  data_tx_urb.flags |= USB_BUFFER_NOTIFY;
  data_tx_urb.next = NULL;
  data_tx_urb.data = &b;
  data_tx_urb.left = 1;
  usb_submit_xmit_buffer(EPIN, &data_tx_urb);
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS(usb_serial_process, "USB-Serial process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(usb_serial_process, ev, data)
{

  PROCESS_BEGIN();

  set_serial_number();

  usb_setup();
  usb_cdc_acm_setup();
  usb_set_global_event_process(&usb_serial_process);
  usb_set_ep_event_process(EPIN, &usb_serial_process);
  usb_set_ep_event_process(EPOUT, &usb_serial_process);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_EXIT) {
      break;
    }
    if(ev == PROCESS_EVENT_POLL) {
      do_work();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
usb_serial_set_input(int (*input)(unsigned char c))
{
  input_handler = input;
}
/*---------------------------------------------------------------------------*/
void
usb_serial_init()
{
  process_start(&usb_serial_process, NULL);
}

/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Alternative implementation for SLIP:
 *         1. Accepts more than two packet
 *         2. Disables UART rx interrupt when buffer is full
 *         (thus invoking flow control if configured)
 * \author
 *         Niklas Finne <nfi@sics.se>
 *         Beshr Al Nahas <beshr@sics.se>
 *
 */

#include "contiki.h"

#include <MicroInt.h>
#include "net/ip/uip.h"
#include "net/ipv4/uip-fw.h"
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#include "dev/slip.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PUTCHAR(X) do { putchar(X); putchar('\n'); } while(0)
#else
#define PRINTF(...) do {} while(0)
#define PUTCHAR(X) do {} while(0)
#endif

#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335
#define SLIP_NEUTRAL  0 /* means: none of the above */
#define SLIP_ESC_XON  0336
#define SLIP_ESC_XOFF 0337
#define XON   ((unsigned char)17)
#define XOFF  ((unsigned char)19)
#if UART_XONXOFF_FLOW_CTRL
volatile unsigned char xonxoff_state = XON;
#endif /* UART_XONXOFF_FLOW_CTRL */

PROCESS(slip_process, "SLIP driver");

#include "dev/uart0.h"
#define STORE_UART_INTERRUPTS       uart0_store_interrupts
#define RESTORE_UART_INTERRUPTS     uart0_restore_interrupts
#define DISABLE_UART_INTERRUPTS     uart0_disable_interrupts
#define ENABLE_UART_INTERRUPTS      uart0_enable_interrupts

/**
 * @brief A block of code may be made atomic by wrapping it with this
 * macro.  Something which is atomic cannot be interrupted by interrupts.
 */
/* A specific ATMOIC that disables UART interrupts only */
#define ATOMIC(blah) \
  { \
    /* STORE_UART_INTERRUPTS(); */ \
    DISABLE_UART_INTERRUPTS(); \
    { blah } \
    /* RESTORE_UART_INTERRUPTS(); */ \
    ENABLE_UART_INTERRUPTS(); \
  }

/* A generic ATMOIC that disables all interrupts */
#define GLOBAL_ATOMIC(blah) \
  { \
    MICRO_DISABLE_INTERRUPTS(); \
    { blah } \
    MICRO_ENABLE_INTERRUPTS(); \
  }

#if 1
#define SLIP_STATISTICS(statement)
#else
uint16_t slip_drop_bytes, slip_overflow, slip_error_drop;
/* No used in this file */
uint16_t slip_rubbish, slip_twopackets, slip_ip_drop;
unsigned long slip_received, slip_frames;
#define SLIP_STATISTICS(statement) statement
#endif

/* Must be at least one byte larger than UIP_BUFSIZE (for SLIP_END)! */
#ifdef SLIP_CONF_RX_BUFSIZE
#define RX_BUFSIZE SLIP_CONF_RX_BUFSIZE

#if RX_BUFSIZE < (UIP_BUFSIZE - UIP_LLH_LEN + 16)
#error "SLIP_CONF_RX_BUFSIZE too small for UIP_BUFSIZE"
#endif

#else
#define RX_BUFSIZE (UIP_CONF_BUFFER_SIZE * 2)
#endif

/*
 * Variables begin and end manage the buffer space in a cyclic
 * fashion. The first used byte is at begin and end is one byte past
 * the last. I.e. [begin, end) is the actively used space.
 */

static volatile uint16_t begin, end, end_counter;
static uint8_t rxbuf[RX_BUFSIZE];
static volatile uint8_t is_dropping = 0;
static volatile uint8_t is_full = 0;

static void (*input_callback)(void) = NULL;
/*---------------------------------------------------------------------------*/
void
slip_set_input_callback(void (*c)(void))
{
  input_callback = c;
}
static void
slip_write_char(uint8_t c)
{
  /* Escape SLIP control characters */
  if(c == SLIP_END) {
    slip_arch_writeb(SLIP_ESC);
    c = SLIP_ESC_END;
  } else if(c == SLIP_ESC) {
    slip_arch_writeb(SLIP_ESC);
    c = SLIP_ESC_ESC;
  }
#if UART_XONXOFF_FLOW_CTRL
  /* Escape XON/XOFF characters */
  else if(c == XON) {
    slip_arch_writeb(SLIP_ESC);
    c = SLIP_ESC_XON;
  } else if(c == XOFF) {
    slip_arch_writeb(SLIP_ESC);
    c = SLIP_ESC_XOFF;
  }
#endif /* UART_XONXOFF_FLOW_CTRL */
  slip_arch_writeb(c);
}
/*---------------------------------------------------------------------------*/
uint8_t
slip_write(const void *_ptr, int len)
{
  const uint8_t *ptr = _ptr;
  uint16_t i;
  uint8_t c;

  slip_arch_writeb(SLIP_END);

  for(i = 0; i < len; ++i) {
    c = *ptr++;
    slip_write_char(c);
  }
  slip_arch_writeb(SLIP_END);

  return len;
}
/*---------------------------------------------------------------------------*/
/* slip_send: forward (IPv4) packets with {UIP_FW_NETIF(..., slip_send)}
 * was used in slip-bridge.c
 */
uint8_t
slip_send(void)
{
  uint16_t i;
  uint8_t *ptr;
  uint8_t c;

  slip_arch_writeb(SLIP_END);

  ptr = &uip_buf[UIP_LLH_LEN];
  for(i = 0; i < uip_len; ++i) {
    c = *ptr++;
    slip_write_char(c);
  }
  slip_arch_writeb(SLIP_END);

  return UIP_FW_OK;
}
/*---------------------------------------------------------------------------*/
static void
rxbuf_init(void)
{
  begin = end = end_counter = 0;
  is_dropping = 0;
}
/*---------------------------------------------------------------------------*/
/* Upper half does the polling. */
static uint16_t
slip_poll_handler(uint8_t *outbuf, uint16_t blen)
{
  uint16_t len;
  uint16_t pos;
  uint8_t c;
  uint8_t state;

  if(end_counter == 0 && is_full == 0) {
    return 0;
  }
  for(len = 0, pos = begin, state = c = SLIP_NEUTRAL;
      len < blen + 1; /* +1 for SLIP_END! */
      ) {

    c = rxbuf[pos++];

    if(pos == RX_BUFSIZE) {
      /* Circular buffer: warp around */
      pos = 0;
    }
    if(c == SLIP_END) {
      /* End of packet */
      break;
    }
    if(len >= blen) {
      /* End of buffer with no SLIP_END
       * ==> something wrong happened */
      break;
    }
    switch(c) {
    case SLIP_ESC:
      state = SLIP_ESC;
      break;
    case SLIP_ESC_END:
      if(state == SLIP_ESC) {
        outbuf[len++] = SLIP_END;
        state = SLIP_NEUTRAL;
      } else {
        outbuf[len++] = c;
      } break;
    case SLIP_ESC_ESC:
      if(state == SLIP_ESC) {
        outbuf[len++] = SLIP_ESC;
        state = SLIP_NEUTRAL;
      } else {
        outbuf[len++] = c;
      } break;
#if UART_XONXOFF_FLOW_CTRL
    case SLIP_ESC_XON:
      if(state == SLIP_ESC) {
        outbuf[len++] = XON;
        state = SLIP_NEUTRAL;
      } else {
        outbuf[len++] = c;
      } break;
    case SLIP_ESC_XOFF:
      if(state == SLIP_ESC) {
        outbuf[len++] = XOFF;
        state = SLIP_NEUTRAL;
      } else {
        outbuf[len++] = c;
      } break;
#endif /* UART_XONXOFF_FLOW_CTRL */
    default:
      outbuf[len++] = c;
      state = SLIP_NEUTRAL;
      break;
    }
  }

  /* Update counters */
  if(c == SLIP_END) {
    ATOMIC(begin = pos;
           if(end_counter) {
             end_counter--;
           }
           )
    PUTCHAR('P');
  } else {
    /* Something went wrong, no SLIP_END found, drop everything */
    ATOMIC(rxbuf_init();
           is_dropping = 1;
           )
    SLIP_STATISTICS(slip_error_drop++);
    len = 0;
    PRINTF("SLIP: *** out of sync!\n");
  }

  if(end_counter > 0) {
    /* One more packet is buffered, need to be polled again! */
    process_poll(&slip_process);
  }
  return len;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(slip_process, ev, data)
{
  PROCESS_BEGIN();

  rxbuf_init();

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    /* Move packet from rxbuf to buffer provided by uIP. */
    uip_len = slip_poll_handler(&uip_buf[UIP_LLH_LEN],
                                UIP_BUFSIZE - UIP_LLH_LEN);

    PRINTF("SLIP: recv bytes %u frames RECV: %u. is_full %u, is_dropping %u.\n",
           end_counter, uip_len, is_full, is_dropping);

    /* We have free space now, resume slip RX */
    if(is_full) {
      is_full = 0;
      ENABLE_UART_INTERRUPTS();
    }

    if(uip_len > 0) {
      if(input_callback) {
        input_callback();
      }
#ifdef SLIP_CONF_TCPIP_INPUT
      SLIP_CONF_TCPIP_INPUT();
#else
      tcpip_input();
#endif
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Return status from slip_input_byte:
 * -1 means RX buffer overflow ==> stop reading
 *  0 means do not exit power saving mode
 *  1 means exit power saving mode
 **/
int
slip_input_byte(unsigned char c)
{
  static int in_frame = 0;
  uint16_t next, next_next;
  int error_return_code = is_full ? -1 : 0;
  int success_return_code = is_full ? -1 : 1;

  SLIP_STATISTICS(slip_received++);

#if UART_XONXOFF_FLOW_CTRL
  if(c == XOFF || c == XON) {
    xonxoff_state = c;
    return 1;
  } else {
    /* ANY char would be XON */
    xonxoff_state = XON;
  }
#endif /* UART_XONXOFF_FLOW_CTRL */

  if(is_dropping) {
    /* Make sure to drop full frames when overflow or
     * out of sync happens */
    if(c != SLIP_END) {
      SLIP_STATISTICS(slip_drop_bytes++);
    } else {
      is_dropping = 0;
      in_frame = 0;
    }
    return error_return_code;
  }

  if(!in_frame && c == SLIP_END) {
    /* Ignore slip end when not receiving frame */
    return error_return_code;
    /* increment and wrap */
  }
  next = end + 1;
  if(next >= RX_BUFSIZE) {
    next = 0;
  }
  next_next = next + 1;
  if(next_next >= RX_BUFSIZE) {
    next_next = 0;
    /* Next byte will overflow. Stop accepting. */
  }
  if(next_next == begin) {
    is_full = 1;
    /* disable UART interrupts */
    DISABLE_UART_INTERRUPTS();
    process_poll(&slip_process);
  }

  /* Buffer is full. We can't store anymore.
   * Shall not happen normally,
   * because of overflow protection above. */
  if(next == begin) {
    is_dropping = 1;
    SLIP_STATISTICS(slip_overflow++);
    is_full = 1;
    /* disable UART interrupts */
    DISABLE_UART_INTERRUPTS();
    process_poll(&slip_process);
    return -1;
  }

  rxbuf[end] = c;
  end = next;
  in_frame = 1;

  if(c == SLIP_END) {
    in_frame = 0;
    end_counter++;
    SLIP_STATISTICS(slip_frames++);
    process_poll(&slip_process);
    return success_return_code;
  }
  return error_return_code;
}
/*---------------------------------------------------------------------------*/
#if SLIP_BRIDGE_CONF_NO_PUTCHAR
int
putchar(int c)
{
  uart0_writeb(c);
  return 1;
}
#endif

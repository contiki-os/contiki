/*
 * Copyright (c) 2014, Analog Devices, Inc.
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
 * \author Ian Martin <martini@redwirellc.com>
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "net/netstack.h"

#include "dev/serial-line.h"

#include "net/ip/uip.h"

#include "dev/button-sensor.h"

#if NETSTACK_CONF_WITH_IPV6
#include "net/ipv6/uip-ds6.h"
#endif /* NETSTACK_CONF_WITH_IPV6 */

#include "net/rime/rime.h"
#include "uart0.h"
#include "contiki-uart.h"
#include "watchdog.h"
#include "slip-arch.h"

#if __GNUC__
#include "write.h"
#endif

SENSORS(&button_sensor);

#ifndef SERIAL_ID
#define SERIAL_ID { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 }
#endif

static uint8_t serial_id[] = SERIAL_ID;
static uint16_t node_id = 0x0102;

/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  linkaddr_t addr;
  int i;

  memset(&addr, 0, sizeof(linkaddr_t));
#if NETSTACK_CONF_WITH_IPV6
  memcpy(addr.u8, serial_id, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < sizeof(linkaddr_t); ++i) {
      addr.u8[i] = serial_id[7 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
#endif
  linkaddr_set_node_addr(&addr);
  printf("Rime started with address ");
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    printf("%d.", addr.u8[i]);
  }
  printf("%d" NEWLINE, addr.u8[i]);
}
/*---------------------------------------------------------------------------*/
int contiki_argc = 0;
char **contiki_argv;

static void
delay_1sec(void)
{
  /* Delay 1 second */
  register unsigned long int i;
  for(i = 0x000FFFFFUL; i; --i) {
    asm ("nop");
  }
}
int
main(int argc, char **argv)
{
  bool flip_flop = false;

  asm ("di");
  /* Setup clocks */
  CMC = 0x11U;                                        /* Enable XT1, disable X1 */
  CSC = 0x80U;                                        /* Start XT1 and HOCO, stop X1 */
  CKC = 0x00U;
  delay_1sec();
  OSMC = 0x00;                                       /* Supply fsub to peripherals, including Interval Timer */
  uart0_init();

#if __GNUC__
  /* Force linking of custom write() function: */
  write(1, NULL, 0);
#endif

  /* Setup 12-bit interval timer */
  RTCEN = 1;                                              /* Enable 12-bit interval timer and RTC */
  ITMK = 1;                                               /* Disable IT interrupt */
  ITPR0 = 0;                                              /* Set interrupt priority - highest */
  ITPR1 = 0;
  ITMC = 0x8FFFU;                                    /* Set maximum period 4096/32768Hz = 1/8 s, and start timer */
  ITIF = 0;                                               /* Clear interrupt request flag */
  ITMK = 0;                                               /* Enable IT interrupt */
  /* asm ("ei");                                             / * Enable interrupts * / */

  /* Disable analog inputs because they can conflict with the SPI buses: */
  ADPC = 0x01;  /* Configure all analog pins as digital I/O. */
  PMC0 &= 0xF0; /* Disable analog inputs. */

  clock_init();

  /* Initialize Joystick Inputs: */
  PM5 |= BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1); /* Set pins as inputs. */
  PU5 |= BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1); /* Enable internal pull-up resistors. */

  /* Initialize LED outputs: */
#define BIT(n) (1 << (n))
  PM12 &= ~BIT(0); /* LED1 */
  PM4 &= ~BIT(3);  /* LED2 */
  PM1 &= ~BIT(6);  /* LED3 */
  PM1 &= ~BIT(5);  /* LED4 */
  PM0 &= ~BIT(6);  /* LED5 */
  PM0 &= ~BIT(5);  /* LED6 */
  PM3 &= ~BIT(0);  /* LED7 */
  PM5 &= ~BIT(0);  /* LED8 */

#if NETSTACK_CONF_WITH_IPV6
#if UIP_CONF_IPV6_RPL
  printf(CONTIKI_VERSION_STRING " started with IPV6, RPL" NEWLINE);
#else
  printf(CONTIKI_VERSION_STRING " started with IPV6" NEWLINE);
#endif
#else
  printf(CONTIKI_VERSION_STRING " started" NEWLINE);
#endif

  /* crappy way of remembering and accessing argc/v */
  contiki_argc = argc;
  contiki_argv = argv;

  process_init();
  process_start(&etimer_process, NULL);
  ctimer_init();

  set_rime_addr();

  queuebuf_init();

  netstack_init();
  printf("MAC %s RDC %s NETWORK %s" NEWLINE, NETSTACK_MAC.name, NETSTACK_RDC.name, NETSTACK_NETWORK.name);

#if NETSTACK_CONF_WITH_IPV6
  memcpy(&uip_lladdr.addr, serial_id, sizeof(uip_lladdr.addr));

  process_start(&tcpip_process, NULL);
  printf("Tentative link-local IPv6 address ");
  {
    uip_ds6_addr_t *lladdr;
    int i;
    lladdr = uip_ds6_get_link_local(-1);
    for(i = 0; i < 7; ++i) {
      printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
             lladdr->ipaddr.u8[i * 2 + 1]);
    }
    /* make it hardcoded... */
    lladdr->state = ADDR_AUTOCONF;

    printf("%02x%02x" NEWLINE, lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
  }
#elif NETSTACK_CONF_WITH_IPV4
  process_start(&tcpip_process, NULL);
#endif

  serial_line_init();

  autostart_start(autostart_processes);

  while(1) {
    watchdog_periodic();

    if(NETSTACK_RADIO.pending_packet()) {
      int len;
      packetbuf_clear();
      len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
      if(len > 0) {
        packetbuf_set_datalen(len);
        NETSTACK_RDC.input();
      }
    }

    while(uart0_can_getchar()) {
      char c;
      UART_RX_LED = 1;
      c = uart0_getchar();
      if(uart0_input_handler) {
        uart0_input_handler(c);
      }
    }
    UART_RX_LED = 0;

    process_run();

    etimer_request_poll();

    HEARTBEAT_LED1 = flip_flop;
    flip_flop = !flip_flop;
    HEARTBEAT_LED2 = flip_flop;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
log_message(char *m1, char *m2)
{
  printf("%s%s" NEWLINE, m1, m2);
}
/*---------------------------------------------------------------------------*/
void
uip_log(char *m)
{
  printf("%s" NEWLINE, m);
}
/*---------------------------------------------------------------------------*/

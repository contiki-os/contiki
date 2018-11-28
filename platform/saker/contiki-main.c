/*
 * Copyright (c) 2017, Weptech elektronik GmbH Germany
 * http://www.weptech.de
 *
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
 * \addtogroup cc2538-platforms
 *
 * @{
 *
 * \defgroup saker The Saker IoT development platform
 *
 * The Saker is a platform sold and maintained by Weptech elektronik GmbH, Germany.
 * It is based on the CC2538 SoC supplemented with a CC1200 sub-GHz transceiver
 * allowing for dual-band operation in the 2.4 and the sub-GHz bands.
 * A Microchip enc28j60 ethernet controller connected via a separate
 * SPI provides a 10BASE-T interface allowing to build low-cost 6LoWPAN
 * gateway solutions.
 *
 * @{
 *
 * \file
 *      Main module for the Saker platform
 */

#include "contiki.h"

#include "cpu.h"
#include "reg.h"
#include "ieee-addr.h"

#include "dev/leds.h"
#include "dev/sys-ctrl.h"
#include "dev/nvic.h"
#include "dev/uart.h"
#include "dev/watchdog.h"
#include "dev/ioc.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/cc2538-rf.h"
#include "dev/udma.h"

#include "usb/usb-serial.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/queuebuf.h"

#include "lpm.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
#if STARTUP_CONF_VERBOSE
#include "cc1200-conf.h"
#include "cc1200-rf-cfg.h"
#define PRINTF(...) printf(__VA_ARGS__)
#else /* STARTUP_CONF_VERBOSE */
#define PRINTF(...)
#endif /* STARTUP_CONF_VERBOSE */

#if UART_CONF_ENABLE
#define PUTS(s) puts(s)
#else
#define PUTS(s)
#endif
/*---------------------------------------------------------------------------*/
/* Import the rf configuration set by CC1200_RF_CFG */
#if PLATFORM_USE_CC1200 && STARTUP_CONF_VERBOSE
extern const cc1200_rf_cfg_t CC1200_RF_CFG;
#endif /* PLATFORM_USE_CC1200 */
/*---------------------------------------------------------------------------*/
static void
fade(unsigned char l)
{

  volatile int i;
  int k, j;

#if (SYS_CTRL_CONF_SYS_DIV == SYS_CTRL_CLOCK_CTRL_SYS_DIV_32MHZ)
#define FADE_DELAY 1200
#else
#define FADE_DELAY 800
#endif

  for(k = 0; k < FADE_DELAY; ++k) {
    j = k > (FADE_DELAY >> 1) ? FADE_DELAY - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      asm ("nop");
    }
    leds_off(l);
    for(i = 0; i < (FADE_DELAY >> 1) - j; ++i) {
      asm ("nop");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_rf_params()
{

  uint16_t short_addr;
  uint8_t ext_addr[8];

  ieee_addr_cpy_to(ext_addr, 8);

  short_addr = ext_addr[7];
  short_addr |= ext_addr[6] << 8;

  /* Populate linkaddr_node_addr. Maintain endianness */
  memcpy(&linkaddr_node_addr, &ext_addr[8 - LINKADDR_SIZE], LINKADDR_SIZE);

#if STARTUP_CONF_VERBOSE
  {
    int i;
    PRINTF("EUI-64 (wireless interface):\n ");
    for(i = 0; i < LINKADDR_SIZE - 1; i++) {
      PRINTF("%02x:", linkaddr_node_addr.u8[i]);
    }
    PRINTF("%02x\n", linkaddr_node_addr.u8[i]);
  }
#endif

#if (!PLATFORM_USE_CC1200)

  NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, IEEE802154_PANID);
  NETSTACK_RADIO.set_value(RADIO_PARAM_16BIT_ADDR, short_addr);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CC2538_RF_CHANNEL);
  NETSTACK_RADIO.set_object(RADIO_PARAM_64BIT_ADDR, ext_addr, 8);

#else

  /*
   * Default parameters are already set at compile time using the
   * configuration parameters given in cc1200-conf.h.
   * No need to update any of them here.
   */

#endif
}
/*---------------------------------------------------------------------------*/
void
send_startup_messages()
{
  PUTS(CONTIKI_VERSION_STRING);
  PUTS(BOARD_STRING);

  PRINTF(" Net       : ");
  PRINTF("%s\n", NETSTACK_NETWORK.name);
  PRINTF(" MAC       : ");
  PRINTF("%s\n", NETSTACK_MAC.name);
  PRINTF(" RDC       : ");
  PRINTF("%s\n", NETSTACK_RDC.name);

  PRINTF(" RF channel: ");
#if PLATFORM_USE_CC1200
  PRINTF("%d\n", CC1200_DEFAULT_CHANNEL);
#else /* PLATFORM_CONF_USE_CC1200 */
  PRINTF("%d\n", CC2538_RF_CHANNEL);
#endif /* PLATFORM_CONF_USE_CC1200 */

#if PLATFORM_USE_CC1200
  PRINTF(" RF cfg    : ");
  PRINTF("%s\n", CC1200_RF_CFG.cfg_descriptor);
#endif /* PLATFORM_CONF_USE_CC1200 */
}
/*---------------------------------------------------------------------------*/
/*
 * Character I/O Initialisation.
 * For both USB and UART we set serial_line_input_byte as a handler for incoming
 * data.
 *
 * If SLIP is enabled, (e.g. if we are a border router), the slip driver will
 * overwrite one of these handlers with its own handler, slip_input_byte. Which
 * one is overwritten depends on the setting SLIP_ARCH_CONF_USB.
 */
void
initialize_character_io()
{

#if UART_CONF_ENABLE
  uart_init(0);
  uart_set_input(SERIAL_LINE_CONF_UART, serial_line_input_byte);
#endif

#if USB_SERIAL_CONF_ENABLE
  usb_serial_init();
  usb_serial_set_input(serial_line_input_byte);
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Main routine for the saker platform.
 */
int
main(void)
{

  /* Processor initialization */
  nvic_init();
  ioc_init();
  sys_ctrl_init();
  clock_init();
  lpm_init();
  rtimer_init();
  gpio_init();
  watchdog_init();
  initialize_character_io();
  random_init(0);
  udma_init();

  /* Relaxen und watchen das Blinkenlichten. */
  leds_init();
  fade(LEDS_RED);

  /* Contiki initialization */
  process_init();
  serial_line_init();
  INTERRUPTS_ENABLE();
  process_start(&etimer_process, NULL);
  ctimer_init();

  /* Say hello, configure networking, energest */
  fade(LEDS_YELLOW);
  send_startup_messages();

  set_rf_params();
  netstack_init();

#if NETSTACK_CONF_WITH_IPV6
  memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
  queuebuf_init();
  process_start(&tcpip_process, NULL);
#endif /* NETSTACK_CONF_WITH_IPV6 */

  /* All done. Start processes and run them. */
  fade(LEDS_GREEN);

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  process_start(&sensors_process, NULL);
  autostart_start(autostart_processes);
  watchdog_start();

  while(1) {
    uint8_t r;
    do {
      /* Reset watchdog and handle polls and events */
      watchdog_periodic();
      r = process_run();
    } while(r > 0);

    /* We have serviced all pending events. Enter a Low-Power mode. */
    lpm_enter();
  }
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */

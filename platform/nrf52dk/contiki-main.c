/*
 * Copyright (c) 2015, Nordic Semiconductor
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup nrf52dk nRF52 Development Kit
 * @{
 */
#include <stdio.h>
#include <stdint.h>

#include "nordic_common.h"
#include "nrf_drv_config.h"
#include "nrf_drv_gpiote.h"
#ifdef SOFTDEVICE_PRESENT
#include "softdevice_handler.h"
#include "ble/ble-core.h"
#include "ble/ble-mac.h"
#endif

#include "contiki.h"
#include "contiki-net.h"
#include "leds.h"
#include "lib/sensors.h"

#include "dev/watchdog.h"
#include "dev/serial-line.h"
#include "dev/uart0.h"
#include "dev/lpm.h"

#define DEBUG 0

#if NETSTACK_CONF_WITH_IPV6
#include "uip-debug.h"
#include "net/ipv6/uip-ds6.h"
#else
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#endif

#if defined(SOFTDEVICE_PRESENT) && PLATFORM_INDICATE_BLE_STATE
PROCESS(ble_iface_observer, "BLE interface observer");

/**
 * \brief A process that handles adding/removing
 *        BLE IPSP interfaces.
 */
PROCESS_THREAD(ble_iface_observer, ev, data)
{
  static struct etimer led_timer;

  PROCESS_BEGIN();

  etimer_set(&led_timer, CLOCK_SECOND/2);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == ble_event_interface_added) {
      etimer_stop(&led_timer);
      leds_off(LEDS_1);
      leds_on(LEDS_2);
    } else if(ev == ble_event_interface_deleted) {
      etimer_set(&led_timer, CLOCK_SECOND/2);
      leds_off(LEDS_2);
    } else if(ev == PROCESS_EVENT_TIMER && etimer_expired(&led_timer)) {
      etimer_reset(&led_timer);
      leds_toggle(LEDS_1);
    }
  }
  PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
/**
 * \brief Board specific initialization
 *
 * This function will enable SoftDevice is present.
 */
static void
board_init(void)
{
#ifdef SOFTDEVICE_PRESENT
  /* Initialize the SoftDevice handler module */
  SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
#endif
#ifdef PLATFORM_HAS_BUTTON
  if (!nrf_drv_gpiote_is_init()) {
    nrf_drv_gpiote_init();
  }
#endif
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Main function for nRF52dk platform.
 * \note This function doesn't return.
 */
int
main(void)
{
  board_init();
  leds_init();

  clock_init();
  rtimer_init();

  watchdog_init();
  process_init();

  // Seed value is ignored since hardware RNG is used.
  random_init(0);

#ifdef UART0_ENABLED
  uart0_init();
#if SLIP_ARCH_CONF_ENABLE
  slip_arch_init(0);
#else
  uart0_set_input(serial_line_input_byte);
  serial_line_init();
#endif
#endif

  PRINTF("Starting " CONTIKI_VERSION_STRING "\n");

  process_start(&etimer_process, NULL);
  ctimer_init();

#if ENERGEST_CONF_ON
  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);
#endif

#ifdef SOFTDEVICE_PRESENT
  ble_stack_init();
  ble_advertising_init(DEVICE_NAME);

#if NETSTACK_CONF_WITH_IPV6
  netstack_init();
  linkaddr_t linkaddr;
  ble_get_mac(linkaddr.u8);
  /* Set link layer address */
  linkaddr_set_node_addr(&linkaddr);
  /* Set device link layer address in uip stack */
  memcpy(&uip_lladdr.addr, &linkaddr, sizeof(uip_lladdr.addr));
  process_start(&ble_iface_observer, NULL);
  process_start(&tcpip_process, NULL);
#endif /* NETSTACK_CONF_WITH_IPV6 */
#endif /* SOFTDEVICE_PRESENT */

  process_start(&sensors_process, NULL);
  autostart_start(autostart_processes);

  watchdog_start();

#ifdef SOFTDEVICE_PRESENT
  ble_advertising_start();
  PRINTF("Advertising name [%s]\n", DEVICE_NAME);
#endif

  while(1) {
    uint8_t r;
    do {
      r = process_run();
      watchdog_periodic();
    } while(r > 0);

    lpm_drop();
  }
}
/**
 * @}
 */

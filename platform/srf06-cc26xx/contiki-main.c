/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-platforms
 * @{
 *
 * \defgroup cc26xx-srf-tag SmartRF+CC13xx/CC26xx EM, CC2650 SensorTag and LaunchPads
 *
 * This platform supports a number of different boards:
 * - A standard TI SmartRF06EB with a CC26xx EM mounted on it
 * - A standard TI SmartRF06EB with a CC1310 EM mounted on it
 * - The new TI SensorTag2.0
 * - The TI CC2650 LaunchPad
 * - The TI CC1310 LaunchPad
 * @{
 */
#include "ti-lib.h"
#include "contiki.h"
#include "contiki-net.h"
#include "leds.h"
#include "lpm.h"
#include "gpio-interrupt.h"
#include "dev/watchdog.h"
#include "dev/oscillators.h"
#include "ieee-addr.h"
#include "vims.h"
#include "dev/cc26xx-uart.h"
#include "dev/soc-rtc.h"
#include "rf-core/rf-core.h"
#include "sys_ctrl.h"
#include "uart.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "sys/node-id.h"
#include "lib/random.h"
#include "lib/sensors.h"
#include "button-sensor.h"
#include "dev/serial-line.h"
#include "net/mac/frame802154.h"

#include "driverlib/driverlib_release.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
unsigned short node_id = 0;
/*---------------------------------------------------------------------------*/
/** \brief Board specific iniatialisation */
void board_init(void);
/*---------------------------------------------------------------------------*/
static void
fade(unsigned char l)
{
  volatile int i;
  int k, j;
  for(k = 0; k < 800; ++k) {
    j = k > 400 ? 800 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      __asm("nop");
    }
    leds_off(l);
    for(i = 0; i < 400 - j; ++i) {
      __asm("nop");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_rf_params(void)
{
  uint16_t short_addr;
  uint8_t ext_addr[8];
  radio_value_t val = 0;

  ieee_addr_cpy_to(ext_addr, 8);

  short_addr = ext_addr[7];
  short_addr |= ext_addr[6] << 8;

  /* Populate linkaddr_node_addr. Maintain endianness */
  memcpy(&linkaddr_node_addr, &ext_addr[8 - LINKADDR_SIZE], LINKADDR_SIZE);

  NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, IEEE802154_PANID);
  NETSTACK_RADIO.set_value(RADIO_PARAM_16BIT_ADDR, short_addr);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, RF_CORE_CHANNEL);
  NETSTACK_RADIO.set_object(RADIO_PARAM_64BIT_ADDR, ext_addr, 8);

  NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &val);
  printf(" RF: Channel %d\n", val);

#if STARTUP_CONF_VERBOSE
  {
    int i;
    printf(" Link layer addr: ");
    for(i = 0; i < LINKADDR_SIZE - 1; i++) {
      printf("%02x:", linkaddr_node_addr.u8[i]);
    }
    printf("%02x\n", linkaddr_node_addr.u8[i]);
  }
#endif

  /* also set the global node id */
  node_id = short_addr;
  printf(" Node ID: %d\n", node_id);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Main function for CC26xx-based platforms
 *
 * The same main() is used for all supported boards
 */
int
main(void)
{
  /* Enable flash cache and prefetch. */
  ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_ENABLED);
  ti_lib_vims_configure(VIMS_BASE, true, true);

  ti_lib_int_master_disable();

  /* Set the LF XOSC as the LF system clock source */
  oscillators_select_lf_xosc();

  lpm_init();

  board_init();

  gpio_interrupt_init();

  leds_init();

  /*
   * Disable I/O pad sleep mode and open I/O latches in the AON IOC interface
   * This is only relevant when returning from shutdown (which is what froze
   * latches in the first place. Before doing these things though, we should
   * allow software to first regain control of pins
   */
  ti_lib_pwr_ctrl_io_freeze_disable();

  fade(LEDS_RED);

  ti_lib_int_master_enable();

  soc_rtc_init();
  clock_init();
  rtimer_init();

  watchdog_init();
  process_init();

  random_init(0x1234);

  /* Character I/O Initialisation */
#if CC26XX_UART_CONF_ENABLE
  cc26xx_uart_init();
#endif

  serial_line_init();

  printf("Starting " CONTIKI_VERSION_STRING "\n");
  printf("With DriverLib v%u.%u\n", DRIVERLIB_RELEASE_GROUP,
         DRIVERLIB_RELEASE_BUILD);
  printf(BOARD_STRING "\n");
  printf("IEEE 802.15.4: %s, Sub-GHz: %s, BLE: %s, Prop: %s\n",
         ti_lib_chipinfo_supports_ieee_802_15_4() == true ? "Yes" : "No",
         ti_lib_chipinfo_chip_family_is_cc13xx() == true ? "Yes" : "No",
         ti_lib_chipinfo_supports_ble() == true ? "Yes" : "No",
         ti_lib_chipinfo_supports_proprietary() == true ? "Yes" : "No");

  process_start(&etimer_process, NULL);
  ctimer_init();

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  fade(LEDS_YELLOW);

  printf(" Net: ");
  printf("%s\n", NETSTACK_NETWORK.name);
  printf(" MAC: ");
  printf("%s\n", NETSTACK_MAC.name);
  printf(" RDC: ");
  printf("%s", NETSTACK_RDC.name);

  if(NETSTACK_RDC.channel_check_interval() != 0) {
    printf(", Channel Check Interval: %u ticks",
           NETSTACK_RDC.channel_check_interval());
  }
  printf("\n");

  netstack_init();

  set_rf_params();

#if NETSTACK_CONF_WITH_IPV6
  memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
  queuebuf_init();
  process_start(&tcpip_process, NULL);
#endif /* NETSTACK_CONF_WITH_IPV6 */

  fade(LEDS_GREEN);

  process_start(&sensors_process, NULL);

  autostart_start(autostart_processes);

  watchdog_start();

  fade(LEDS_ORANGE);

  while(1) {
    uint8_t r;
    do {
      r = process_run();
      watchdog_periodic();
    } while(r > 0);

    /* Drop to some low power mode */
    lpm_drop();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

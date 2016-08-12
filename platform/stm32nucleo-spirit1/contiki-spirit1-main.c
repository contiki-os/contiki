/*
 * Copyright (c) 2012, STMicroelectronics.
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
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup stm32nucleo-spirit1
 * @{
 *
 * \file
 * main file for stm32nucleo-spirit1 platform
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32cube_hal_init.h"
#include "contiki.h"
#include "contiki-net.h"
#include "sys/autostart.h"
#include "dev/leds.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/watchdog.h"
#include "dev/xmem.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/ip/uip.h"
#include "net/mac/frame802154.h"
#include "net/rime/rime.h"
#include "stm32l1xx.h"
#include "SPIRIT_Config.h"
#include "SPIRIT_Management.h"
#include "spirit1.h"
#include "spirit1-arch.h"
#include "node-id.h"
#include "hw-config.h"
#include "stdbool.h"
#include "dev/button-sensor.h"
#include "dev/radio-sensor.h"
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV6
#include "net/ipv6/uip-ds6.h"
#endif /*NETSTACK_CONF_WITH_IPV6*/
/*---------------------------------------------------------------------------*/
#ifdef X_NUCLEO_IKS01A1
extern const struct sensors_sensor temperature_sensor;
extern const struct sensors_sensor humidity_sensor;
extern const struct sensors_sensor pressure_sensor;
extern const struct sensors_sensor magneto_sensor;
extern const struct sensors_sensor acceleration_sensor;
extern const struct sensors_sensor gyroscope_sensor;
SENSORS(&button_sensor,
        &radio_sensor,
        &temperature_sensor,
        &humidity_sensor,
        &pressure_sensor,
        &magneto_sensor,
        &acceleration_sensor,
        &gyroscope_sensor);
#else /*X_NUCLEO_IKS01A1*/
SENSORS(&button_sensor,
        &radio_sensor);
#endif /*X_NUCLEO_IKS01A1*/
/*---------------------------------------------------------------------------*/
extern unsigned char node_mac[8];
/*---------------------------------------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE * f)
#endif /* __GNUC__ */
/*---------------------------------------------------------------------------*/
#if NETSTACK_CONF_WITH_IPV6
PROCINIT(&etimer_process, &tcpip_process);
#else /*NETSTACK_CONF_WITH_IPV6*/
PROCINIT(&etimer_process);
#warning "No TCP/IP process!"
#endif /*NETSTACK_CONF_WITH_IPV6*/
/*---------------------------------------------------------------------------*/
#define BUSYWAIT_UNTIL(cond, max_time) \
  do { \
    rtimer_clock_t t0; \
    t0 = RTIMER_NOW(); \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) ; \
  } while(0)
/*---------------------------------------------------------------------------*/
static void set_rime_addr(void);
void stm32cube_hal_init();
/*---------------------------------------------------------------------------*/
#if 0
static void
panic_main(void)
{
  volatile uint16_t k;
  while(1) {
    leds_toggle(LEDS_ALL);
    for(k = 0; k < 0xffff / 8; k += 1) {
    }
  }
}
#endif
/*---------------------------------------------------------------------------*/
int
main(int argc, char *argv[])
{
  stm32cube_hal_init();

  /* init LEDs */
  leds_init();

  /* Initialize Contiki and our processes. */
  clock_init();
  ctimer_init();
  rtimer_init();
  watchdog_init();
  process_init();
  process_start(&etimer_process, NULL);

  /* Restore node id if such has been stored in external mem */
  node_id_restore(); /* also configures node_mac[] */

  set_rime_addr();
  random_init(node_id);

  netstack_init();
  spirit_radio_driver.on();

  energest_init();

#if NETSTACK_CONF_WITH_IPV6
  memcpy(&uip_lladdr.addr, node_mac, sizeof(uip_lladdr.addr));

  queuebuf_init();
  process_start(&tcpip_process, NULL);

  uip_ipaddr_t ipaddr;
  uip_ip6addr(&ipaddr, 0xfc00, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
#endif /* NETSTACK_CONF_WITH_IPV6*/

  process_start(&sensors_process, NULL);

  autostart_start(autostart_processes);

  watchdog_start();

  while(1) {
    int r = 0;
    do {
      r = process_run();
    } while(r > 0);
  }
}
/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  linkaddr_t addr;

  memset(&addr, 0, sizeof(linkaddr_t));
  memcpy(addr.u8, node_mac, sizeof(addr.u8));

  linkaddr_set_node_addr(&addr);
}
/*---------------------------------------------------------------------------*/
/** @} */

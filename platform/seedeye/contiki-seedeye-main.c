/*
 * Contiki SeedEye Platform project
 *
 * Copyright (c) 2012,
 *  Scuola Superiore Sant'Anna (http://www.sssup.it) and
 *  Consorzio Nazionale Interuniversitario per le Telecomunicazioni
 *  (http://www.cnit.it).
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

/**
 * \addtogroup platform
 * @{ */

/**
 * \defgroup SeedEye Contiki SEEDEYE Platform
 *
 * @{
 */

/**
 * \file   contiki-seedeye-main.c
 * \brief  Main program for the SEEDEYE port.
 * \author Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
 * \date   2012-03-21
 */

#include <contiki.h>
#include <clock.h>
#include <lib/random.h>
#include <dev/leds.h>
#include "dev/button-sensor.h"
#include "dev/battery-sensor.h"
#include <dev/watchdog.h>

#include <pic32.h>
#include <pic32_clock.h>

#include <debug-uart.h>

#include <init-net.h>

#include <stdio.h>
#include <string.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if UIP_CONF_ROUTER
#ifndef UIP_ROUTER_MODULE
#ifdef UIP_CONF_ROUTER_MODULE
#define UIP_ROUTER_MODULE UIP_CONF_ROUTER_MODULE
#else /* UIP_CONF_ROUTER_MODULE */
#define UIP_ROUTER_MODULE rimeroute
#endif /* UIP_CONF_ROUTER_MODULE */
#endif /* UIP_ROUTER_MODULE */
extern const struct uip_router UIP_ROUTER_MODULE;
#endif /* UIP_CONF_ROUTER */

SENSORS(&button_sensor);

/*---------------------------------------------------------------------------*/
static void
print_processes(struct process *const processes[])
{
  PRINTF("Starting:\n");

  while(*processes != NULL) {
    PRINTF(" '%s'\n", (*processes)->name);
    processes++;
  }

  PRINTF("\n");
}
/*---------------------------------------------------------------------------*/
int
main(int argc, char **argv)
{
  int32_t r;
  
  /* Initalizing main hardware */
  pic32_init();
  watchdog_init();
  leds_init();
  
  clock_init();

  dbg_setup_uart(UART_DEBUG_BAUDRATE);
  
  PRINTF("Initialising Node: %d\n", SEEDEYE_ID);

  PRINTF("CPU Clock: %uMhz\n", pic32_clock_get_system_clock() / 1000000);
  PRINTF("Peripheral Clock: %uMhz\n", pic32_clock_get_peripheral_clock() / 1000000);
 
  random_init(SEEDEYE_ID);

  process_init();
  process_start(&etimer_process, NULL);
  ctimer_init();
  rtimer_init();

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);
  
  process_start(&sensors_process, NULL);
  SENSORS_ACTIVATE(battery_sensor);

  leds_on(LEDS_RED);

  /* Inizialize Network! */

  init_net(SEEDEYE_ID);

  leds_on(LEDS_RED);

  /* Starting autostarting process */
  print_processes(autostart_processes);
  autostart_start(autostart_processes);

  PRINTF("Processes running\n");

  leds_off(LEDS_ALL);

  watchdog_start();
  
  /*
   * This is the scheduler loop.
   */
  while(1) {

    do {
      /* Reset watchdog. */
      watchdog_periodic();
      r = process_run();
    } while(r > 0);

    ENERGEST_SWITCH(ENERGEST_TYPE_CPU, ENERGEST_TYPE_LPM);

    watchdog_stop();
    asm volatile("wait");
    watchdog_start();

    ENERGEST_SWITCH(ENERGEST_TYPE_LPM, ENERGEST_TYPE_CPU);

  }

  return 0;
}
/*---------------------------------------------------------------------------*/

/** @} */
/** @} */


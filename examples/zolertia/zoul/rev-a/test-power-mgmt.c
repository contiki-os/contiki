/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
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
 * \addtogroup remote-power-management-test
 * @{
 *
 * Test the RE-Mote's (revision A) power management features, shutdown mode and
 * battery management
 *
 * @{
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 *         Aitor Mejias <amejias@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "cpu.h"
#include "sys/etimer.h"
#include "sys/process.h"
#include "dev/leds.h"
#include "dev/sys-ctrl.h"
#include "dev/gpio.h"
#include "lib/list.h"
#include "power-mgmt.h"
#include "net/rime/broadcast.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* RE-Mote revision A */
#define PM_EXPECTED_VERSION  0x00
#define ENTER_SHUTDOWN_COUNT   10
/*---------------------------------------------------------------------------*/
PROCESS(test_remote_pm, "RE-Mote Power Management Test");
AUTOSTART_PROCESSES(&test_remote_pm);
/*---------------------------------------------------------------------------*/
static struct etimer et;
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  leds_toggle(LEDS_BLUE);
  printf("*** Received %u bytes from %u:%u: '0x%04x'\n", packetbuf_datalen(),
         from->u8[0], from->u8[1], *(uint16_t *)packetbuf_dataptr());
}
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks bc_rx = { broadcast_recv };
static struct broadcast_conn bc;
/*---------------------------------------------------------------------------*/
static char *
print_pm(uint8_t state)
{
  switch(state) {
  case PM_SYSOFF_ON:
    return "Battery on";
  case PM_SYSOFF_OFF:
    return "Battery off";
  case PM_TIMER_ENABLED:
    return "Nano Timer enabled";
  case PM_TIMER_DISABLED:
    return "Nano Timer disabled";
  case PM_AWAITING_RTC_EVENT:
    return "Awaiting RTC event";
  default:
    return "UNKNOWN";
  }
}
/*---------------------------------------------------------------------------*/
static int8_t
get_status(uint8_t mask, uint8_t *val)
{
  uint8_t status, print_msg;

  /* Retrieve the status of the power management block */
  if(pm_get_state(&status) != PM_SUCCESS) {
    printf("Failed to retrieve the power management status\n");
    return PM_ERROR;
  }

  if(!mask) {
    printf("STATUS %u\n", status);
    *val = PM_IDLE;
    return PM_SUCCESS;
  }

  /* Read back ony the requested status bit */
  switch(mask) {
  case PM_SYSOFF_ON_MASK:
    print_msg = (status & mask) ? PM_SYSOFF_ON : PM_SYSOFF_OFF;
    break;
  case PM_TIMER_ENABLED_MASK:
    print_msg = (status & mask) ? PM_TIMER_ENABLED : PM_TIMER_DISABLED;
    break;
  case PM_AWAITING_RTC_EVENT_MASK:
    print_msg = (status & mask) ? PM_AWAITING_RTC_EVENT : PM_AWAITING_RTC_DIS;
    break;
  default:
    return PM_ERROR;
  }

  printf("Status -> %s\n", print_pm(print_msg));
  *val = print_msg;
  return PM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_remote_pm, ev, data)
{
  static uint8_t aux;

  PROCESS_BEGIN();

  /* Configures the pins and initializes the driver */
  if(pm_init() != PM_SUCCESS) {
    printf("Failed to initialize\n");
    PROCESS_EXIT();
  }

  /* Get the current firmware version to track the driver implementation on the
   * low-power MCU
   */
  if(pm_get_firmware_version(&aux) != PM_SUCCESS) {
    printf("Failed to retrieve PIC's fw version\n");
    PROCESS_EXIT();
  }
  printf("Firmware PIC-Power Manager Version: V0.%u\n", aux);

  if(aux != PM_EXPECTED_VERSION) {
    printf("Unexpected firmware version\n");
    PROCESS_EXIT();
  }

  /* Enables the nano timer, the power management block will be driven by the
   * nano timer now, putting the node in shutdown mode every minute (as
   * default).  For this test you need to disconnect the USB cable off to power
   * the RE-Mote ONLY from the external battery, so after veryfing the above
   * works, disconnect the USB cable and hit the reset button
   * When the nano timer is enabled, the external battery is shutdown for a
   * couple of usecs, so the CC2538 is restarted.  After this as the nano timer
   * is enabled, then it will run as intended
   */

  if(get_status(PM_TIMER_ENABLED_MASK, &aux) != PM_SUCCESS) {
    PROCESS_EXIT();
  }

  if(aux == PM_TIMER_DISABLED) {
    printf("Enabling the nano Timer...\n");
    if(pm_enable_timer() != PM_SUCCESS) {
      printf("Failed to set the nano Timer\n");
      PROCESS_EXIT();
    }
  }

  /* At this point as the RE-Mote is powered over USB you should see the prints,
   * disconnect the USB cable and power only with the external battery.  If
   * something fails, then you should not see the red LED blinking
   */
  aux = ENTER_SHUTDOWN_COUNT;

  /* Open the broadcast channel */
  broadcast_open(&bc, BROADCAST_CHANNEL, &bc_rx);

  /* Send a message */
  packetbuf_copyfrom(&aux, sizeof(aux));
  broadcast_send(&bc);

  /* And wait a few seconds before going to sleep */
  while(1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT();

    /* Enter shutdown mode before the shutdown period (1 min default) expires */
    if(!aux) {
      /* Say goodnight */
      PM_SHUTDOWN_NOW;
      printf("Goodnight!\n");
      PROCESS_EXIT();
    }

    aux--;
    leds_toggle(LEDS_RED);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */


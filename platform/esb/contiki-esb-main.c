/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "contiki-esb.h"

#include "dev/watchdog.h"
#include "sys/autostart.h"
#include "net/uip-driver.h"
#include "net/netstack.h"

#if WITH_UIP

static struct uip_fw_netif tr1001if =
  {UIP_FW_NETIF(0,0,0,0, 0,0,0,0, uip_driver_send)};

#if WITH_SLIP
static struct uip_fw_netif slipif =
  {UIP_FW_NETIF(172,16,0,0, 255,255,255,0, slip_send)};
#endif /* WITH_SLIP */

#endif /* WITH_UIP */

#ifdef DCOSYNCH_CONF_PERIOD
#define DCOSYNCH_PERIOD DCOSYNCH_CONF_PERIOD
#else
#define DCOSYNCH_PERIOD 30
#endif /* DCOSYNCH_CONF_PERIOD */

#ifdef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_ENABLED DCOSYNCH_CONF_ENABLED
#else
#define DCOSYNCH_ENABLED 0
#endif /* DCOSYNCH_CONF_ENABLED */

#if DCOSYNCH_ENABLED
static struct timer dco_timer;
#endif /* DCOSYNCH_ENABLED */

SENSORS(&button_sensor, &sound_sensor, &vib_sensor,
	&pir_sensor, &radio_sensor, &battery_sensor, &ctsrts_sensor,
	&temperature_sensor);

/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  int i;
  rimeaddr_t rimeaddr;

  rimeaddr.u8[0] = node_id & 0xff;
  rimeaddr.u8[1] = node_id >> 8;
  rimeaddr_set_node_addr(&rimeaddr);

  printf("Rime started with address ");
  for(i = 0; i < sizeof(rimeaddr.u8) - 1; i++) {
    printf("%u.", rimeaddr.u8[i]);
  }
  printf("%u\n", rimeaddr.u8[i]);
}
/*---------------------------------------------------------------------------*/
#if WITH_UIP
static void
init_uip_net(void)
{
  uip_ipaddr_t hostaddr;

  uip_init();
  uip_fw_init();

  process_start(&tcpip_process, NULL);
#if WITH_SLIP
  process_start(&slip_process, NULL);
  rs232_set_input(slip_input_byte);
#endif /* WITH_SLIP */
  process_start(&uip_fw_process, NULL);

  if (node_id > 0) {
    /* node id is set, construct an ip address based on the node id */
    uip_ipaddr(&hostaddr, 172, 16, 1, node_id & 0xff);
    uip_sethostaddr(&hostaddr);
  }

#if WITH_SLIP
  uip_fw_register(&slipif);
#endif /* WITH_SLIP */

  uip_fw_default(&tr1001if);
}
#endif /* WITH_UIP */
/*---------------------------------------------------------------------------*/
static void
print_processes(struct process * const processes[])
{
  printf("Starting");
  while(*processes != NULL) {
    printf(" '%s'", (*processes)->name);
    processes++;
  }
  /* Needed to force link with putchar */
  putchar('\n');
}
/*---------------------------------------------------------------------------*/
static void init_ports_toberemoved() {
    ////////// Port 1 ////
  P1SEL = 0x00;
  P1DIR = 0x81;       // Outputs: P10=IRSend, P17=RS232RTS
                      // Inputs: P11=Light, P12=IRRec, P13=PIR, P14=Vibration,
                      //         P15=Clockalarm, P16=RS232CTS
  P1OUT = 0x00;

  ////////// Port 2 ////
  P2SEL = 0x00;       // No Sels
  P2DIR = 0x7F;       // Outpus: P20..P23=Leds+Beeper, P24..P26=Poti
                      // Inputs: P27=Taster
  P2OUT = 0x77;

  ////////// Port 3 ////
  P3SEL = 0xE0;       // Sels for P34..P37 to activate UART,
  P3DIR = 0x5F;       // Inputs: P30..P33=CON4, P35/P37=RXD Transceiver/RS232
                      // OutPuts: P36/P38=TXD Transceiver/RS232
  P3OUT = 0xE0;       // Output a Zero on P34(TXD Transceiver) and turn SELECT off when receiving!!!

  ////////// Port 4 ////
  P4SEL = 0x00;       // CON5 Stecker
  P4DIR = 0xFF;
  P4OUT = 0x00;

  ////////// Port 5 ////
  P5SEL = 0x00;       // P50/P51= Clock SDA/SCL, P52/P53/P54=EEPROM SDA/SCL/WP
  P5DIR = 0xDA;       // P56/P57=Transceiver CNTRL0/1
  P5OUT = 0x0F;

  ////////// Port 6 ////
  P6SEL = 0x00;       // P60=Microphone, P61=PIR digital (same as P13), P62=PIR analog
  P6DIR = 0x00;       // P63=extern voltage, P64=battery voltage, P65=Receive power
  P6OUT = 0x00;
}
/*---------------------------------------------------------------------------*/
int
main(void)
{
  msp430_cpu_init();

  init_ports_toberemoved();
  
  init_lowlevel();

  clock_init();

  rtimer_init();
  
  process_init();

  random_init(0);

  node_id_restore();
  
  process_start(&etimer_process, NULL);
  process_start(&sensors_process, NULL);

  ctimer_init();

  set_rime_addr();

  printf(CONTIKI_VERSION_STRING " started. ");
  if(node_id > 0) {
    printf("Node id is set to %u.\n", node_id);
  } else {
    printf("Node id is not set.\n");
  }

  netstack_init();

  printf("%s %s, channel check rate %lu Hz\n",
         NETSTACK_MAC.name, NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
                         NETSTACK_RDC.channel_check_interval()));

  beep_spinup();
  leds_on(LEDS_RED);
  clock_delay(100);
  leds_off(LEDS_RED);

#if !WITH_SLIP
  rs232_set_input(serial_line_input_byte);
  serial_line_init();
#endif

#if WITH_UIP
  init_uip_net();
#endif /* WITH_UIP */

#if PROFILE_CONF_ON
  profile_init();
#endif /* PROFILE_CONF_ON */

#if ENERGEST_CONF_ON
  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);
#endif /* ENERGEST_CONF_ON */

  init_apps();
  print_processes(autostart_processes);
  autostart_start(autostart_processes);

#if DCOSYNCH_ENABLED
  timer_set(&dco_timer, DCOSYNCH_PERIOD * CLOCK_SECOND);
#endif /* DCOSYNCH_ENABLED */

  /*
   * This is the scheduler loop.
   */
  watchdog_start();
  while(1) {
    int r;
#if PROFILE_CONF_ON
    profile_episode_start();
#endif /* PROFILE_CONF_ON */
    do {
      /* Reset watchdog. */
      watchdog_periodic();
      r = process_run();
    } while(r > 0);
#if PROFILE_CONF_ON
    profile_episode_end();
#endif /* PROFILE_CONF_ON */

    /*
     * Idle processing.
     */
    dint();
    if(process_nevents() != 0) {
      eint();
    } else {
#if ENERGEST_CONF_ON
      static unsigned long irq_energest = 0;
#endif /* ENERGEST_CONF_ON */

#if DCOSYNCH_CONF_ENABLED
      /* before going down to sleep possibly do some management */
      if(timer_expired(&dco_timer)) {
        timer_reset(&dco_timer);
        msp430_sync_dco();
      }
#endif /* DCOSYNCH_CONF_ENABLED */

#if ENERGEST_CONF_ON
      /* Re-enable interrupts and go to sleep atomically. */
      ENERGEST_OFF(ENERGEST_TYPE_CPU);
      ENERGEST_ON(ENERGEST_TYPE_LPM);

      /* We only want to measure the processing done in IRQs when we
	 are asleep, so we discard the processing time done when we
	 were awake. */
      energest_type_set(ENERGEST_TYPE_IRQ, irq_energest);
#endif /* ENERGEST_CONF_ON */

      watchdog_stop();
      _BIS_SR(GIE | SCG0 | CPUOFF); /* LPM1 sleep. */

#if ENERGEST_CONF_ON
      /* We get the current processing time for interrupts that was
	 done during the LPM and store it for next time around.  */
      dint();
      irq_energest = energest_type_time(ENERGEST_TYPE_IRQ);
      eint();
      ENERGEST_OFF(ENERGEST_TYPE_LPM);
      ENERGEST_ON(ENERGEST_TYPE_CPU);
#endif /* ENERGEST_CONF_ON */

      watchdog_start();
    }
  }
}
/*---------------------------------------------------------------------------*/
/* char *arg_alloc(char size) {return NULL;} */
/* void  arg_init(void) {} */
/* void  arg_free(char *arg) {} */
/*---------------------------------------------------------------------------*/
#if UIP_LOGGING
void
uip_log(char *m)
{
  printf("uIP log: '%s'\n", m);
}
#endif /* UIP_LOGGING */

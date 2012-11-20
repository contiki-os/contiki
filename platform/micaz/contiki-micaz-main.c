/*
 * Copyright (c) 2009, University of Colombo School of Computing
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
 * @(#)$$
 */


/**
 * \file
 *         Main file of the MICAz port.
 *
 * \author
 *         Kasun Hewage <kasun.ch@gmail.com>
 */

#include <stdio.h>
#include <avr/pgmspace.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "net/rime.h"
#include "dev/leds.h"
#include "dev/rs232.h"
#include "dev/watchdog.h"
#include "dev/slip.h"

#include "init-net.h"
#include "dev/ds2401.h"
#include "sys/node-id.h"

/*---------------------------------------------------------------------------*/
void
init_usart(void)
{
  /* First rs232 port for debugging */
  rs232_init(RS232_PORT_0, USART_BAUD_115200,
             USART_PARITY_NONE | USART_STOP_BITS_1 | USART_DATA_BITS_8);

#if WITH_UIP || WITH_UIP6
 // slip_arch_init(USART_BAUD_115200);
    rs232_redirect_stdout(RS232_PORT_0);
#else
  rs232_redirect_stdout(RS232_PORT_0);
#endif /* WITH_UIP */

}
/*---------------------------------------------------------------------------*/
int
main(void)
{

  leds_init();

  leds_on(LEDS_RED);

  /* Initialize USART */
  init_usart();
  
  /* Clock */
  clock_init();

  leds_on(LEDS_GREEN);

  ds2401_init();
  
  node_id_restore();

  random_init(ds2401_id[0] + node_id);

  rtimer_init();

  /* Process subsystem */
  process_init();

  process_start(&etimer_process, NULL);

  ctimer_init();

  leds_on(LEDS_YELLOW);

  init_net();
  
  printf_P(PSTR(CONTIKI_VERSION_STRING " started. Node id %u\n"), node_id);

  leds_off(LEDS_ALL);

  /* Autostart processes */
  autostart_start(autostart_processes);

  /* Main scheduler loop */
  do {

    process_run();

  }while(1);

  return 0;
}

/*
 * Copyright (c) 2012, Alex Barclay.
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
 *
 * Author: Alex Barclay <alex@planet-barclay.com>
 *
 */

#include <p33Fxxxx.h>
#include <stdio.h>

#include "contiki.h"

#include "dev/serial-line.h"

#include "dev/rs232.h"
#include "dev/sysclock.h"
#include "dev/dspictimer.h"
#include "net/enc424mac.h"
#include "dev/spidrv.h"

uint32_t getErrLoc();  /* Get Address Error Loc */

#ifdef NOBOOTLOADER
_FOSCSEL(FNOSC_FRC);
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_HS & IOL1WAY_ON);
_FWDT(FWDTEN_OFF);
#endif

void __attribute__((interrupt, no_auto_psv))
_U1ErrInterrupt()
{
  puts("UART 0 Error");
  IFS4bits.U1EIF = 0;
}
#if 0
void __attribute__((interrupt, no_auto_psv))
_DefaultInterrupt()
{
  printf("BOOM: %08lx\n", getErrLoc());

  uint16_t *addr = 0;
  addr = (uint16_t *)&addr - 32;
  while(addr != (uint16_t *)&addr) {
    printf("%04x ", *addr++);
  }
  putchar('\n');
  puts("INTCON1 INTCON2 IFS1 IFS2 IFS3 IFS4");
  printf("%04x %04x %04x %04x %04x %04x\r\n", INTCON1, INTCON2, IFS1, IFS2, IFS3, IFS4);
  while(1) ;  /* Let the watchdog clean this mess up if enabled */
}
#endif

void __attribute__((__interrupt__, auto_psv))
_StackError()
{
  puts("Stack overflow!");
  while(1) ;
}
void
lowlevel_init()
{
  /* Allow nested interrupts */
  INTCON1bits.NSTDIS = 0;

#ifdef NOBOOTLOADER
  sysclock_init();
#endif
  dspic_timer_init();
  rs232_init(9600);
  spi_init();
}
/* Note - don't start tcpip_process here. We need to start the enc424mac first. It will then start */
/* the tcpip_process once it's read it's MAC address. */
PROCINIT(&etimer_process, &enc424mac_process);

int
main(void)
{
  lowlevel_init();
  printf("Starting Contiki\n");

  process_init();
  ctimer_init();
  procinit_init();
  serial_line_init();
  autostart_start(autostart_processes);
  while(1) {
    int n;
    n = process_run();
    etimer_request_poll();
  }

  return 0;
}
void
log_message(char *m1, char *m2)
{
}
void
uip_log(char *m)
{
  puts(m);
}

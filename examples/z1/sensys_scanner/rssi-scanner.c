/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: rssi-scanner.c,v 1.2 2009/04/25 22:38:00 joxe Exp $
 */

/**
 * \file
 *         Scanning 2.4 GHz radio frequencies using CC2420 and prints
 *         the values
 * \author
 *         Joakim Eriksson, joakime@sics.se
 */

#include "contiki.h"
#include "net/rime.h"

#include "dev/leds.h"
#include "dev/cc2420.h"
#include "dev/cc2420_const.h"
#include "dev/spi.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
/* This assumes that the CC2420 is always on and "stable" */
static void
set_frq(int c)
{
  int f;
  // We can read even other channels with CC2420!
  // Fc = 2048 + FSCTRL  ;  Fc = 2405 + 5(k-11) MHz, k=11,12, ... , 26
  f = c + 352; // Start from 2400 MHz to 2485 MHz,
  //FASTSPI_SETREG(CC2420_FSCTRL, f);
  //FASTSPI_STROBE(CC2420_SRXON);
  CC2420_WRITE_REG(CC2420_FSCTRL, f);
  CC2420_STROBE(CC2420_SRXON);
}

static void
do_rssi(void)
{
  int channel;
  printf("RSSI:");
  for(channel = 0; channel <= 85; ++channel) {
    set_frq(channel);
    printf("%d ", cc2420_rssi() + 55);
  }
  printf("\n");
}

/*---------------------------------------------------------------------------*/
PROCESS(scanner_process, "RSSI Scanner");
AUTOSTART_PROCESSES(&scanner_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(scanner_process, ev, data)
{
  PROCESS_BEGIN();
  /* switch mac layer off, and turn radio on */
  rime_mac->off(0);
  cc2420_on();

  while(1) {
    do_rssi();
    PROCESS_PAUSE();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

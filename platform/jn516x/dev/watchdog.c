/*
 * Copyright (c) 2014, SICS Swedish ICT.
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

/**
 * \file
 *         JN516X watchdog support.
 * \author
 *         Beshr Al Nahas <beshr@sics.se>
 *
 */

#include "dev/watchdog.h"
#include "AppHardwareApi.h"

/*---------------------------------------------------------------------------*/
static int counter = 0;

/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
  counter = 0;
  watchdog_stop();
  /* enable WDT interrupt */
  vAHI_WatchdogException(1);
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  /* We setup the watchdog to reset the device after two seconds,
     unless watchdog_periodic() is called. */
  counter--;
  if(counter == 0) {
    vAHI_WatchdogStart(9); /* about 8*2^(9-1)ms=2.048s timeout */
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
  /* This function is called periodically to restart the watchdog
     timer. */
  vAHI_WatchdogRestart();
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
  counter++;
  if(counter == 1) {
    vAHI_WatchdogStop();
  }
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
  vAHI_SwReset();
}
/*---------------------------------------------------------------------------*/

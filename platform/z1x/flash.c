/**
 * \file
 * Functions for reading and writing flash ROM.
 * \author Adam Dunkels <adam@sics.se>
 */

/* Copyright (c) 2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "contiki.h"
#include "dev/flash.h"
#include "dev/watchdog.h"

#define FLASH_TIMEOUT 30
#define FLASH_REQ_TIMEOUT 150

static uint16_t sfrie;

/*---------------------------------------------------------------------------*/
void
flash_setup(void)
{
  /* disable all interrupts to protect CPU
     during programming from system crash */
  dint();

  /* Clear interrupt flag1. */
  SFRIFG1 = 0;
  /* The IFG1 = 0; statement locks up contikimac - not sure if this
     statement needs to be here at all. I've removed it for now, since
     it seems to work, but leave this little note here in case someone
     stumbles over this code at some point. */

  /* Stop watchdog. */
  watchdog_stop();

  /* disable all NMI-Interrupt sources */
  sfrie = SFRIE1;
  SFRIE1 = 0x00;
}
/*---------------------------------------------------------------------------*/
void
flash_done(void)
{
  /* Enable interrupts. */
  SFRIE1 = sfrie;
  eint();
  watchdog_start();
}
/*---------------------------------------------------------------------------*/
void
flash_clear(unsigned short *ptr)
{
  uint8_t r;
  FCTL3 = 0xA500;               /* Lock = 0 */
  while(FCTL3 & 0x0001) r++;  /* Wait for BUSY = 0, not needed
                                     unless run from RAM */
  FCTL1 = 0xA502;               /* ERASE = 1 */
  *ptr  = 0;                    /* erase Flash segment */
  FCTL1 = 0xA500;               /* ERASE = 0 automatically done?! */
  FCTL3 = 0xA510;               /* Lock = 1 */
}
/*---------------------------------------------------------------------------*/
void
flash_write(unsigned short *ptr, unsigned short word)
{
  uint8_t r;
  FCTL3 = 0xA500;              /* Lock = 0 */
  while(FCTL3 & 0x0001) r++; /* Wait for BUSY = 0, not needed unless
                                  run from RAM */
  FCTL1 = 0xA540;              /* WRT = 1 */
  *ptr  = word;                /* program Flash word */
  FCTL1 = 0xA500;              /* WRT = 0 */
  FCTL3 = 0xA510;              /* Lock = 1 */
}
/*---------------------------------------------------------------------------*/

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

#include "contiki-conf.h"
#include "sys/node-id.h"

unsigned short node_id = 0;

/*---------------------------------------------------------------------------*/
void
node_id_restore(void)
{
  uint16_t newid[2];
  uint8_t volatile sreg;

  sreg = SREG; /* Save status register before disabling interrupts. */
  cli();    /* Disable interrupts. */
  eeprom_read(EEPROM_NODE_ID_START, (unsigned char *)newid, sizeof(newid));
  node_id = (newid[0] == 0xdead) ? newid[1] : 0;
  SREG = sreg;    /* Enable interrupts. */
}
/*---------------------------------------------------------------------------*/
void
node_id_burn(unsigned short id)
{
  uint16_t buffer[2] = { 0xdead, id };
  uint8_t volatile sreg;

  sreg = SREG; /* Save status register before disabling interrupts. */
  cli();    /* Disable interrupts. */
  eeprom_write(EEPROM_NODE_ID_START, (unsigned char *)buffer, sizeof(buffer));
  SREG = sreg;    /* Enable interrupts. */
}
/*---------------------------------------------------------------------------*/

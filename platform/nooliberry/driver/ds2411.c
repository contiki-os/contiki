/*
 * Copyright (c) 2013, NooliTIC
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
 */

/**
 * \file DS2411.c
 *  Provides the functionality for Serial ROM ds2411
 *
 * \author
 *  Ludovic WIART <ludovic.wiart@noolitic.biz>
 *
 *  History:
 *     19/03/2012 L. Wiart - Created
 *     08/02/2013 S. Dawans - Code Style & Integration in Contiki fork
 */

/*----------------------------------------------------------------------------*
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 *----------------------------------------------------------------------------*/

#include "contiki.h"
#include "1wr.h"
#include "ds2411.h"

/*----------------------------------------------------------------------------*
                   functions section
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
CRC calculation for ds2411   Polynomial ^8 + ^5 + ^4 + 1 
Parameters:
  actual crc and new byte
Returns:
  crc update
 *----------------------------------------------------------------------------*/
static unsigned
crc8_add(unsigned a_crc, unsigned byte)
{
  int i;

  a_crc ^= byte;
  for(i = 0; i < 8; i++) {
    if(a_crc & 1) {
      a_crc = (a_crc >> 1) ^ 0x8c;
    } else {
      a_crc >>= 1;
    }
  }
  return a_crc;
}

/*----------------------------------------------------------------------------*
ds2411_read_temp.  read the serail on ds2411
Parameters:
  8 bytes char array pointer
Returns:
  0 if error or 1 if ok
  and value in ds2411_id
 *----------------------------------------------------------------------------*/
int
ds2411_read(unsigned char *ds2411_id)
{
  int retry;
  int i;
  unsigned family, crc, read_crc;

  retry = 4;
  do {
    crc = 0;
    if(owr_reset()) {           /* Reset ds2411  */
      owr_writeb(0x33);         /* Read ds2411 command */
      family = owr_readb();
      crc = crc8_add(crc, family);
      if(family == 1) {
        for(i = 5; i >= 0; i--) {
          ds2411_id[i] = owr_readb();
          crc = crc8_add(crc, ds2411_id[i]);
        }
      } else {
        crc = 99;               /* retry */
      }
    } else {
      crc = 99;                 /* retry */
    }
    retry--;
    read_crc = owr_readb();
  }
  while((crc != read_crc) & (retry > 0));
  if(retry) {
    return 1;
  } else {
    return 0;
  }
}

/*---------------------------------------------------------------------------*/

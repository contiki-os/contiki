/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: cc2420-aes.c,v 1.3 2009/04/07 09:22:58 nifi Exp $
 */

/**
 * \file
 *         AES encryption/decryption functions.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "dev/cc2420-aes.h"
#include "dev/cc2420_const.h"
#include "dev/spi.h"

#if defined(__AVR__)
#include <avr/io.h>
#elif defined(__MSP430__)
#include <io.h>
#endif

#define KEYLEN 16
#define MAX_DATALEN 16

#define MIN(a,b) ((a) < (b)? (a): (b))

/*---------------------------------------------------------------------------*/
void
cc2420_aes_set_key(uint8_t *key, int index)
{
  uint16_t f;
  
  switch(index) {
  case 0:
    FASTSPI_WRITE_RAM_LE(key, CC2420RAM_KEY0, KEYLEN, f);
    break;
  case 1:
    FASTSPI_WRITE_RAM_LE(key, CC2420RAM_KEY1, KEYLEN, f);
    break;
  }
}
/*---------------------------------------------------------------------------*/
/* Encrypt at most 16 bytes of data. */
static void
cipher16(uint8_t *data, int len)
{
  uint16_t f;

  len = MIN(len, MAX_DATALEN);
  
  FASTSPI_WRITE_RAM_LE(data, CC2420RAM_SABUF, len, f);
  FASTSPI_STROBE(CC2420_SAES);
  FASTSPI_READ_RAM_LE(data, CC2420RAM_SABUF, len, f);
}
/*---------------------------------------------------------------------------*/
void
cc2420_aes_cipher(uint8_t *data, int len, int key_index)
{
  int i;
  uint16_t secctrl0;

  FASTSPI_GETREG(CC2420_SECCTRL0, secctrl0);

  secctrl0 &= ~(CC2420_SECCTRL0_SAKEYSEL0 | CC2420_SECCTRL0_SAKEYSEL1);

  switch(key_index) {
  case 0:
    secctrl0 |= CC2420_SECCTRL0_SAKEYSEL0;
    break;
  case 1:
    secctrl0 |= CC2420_SECCTRL0_SAKEYSEL1;
    break;
  }
  FASTSPI_SETREG(CC2420_SECCTRL0, secctrl0);

  for(i = 0; i < len; i = i + MAX_DATALEN) {
    cipher16(data + i, MIN(len - i, MAX_DATALEN));
  }
}
/*---------------------------------------------------------------------------*/

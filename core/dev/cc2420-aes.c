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
 * $Id: cc2420-aes.c,v 1.5 2010/06/24 11:25:55 nifi Exp $
 */

/**
 * \file
 *         AES encryption functions.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#endif
#include "dev/cc2420.h"
#include "dev/cc2420-aes.h"
#include "dev/spi.h"

#define KEYLEN 16
#define MAX_DATALEN 16

#define CC2420_WRITE_RAM_REV(buffer,adr,count)               \
  do {                                                       \
    uint8_t i;                                               \
    CC2420_SPI_ENABLE();                                     \
    SPI_WRITE_FAST(0x80 | (adr & 0x7f));                     \
    SPI_WRITE_FAST((adr >> 1) & 0xc0);                       \
    for(i = (count); i > 0; i--) {                           \
      SPI_WRITE_FAST(((uint8_t*)(buffer))[i - 1]);           \
    }                                                        \
    SPI_WAITFORTx_ENDED();                                   \
    CC2420_SPI_DISABLE();                                    \
  } while(0)

#define MIN(a,b) ((a) < (b)? (a): (b))

/*---------------------------------------------------------------------------*/
void
cc2420_aes_set_key(const uint8_t *key, int index)
{
  switch(index) {
  case 0:
    CC2420_WRITE_RAM_REV(key, CC2420RAM_KEY0, KEYLEN);
    break;
  case 1:
    CC2420_WRITE_RAM_REV(key, CC2420RAM_KEY1, KEYLEN);
    break;
  }
}
/*---------------------------------------------------------------------------*/
/* Encrypt at most 16 bytes of data. */
static void
cipher16(uint8_t *data, int len)
{
  uint8_t status;

  len = MIN(len, MAX_DATALEN);

  CC2420_WRITE_RAM(data, CC2420RAM_SABUF, len);
  CC2420_STROBE(CC2420_SAES);
  /* Wait for the encryption to finish */
  do {
    CC2420_GET_STATUS(status);
  } while(status & BV(CC2420_ENC_BUSY));
  CC2420_READ_RAM(data, CC2420RAM_SABUF, len);
}
/*---------------------------------------------------------------------------*/
void
cc2420_aes_cipher(uint8_t *data, int len, int key_index)
{
  int i;
  uint16_t secctrl0;

  CC2420_READ_REG(CC2420_SECCTRL0, secctrl0);

  secctrl0 &= ~(CC2420_SECCTRL0_SAKEYSEL0 | CC2420_SECCTRL0_SAKEYSEL1);

  switch(key_index) {
  case 0:
    secctrl0 |= CC2420_SECCTRL0_SAKEYSEL0;
    break;
  case 1:
    secctrl0 |= CC2420_SECCTRL0_SAKEYSEL1;
    break;
  }
  CC2420_WRITE_REG(CC2420_SECCTRL0, secctrl0);

  for(i = 0; i < len; i = i + MAX_DATALEN) {
    cipher16(data + i, MIN(len - i, MAX_DATALEN));
  }
}
/*---------------------------------------------------------------------------*/

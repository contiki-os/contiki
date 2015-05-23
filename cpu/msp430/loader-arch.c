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

#include "contiki.h"
#include "sys/clock.h"

#include "net/ip/uip.h"

#include "dev/leds.h"
#include "dev/eeprom.h"
#include "dev/flash.h"

#include "loader/loader-arch.h"

void *loader_arch_codeaddr, *loader_arch_dataaddr;


#define FLASHADDR ((char *)0x8000)
#define DATAADDR ((char *)0x900)
#define READSIZE 0x10

#define beep(n) do { } while (0)
#define beep_beep(n) do { } while (0)

/*----------------------------------------------------------------------------------*/
void
loader_arch_load(unsigned short startaddr)
{
  unsigned short tmp;
  unsigned short codelen, datalen, sumlen;
  int i, j;
  unsigned short ptr;
  unsigned short *flashptr;
  void (* init)(void *);
  unsigned char tmpdata[READSIZE];
  unsigned char sum;

  /* Read the magic word and version number from the first four bytes
     in EEPROM. */
  eeprom_read(startaddr, (char *)&tmp, 2);
  if(tmp != UIP_HTONS(LOADER_ARCH_MAGIC)) {
    beep_beep(60000);
    return;
  }

  eeprom_read(startaddr + 2, (char *)&tmp, 2);
  if(tmp != UIP_HTONS(LOADER_ARCH_VERSION)) {
    return;
  }

  startaddr += 4;

  /* Read the total lenghth that the checksum covers. */
  eeprom_read(startaddr, (char *)&sumlen, 2);
  
  sumlen = uip_htons(sumlen);
  
  sum = 0;

  for(i = 0; sumlen - i > READSIZE; i += READSIZE) {
    eeprom_read(startaddr + 2 + i, tmpdata, READSIZE);

    for(j = 0; j < READSIZE; ++j) {
      sum += tmpdata[j];
      if(sum < tmpdata[j]) {
        ++sum;
      }
    }
  }
  if(sumlen - i > 0) {
    eeprom_read(startaddr + 2 + i, tmpdata, READSIZE);

    for(j = 0; j < sumlen - i; ++j) {
      sum += tmpdata[j];
      if(sum < tmpdata[j]) {
        ++sum;
      }
    }
  }

  /* If the checksum was wrong, we beep. The number of beeps indicate
     the numerival value of the calculated checksum. */
  if(sum != 0xff) {
    leds_on(LEDS_RED);
    
    for(i = 0; i < (sum >> 4); ++i) {
      beep_beep(200);
      for(j = 0; j < 2; ++j) {
	clock_delay(60000);
      }
    }
    
    for(j = 0; j < 8; ++j) {
      clock_delay(60000);
    }
    
    for(i = 0; i < (sum & 0x0f); ++i) {
      beep_beep(200);
      for(j = 0; j < 2; ++j) {
	clock_delay(60000);
      }
    }

    leds_off(LEDS_RED);
    
    return;
    
  } else {
    leds_on(LEDS_GREEN);
    for(i = 0; i < 4; ++i) {
      beep_beep(200);
      for(j = 0; j < 2; ++j) {
	clock_delay(60000);
      }
    }
    leds_off(LEDS_GREEN);
  }


  leds_on(LEDS_YELLOW);
  startaddr += 2;
  
  /* Read the size of the code segment from the next two bytes in EEPROM. */
  eeprom_read(startaddr, (char *)&codelen, 2);
  /* Convert from network byte order to host byte order. */
  codelen = uip_htons(codelen);
  

  /* Flash program code into ROM. We use the available space in the
     program's data memory to temporarily store the code before
     flashing it into ROM. */
  flash_setup();

  flashptr = (unsigned short *)FLASHADDR;
  for(ptr = startaddr + 2; ptr < startaddr + 2 + codelen; ptr += READSIZE) {
    
    /* Read data from EEPROM into RAM. */
    eeprom_read(ptr, DATAADDR, READSIZE);

    /* Clear flash page on 512 byte boundary. */
    if((((unsigned short)flashptr) & 0x01ff) == 0) {
      flash_clear(flashptr);
    }
    
    /* Burn data from RAM into flash ROM. Flash is burned one 16-bit
       word at a time, so we need to be careful when incrementing
       pointers. The flashptr is already a short pointer, so
       incrementing it by one will actually increment the address by
       two. */
    for(i = 0; i < READSIZE / 2; ++i) {
      flash_write(flashptr, ((unsigned short *)DATAADDR)[i]);
      ++flashptr;
    }
  }

  flash_done();

  leds_off(LEDS_YELLOW);

  leds_on(LEDS_GREEN);
  
  /* Read the size of the code segment from the first two bytes in EEPROM. */
  eeprom_read(startaddr + 2 + codelen, (char *)&datalen, 2);
  
  /* Convert from network byte order to host byte order. */
  datalen = uip_htons(datalen);

  if(datalen > 0) {
    /* Read the contents of the data memory into RAM. */
    eeprom_read(startaddr + 2 + codelen + 2, DATAADDR, datalen);
  }

  for(i = 0; i < 4; ++i) {
    beep_beep(2000);
    clock_delay(20000);
  }

  leds_off(LEDS_GREEN);
  
  /* Execute the loaded program. */
  init = ((void (*)(void *))FLASHADDR);
  init((void *)0);
}
/*----------------------------------------------------------------------------------*/
void
loader_arch_free(void *code, void *data)
{
}
/*----------------------------------------------------------------------------------*/

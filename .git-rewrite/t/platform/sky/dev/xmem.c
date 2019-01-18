/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * @(#)$Id: xmem.c,v 1.5 2008/01/21 10:40:26 nvt-se Exp $
 */

/**
 * \file
 *         Device driver for the ST M25P80 40MHz 1Mbyte external memory.
 * \author
 *         Björn Grönvall <bg@sics.se>
 *
 *         Data is written bit inverted (~-operator) to flash so that
 *         unwritten data will read as zeros (UNIX style).
 */

#include <stdio.h>
#include <string.h>

#include <io.h>
#include <signal.h>

#include "contiki.h"

#include "dev/spi.h"
#include "dev/xmem.h"
#include "dev/watchdog.h"

#if 0
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define  SPI_FLASH_INS_WREN        0x06
#define  SPI_FLASH_INS_WRDI        0x04
#define  SPI_FLASH_INS_RDSR        0x05
#define  SPI_FLASH_INS_WRSR        0x01
#define  SPI_FLASH_INS_READ        0x03
#define  SPI_FLASH_INS_FAST_READ   0x0b
#define  SPI_FLASH_INS_PP          0x02
#define  SPI_FLASH_INS_SE          0xd8
#define  SPI_FLASH_INS_BE          0xc7
#define  SPI_FLASH_INS_DP          0xb9
#define  SPI_FLASH_INS_RES         0xab
/*---------------------------------------------------------------------------*/
static void
spi_tx(unsigned byte)
{
  FASTSPI_TX(byte);
}
/*---------------------------------------------------------------------------*/
static void
write_enable(void)
{
  int s;

  s = splhigh();
  SPI_FLASH_ENABLE();
  
  spi_tx(SPI_FLASH_INS_WREN);

  SPI_FLASH_DISABLE();
  splx(s);
}
/*---------------------------------------------------------------------------*/
static unsigned
read_status_register(void)
{
  unsigned char u;

  int s;

  s = splhigh();
  SPI_FLASH_ENABLE();
  
  spi_tx(SPI_FLASH_INS_RDSR);

  FASTSPI_CLEAR_RX();
  FASTSPI_RX(u);

  SPI_FLASH_DISABLE();
  splx(s);

  return u;
}
/*---------------------------------------------------------------------------*/
/*
 * Wait for a write/erase operation to finish.
 */
static unsigned
wait_ready(void)
{
  unsigned u;
  do {
    u = read_status_register();
  } while(u & 0x01);		/* WIP=1, write in progress */
  return u;
}
/*---------------------------------------------------------------------------*/
/*
 * Erase 64k bytes of data. It takes about 1s before WIP goes low!
 */
static void
erase_sector(off_t offset)
{
  int s;

  wait_ready();

  write_enable();

  s = splhigh();
  SPI_FLASH_ENABLE();
  
  spi_tx(SPI_FLASH_INS_SE);
  spi_tx(offset >> 16);	/* MSB */
  spi_tx(offset >> 8);
  spi_tx(offset >> 0);	/* LSB */

  SPI_FLASH_DISABLE();
  splx(s);
}
/*---------------------------------------------------------------------------*/
/*
 * Initialize external flash *and* SPI bus!
 */
void
xmem_init(void)
{
  spi_init();

  P4DIR |= BV(FLASH_CS) | BV(FLASH_HOLD) | BV(FLASH_PWR);
  P4OUT |= BV(FLASH_PWR);       /* P4.3 Output, turn on power! */

  SPI_FLASH_DISABLE();		/* Unselect flash. */
  SPI_FLASH_UNHOLD();
}
/*---------------------------------------------------------------------------*/
int
xmem_pread(void *_p, int size, off_t offset)
{
  unsigned char *p = _p;
  const unsigned char *end = p + size;
  int s;

  wait_ready();

  s = splhigh();
  SPI_FLASH_ENABLE();

  spi_tx(SPI_FLASH_INS_READ);
  spi_tx(offset >> 16);	/* MSB */
  spi_tx(offset >> 8);
  spi_tx(offset >> 0);	/* LSB */
  
  FASTSPI_CLEAR_RX();
  for(; p < end; p++) {
    unsigned char u;
    FASTSPI_RX(u);
    *p = ~u;
  }

  SPI_FLASH_DISABLE();
  splx(s);
  return size;
}
/*---------------------------------------------------------------------------*/
static const char *
program_page(off_t offset, const unsigned char *p, int nbytes)
{
  const unsigned char *end = p + nbytes;
  int s;

  wait_ready();

  write_enable();

  s = splhigh();
  SPI_FLASH_ENABLE();
  
  spi_tx(SPI_FLASH_INS_PP);
  spi_tx(offset >> 16);	/* MSB */
  spi_tx(offset >> 8);
  spi_tx(offset >> 0);	/* LSB */

  for(; p < end; p++) {
    spi_tx(~*p);
  }

  SPI_FLASH_DISABLE();
  splx(s);

  return p;
}
/*---------------------------------------------------------------------------*/
int
xmem_pwrite(const void *_buf, int size, off_t addr)
{
  const unsigned char *p = _buf;
  const off_t end = addr + size;
  off_t i, next_page;
  
  for(i = addr; i < end;) {
    next_page = (i | 0xff) + 1;
    if(next_page > end)
      next_page = end;
    p = program_page(i, p, next_page - i);
    i = next_page;
  }
  return size;
}
/*---------------------------------------------------------------------------*/
int
xmem_erase(long size, off_t addr)
{
  off_t end = addr + size;

  if(size % XMEM_ERASE_UNIT_SIZE != 0) {
    PRINTF("xmem_erase: bad size\n");
    return -1;
  }

  if(addr % XMEM_ERASE_UNIT_SIZE != 0) {
    PRINTF("xmem_erase: bad offset\n");
    return -1;
  }

  watchdog_stop();

  for (; addr < end; addr += XMEM_ERASE_UNIT_SIZE) {
    erase_sector(addr);
  }

  watchdog_start();

  return size;
}
/*---------------------------------------------------------------------------*/

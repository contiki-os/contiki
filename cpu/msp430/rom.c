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
 * @(#)$Id: rom.c,v 1.2 2006/12/01 15:06:42 bg- Exp $
 */

#include "contiki.h"

#include "dev/rom.h"

struct ictx {
  int s;
  unsigned short ie1, ie2, wdtctl;
};

static void
mask_intr(struct ictx *c)
{
  /* Disable all interrupts. */
  c->s = splhigh();

#define WDTRPW               0x6900  /* Watchdog key returned by read */

  /* Disable all NMI-Interrupt sources */
  c->wdtctl = WDTCTL;
  c->wdtctl ^= (WDTRPW ^ WDTPW);
  WDTCTL = WDTPW | WDTHOLD;    
  c->ie1 = IE1;
  IE1 = 0x00;                  
  c->ie2 = IE2;
  IE2 = 0x00;

  /* MSP430F1611 257 < f < 476 kHz, 2.4576MHz/(5+1) = 409.6 kHz. */
  FCTL2 = FWKEY | FSSEL_SMCLK | (FN2 | FN1);
  FCTL3 = FWKEY;		/* Unlock flash. */
}

static void
rest_intr(struct ictx *c)
{
  FCTL1 = FWKEY;		/* Disable erase or write. */
  FCTL3 = FWKEY | LOCK;		/* Lock flash. */
  /* Restore interrupts. */
  IE2 = c->ie2;
  IE1 = c->ie1;
  WDTCTL = c->wdtctl;
  splx(c->s);
}

/*
 * This helper routine must reside in RAM!
 */
static void
blkwrt(void *to, const void *from, const void *from_end)
     // __attribute__ ((section(".data")))
     ;

int
rom_erase(long nbytes, off_t offset)
{
  int nb = nbytes;
  char *to = (char *)(uintptr_t)offset;
  struct ictx c;
  
  if(nbytes % ROM_ERASE_UNIT_SIZE != 0) {
    return -1;
  }

  if(offset % ROM_ERASE_UNIT_SIZE != 0) {
    return -1;
  }

  while (nbytes > 0) {
    mask_intr(&c);

    FCTL1 = FWKEY | ERASE;	/* Segment erase. */
    *to  = 0;			/* Erase segment containing to. */
    nbytes -= ROM_ERASE_UNIT_SIZE;
    to += ROM_ERASE_UNIT_SIZE;

    rest_intr(&c);
  }

  return nb;
}

int
rom_pwrite(const void *buf, int nbytes, off_t offset)
{
  const char *from = buf;
  int nb = nbytes;
  char *to = (char *)(uintptr_t)offset;
  struct ictx c;
  
  mask_intr(&c);

  while(nbytes > 0) {
    int n = (nbytes > 64) ? 64 : nbytes;
    FCTL1 = FWKEY | BLKWRT | WRT;	/* Enable block write. */
    blkwrt(to, from, from + n);
    while(FCTL3 & BUSY);
    to += 64;
    from += 64;
    nbytes -= n;
  }

  rest_intr(&c);

  return nb;
}

/*
 * This helper routine must reside in RAM!
 */
asm(".data");
asm(".p2align 1,0");
asm(".type   blkwrt,@function");
asm(".section        .data");

static void
blkwrt(void *_to, const void *_from, const void *_from_end)
{
  unsigned short *to = _to;
  const unsigned short *from = _from;
  const unsigned short *from_end = _from_end;
  do {
    *to++ = *from++;
    while(!(FCTL3 & WAIT));
  } while(from < from_end);
  FCTL1 = FWKEY;		/* Disable block write. */
  /* Now ROM is available again! */
}

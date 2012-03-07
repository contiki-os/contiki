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
 * $Id: uip-ipchksum.c,v 1.4 2007/04/04 11:53:07 bg- Exp $
 */

/**
 * \file
 *         uIP checksum calculation for MSP430
 */

#include "net/uip.h"

#define asmv(arg) __asm__ __volatile__(arg)
/*---------------------------------------------------------------------------*/
#ifdef UIP_ARCH_IPCHKSUM
#ifdef __IAR_SYSTEMS_ICC__
uint16_t
uip_ipchksum(void)
{
  return 0;
}
#else
uint16_t
uip_ipchksum(void)
{
  /* Assumes proper alignement of uip_buf. */
  uint16_t *p = (uint16_t *)&uip_buf[UIP_LLH_LEN];
  register uint16_t sum;

  sum = p[0];
  asmv("add  %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[1]));
  asmv("addc %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[2]));
  asmv("addc %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[3]));
  asmv("addc %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[4]));
  asmv("addc %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[5]));
  asmv("addc %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[6]));
  asmv("addc %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[7]));
  asmv("addc %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[8]));
  asmv("addc %[p], %[sum]": [sum] "+r" (sum): [p] "m" (p[9]));

  /* Finally, add the remaining carry bit. */
  asmv("addc #0, %[sum]": [sum] "+r" (sum));

  /* Return sum in network byte order. */
  return (sum == 0) ? 0xffff : sum;
}
#endif
#endif
/*---------------------------------------------------------------------------*/

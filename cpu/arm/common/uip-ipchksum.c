/*
 *
 */

/**
 * \file
 *         uIP checksum calculation for ARM
 */

#include "net/uip.h"

#define asmv(arg) __asm__ __volatile__(arg)
/*---------------------------------------------------------------------------*/
#ifdef UIP_ARCH_IPCHKSUM

uint16_t
uip_ipchksum(void)
{
  /* Assumes proper alignement of uip_buf. */
  uint16_t *p = (uint16_t *)&uip_buf[UIP_LLH_LEN];
  register uint16_t sum;

  sum = p[0];

#if 0
  // MSP430 example :
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
#endif

}
#endif
/*---------------------------------------------------------------------------*/

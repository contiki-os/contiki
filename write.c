/*

Copyright (c) 2010 Red Hat Incorporated.
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

    Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of Red Hat Incorporated may not be used to endorse 
    or promote products derived from this software without specific 
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

/* These definitions are for the RL78/G13, which the simulator
   simulates.  */

typedef unsigned char UQI __attribute__((mode(QI)));
typedef unsigned int UHI __attribute__((mode(HI)));

#define s8(x)	(*(volatile UQI *)(x))
#define s16(x)	(*(volatile UHI *)(x))

#define P(x)	s8(0xfff00+(x))
#define PM(x)	s8(0xfff20+(x))

#define PER0	s8(0xf00f0)

#define SSR00	s16(0xf0100)
#define SMR00	s16(0xf0110)
#define SCR00	s16(0xf0118)
#define SS0	s16(0xf0122)
#define SPS0	s16(0xf0126)
#define SO0	s16(0xf0128)
#define SOE0	s16(0xf012a)
#define SOL0	s16(0xf0134)

#define SDR00	s16(0xfff10)

static int initted = 0;

static void
init_uart0 ()
{
  initted = 1;

  PER0 = 0xff;
  SPS0 = 0x0011; /* 16 MHz */
  SMR00 = 0x0022; /* uart mode */
  SCR00 = 0x8097; /* 8-N-1 */
  SDR00 = 0x8a00; /* baud in MSB - 115200 */
  SOL0 = 0x0000; /* not inverted */
  SO0 = 0x000f; /* initial value */
  SOE0 = 0x0001; /* enable TxD0 */
  P(1)  |= 0b00000100;
  PM(1) &= 0b11111011;
  SS0 = 0x0001;
}

static void
tputc (char c)
{
  /* Wait for transmit buffer to be empty.  */
  while (SSR00 & 0x0020)
    asm("");
  /* Transmit that byte.  */
  SDR00 = c;
}

int
_write(int fd, char *ptr, int len)
{
  int rv = len;

  if (!initted)
    init_uart0 ();

  while (len != 0)
    {
      if (*ptr == '\n')
	tputc ('\r');
      tputc (*ptr);
      ptr ++;
      len --;
    }
  return rv;
}

char * write (int) __attribute__((weak, alias ("_write")));

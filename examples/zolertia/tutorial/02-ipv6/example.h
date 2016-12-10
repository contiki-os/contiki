/*
 * Copyright (c) 2016, Zolertia - http://www.zolertia.com
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
 */
/**
 * \author Antonio Lignan <alinan@zolertia.com>
 */

#ifndef EXAMPLE_
#define EXAMPLE_
/*---------------------------------------------------------------------------*/
/* This is the UDP port used to send and receive data */
#define UDP_CLIENT_PORT   8765
#define UDP_SERVER_PORT   5678

/* Radio values to be configured for the 01-udp-local-multicast example */
#if CONTIKI_TARGET_ZOUL
#define EXAMPLE_TX_POWER  0xFF
#else /* default is Z1 */
#define EXAMPLE_TX_POWER  31
#endif
#define EXAMPLE_CHANNEL   15
#define EXAMPLE_PANID     0xBEEF

/*---------------------------------------------------------------------------*/
/* This data structure is used to store the packet content (payload) */
struct my_msg_t {
  uint8_t  id;
  uint16_t counter;
  uint16_t value1;
  uint16_t value2;
  uint16_t value3;
  uint16_t value4;
  uint16_t battery;
};
/*---------------------------------------------------------------------------*/
#endif /* __TEST_EXAMPLE__ */

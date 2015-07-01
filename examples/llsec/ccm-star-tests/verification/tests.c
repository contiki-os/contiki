/*
 * Copyright (c) 2013, Hasso-Plattner-Institut.
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

/**
 * \file
 *         Testing CCM*-MICs
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/llsec/llsec802154.h"
#include "net/llsec/ccm-star-packetbuf.h"
#include "net/mac/frame802154.h"
#include "lib/aes-128.h"
#include "lib/ccm-star.h"
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Test vector C.1 from FIPS Pub 197 */
static void
test_aes_128()
{
  uint8_t key[16] = { 0x00 , 0x01 , 0x02 , 0x03 ,
                      0x04 , 0x05 , 0x06 , 0x07 ,
                      0x08 , 0x09 , 0x0A , 0x0B ,
                      0x0C , 0x0D , 0x0E , 0x0F };
  uint8_t data[16] = { 0x00 , 0x11 , 0x22 , 0x33 ,
                       0x44 , 0x55 , 0x66 , 0x77 ,
                       0x88 , 0x99 , 0xAA , 0xBB ,
                       0xCC , 0xDD , 0xEE , 0xFF };
  uint8_t oracle[16] = { 0x69 , 0xC4 , 0xE0 , 0xD8 ,
                         0x6A , 0x7B , 0x04 , 0x30 ,
                         0xD8 , 0xCD , 0xB7 , 0x80 ,
                         0x70 , 0xB4 , 0xC5 , 0x5A };
  
  printf("Testing AES-128 ... ");
  
  AES_128.set_key(key);
  AES_128.encrypt(data);
  
  if(memcmp(data, oracle, 16) == 0) {
    printf("Success\n");
  } else {
    printf("Failure\n");
  }
}
/*---------------------------------------------------------------------------*/
/* Test vector C.2.1.2 from IEEE 802.15.4-2006 */
static void
test_sec_lvl_2()
{
  uint8_t key[16] = { 0xC0 , 0xC1 , 0xC2 , 0xC3 ,
                      0xC4 , 0xC5 , 0xC6 , 0xC7 ,
                      0xC8 , 0xC9 , 0xCA , 0xCB ,
                      0xCC , 0xCD , 0xCE , 0xCF };
  uint8_t extended_source_address[8] = { 0xAC , 0xDE , 0x48 , 0x00 ,
                                         0x00 , 0x00 , 0x00 , 0x01 };
  uint8_t data[26] = { 0x08 , 0xD0 , 0x84 , 0x21 , 0x43 ,
                       /* Source Address */
                       0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x48 , 0xDE , 0xAC ,
                       /* Security Level*/
                       0x02 ,
                       /* Frame Counter */
                       0x05 , 0x00 , 0x00 , 0x00 ,
                       /* Payload */
                       0x55 , 0xCF , 0x00 , 0x00 , 0x51 , 0x52 , 0x53 , 0x54 };
  uint8_t oracle[LLSEC802154_MIC_LENGTH] = { 0x22 , 0x3B , 0xC1 , 0xEC ,
                                             0x84 , 0x1A , 0xB5 , 0x53 };
  frame802154_frame_counter_t counter;
  uint8_t mic[LLSEC802154_MIC_LENGTH];
  
  printf("Testing verification ... ");
  
  packetbuf_clear();
  packetbuf_set_datalen(26);
  memcpy(packetbuf_hdrptr(), data, 26);
  counter.u32 = 5;
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1, counter.u16[0]);
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3, counter.u16[1]);
  packetbuf_set_attr(PACKETBUF_ATTR_SECURITY_LEVEL, LLSEC802154_SECURITY_LEVEL);
  packetbuf_hdrreduce(18);
  
  CCM_STAR.set_key(key);
  ccm_star_mic_packetbuf(extended_source_address,mic, LLSEC802154_MIC_LENGTH);
  
  if(memcmp(mic, oracle, LLSEC802154_MIC_LENGTH) == 0) {
    printf("Success\n");
  } else {
    printf("Failure\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(ccm_star_tests_process, "CCM* tests process");
AUTOSTART_PROCESSES(&ccm_star_tests_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ccm_star_tests_process, ev, data)
{
  PROCESS_BEGIN();
  
  test_aes_128();
  test_sec_lvl_2();
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

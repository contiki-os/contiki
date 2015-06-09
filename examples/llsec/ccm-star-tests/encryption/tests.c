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
 *         Testing CTR
 * \author
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "net/llsec/llsec802154.h"
#include "lib/ccm-star.h"
#include "net/llsec/ccm-star-packetbuf.h"
#include "net/mac/frame802154.h"
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Test vector C.2.1.2 from IEEE 802.15.4-2006 */
static void
test_sec_lvl_6()
{
  uint8_t key[16] = { 0xC0 , 0xC1 , 0xC2 , 0xC3 ,
                      0xC4 , 0xC5 , 0xC6 , 0xC7 ,
                      0xC8 , 0xC9 , 0xCA , 0xCB ,
                      0xCC , 0xCD , 0xCE , 0xCF };
  uint8_t extended_source_address[8] = { 0xAC , 0xDE , 0x48 , 0x00 ,
                                         0x00 , 0x00 , 0x00 , 0x01 };
  uint8_t data[30] = { 0x2B , 0xDC , 0x84 , 0x21 , 0x43 ,
                       /* Destination Address */
                       0x02 , 0x00 , 0x00 , 0x00 , 0x00 , 0x48 , 0xDE , 0xAC ,
                       /* PAN-ID */
                       0xFF , 0xFF ,
                       /* Source Address */
                       0x01 , 0x00 , 0x00 , 0x00 , 0x00 , 0x48 , 0xDE , 0xAC ,
                       /* Security Level */
                       0x06 ,
                       /* Frame Counter */
                       0x05 , 0x00 , 0x00 , 0x00 ,
                       0x01 , 0xCE };
  uint8_t oracle[LLSEC802154_MIC_LENGTH] = { 0x4F , 0xDE , 0x52 , 0x90 ,
                                             0x61 , 0xF9 , 0xC6 , 0xF1 };
  frame802154_frame_counter_t counter;
  uint8_t mic[LLSEC802154_MIC_LENGTH];
  
  printf("Testing verification ... ");
  
  packetbuf_clear();
  packetbuf_set_datalen(30);
  memcpy(packetbuf_hdrptr(), data, 30);
  counter.u32 = 5;
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1, counter.u16[0]);
  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3, counter.u16[1]);
  packetbuf_set_attr(PACKETBUF_ATTR_SECURITY_LEVEL, LLSEC802154_SECURITY_LEVEL);
  packetbuf_hdrreduce(29);
  
  CCM_STAR.set_key(key);
  ccm_star_mic_packetbuf(extended_source_address, mic, LLSEC802154_MIC_LENGTH);
  
  if(memcmp(mic, oracle, LLSEC802154_MIC_LENGTH) == 0) {
    printf("Success\n");
  } else {
    printf("Failure\n");
  }
  
  printf("Testing encryption ... ");
  
  ccm_star_ctr_packetbuf(extended_source_address);
  if(((uint8_t *) packetbuf_hdrptr())[29] == 0xD8) {
    printf("Success\n");
  } else {
    printf("Failure\n");
  }
  
  printf("Testing decryption ... ");
  ccm_star_ctr_packetbuf(extended_source_address);
  if(((uint8_t *) packetbuf_hdrptr())[29] == 0xCE) {
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
  
  test_sec_lvl_6();
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

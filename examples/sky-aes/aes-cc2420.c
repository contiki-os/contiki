/*
 * Copyright (c) 2013, RWTH Aachen University.
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
 */

/**
 * \file
 *         Example on how to use Hardware AES.
 * \author
 *         Jens Hiller <jens.hiller@rwth-aachen.de>
 */

#include "contiki.h"
#include <stdio.h>
#include <string.h>
#include "../../core/dev/cc2420.h"

PROCESS(aes_cc2420, "aes_cc2420");
AUTOSTART_PROCESSES(&aes_cc2420);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aes_cc2420, ev, data)
{

  PROCESS_BEGIN();

  static unsigned char key[16] = {
      0xd2, 0xc0, 0x8c, 0x5a, 0x10, 0x88, 0xbe, 0xf7,
      0x7d, 0x4a, 0xb8, 0x26, 0xfb, 0x82, 0xfb, 0xe1
  };
  static unsigned char compare[16];
  static unsigned char data[16] = {
      0x95, 0x5c, 0xee, 0x3f, 0x33, 0x3a, 0x81, 0xcf,
      0x76, 0x2e, 0x44, 0x9f, 0x39, 0x83, 0x2a, 0x0f
  };
  unsigned char current[16];

  memcpy(current, data, 16);
  cc2420_aes_set_key(key, 0);
  cc2420_aes_cipher(current, 16, 0);

  memcpy(compare, current, 16);

  int count = 0;
  int i;
  for(i=0; i<10000; i++){
    memcpy(current, data, 16);
    cc2420_aes_cipher(current, 16, 0);
    if( memcmp(current, compare, 16) ){
      printf("%i FAIL\n", i);
      count++;
    }else{
      printf("%i ok\n", i);
    }
  }

  printf("done\n");
  printf("%i from 10000 failed\n", count);


  PROCESS_END();
}


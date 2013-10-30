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

  /* official NIST test vector
   *
   * http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
   * C.1 AES-128 (Nk=4, Nr=10) [page 35]
   */
  static unsigned char plaintext[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
  };
  static unsigned char key[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
  };
  static unsigned char ciphertext[16] = {
    0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
    0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a
  };
  unsigned char tmp[16];

  cc2420_aes_set_key(key, 0);

  int count = 0;
  int i;
  for(i=0; i<10000; i++){
    memcpy(tmp, plaintext, 16);
    cc2420_aes_cipher(tmp, 16, 0);
    if( memcmp(tmp, ciphertext, 16) ){
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


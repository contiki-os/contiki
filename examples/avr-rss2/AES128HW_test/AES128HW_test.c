/*
 * Copyright (c) 2017, Robert Olsson 
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
 * Author  : Robert Olsson 
 * roolss@kth.se & robert@radio-sensors.com
 * Created : 2017-04-22
 */

/**
 * \file
 *         A simple AES128 crypto emmgine test for Atmel radios
 */

#include "contiki.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rf230bb.h"

PROCESS(aes_crypto_process, "AES HW crypto process");
AUTOSTART_PROCESSES(&aes_crypto_process);

unsigned char aes_key[16]   = "abcdefghijklmnop";
unsigned char aes_p[128];
unsigned char aes_c[128];
unsigned char aes_s[128];
unsigned char tmp[16];
uint8_t i;
int res;

PROCESS_THREAD(aes_crypto_process, ev, data)
{
  PROCESS_BEGIN();

  /* AES engine on */
  NETSTACK_RADIO.on();

  strcpy((char *)aes_s, "Teststring______");

  for(i = 0; i < 16; i++) {
    printf("%02X", aes_s[i]);
  }
  printf(" Uncrypted \n");

  res = rf230_aes_encrypt_ebc(aes_key, aes_s, aes_c);
  if(!res) {
    printf("ERR encryption\n");
    exit(0);
  }
  for(i = 0; i < 16; i++) {
    printf("%02X", aes_c[i]);
  }
  printf(" AES-128 EBC Crypted\n");

  res = rf230_aes_decrypt_ebc(aes_key, aes_c, aes_p);
  if(!res) {
    printf("ERR decryption\n");
    exit(0);
  }
  for(i = 0; i < 16; i++) {
    printf("%02X", aes_p[i]);
  }
  printf(" Decrypted \n");

  res = rf230_aes_encrypt_cbc(aes_key, aes_s, sizeof(aes_s), aes_c);
  if(!res) {
    printf("ERR encryption\n");
    exit(0);
  }
  for(i = 0; i < 16; i++) {
    printf("%02X", aes_c[i]);
  }
  printf(" AES-128 MIC\n");

  PROCESS_END();
}


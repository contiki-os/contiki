/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 *         Block-wise hexadecimal conversion and CRC commands
 * \author
 *         Fredrik Osterlind <fros@sics.se>
 */

#include "contiki.h"
#include "shell.h"
#include "lib/crc16.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
PROCESS(shell_bin2hex_process, "bin2hex");
SHELL_COMMAND(bin2hex_command,
	      "bin2hex",
	      "bin2hex: binary to hexadecimal",
	      &shell_bin2hex_process);
PROCESS(shell_hex2bin_process, "hex2bin");
SHELL_COMMAND(hex2bin_command,
	      "hex2bin",
	      "hex2bin: hexadecimal to binary",
	      &shell_hex2bin_process);
PROCESS(shell_crc_process, "crc");
SHELL_COMMAND(crc_command,
	      "crc",
	      "crc: append per-block crc",
	      &shell_crc_process);
PROCESS(shell_crcvalidate_process, "crc-v");
SHELL_COMMAND(crcvalidate_command,
	      "crc-v",
	      "crc-v: verify crc and output if valid",
	      &shell_crcvalidate_process);
/*---------------------------------------------------------------------------*/
static unsigned char
fromhexchar(unsigned char c)
{
  unsigned char h;
  if(c >= '0' && c <= '9') {
    h = c-'0';
  } else if(c >= 'a' && c <= 'f') {
    h = c-'a'+10;
  } else if(c >= 'A' && c <= 'F') {
    h = c-'A'+10;
  } else {
    PRINTF("Bad hex input: %c", c);
    h = 0;
  }
  return h;
}
/*---------------------------------------------------------------------------*/
static unsigned char
fromhex(unsigned char c1, unsigned char c2)
{
  return (fromhexchar(c1)<<4) + fromhexchar(c2);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_bin2hex_process, ev, data)
{
  struct shell_input *input;
  int i;
  char *bufptr;
  char *buf;

  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }

    buf = alloca((input->len1 + input->len2)*2);

    bufptr = buf;
    for(i = 0; i < input->len1; i++) {
      bufptr += sprintf(bufptr, "%02x", 0xff&((char*)input->data1)[i]);
    }
    for(i = 0; i < input->len2; i++) {
      bufptr += sprintf(bufptr, "%02x", 0xff&((char*)input->data2)[i]);
    }

    shell_output(
        &bin2hex_command,
        buf, ((input->len1 + input->len2)*2), "", 0);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_hex2bin_process, ev, data)
{
  struct shell_input *input;
  int i, cnt;
  char* buf;

  PROCESS_BEGIN();

  /* Reads data in hexadecimal format and prints in binary */

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }

    if(input->len1 % 2 != 0) {
      PRINTF("Bad input length 1: %d\n", input->len1);
      continue;
    }
    if(input->len2 % 2 != 0) {
      PRINTF("Bad input length 2: %d\n", input->len2);
      continue;
    }

    buf = alloca((input->len1 + input->len2)/2+1);

    cnt = 0;
    for(i = 0; i < input->len1; i += 2) {
      buf[cnt++] = fromhex(
          ((char*)input->data1)[i],
          ((char*)input->data1)[i+1]);
    }
    for(i = 0; i < input->len2; i += 2) {
      buf[cnt++] = fromhex(
          ((char*)input->data2)[i],
          ((char*)input->data2)[i+1]);
    }

    shell_output(&hex2bin_command, buf, cnt, "", 0);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_crc_process, ev, data)
{
  struct shell_input *input;
  int i;
  uint16_t crc;
  char *buf;

  PROCESS_BEGIN();

  /* Append per-block 16-bit CRC */

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }

    /* calculate crc */
    crc = 0;
    for(i = 0; i < input->len1; i++) {
      crc = crc16_add(((char*)(input->data1))[i], crc);
    }
    for(i = 0; i < input->len2; i++) {
      crc = crc16_add(((char*)(input->data2))[i], crc);
    }

    /* input + 16-bit CRC */
    buf = alloca(input->len2+2);

    memcpy(buf, input->data2, input->len2);
    buf[input->len2] = crc&0xff;
    buf[input->len2+1] = (crc>>8)&0xff;

    shell_output(&crc_command, input->data1, input->len1, buf, input->len2+2);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_crcvalidate_process, ev, data)
{
  struct shell_input *input;
  int i;
  char crc1, crc2;
  uint16_t crc, crc_footer;

  PROCESS_BEGIN();

  /* Per-block 16-bit CRC verification:
   * outputs data without CRCs matches, otherwise nothing */

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }

    if(input->len1 + input->len2 < 2) {
      /* too short - no output */
      PRINTF("Too short input: %d+%d\n", input->len1, input->len2);
      continue;
    }

    if(input->len2 == 1) {
      crc1 = ((char*)input->data1)[input->len1-1];
      crc2 = ((char*)input->data2)[input->len2-1];
      input->len1 -= 1;
      input->len2 -= 1;
    } else if(input->len2 >= 2) {
      crc1 = ((char*)input->data2)[input->len2-2];
      crc2 = ((char*)input->data2)[input->len2-1];
      input->len2 -= 2;
    } else {
      crc1 = ((char*)input->data1)[input->len1-2];
      crc2 = ((char*)input->data1)[input->len1-1];
      input->len1 -= 2;
    }

    /* recalculate crc */
    crc = 0;
    for(i = 0; i < input->len1; i++) {
      crc = crc16_add(((char*)(input->data1))[i], crc);
    }
    for(i = 0; i < input->len2; i++) {
      crc = crc16_add(((char*)(input->data2))[i], crc);
    }

    /* compare with input crc */
    crc_footer = ((0xff&crc2)<<8) | (0xff&crc1);

    /* output if matching crcs */
    if(crc_footer == crc) {
      shell_output(
          &crcvalidate_command,
          input->data1, input->len1, input->data2, input->len2);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_crc_init(void)
{
  shell_register_command(&bin2hex_command);
  shell_register_command(&hex2bin_command);
  shell_register_command(&crc_command);
  shell_register_command(&crcvalidate_command);
}
/*---------------------------------------------------------------------------*/

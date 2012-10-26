/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *         A brief description of what this file is.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "shell-netfile.h"
#include "net/rime.h"
#include "net/rime/rudolph0.h"
#include "dev/leds.h"

#include "cfs/cfs.h"

#include <stdio.h>
#include <string.h>

#define FILENAME_LEN 20

static struct rudolph0_conn rudolph0_conn;
static char filename[FILENAME_LEN];
static int receiving_file;
static struct pt recvnetfilept;
/*---------------------------------------------------------------------------*/
PROCESS(shell_netfile_process, "netfile");
SHELL_COMMAND(netfile_command,
	      "netfile",
	      "netfile: send file to entire network",
	      &shell_netfile_process);
PROCESS(shell_recvnetfile_process, "recvnetfile");
SHELL_COMMAND(recvnetfile_command,
	      "recvnetfile",
	      "recvnetfile: receive file from network and print to output",
	      &shell_recvnetfile_process);
/*---------------------------------------------------------------------------*/
static int
write_chunk_pt(struct rudolph0_conn *c, int offset, int flag,
	    uint8_t *data, int datalen)
{
  PT_BEGIN(&recvnetfilept);

  PT_WAIT_UNTIL(&recvnetfilept, receiving_file);
  leds_on(LEDS_YELLOW);
  leds_on(LEDS_RED);
  PT_WAIT_UNTIL(&recvnetfilept, flag == RUDOLPH0_FLAG_NEWFILE);
  leds_off(LEDS_RED);

  do {
    if(datalen > 0) {
      shell_output(&recvnetfile_command, data, datalen, "", 0);
      /*      printf("write_chunk wrote %d bytes at %d\n", datalen, offset);*/
    }
    PT_YIELD(&recvnetfilept);
  } while(flag != RUDOLPH0_FLAG_LASTCHUNK);

  shell_output(&recvnetfile_command, data, datalen, "", 0);
  /*  printf("write_chunk wrote %d bytes at %d\n", datalen, offset);*/
  shell_output(&recvnetfile_command, "", 0, "", 0);
  leds_off(LEDS_YELLOW);
  receiving_file = 0;
  process_post(&shell_recvnetfile_process, PROCESS_EVENT_CONTINUE, NULL);

  PT_END(&recvnetfilept);
}
static void
write_chunk(struct rudolph0_conn *c, int offset, int flag,
	    uint8_t *data, int datalen)
{
  write_chunk_pt(c, offset, flag, data, datalen);
}

static int
read_chunk(struct rudolph0_conn *c, int offset, uint8_t *to, int maxsize)
{
  int ret;
  int fd;

  fd = cfs_open(filename, CFS_READ);

  cfs_seek(fd, offset, CFS_SEEK_SET);
  ret = cfs_read(fd, to, maxsize);
  /*  printf("read_chunk %d bytes at %d, %d\n", ret, offset, (unsigned char)to[0]);*/
  cfs_close(fd);
  return ret;
}
CC_CONST_FUNCTION static struct rudolph0_callbacks rudolph0_callbacks =
  {/*(void (*)(struct rudolph0_conn *, int, int, uint8_t *, int))*/
    write_chunk,
    read_chunk};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_netfile_process, ev, data)
{
  int fd;

  PROCESS_BEGIN();

  if(data != NULL) {
    rudolph0_send(&rudolph0_conn, CLOCK_SECOND);
    
    strncpy(filename, data, FILENAME_LEN);
    fd = cfs_open(filename, CFS_READ);
    if(fd < 0) {
      shell_output_str(&netfile_command, "netfile: could not open file ", filename);
    } else {
      cfs_close(fd);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_recvnetfile_process, ev, data)
{
  PROCESS_BEGIN();

  PT_INIT(&recvnetfilept);
  receiving_file = 1;
  while(1) {
    struct shell_input *input;

    PROCESS_WAIT_EVENT();

    if(ev == shell_event_input) {
      input = data;
      if(input->len1 + input->len2 == 0) {
	receiving_file = 0;
	PROCESS_EXIT();
      }
    } else if(receiving_file == 0) {
      PROCESS_EXIT();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_netfile_init(void)
{
  receiving_file = 0;
  shell_register_command(&netfile_command);
  shell_register_command(&recvnetfile_command);

  rudolph0_open(&rudolph0_conn, SHELL_RIME_CHANNEL_NETFILE,
		&rudolph0_callbacks);
}
/*---------------------------------------------------------------------------*/

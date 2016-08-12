/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *
 * Shell command for downloading files from a remote node.
 * Example usage:
 * 'download \<node addr\> \<filename\> | write \<local_filename\>'.
 *
 * \author Luca Mottola <luca@sics.se>, Fredrik Osterlind <fros@sics.se>
 */

#include "contiki.h"
#include "shell.h"

#include "net/rime/rime.h"
#include "cfs/cfs.h"

#include "dev/leds.h"

#include <stdio.h>
#include <string.h>

#define RUNICAST_CHANNEL SHELL_RIME_CHANNEL_DOWNLOAD
#define RUCB_CHANNEL (SHELL_RIME_CHANNEL_DOWNLOAD+1)
#define MAX_RETRANSMISSIONS 8

#define DEBUG 0

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
PROCESS(shell_download_process, "download");
PROCESS(shell_download_server_process, "download server");
SHELL_COMMAND(download_command,
    "download",
    "download <node addr> <filename>: download file from remote node",
    &shell_download_process);
/*---------------------------------------------------------------------------*/
static struct runicast_conn runicast;
static struct rucb_conn rucb;
static uint8_t downloading;
static uint8_t req_seq_counter;
static uint8_t req_last_seq;
static int fd;
/*---------------------------------------------------------------------------*/
static void
write_chunk(struct rucb_conn *c, int offset, int flag, char *data, int datalen)
{
  if(datalen > 0) {
    shell_output(&download_command,
        data, datalen, NULL, 0);
    PRINTF("write_chunk %d at %d bytes\n", datalen, offset);
  }

  if(flag == RUCB_FLAG_NEWFILE) {
    PRINTF("RUCB_FLAG_NEWFILE\n");
  } else if(flag == RUCB_FLAG_NONE) {
    PRINTF("RUCB_FLAG_NONE\n");
  }
  if(flag == RUCB_FLAG_LASTCHUNK) {
    PRINTF("RUCB_FLAG_LASTCHUNK\n");
    downloading = 0;
    process_poll(&shell_download_process);
  }
}
/*---------------------------------------------------------------------------*/
static int
read_chunk(struct rucb_conn *c, int offset, char *to, int maxsize)
{
  int ret;
  if(fd < 0) {
    /* No file, send EOF */
    leds_off(LEDS_GREEN);
    return 0;
  }

  cfs_seek(fd, offset, CFS_SEEK_SET);
  ret = cfs_read(fd, to, maxsize);
  PRINTF("read_chunk %d bytes at %d\n", ret, offset);

  if(ret < maxsize) {
    PRINTF("read_chunk DONE\n");
    cfs_close(fd);
    fd = -1;
  }

  return ret;
}
/*---------------------------------------------------------------------------*/
static void
timedout(struct rucb_conn *c)
{
}
/*---------------------------------------------------------------------------*/
static const struct rucb_callbacks rucb_call = { write_chunk, read_chunk, timedout };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_download_process, ev, data)
{
  const char *nextptr;
  static linkaddr_t addr;
  int len;
  char buf[32];

  PROCESS_BEGIN();

  /* Parse node addr */
  addr.u8[0] = shell_strtolong(data, &nextptr);
  if(nextptr == data || *nextptr != '.') {
    shell_output_str(&download_command,
        "download <node addr> <filename>: need node address", "");
    PROCESS_EXIT();
  }
  ++nextptr;
  addr.u8[1] = shell_strtolong(nextptr, &nextptr);

  /* Get the length of the file, excluding a terminating NUL character. */
  while(nextptr[0] == ' ') {
    nextptr++;
  }
  len = strlen(nextptr);

  /*snprintf(buf, sizeof(buf), "%d.%d", addr.u8[0], addr.u8[1]);*/
  /*shell_output_str(&download_command, "Downloading from: ", buf);*/

  if(len > PACKETBUF_SIZE - 32) {
    snprintf(buf, sizeof(buf), "%d", len);
    shell_output_str(&download_command, "filename too large: ", buf);
    PROCESS_EXIT();
  }

  /*shell_output_str(&download_command, "Downloading file: ", nextptr);*/

  /* Send file request */
  downloading = 1;
  rucb_open(&rucb, RUCB_CHANNEL, &rucb_call);
  packetbuf_clear();
  *((uint8_t *)packetbuf_dataptr()) = ++req_seq_counter;
  memcpy(((char *)packetbuf_dataptr()) + 1, nextptr, len + 1);
  packetbuf_set_datalen(len + 2);
  PRINTF("requesting '%s'\n", nextptr);
  runicast_send(&runicast, &addr, MAX_RETRANSMISSIONS);

  /* Wait for download to finish */
  leds_on(LEDS_BLUE);
  PROCESS_WAIT_UNTIL(!runicast_is_transmitting(&runicast) && !downloading);
  leds_off(LEDS_BLUE);

  rucb_close(&rucb);
  /*shell_output_str(&download_command, "Done!", "");*/

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
request_recv(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno)
{
  const char *filename;
  uint8_t seq;

  if(packetbuf_datalen() < 2) {
    /* Bad filename, ignore request */
    printf("download: bad filename request (null)\n");
    return;
  }

  seq = ((uint8_t *)packetbuf_dataptr())[0];
  if(seq == req_last_seq) {
    PRINTF("download: ignoring duplicate request\n");
    return;
  }
  req_last_seq = seq;
  filename = ((char *)packetbuf_dataptr()) + 1;

  PRINTF("file requested: '%s'\n", filename);

  /* Initiate file transfer */
  leds_on(LEDS_GREEN);
  if(fd >= 0) {
    cfs_close(fd);
  }
  fd = cfs_open(filename, CFS_READ);
  if(fd < 0) {
    printf("download: bad filename request (no read access): %s\n", filename);
  } else {
    PRINTF("download: sending file: %s\n", filename);
  }

  rucb_close(&rucb);
  rucb_open(&rucb, RUCB_CHANNEL, &rucb_call);
  rucb_send(&rucb, from);
}
/*---------------------------------------------------------------------------*/
static void
request_sent(struct runicast_conn *c, const linkaddr_t *to,
	     uint8_t retransmissions)
{
  process_poll(&shell_download_process);
}
/*---------------------------------------------------------------------------*/
static void
request_timedout(struct runicast_conn *c, const linkaddr_t *to,
		 uint8_t retransmissions)
{
  shell_output_str(&download_command, "download: request timed out", "");
  downloading = 0;
  process_poll(&shell_download_process);
}
/*---------------------------------------------------------------------------*/
static const struct runicast_callbacks runicast_callbacks =
{request_recv, request_sent, request_timedout};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_download_server_process, ev, data)
{
  PROCESS_EXITHANDLER( runicast_close(&runicast); rucb_close(&rucb); );

  PROCESS_BEGIN();

  runicast_open(&runicast, RUNICAST_CHANNEL, &runicast_callbacks);
  PROCESS_WAIT_UNTIL(0);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_download_init(void)
{
  req_seq_counter = 0;
  req_last_seq = -1;
  fd = -1;
  downloading = 0;
  shell_register_command(&download_command);
  process_start(&shell_download_server_process, NULL);
}
/*---------------------------------------------------------------------------*/

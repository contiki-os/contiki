/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: tcprudolph0.c,v 1.13 2010/10/19 18:29:05 adamdunkels Exp $
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "sys/etimer.h"
#include "loader/elfloader.h"

#include "net/uip.h"

#include "dev/leds.h"

#include "cfs/cfs.h"

#include "codeprop.h"

#include "net/rime/rudolph0.h"

#include <io.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(tcp_loader_process, "TCP loader");
AUTOSTART_PROCESSES(&tcp_loader_process);

static
struct codeprop_state {
  uint16_t addr;
  uint16_t len;
  struct pt tcpthread_pt;
  int fd;
} s;

static char msg[30 + 10];

static struct rudolph0_conn rudolph0;

/*---------------------------------------------------------------------*/
static int
start_program(void)
{
  /* Link, load, and start new program. */
  int ret;
  s.fd = cfs_open("codeprop.out", CFS_READ);
  ret = elfloader_load(s.fd);

  /* XXX: Interrupts seems to be turned off a little too long during the
     ELF loading process, so we need to "manually" trigger a timer
     interrupt here. */
  TACCR1 = TAR + 1000;
  
  if(ret == ELFLOADER_OK) {
    sprintf(msg, "ok\n");
    PRINTF("Ok, starting new program.\n");
    /* Start processes. */
    autostart_start(elfloader_autostart_processes);
  } else {
    sprintf(msg, "err %d %s", ret, elfloader_unknown);
    PRINTF("Error: '%s'.\n", msg);
  }
  cfs_close(s.fd);
  return ret;
}
/*---------------------------------------------------------------------*/
static
PT_THREAD(recv_tcpthread(struct pt *pt))
{
  PT_BEGIN(pt);

  /* Read the header. */
  PT_WAIT_UNTIL(pt, uip_newdata() && uip_datalen() > 0);

  if(uip_datalen() < sizeof(struct codeprop_tcphdr)) {
    PRINTF(("codeprop: header not found in first tcp segment\n"));
    uip_abort();
    goto thread_done;
  }

  /* Kill old program. */
  rudolph0_stop(&rudolph0);
  /*  elfloader_unload();*/
  
  s.len = uip_htons(((struct codeprop_tcphdr *)uip_appdata)->len);
  s.addr = 0;
  uip_appdata += sizeof(struct codeprop_tcphdr);
  uip_len -= sizeof(struct codeprop_tcphdr);

  s.fd = cfs_open("codeprop.out", CFS_WRITE);
  cfs_close(s.fd);
  /*  xmem_erase(XMEM_ERASE_UNIT_SIZE, EEPROMFS_ADDR_CODEPROP);*/

  /* Read the rest of the data. */
  do {
    leds_toggle(LEDS_RED);
    if(uip_len > 0) {
      s.fd = cfs_open("codeprop.out", CFS_WRITE + CFS_APPEND);
      cfs_seek(s.fd, s.addr, CFS_SEEK_SET);
      /*      xmem_pwrite(uip_appdata, uip_len, EEPROMFS_ADDR_CODEPROP + s.addr);*/
      cfs_write(s.fd, uip_appdata, uip_len);
      cfs_close(s.fd);
      
      PRINTF("Wrote %d bytes to file\n", uip_len);
      s.addr += uip_len;
    }
    if(s.addr < s.len) {
      PT_YIELD_UNTIL(pt, uip_newdata());
    }
  } while(s.addr < s.len);
  leds_off(LEDS_RED);

#if DEBUG
  {
    int i, fd, j;
    printf("Contents of file:\n");
    fd = cfs_open("codeprop.out", CFS_READ);
    j = 0;
    printf("\n0x%04x: ", 0);
    for(i = 0; i < s.len; ++i) {
      unsigned char byte;
      cfs_read(fd, &byte, 1);
      printf("0x%02x, ", byte);
      ++j;
      if(j == 8) {
	printf("\n0x%04x: ", i + 1);
	j = 0;
      }
      clock_delay(400);
    }
    cfs_close(fd);
  }
#endif

  int ret;
  
  ret = start_program();
  
#if CONTIKI_TARGET_NETSIM
  rudolph0_send(&rudolph0, CLOCK_SECOND / 4);
#else /* CONTIKI_TARGET_NETSIM */
  if(ret == ELFLOADER_OK) {
    /* Propagate program. */
    rudolph0_send(&rudolph0, CLOCK_SECOND / 4);
  }
#endif /* CONTIKI_TARGET_NETSIM */
  
  /* Return "ok" message. */
  do {
    ret = strlen(msg);
    uip_send(msg, ret);
    PT_WAIT_UNTIL(pt, uip_acked() || uip_rexmit() || uip_closed());
  } while(uip_rexmit());

  /* Close the connection. */
  uip_close();

    
 thread_done:;
  PT_END(pt);
}
/*---------------------------------------------------------------------*/
static void
write_chunk(struct rudolph0_conn *c, int offset, int flag,
	    uint8_t *data, int datalen)
{
  int fd;

  leds_toggle(LEDS_YELLOW);
  
  if(flag == RUDOLPH0_FLAG_NEWFILE) {
    printf("+++ rudolph0 new file incoming at %u\n", clock_time());
    fd = cfs_open("codeprop.out", CFS_WRITE);
    
    if(elfloader_autostart_processes != NULL) {
      PRINTF("Stopping old programs.\n");
      autostart_exit(elfloader_autostart_processes);
      elfloader_autostart_processes = NULL;
    }

  } else {
    fd = cfs_open("codeprop.out", CFS_WRITE + CFS_APPEND);
  }
  
  if(datalen > 0) {
    int ret;
    cfs_seek(fd, offset, CFS_SEEK_SET);
    ret = cfs_write(fd, data, datalen);
    /*    printf("write_chunk wrote %d bytes at %d, %d\n", ret, offset, (unsigned char)data[0]);*/
  }

  cfs_close(fd);

  if(flag == RUDOLPH0_FLAG_LASTCHUNK) {
    printf("+++ rudolph0 entire file received at %u\n", clock_time());
    start_program();
    leds_off(LEDS_YELLOW);
  }
}
static int
read_chunk(struct rudolph0_conn *c, int offset, uint8_t *to, int maxsize)
{
  int fd;
  int ret;

  leds_toggle(LEDS_GREEN);
  
  fd = cfs_open("codeprop.out", CFS_READ);

  cfs_seek(fd, offset, CFS_SEEK_SET);
  ret = cfs_read(fd, to, maxsize);
  /*  printf("read_chunk %d bytes at %d, %d\n", ret, offset, (unsigned char)to[0]);*/
  if(ret < maxsize) {
    leds_off(LEDS_GREEN);
  }
  cfs_close(fd);
  return ret;
}
const static struct rudolph0_callbacks rudolph0_call = {write_chunk,
							read_chunk};
/*---------------------------------------------------------------------*/
PROCESS_THREAD(tcp_loader_process, ev, data)
{
  PROCESS_BEGIN();

  rudolph0_open(&rudolph0, 20, &rudolph0_call);
  
  tcp_listen(UIP_HTONS(CODEPROP_DATA_PORT));
  
  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event && uip_conn->lport == UIP_HTONS(CODEPROP_DATA_PORT)) {
      if(uip_connected()) {	/* Really uip_connecting()!!! */
	if(data == NULL) {
	  PT_INIT(&s.tcpthread_pt);
	  process_poll(&tcp_loader_process);
	  tcp_markconn(uip_conn, &s);

	  if(elfloader_autostart_processes != NULL) {
	    PRINTF("Stopping old programs.\n");
	    autostart_exit(elfloader_autostart_processes);
	    elfloader_autostart_processes = NULL;
	  }
	} else {
	  PRINTF(("codeprop: uip_connected() and data != NULL\n"));
	  uip_abort();
	}
      }
      recv_tcpthread(&s.tcpthread_pt); /* Run thread */

      if(uip_closed() || uip_aborted() || uip_timedout()) {
	PRINTF(("codeprop: connection down\n"));
	tcp_markconn(uip_conn, NULL);
      }
    }
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------*/
#include "net/rime/collect.h"
#include "net/rime/mesh.h"
#include "net/rime/rudolph0.h"
#include "net/rime/rudolph1.h"
void
dummy(void)
{
  /* Make sure that all Rime modules are present in the core */
  collect_close(NULL);
  mesh_close(NULL);
  ipolite_close(NULL);
  polite_close(NULL);
  ruc_close(NULL);
  sibc_close(NULL);
  rudolph0_close(NULL);
  rudolph1_close(NULL);

  /* Make sure psock is included */
  psock_datalen(NULL);
}
/*---------------------------------------------------------------------*/

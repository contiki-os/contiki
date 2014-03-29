/*
  Copyright (c) 2010-2014 Alex Barclay.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "contiki.h"
#include "contiki-net.h"
#include <p33Fxxxx.h>

#include <stdio.h> /* For printf() */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "intsafe.h"
#include "dspicflash.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

// Alloc memory from 0x3800 to 0x4000 for the loaded module
// This is just to reserve memory, we don't actually use it outside of the module
uint8_t dummy[0x800] __attribute__((address(0x3800)));
uint8_t dummy2[0x80] __attribute__((address(0x4780))); // This is some DMA memory

void module_entry();
void module_exit();

static struct etimer timer;

uint32_t
read_prog_mem(uint32_t addr)
{
  TBLPAG = addr >> 16;
  addr = addr & 0xffff;
  uint32_t r = ((uint32_t)__builtin_tblrdh(addr) << 16) | (uint32_t)__builtin_tblrdl(addr);
  return r;
}
void
read_page()
{
  uint32_t i = 0;
  while (i < 0x1000) {
    if (i % 8 == 0)
      printf("%08lx  ", i);
    
    uint32_t val = read_prog_mem(i);
    printf("%08lx  ", val);
    ++i;
    if (i % 8 == 0)
      printf("\n");
  }
}

uint16_t
read_prog_line(char *buf, uint32_t addr, uint16_t len)
{
  int cuml = 0;
  int r;

  r = sprintf(buf, "%08lx:  ", addr);
  cuml += r;

  while(len > 0) {
    uint32_t v = read_prog_mem(addr);
    r = sprintf(buf + cuml, "%08lx ", v);
    cuml += r;
    len -= 2;
    addr += 2;
  }

  r = sprintf(buf, "\r\n");
  cuml += r;
  return cuml;
}
#define CMD_INITPROG  0
#define CMD_ERASEPROG 1
#define CMD_WRITEBUF  2
#define CMD_WRITEPROG 3
#define CMD_READPROG  4
#define CMD_DONEPROG  5

#define CMD_CMDDONE 0

const char primePrintf[] = " ";

static void
print_local_addresses()
{
  printf("Server IPv6 addresses: ");
#if 0
  uip_ds6_addr_t *addr = uip_ds6_get_link_local(-1);
  if(addr) {
    printf("%p ", &addr->ipaddr);
    uip_debug_ipaddr_print(&addr->ipaddr);
    puts("");
  }
  addr = uip_ds6_get_global(-1);
  if(addr) {
    printf("%p ", &addr->ipaddr);
    uip_debug_ipaddr_print(&addr->ipaddr);
    puts("");
  }
  addr = uip_ds6_if.addr_list;
  addr++;
  printf("%p ", &addr->ipaddr);
  uip_debug_ipaddr_print(&addr->ipaddr);
  puts("");
#else
  uip_ds6_addr_t *addr;
  char first = 1;
  for(addr = uip_ds6_if.addr_list;
      addr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; ++addr) {
    if(addr->isused) {
      if(!first) {
        printf(", ");
      } else {
        first = 0;
      } uip_debug_ipaddr_print(&addr->ipaddr);
    }
  }
  puts("");
#endif
}
uint16_t
psv_read_uint16(uint16_t __psv__ *val)
{
  return *val;
}

static char module_executing = 0;

void
check_load(int load)
{
  if(psv_read_uint16((uint16_t __psv__ *)0x10000UL) == 0xffff) {
    printf("No module loaded %d\n", load);
  } else {
    printf("Module loaded %d\n", load);
    if (load) {
      module_entry();
      module_executing = 1;
    } else if (module_executing) {
      module_exit();
      module_executing = 0;
    }
  }
}
/* static struct uip_udp_conn *server_conn; */

uint16_t
readWord(uint8_t *p)
{
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

uint32_t
readLong(uint8_t* p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

void
writeLong(uint8_t *p, uint32_t val)
{
  p[0] = val & 0xff;
  p[1] = (val >> 8) & 0xff;
  p[2] = (val >> 16) & 0xff;
  p[3] = (val >> 24) & 0xff;
}

void
writeWord(uint8_t *p, uint16_t val)
{
  p[0] = val & 0xff;
  p[1] = (val >> 8) & 0xff;
}

uint16_t
init_math(float f)
{
  float j = f / 128;
  float k = 0;
  float v = sinf(k);
  k += j;
  v += sin(k);
  return (uint16_t)((v * 2000) + 2000);
}

enum CmdType {
  PKT_READPAGE = 0,
  PKT_WRITEPAGE = 1,
  PKT_ERASEPAGE = 2,
  PKT_UNLOADMOD = 3,
  PKT_LOADMOD = 4,
};

enum ProgramContextState {
  CTXT_READHDR,
  CTXT_READBODY,
  CTXT_READPAGE,
  CTXT_ERASEPAGE,
  CTXT_WRITEPAGE,
  CTXT_UNLOADMOD,
  CTXT_LOADMOD,
  CTXT_END,
};

struct ProgramContext {
  enum ProgramContextState state;
  uint16_t toRead;
  uint16_t toSend;
  uint32_t pageAddr;
  uint16_t pageLen;
  struct psock ps;
  uint8_t sockbuf[10];
  uint8_t *progBuf;
  uint8_t *bptr;
  uint8_t cmd;
};

/* Packet Format */
/* cmd(1) = 0 - read, 1 - write, 2 - erase, 3 - unload modules, 4 - load modules */
/* len(2) = length in bytes of whole pkt */
/* pageAddr(4) = word address of page base */
/* words to read or write (2) = max 128, not used for erase */
struct ProgramContext *progContext;

static void
process_sockchar(struct ProgramContext *ctxt, uint8_t d)
{
  switch(ctxt->state) {
  case CTXT_READHDR:
    *ctxt->bptr++ = d;
    if(--ctxt->toRead == 0) {
      ctxt->toRead = readWord(ctxt->progBuf + 1) - 9;
      ctxt->pageAddr = readLong(ctxt->progBuf + 3);
      ctxt->pageLen = readWord(ctxt->progBuf + 7);
      ctxt->cmd = *ctxt->progBuf;
      ctxt->bptr = ctxt->progBuf;
      if(ctxt->toRead > 0) {
        ctxt->state = CTXT_READBODY;
      } else {
        switch(ctxt->cmd) {
        case PKT_READPAGE:
          ctxt->state = CTXT_READPAGE;
          break;
        case PKT_ERASEPAGE:
          ctxt->state = CTXT_ERASEPAGE;
          break;
        case PKT_UNLOADMOD:
          ctxt->state = CTXT_UNLOADMOD;
          break;
        case PKT_LOADMOD:
          ctxt->state = CTXT_LOADMOD;
          break;
        }
      }
    }
    break;
  case CTXT_READBODY:
    *ctxt->bptr++ = d;
    if(--ctxt->toRead == 0 && ctxt->cmd == PKT_WRITEPAGE) {
      ctxt->state = CTXT_WRITEPAGE;
    }
    break;
  default:
    break;
  }
}
void
dumpbuf(uint32_t pageAddr, uint8_t *buf, uint16_t len)
{
  while(len) {
    printf("%08lx ", pageAddr);
    int cnt = 16;
    while(cnt-- && len) {
      printf(" %02x", *buf++);
      --len;
      pageAddr++;
    }
    printf("\n");
  }
}

static PT_THREAD(handle_connection(struct ProgramContext *progContext))
{
  PSOCK_BEGIN(&progContext->ps);

  /* Start everything off by writing out the processor ID and revision ID */
  uint16_t v = TBLPAG;
  TBLPAG = 0xff;
  writeWord(progContext->progBuf, __builtin_tblrdl(0x0000));
  writeWord(progContext->progBuf + 2, __builtin_tblrdl(0x0002));
  progContext->toSend = 4;
  TBLPAG = v;
  PSOCK_SEND(&progContext->ps, progContext->progBuf, progContext->toSend);

  progContext->state = CTXT_READHDR;
  progContext->toRead = 9;
  progContext->bptr = progContext->progBuf;

  do {
    PSOCK_WAIT_UNTIL(&progContext->ps, PSOCK_NEWDATA(&progContext->ps));
    PSOCK_READBUF_LEN(&progContext->ps, 1);
    int i = PSOCK_DATALEN(&progContext->ps);
    uint8_t *s = progContext->sockbuf;
    while(i--) {
      process_sockchar(progContext, *s++);
    }
    if(progContext->state == CTXT_READHDR || progContext->state == CTXT_READBODY) {
      continue;
    }

    switch(progContext->state) {
    case CTXT_READPAGE:
      printf("Read requested %08lx %04x\n", progContext->pageAddr, progContext->pageLen);
      progContext->toSend = progContext->pageLen << 1;
      read_flash_page(progContext->pageAddr, progContext->progBuf,
                      progContext->pageLen, PAGE_NOPACK);
      /*      dumpbuf(progContext->pageAddr, progContext->progBuf, progContext->pageLen); */
      break;
    case CTXT_ERASEPAGE:
      printf("Erase requested %08lx %04x\n", progContext->pageAddr, progContext->pageLen);
      erase_flash_page(progContext->pageAddr);
      *progContext->progBuf = 0;
      progContext->toSend = 1;
      break;
    case CTXT_WRITEPAGE:
      printf("Write requested %08lx %04x\n", progContext->pageAddr, progContext->pageLen);
      /*      dumpbuf(progContext->pageAddr, progContext->progBuf, progContext->pageLen); */
      write_flash_page(progContext->pageAddr, progContext->progBuf,
                       progContext->pageLen, PAGE_NOPACK);
      *progContext->progBuf = 0;
      progContext->toSend = 1;
      break;
    case CTXT_UNLOADMOD:
      check_load(0);
      *progContext->progBuf = 0;
      progContext->toSend = 1;
      break;
    case CTXT_LOADMOD:
      check_load(1);
      *progContext->progBuf = 0;
      progContext->toSend = 1;
      break;
    default:
      *progContext->progBuf = 1;
      progContext->toSend = 1;
      break;
    }
    PSOCK_SEND(&progContext->ps, progContext->progBuf, progContext->toSend);

    progContext->state = CTXT_READHDR;
    progContext->toRead = 9;
    progContext->bptr = progContext->progBuf;
  } while(1);

  PSOCK_CLOSE(&progContext->ps);
  PSOCK_END(&progContext->ps);
}

PROCESS(sockserver_process, "OTAProgrammer");
PROCESS_THREAD(sockserver_process, ev, data)
{
  PROCESS_BEGIN();
  puts("Sockserver waiting for tcp");
  while(!process_is_running(&tcpip_process)) {
    PROCESS_PAUSE();
  }
  puts("Sockserver running");

  /* Wait for an incoming connection - programming port */
  tcp_listen(UIP_HTONS(1234));
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    if(uip_connected()) {
      /* Setup the programming context */
      progContext = (struct ProgramContext *)malloc(sizeof(struct ProgramContext));
      if(progContext) {
        progContext->progBuf = (uint8_t *)malloc(256);
        if(progContext->progBuf) {
          printf("alloc memory %04x\n", (int)progContext);

          PSOCK_INIT(&progContext->ps, progContext->sockbuf, sizeof(progContext->sockbuf));
          while(!(uip_aborted() || uip_closed() || uip_timedout())) {
            PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
            handle_connection(progContext);
          }
          free(progContext->progBuf);
        } else {
          uip_abort();
        } free(progContext);
      } else {
        uip_abort();
      } puts("socket exited");
    }
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS(bootloader_process, "Bootloader");
AUTOSTART_PROCESSES(&bootloader_process, &sockserver_process);
/*---------------------------------------------------------------------------*/
uint32_t timecnt;
PROCESS_THREAD(bootloader_process, ev, data)
{
  PROCESS_BEGIN();
  timecnt = 0;
  while(!process_is_running(&tcpip_process)) {
    PROCESS_PAUSE();
  }

  etimer_set(&timer, CLOCK_SECOND * 5);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    printf("tick %ld - ", timecnt++);
    print_local_addresses();
    etimer_restart(&timer);
  }

  printf("%f\n", 1.0);
  (void)init_math(42.5);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

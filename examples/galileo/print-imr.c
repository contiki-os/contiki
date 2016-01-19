/*
 * Copyright (C) 2015-2016, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

#include "contiki.h"

#include "imr.h"
#include "msg-bus.h"

#define HOST_BRIGE_PORT  3

#define HSMMCTL_OFFSET   4

#define HMBOUND_OFFSET   8

/* Refer to Intel Quark SoC X1000 Datasheet, Section 12.7.2.2 for more details
 * on the Host System Management Mode Controls register.
 */
typedef union hsmmctl {
  struct {
    uint32_t lock             : 1;
    uint32_t rd_open          : 1;
    uint32_t wr_open          : 1;
    uint32_t                  : 1;
    uint32_t start            : 12;
    uint32_t                  : 1;
    uint32_t non_host_rd_open : 1;
    uint32_t non_host_wr_open : 1;
    uint32_t                  : 1;
    uint32_t end              : 12;
  };
  uint32_t raw;
} hsmmctl_t;

/* Amount to shift hsmmctl.start/end left to obtain the bound address */
#define HSMMCTL_SHAMT 20

/* Refer to Intel Quark SoC X1000 Datasheet, Section 12.7.2.3 for more details
 * on the Host Memory I/O Boundary register.
 */
typedef union hmbound {
  struct {
    uint32_t lock   : 1;
    uint32_t io_dis : 1;
    uint32_t        : 10;
    uint32_t io_bnd : 20;
  };
  uint32_t raw;
} hmbound_t;

/* Amount to shift hmbound.io_bnd left to obtain the bound address */
#define HMBOUND_SHAMT 12

PROCESS(imr_process, "IMR Process");
AUTOSTART_PROCESSES(&imr_process);
/*---------------------------------------------------------------------------*/
static hsmmctl_t
hsmmctl_read(void)
{
  hsmmctl_t hsmm;

  quarkX1000_msg_bus_read(HOST_BRIGE_PORT, HSMMCTL_OFFSET, &hsmm.raw);

  return hsmm;
}
/*---------------------------------------------------------------------------*/
static void
hsmmctl_print(hsmmctl_t hsmm)
{
  printf("[%08x, %08x) %slocked, non-SMM host: %c%c, non-host: %c%c",
         hsmm.start << HSMMCTL_SHAMT, hsmm.end << HSMMCTL_SHAMT,
         hsmm.lock ? "" : "un",
         hsmm.rd_open ? 'R' : '-', hsmm.wr_open ? 'W' : '-',
         hsmm.non_host_rd_open ? 'R' : '-', hsmm.non_host_wr_open ? 'W' : '-');
}
/*---------------------------------------------------------------------------*/
static hmbound_t
hmbound_read(void)
{
  hmbound_t hmb;

  quarkX1000_msg_bus_read(HOST_BRIGE_PORT, HMBOUND_OFFSET, &hmb.raw);

  return hmb;
}
/*---------------------------------------------------------------------------*/
static void
hmbound_print(hmbound_t hmb)
{
  printf("%08x %slocked, IO %sabled",
         hmb.io_bnd << HMBOUND_SHAMT,
         hmb.lock ? "" : "un",
         hmb.io_dis ? "dis" : "en");
}
/*---------------------------------------------------------------------------*/
static void
imr_print(quarkX1000_imr_t imr)
{
  printf("[%08x, %08x) %slocked, rdmsk: %08x, wrmsk: %08x",
         imr.lo.addr << QUARKX1000_IMR_SHAMT,
         imr.hi.addr << QUARKX1000_IMR_SHAMT,
         imr.lo.lock ? "" : "un", imr.rdmsk.raw, imr.wrmsk.raw);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(imr_process, ev, data)
{
  int i;
  quarkX1000_imr_t imr;
  hsmmctl_t hsmm;
  hmbound_t hmb;

  PROCESS_BEGIN();

  fputs("Host SMM Controls: ", stdout);
  hsmm = hsmmctl_read();
  hsmmctl_print(hsmm);
  fputs("\nHost Memory IO Boundary: ", stdout);
  hmb = hmbound_read();
  hmbound_print(hmb);
  puts("\nIsolated Memory Regions:");
  for(i = 0; i < QUARKX1000_IMR_CNT; i++) {
    printf(" - #%d: ", i);
    imr = quarkX1000_imr_read(i);
    imr_print(imr);
    puts("");
  }

  PROCESS_END();
}

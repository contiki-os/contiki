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

#include <assert.h>
#include <stdio.h>
#include <x86intrin.h>

#include "contiki.h"
#include "prot-domains.h"
#include "startup.h"
#include "syscalls.h"

#define CPU_FREQ (400 * 1000 * 1000)
/* Run the test for approximately eight seconds.
 *
 * Duration expressed as shift amount to avoid integer overflow.
 */
#define DURATION_SECONDS_SHAMT 3

#ifdef SAMPLE_METADATA
typedef struct sample_meta {
  int cnt;

#if X86_CONF_PROT_DOMAINS == X86_CONF_PROT_DOMAINS__PAGING
  /**
   * See the comment on the padding in the metadata for the Intel Quark X1000
   * Ethernet driver for an explanation of why it is sized and structured like
   * this.
   */
  uint8_t pad[MIN_PAGE_SIZE - sizeof(int)];
#endif
} __attribute__((packed)) sample_meta_t;

static sample_meta_t ATTR_BSS_META meta = { .cnt = 0 };
#endif

PROT_DOMAINS_ALLOC(dom_client_data_t, ping_dcd);
PROT_DOMAINS_ALLOC(dom_client_data_t, pong_dcd);

PROCESS(prot_domain_latency_process, "Ping-Pong Process");
AUTOSTART_PROCESSES(&prot_domain_latency_process);
/*---------------------------------------------------------------------------*/
void pong(uint64_t *mid, int *cnt);
SYSCALLS_DEFINE_SINGLETON(pong, pong_dcd,
                          uint64_t *mid, int *cnt)
{
#ifdef SAMPLE_METADATA
  sample_meta_t *loc_meta = (sample_meta_t *)PROT_DOMAINS_META(pong_dcd);
#endif

  *mid = _rdtsc();

#ifdef SAMPLE_METADATA
  META_READL(*cnt, loc_meta->cnt);
  META_WRITEL(loc_meta->cnt, *cnt + 1);
#endif
}
/*---------------------------------------------------------------------------*/
void ping(void);
SYSCALLS_DEFINE_SINGLETON(ping, ping_dcd)
{
  uint64_t start, mid, end;
  uint64_t diff1 = 0, diff2 = 0;
  double diff1_d, diff2_d;
  int i = 0;
  int cnt;

  while(((diff1 + diff2) >> DURATION_SECONDS_SHAMT) < CPU_FREQ) {
    start = _rdtsc();
    pong(&mid, &cnt);
    end = _rdtsc();

#ifdef SAMPLE_METADATA
    assert(cnt == i);
#endif

    /* exclude the warm-up round */
    if(i != 0) {
      diff1 += mid - start;
      diff2 += end - mid;
    }

    i++;
  }

  diff1_d = diff1;
  diff2_d = diff2;

  diff1_d /= i - 1;
  diff2_d /= i - 1;

  puts(  "Sample protection domain ping-pong switching latency measurements:");
  printf(" %u iterations\n", i - 1);
  printf("   Avg. # cycles ping -> pong: %.2f\n", diff1_d);
  printf(" + Avg. # cycles pong -> ping: %.2f\n", diff2_d);
  puts(  " ----------------------------------------");
  printf("   Avg. # cycles round-trip:   %.2f\n", diff1_d + diff2_d);
}
/*---------------------------------------------------------------------------*/
KERN_STARTUP_FUNC(sample_domain_init)
{
  PROT_DOMAINS_INIT_ID(ping_dcd);
  prot_domains_reg(&ping_dcd, 0, 0, 0, 0, false);
  SYSCALLS_INIT(ping);
  SYSCALLS_AUTHZ(ping, ping_dcd);

  PROT_DOMAINS_INIT_ID(pong_dcd);
  prot_domains_reg(&pong_dcd, 0, 0,
#ifdef SAMPLE_METADATA
                   (uintptr_t)&meta, sizeof(meta), false);
#else
                   0, 0, false);
#endif
  SYSCALLS_INIT(pong);
  SYSCALLS_AUTHZ(pong, pong_dcd);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(prot_domain_latency_process, ev, data)
{
  PROCESS_BEGIN();

  /* Run the latency test from the ping domain so that interrupts
   * are disabled during the test.
   */
  ping();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

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

#ifndef CPU_X86_DRIVERS_QUARKX1000_IMR_H_
#define CPU_X86_DRIVERS_QUARKX1000_IMR_H_

#include <stdint.h>

typedef union quarkX1000_imr_lo {
  struct {
    uint32_t      : 2;
    uint32_t addr : 22;
    uint32_t      : 7;
    uint32_t lock : 1;
  };
  uint32_t raw;
} quarkX1000_imr_lo_t;

typedef union quarkX1000_imr_hi {
  struct {
    uint32_t      : 2;
    uint32_t addr : 22;
    uint32_t      : 8;
  };
  uint32_t raw;
} quarkX1000_imr_hi_t;

/* Amount to shift imr_lo/hi.addr left to obtain the bound address */
#define QUARKX1000_IMR_SHAMT 10

typedef union quarkX1000_imr_rdmsk {
  struct {
    uint32_t cpu0             : 1;
    uint32_t cpu_0            : 1;
    uint32_t                  : 6;
    uint32_t vc0_sai_id0      : 1;
    uint32_t vc0_sai_id1      : 1;
    uint32_t vc0_sai_id2      : 1;
    uint32_t vc0_sai_id3      : 1;
    uint32_t vc1_sai_id0      : 1;
    uint32_t vc1_sai_id1      : 1;
    uint32_t vc1_sai_id2      : 1;
    uint32_t vc1_sai_id3      : 1;
    uint32_t                  : 13;
    uint32_t punit            : 1;
    uint32_t                  : 1;
    uint32_t esram_flush_init : 1;
  };
  uint32_t raw;
} quarkX1000_imr_rdmsk_t;

typedef union quarkX1000_imr_wrmsk {
  struct {
    uint32_t cpu0             : 1;
    uint32_t cpu_0            : 1;
    uint32_t                  : 6;
    uint32_t vc0_sai_id0      : 1;
    uint32_t vc0_sai_id1      : 1;
    uint32_t vc0_sai_id2      : 1;
    uint32_t vc0_sai_id3      : 1;
    uint32_t vc1_sai_id0      : 1;
    uint32_t vc1_sai_id1      : 1;
    uint32_t vc1_sai_id2      : 1;
    uint32_t vc1_sai_id3      : 1;
    uint32_t                  : 13;
    uint32_t punit            : 1;
    uint32_t cpu_snoop        : 1;
    uint32_t esram_flush_init : 1;
  };
  uint32_t raw;
} quarkX1000_imr_wrmsk_t;

/* Refer to Intel Quark SoC X1000 Datasheet, Section 12.7.4 for more details on
 * the IMR registers.
 */
typedef struct quarkX1000_imr {
  quarkX1000_imr_lo_t lo;
  quarkX1000_imr_hi_t hi;
  quarkX1000_imr_rdmsk_t rdmsk;
  quarkX1000_imr_wrmsk_t wrmsk;
} quarkX1000_imr_t;

/* The Intel Quark SoC X1000 defines eight general IMRs. */
#define QUARKX1000_IMR_CNT 8

/* Routines for accessing the Isolated Memory Region (IMR) feature.
 *
 * The Intel Quark X1000 SoC includes support for Isolated Memory Regions
 * (IMRs), which are specified using range registers and associated
 * control registers that are accessible via the message bus.
 *
 * Refer to Intel Quark SoC X1000 Datasheet, Section 12.2 for more information.
 */

quarkX1000_imr_t quarkX1000_imr_read(uint32_t imr_idx);
void quarkX1000_imr_write(uint32_t imr_idx, quarkX1000_imr_t imr);

#endif /* CPU_X86_DRIVERS_QUARKX1000_IMR_H_ */

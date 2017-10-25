/*
 * Copyright (c) 2016, Benoît Thébaudeau <benoit.thebaudeau.dev@gmail.com>
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
/**
 * \addtogroup cc2538-soc
 * @{
 *
 * \file
 * Implementation of the cc2538 SoC driver
 */
#include "contiki-conf.h"
#include "dev/rom-util.h"
#include "dev/sys-ctrl.h"
#include "reg.h"
#include "soc.h"

#include <stdint.h>
#include <stdio.h>
/*----------------------------------------------------------------------------*/
#define DIECFG0         0x400d3014
#define DIECFG0_SRAM_SIZE_OFS   7
#define DIECFG0_SRAM_SIZE_SZ    3
#define DIECFG0_SRAM_SIZE_MSK   (((1 << DIECFG0_SRAM_SIZE_SZ) - 1) << \
                                 DIECFG0_SRAM_SIZE_OFS)
#define DIECFG2         0x400d301c
#define DIECFG2_DIE_REV_OFS     8
#define DIECFG2_DIE_REV_SZ      8
#define DIECFG2_DIE_REV_MSK     (((1 << DIECFG2_DIE_REV_SZ) - 1) << \
                                 DIECFG2_DIE_REV_OFS)
#define DIECFG2_AES_EN          0x00000002
#define DIECFG2_PKA_EN          0x00000001
/*----------------------------------------------------------------------------*/
uint8_t
soc_get_rev(void)
{
  uint8_t rev = (REG(DIECFG2) & DIECFG2_DIE_REV_MSK) >> DIECFG2_DIE_REV_OFS;

  /* PG1.0 is encoded as 0x00. */
  if(!(rev >> 4))
    rev += 0x10;
  return rev;
}
/*----------------------------------------------------------------------------*/
uint32_t
soc_get_sram_size(void)
{
  uint32_t size_code = (REG(DIECFG0) & DIECFG0_SRAM_SIZE_MSK) >>
                       DIECFG0_SRAM_SIZE_OFS;

  return size_code <= 1 ? (2 - size_code) << 13 : 32 << 10;
}
/*----------------------------------------------------------------------------*/
uint32_t
soc_get_features(void)
{
  return REG(DIECFG2) & (DIECFG2_AES_EN | DIECFG2_PKA_EN);
}
/*----------------------------------------------------------------------------*/
void
soc_print_info(void)
{
  uint8_t rev = soc_get_rev();
  uint32_t features = soc_get_features();

  printf("CC2538: ID: 0x%04lx, rev.: PG%d.%d, Flash: %lu KiB, SRAM: %lu KiB, "
           "AES/SHA: %u, ECC/RSA: %u\n"
         "System clock: %lu Hz\n"
         "I/O clock: %lu Hz\n"
         "Reset cause: %s\n",
         rom_util_get_chip_id(),
         rev >> 4, rev & 0x0f,
         rom_util_get_flash_size() >> 10,
         soc_get_sram_size() >> 10,
         !!(features & SOC_FEATURE_AES_SHA),
         !!(features & SOC_FEATURE_ECC_RSA),
         sys_ctrl_get_sys_clock(),
         sys_ctrl_get_io_clock(),
         sys_ctrl_get_reset_cause_str());
}

/** @} */

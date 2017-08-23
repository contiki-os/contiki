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
#include "imr.h"
#include "msg-bus.h"

#define MEM_MANAGER_PORT 5

#define IMR_BASE_OFFSET  0x40
#define IMR_REG_COUNT    4

#define IMR_LO_OFFSET    0
#define IMR_HI_OFFSET    1
#define IMR_RDMSK_OFFSET 2
#define IMR_WRMSK_OFFSET 3

/*---------------------------------------------------------------------------*/
/**
 * \brief Read the contents of the specified IMR.
 */
quarkX1000_imr_t
quarkX1000_imr_read(uint32_t imr_idx)
{
  quarkX1000_imr_t imr;
  uint32_t reg_base = IMR_BASE_OFFSET + (IMR_REG_COUNT * imr_idx);

  assert(imr_idx < QUARKX1000_IMR_CNT);

  quarkX1000_msg_bus_read(MEM_MANAGER_PORT,
                          reg_base + IMR_LO_OFFSET, &imr.lo.raw);
  quarkX1000_msg_bus_read(MEM_MANAGER_PORT,
                          reg_base + IMR_HI_OFFSET, &imr.hi.raw);
  quarkX1000_msg_bus_read(MEM_MANAGER_PORT,
                          reg_base + IMR_RDMSK_OFFSET, &imr.rdmsk.raw);
  quarkX1000_msg_bus_read(MEM_MANAGER_PORT,
                          reg_base + IMR_WRMSK_OFFSET, &imr.wrmsk.raw);

  return imr;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Overwrite the contents of the specified IMR.
 */
void
quarkX1000_imr_write(uint32_t imr_idx, quarkX1000_imr_t imr)
{
  uint32_t reg_base = IMR_BASE_OFFSET + (IMR_REG_COUNT * imr_idx);

  assert(imr_idx < QUARKX1000_IMR_CNT);

  quarkX1000_msg_bus_write(MEM_MANAGER_PORT,
                           reg_base + IMR_HI_OFFSET, imr.hi.raw);
  quarkX1000_msg_bus_write(MEM_MANAGER_PORT,
                           reg_base + IMR_RDMSK_OFFSET, imr.rdmsk.raw);
  quarkX1000_msg_bus_write(MEM_MANAGER_PORT,
                           reg_base + IMR_WRMSK_OFFSET, imr.wrmsk.raw);
  /* This register must be programmed last, in case it sets the lock bit. */
  quarkX1000_msg_bus_write(MEM_MANAGER_PORT,
                           reg_base + IMR_LO_OFFSET, imr.lo.raw);
}
/*---------------------------------------------------------------------------*/

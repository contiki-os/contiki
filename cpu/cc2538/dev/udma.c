/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup cc2538-udma
 * @{
 *
 * \file
 * Implementation of the cc2538 micro-DMA driver
 */
#include "contiki-conf.h"
#include "dev/udma.h"
#include "dev/nvic.h"
#include "reg.h"

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
struct channel_ctrl {
  uint32_t src_end_ptr;
  uint32_t dst_end_ptr;
  uint32_t ctrl_word;
  uint32_t unused;
};

static volatile struct channel_ctrl channel_config[UDMA_CONF_MAX_CHANNEL + 1]
  __attribute__ ((aligned(1024)));
/*---------------------------------------------------------------------------*/
void
udma_init()
{
  memset(&channel_config, 0, sizeof(channel_config));

  REG(UDMA_CFG) = UDMA_CFG_MASTEN;

  REG(UDMA_CTLBASE) = (uint32_t)(&channel_config);

  nvic_interrupt_enable(NVIC_INT_UDMA);
  nvic_interrupt_enable(NVIC_INT_UDMA_ERR);
}
/*---------------------------------------------------------------------------*/
void
udma_set_channel_src(uint8_t channel, uint32_t src_end)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  channel_config[channel].src_end_ptr = src_end;
}
/*---------------------------------------------------------------------------*/
void
udma_set_channel_dst(uint8_t channel, uint32_t dst_end)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  channel_config[channel].dst_end_ptr = dst_end;
}
/*---------------------------------------------------------------------------*/
void
udma_set_channel_control_word(uint8_t channel, uint32_t ctrl)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  channel_config[channel].ctrl_word = ctrl;
}
/*---------------------------------------------------------------------------*/
void
udma_set_channel_assignment(uint8_t channel, uint8_t enc)
{
  uint32_t base_chmap = UDMA_CHMAP0;
  uint8_t shift;

  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  /* Calculate the address of the relevant CHMAP register */
  base_chmap += (channel >> 3) * 4;

  /* Calculate the shift value for the correct CHMAP register bits */
  shift = (channel & 0x07);

  /* Read CHMAPx value, zero out channel's bits and write the new value */
  REG(base_chmap) = (REG(base_chmap) & ~(0x0F << shift)) | (enc << shift);
}
/*---------------------------------------------------------------------------*/
void
udma_channel_enable(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  REG(UDMA_ENASET) |= 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_disable(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  /* Writes of 0 have no effect, this no need for RMW */
  REG(UDMA_ENACLR) = 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_use_alternate(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  REG(UDMA_ALTSET) |= 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_use_primary(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  /* Writes of 0 have no effect, this no need for RMW */
  REG(UDMA_ALTCLR) = 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_prio_set_high(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  REG(UDMA_PRIOSET) |= 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_prio_set_default(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  /* Writes of 0 have no effect, this no need for RMW */
  REG(UDMA_PRIOCLR) = 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_use_burst(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  REG(UDMA_USEBURSTSET) |= 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_use_single(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  /* Writes of 0 have no effect, this no need for RMW */
  REG(UDMA_USEBURSTCLR) = 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_mask_set(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  REG(UDMA_REQMASKSET) |= 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_mask_clr(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  /* Writes of 0 have no effect, this no need for RMW */
  REG(UDMA_REQMASKCLR) = 1 << channel;
}
/*---------------------------------------------------------------------------*/
void
udma_channel_sw_request(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return;
  }

  REG(UDMA_SWREQ) |= 1 << channel;
}
/*---------------------------------------------------------------------------*/
uint8_t
udma_channel_get_mode(uint8_t channel)
{
  if(channel > UDMA_CONF_MAX_CHANNEL) {
    return 0;
  }

  return (channel_config[channel].ctrl_word & 0x07);
}
/*---------------------------------------------------------------------------*/
void
udma_isr()
{
  /* Simply clear Channel interrupt status for now */
  REG(UDMA_CHIS) = UDMA_CHIS_CHIS;
}
/*---------------------------------------------------------------------------*/
void
udma_err_isr()
{
  /* Stub Implementation, just clear the error flag */
  REG(UDMA_ERRCLR) = 1;
}
/*---------------------------------------------------------------------------*/

/** @} */

/*
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-oscillators
 * @{
 *
 * \file
 * Implementation of CC26xxware oscillator control wrappers.
 */
/*---------------------------------------------------------------------------*/
#include "ti-lib.h"

#include <stdint.h>
/*---------------------------------------------------------------------------*/
static uint32_t
osc_interface_en(void)
{
  uint32_t smph_clk_state;

  /* Enable OSC DIG interface to change clock sources */
  ti_lib_osc_interface_enable();

  /* Save the state of the SMPH clock within AUX */
  smph_clk_state = ti_lib_aux_wuc_clock_status(AUX_WUC_SMPH_CLOCK);

  /* Make sure the SMPH clock within AUX is enabled */
  ti_lib_aux_wuc_clock_enable(AUX_WUC_SMPH_CLOCK);
  while(ti_lib_aux_wuc_clock_status(AUX_WUC_SMPH_CLOCK) != AUX_WUC_CLOCK_READY);

  return smph_clk_state;
}
/*---------------------------------------------------------------------------*/
static void
osc_interface_dis(uint32_t smph_clk_state)
{
  /* If the SMPH clock was off, turn it back off */
  if(smph_clk_state == AUX_WUC_CLOCK_OFF) {
    ti_lib_aux_wuc_clock_disable(AUX_WUC_SMPH_CLOCK);
  }

  /* Disable OSC DIG interface */
  ti_lib_osc_interface_disable();
}
/*---------------------------------------------------------------------------*/
void
oscillators_select_lf_xosc(void)
{
  /* Enable the Osc interface and remember the state of the SMPH clock */
  uint32_t smph_clk_state = osc_interface_en();

  /* Switch LF clock source to the LF XOSC if required */
  if(ti_lib_osc_clock_source_get(OSC_SRC_CLK_LF) != OSC_XOSC_LF) {
    ti_lib_osc_clock_source_set(OSC_SRC_CLK_LF, OSC_XOSC_LF);

    /* Wait for LF clock source to become XOSC_LF */
    while(ti_lib_osc_clock_source_get(OSC_SRC_CLK_LF) != OSC_XOSC_LF);

    /* Disable the LF clock qualifiers */
    ti_lib_ddi_16_bit_field_write(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL0,
                                  DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL_M |
                                  DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_M,
                                  DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_S,
                                  0x3);
  }

  /* Restore the SMPH clock and disable the OSC interface */
  osc_interface_dis(smph_clk_state);
}
/*---------------------------------------------------------------------------*/
void
oscillators_select_lf_rcosc(void)
{
  /* Enable the Osc interface and remember the state of the SMPH clock */
  uint32_t smph_clk_state = osc_interface_en();

  /* Switch LF clock source to the LF XOSC if required */
  if(ti_lib_osc_clock_source_get(OSC_SRC_CLK_LF) != OSC_RCOSC_LF) {
    ti_lib_osc_clock_source_set(OSC_SRC_CLK_LF, OSC_RCOSC_LF);

    /* Wait for LF clock source to become XOSC_LF */
    while(ti_lib_osc_clock_source_get(OSC_SRC_CLK_LF) != OSC_RCOSC_LF);
  }

  /* Restore the SMPH clock and disable the OSC interface */
  osc_interface_dis(smph_clk_state);
}
/*---------------------------------------------------------------------------*/
void
oscillators_request_hf_xosc(void)
{
  /* Enable the Osc interface and remember the state of the SMPH clock */
  uint32_t smph_clk_state = osc_interface_en();

  if(ti_lib_osc_clock_source_get(OSC_SRC_CLK_HF) != OSC_XOSC_HF) {
    /*
     * Request to switch to the crystal to enable radio operation. It takes a
     * while for the XTAL to be ready so instead of performing the actual
     * switch, we return and we do other stuff while the XOSC is getting ready.
     */
    ti_lib_osc_clock_source_set(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_XOSC_HF);
  }

  /* Restore the SMPH clock and disable the OSC interface */
  osc_interface_dis(smph_clk_state);
}
/*---------------------------------------------------------------------------*/
void
oscillators_switch_to_hf_xosc(void)
{
  /* Enable the Osc interface and remember the state of the SMPH clock */
  uint32_t smph_clk_state = osc_interface_en();

  if(ti_lib_osc_clock_source_get(OSC_SRC_CLK_HF) != OSC_XOSC_HF) {
    /* Switch the HF clock source (cc26xxware executes this from ROM) */
    ti_lib_osc_hf_source_switch();
  }

  /* Restore the SMPH clock and disable the OSC interface */
  osc_interface_dis(smph_clk_state);
}
/*---------------------------------------------------------------------------*/
void
oscillators_switch_to_hf_rc(void)
{
  /* Enable the Osc interface and remember the state of the SMPH clock */
  uint32_t smph_clk_state = osc_interface_en();

  /* Set all clock sources to the HF RC Osc */
  ti_lib_osc_clock_source_set(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_RCOSC_HF);

  /* Check to not enable HF RC oscillator if already enabled */
  if(ti_lib_osc_clock_source_get(OSC_SRC_CLK_HF) != OSC_RCOSC_HF) {
    /* Switch the HF clock source (cc26xxware executes this from ROM) */
    ti_lib_osc_hf_source_switch();
  }

  /* Restore the SMPH clock and disable the OSC interface */
  osc_interface_dis(smph_clk_state);
}
/*---------------------------------------------------------------------------*/
/** @} */

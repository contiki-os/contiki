/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Power control pins built in on the Mulle board.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 *
 */

#include "power-control.h"
#include "power-control-board.h"
#include "port.h"

void
power_control_init()
{
  /* Enable clock gate on the hardware ports controlling the power switches */
  port_module_enable(POWER_CONTROL_VPERIPH_PORT);
  port_module_enable(POWER_CONTROL_VSEC_PORT);
  port_module_enable(POWER_CONTROL_AVDD_PORT);

  /* Power control pins */

  /* PTD7 Vperiph control pin */

  /* Select GPIO function in pin multiplexing */
  POWER_CONTROL_VPERIPH_PORT->PCR[POWER_CONTROL_VPERIPH_PIN_NUMBER] =
    PORT_PCR_MUX(POWER_CONTROL_GPIO_MUX_NUMBER);
  /* Set as output */
  POWER_CONTROL_VPERIPH_GPIO->PDDR |= GPIO_PDDR_PDD(1 << POWER_CONTROL_VPERIPH_PIN_NUMBER);

  /* PTB16 Vsec control pin */

  /* Select GPIO function in pin multiplexing */
  POWER_CONTROL_VSEC_PORT->PCR[POWER_CONTROL_VSEC_PIN_NUMBER] =
    PORT_PCR_MUX(POWER_CONTROL_GPIO_MUX_NUMBER);
  /* Set as output */
  POWER_CONTROL_VSEC_GPIO->PDDR |= GPIO_PDDR_PDD(1 << POWER_CONTROL_VSEC_PIN_NUMBER);

  /* PTB17 AVDD control pin */

  /* Select GPIO function in pin multiplexing */
  POWER_CONTROL_AVDD_PORT->PCR[POWER_CONTROL_AVDD_PIN_NUMBER] =
    PORT_PCR_MUX(POWER_CONTROL_GPIO_MUX_NUMBER);
  /* Set as output */
  POWER_CONTROL_AVDD_GPIO->PDDR |= GPIO_PDDR_PDD(1 << POWER_CONTROL_AVDD_PIN_NUMBER);
}

void
power_control_vperiph_set(bool state)
{
  BITBAND_REG(POWER_CONTROL_VPERIPH_GPIO->PDOR, POWER_CONTROL_VPERIPH_PIN_NUMBER) = (state ? 1 : 0);
}

void
power_control_vsec_set(bool state)
{
  BITBAND_REG(POWER_CONTROL_VSEC_GPIO->PDOR, POWER_CONTROL_VSEC_PIN_NUMBER) = (state ? 1 : 0);
}

void
power_control_avdd_set(bool state)
{
  BITBAND_REG(POWER_CONTROL_AVDD_GPIO->PDOR, POWER_CONTROL_AVDD_PIN_NUMBER) = (state ? 1 : 0);
}

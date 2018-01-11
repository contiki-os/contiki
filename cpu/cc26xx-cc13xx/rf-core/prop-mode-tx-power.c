/*
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup rf-core-prop
 * @{
 *
 * \file
 * Default TX power settings. The board can override
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "dev/radio.h"
#include "rf-core/prop-mode.h"
/*---------------------------------------------------------------------------*/
/* Default TX power settings for the 779-930MHz band */
const prop_mode_tx_power_config_t prop_mode_tx_power_779_930[] = {
  {  14, 0xa73f },
  {  13, 0xa63f }, /* 12.5 */
  {  12, 0xb818 },
  {  11, 0x50da },
  {  10, 0x38d3 },
  {   9, 0x2ccd },
  {   8, 0x24cb },
  {   7, 0x20c9 },
  {   6, 0x1cc7 },
  {   5, 0x18c6 },
  {   4, 0x18c5 },
  {   3, 0x14c4 },
  {   2, 0x1042 },
  {   1, 0x10c3 },
  {   0, 0x0041 },
  { -10, 0x08c0 },
  {-128, 0xFFFF },
};
/*---------------------------------------------------------------------------*/
/* Default TX power settings for the 431-527MHz band */
const prop_mode_tx_power_config_t prop_mode_tx_power_431_527[] = {
  {  15, 0x003f },
  {  14, 0xbe3f }, /* 13.7 */
  {  13, 0x6a0f },
  {  10, 0x3dcb },
  {   6, 0x22c4 },
  {-128, 0xFFFF },
};
/*---------------------------------------------------------------------------*/
/**
 * @}
 */

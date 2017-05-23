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
 * TX power settings for the CC1350 LP
 */
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "dev/radio.h"
#include "rf-core/prop-mode.h"
/*---------------------------------------------------------------------------*/
/* TX power settings for the 779-930MHz band */
const prop_mode_tx_power_config_t tx_power_driver_779_930[] = {
  {  14, 0xab3f },
  {  12, 0xbc2b },
  {  11, 0x90e5 },
  {  10, 0x58d8 },
  {   9, 0x40d2 },
  {   8, 0x32ce },
  {   7, 0x2acb },
  {   6, 0x24c9 },
  {   5, 0x20c8 },
  {   4, 0x1844 },
  {   3, 0x1cc6 },
  {   2, 0x18c5 },
  {   1, 0x16c4 },
  {   0, 0x12c3 },
  { -10, 0x04c0 },
  {-128, 0xFFFF },
};
/*---------------------------------------------------------------------------*/
/**
 * @}
 */

/*
 * Copyright (c) 2013, Kerlink
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \addtogroup efm32
 *
 * @{
 */

/**
 * \file
 *         Power related functions / helpers implementation
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#include "contiki.h"
#include "em_emu.h"
#include "em_rmu.h"

/*---------------------------------------------------------------------------*/
/*
 * Initialize Backup Power domain
 */
void power_init_backuppd(void)
{
  EMU_BUPDInit_TypeDef bupdInit = EMU_BUPDINIT_DEFAULT;

  /* Unlock configuration */
  EMU_EM4Lock(false);

  /* No additional backup power source :
   * connect directly main power to backup power */
  bupdInit.inactivePower = emuPower_MainBU;
  bupdInit.activePower = emuPower_None;

  /* Disable BUPD Vin Pin backup power domain */
  bupdInit.enable = true;

  EMU_BUPDInit(&bupdInit);

  /* Wait until backup power functionality is ready */
  EMU_BUReady();

  // Disable BUPD Vin pin
  EMU_BUPinEnable(false);

  /* Release reset for backup domain */
  RMU_ResetControl(rmuResetBU, false);
}

/*---------------------------------------------------------------------------*/
/*
 * Initialize EM4 config
 */
static void _init_em4(void)
{
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
  /* Enable voltage regulator in backup mode */
  em4Init.vreg = true;

  /* Configure oscillators in EM4 */
  em4Init.osc = emuEM4Osc_LFXO;

  /* Lock configuration in case of brown out */
  em4Init.lockConfig = true;

  EMU_EM4Init(&em4Init);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief      Initialize power module
 *
 */
void
power_init(void)
{
  power_init_backuppd();
  _init_em4();
}

/*---------------------------------------------------------------------------*/

uint32_t enter_standby(void)
{
  //  gpio_suspend();

  EMU_EnterEM2(false);

  return 0;
}
/** @} */

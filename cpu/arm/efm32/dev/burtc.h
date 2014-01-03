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
 * \addtogroup efm32-devices
 * @{
 */

/**
 * \file
 *         EFM32 Backup RTC driver Header file
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#ifndef __BURTC_H__
#define __BURTC_H__

typedef struct {
  volatile uint32_t u32_prod_timestamp;
  volatile int32_t  s32_overflow_uptime;
  volatile void*    p_alarm_callback;
  volatile int32_t  s32_pcount;

  //uint32_t reserved[124];

  // Max 128 entries
} burtc_retreg_struct;

#define BURTC_RETREG ((burtc_retreg_struct *)&BURTC->RET[0].REG)
//volatile burtc_retreg_struct *burtc_retreg = (burtc_retreg_struct *)&BURTC->RET[0].REG;

typedef void (*alarm_callback_t)(void);

void burtc_crystal_init(uint32_t resetcause);

// Time manipulation
uint32_t burtc_getvalue(void);
uint32_t burtc_gettime(void);
void burtc_set_offset(int32_t timeoffset);
void burtc_set_driftcorrection(int8_t sec_offset, uint8_t nbdays);
void burtc_set_useralarm(uint32_t sec_offset, alarm_callback_t alarm_callback);


/** @} */
#endif /* __BURTC_H__ */

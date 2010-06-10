/*
 * Copyright (c) 2010, Mariano Alvira <mar@devl.org> and other contributors
 * to the MC1322x project (http://mc1322x.devl.org)
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
 * This file is part of libmc1322x: see http://mc1322x.devl.org
 * for details. 
 *
 * $Id: crm.h,v 1.1 2010/06/10 14:55:39 maralvira Exp $
 */

#ifndef CRM_H
#define CRM_H

#include <types.h>

#define CRM_BASE         (0x80003000)
#define CRM_SYS_CNTL     ((volatile uint32_t *) (CRM_BASE+0x00))
#define CRM_WU_CNTL      ((volatile uint32_t *) (CRM_BASE+0x04))
#define CRM_SLEEP_CNTL   ((volatile uint32_t *) (CRM_BASE+0x08))
#define CRM_BS_CNTL      ((volatile uint32_t *) (CRM_BASE+0x0c))
#define CRM_COP_CNTL     ((volatile uint32_t *) (CRM_BASE+0x10))
#define CRM_COP_SERVICE  ((volatile uint32_t *) (CRM_BASE+0x14))
#define CRM_STATUS       ((volatile uint32_t *) (CRM_BASE+0x18))
#define CRM_MOD_STATUS   ((volatile uint32_t *) (CRM_BASE+0x1c))
#define CRM_WU_COUNT     ((volatile uint32_t *) (CRM_BASE+0x20))
#define CRM_WU_TIMEOUT   ((volatile uint32_t *) (CRM_BASE+0x24))
#define CRM_RTC_COUNT    ((volatile uint32_t *) (CRM_BASE+0x28))
#define CRM_RTC_TIMEOUT  ((volatile uint32_t *) (CRM_BASE+0x2c))
#define CRM_CAL_CNTL     ((volatile uint32_t *) (CRM_BASE+0x34))
#define CRM_CAL_COUNT    ((volatile uint32_t *) (CRM_BASE+0x38))
#define CRM_RINGOSC_CNTL ((volatile uint32_t *) (CRM_BASE+0x3c))
#define CRM_XTAL_CNTL    ((volatile uint32_t *) (CRM_BASE+0x40))
#define CRM_XTAL32_CNTL  ((volatile uint32_t *) (CRM_BASE+0x44))
#define CRM_VREG_CNTL    ((volatile uint32_t *) (CRM_BASE+0x48))
#define CRM_SW_RST       ((volatile uint32_t *) (CRM_BASE+0x50))

/* CRM_SYS_CNTL bit locations */
#define XTAL32_EXISTS 5 

/* CRM_WU_CNTL bit locations */
#define EXT_WU_IEN   20      /* 4 bits */ 
#define EXT_WU_EN    4       /* 4 bits */ 
#define EXT_WU_EDGE  8       /* 4 bits */ 
#define EXT_WU_POL   12      /* 4 bits */ 
#define TIMER_WU_EN  0 
#define RTC_WU_EN    1 
#define TIMER_WU_IEN  16 
#define RTC_WU_IEN    17

/* CRM_STATUS bit locations */
#define EXT_WU_EVT 4       /* 4 bits, rw1c */
#define RTC_WU_EVT 3       /* rw1c */

/* RINGOSC_CNTL bit locations */
#define ROSC_CTUNE 9       /* 4 bits */
#define ROSC_FTUNE 4       /* 4 bits */
#define ROSC_EN    0

#define ring_osc_on() (set_bit(*CRM_RINGOSC_CNTL,ROSC_EN))
#define ring_osc_off() (clear_bit(*CRM_RINGOSC_CNTL,ROSC_EN))

#define REF_OSC 24000000ULL          /* reference osc. frequency */
#define NOMINAL_RING_OSC_SEC 2000 /* nominal ring osc. frequency */
extern uint32_t cal_rtc_secs;      /* calibrated 2khz rtc seconds */

/* XTAL32_CNTL bit locations */
#define XTAL32_GAIN 4      /* 2 bits */
#define XTAL32_EN   0

#define xtal32_on() (set_bit(*CRM_XTAL32_CNTL,XTAL32_EN))
#define xtal32_off() (clear_bit(*CRM_XTAL32_CNTL,XTAL32_EN))
#define xtal32_exists() (set_bit(*CRM_SYS_CNTL,XTAL32_EXISTS))

/* enable external wake-ups on kbi 4-7 */ 
/* see kbi.h for other kbi specific macros */
#define enable_ext_wu(kbi) (set_bit(*CRM_WU_CNTL,(EXT_WU_EN+kbi-4)))
#define disable_ext_wu(kbi) (clear_bit(*CRM_WU_CNTL,(EXT_WU_EN+kbi-4)))

#define is_ext_wu_evt(kbi) (bit_is_set(*CRM_STATUS,(EXT_WU_EVT+kbi-4)))
#define clear_ext_wu_evt(kbi) (set_bit(*CRM_STATUS,(EXT_WU_EVT+kbi-4))) /* r1wc bit */

/* enable wake-up timer */
#define enable_timer_wu_irq() ((set_bit(*CRM_WU_CNTL,(TIMER_WU_IEN))))
#define disable_timer_wu_irq() ((clear_bit(*CRM_WU_CNTL,(TIMER_WU_IEN))))

#define enable_timer_wu() ((set_bit(*CRM_WU_CNTL,(TIMER_WU_EN))))
#define disable_timer_wu() ((clear_bit(*CRM_WU_CNTL,(TIMER_WU_EN))))

/* enable wake-up from RTC compare */
#define enable_rtc_wu_irq() (set_bit(*CRM_WU_CNTL,RTC_WU_IEN))
#define disable_rtc_wu_irq() (clear_bit(*CRM_WU_CNTL,RTC_WU_IEN))

#define enable_rtc_wu() ((set_bit(*CRM_WU_CNTL,(RTC_WU_EN))))
#define disable_rtc_wu() ((clear_bit(*CRM_WU_CNTL,(RTC_WU_EN))))

#define clear_rtc_wu_evt() (set_bit(*CRM_STATUS,RTC_WU_EVT))
#define rtc_wu_evt() (bit_is_set(*CRM_STATUS,RTC_WU_EVT))

#define SLEEP_MODE_HIBERNATE bit(0)
#define SLEEP_MODE_DOZE      bit(1)

#define SLEEP_PAD_PWR     bit(7)
#define SLEEP_RETAIN_MCU bit(6)
#define sleep_ram_retain(x) (x<<4)   /* 0-3 */
#define SLEEP_RAM_8K sleep_ram_retain(0)
#define SLEEP_RAM_32K sleep_ram_retain(1)
#define SLEEP_RAM_64K sleep_ram_retain(2)
#define SLEEP_RAM_96K sleep_ram_retain(3)

#define pack_XTAL_CNTL(ctune4pf, ctune, ftune, ibias) \
	(*CRM_XTAL_CNTL = ((ctune4pf << 25) | (ctune << 21) | ( ftune << 16) | (ibias << 8) | 0x52))

#endif

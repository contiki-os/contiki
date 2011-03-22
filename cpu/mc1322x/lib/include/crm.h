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
 *
 */

#ifndef CRM_H
#define CRM_H

#include <stdint.h>

#define CRM_BASE         (0x80003000)

/* Structure-based CRM access */
struct CRM_struct {
	union {
		uint32_t SYS_CNTL;
		struct CRM_SYS_CNTL {
			uint32_t PWR_SOURCE:2;
			uint32_t PADS_1P8V_SEL:1;
			uint32_t :1;
			uint32_t JTAG_SECU_OFF:1;
			uint32_t XTAL32_EXISTS:1;
			uint32_t :2;
			uint32_t XTAL_CLKDIV:6;
			uint32_t :18;
		} SYS_CNTLbits;
	};
	union {
		uint32_t WU_CNTL;
		struct CRM_WU_CNTL {
			uint32_t TIMER_WU_EN:1;
			uint32_t RTC_WU_EN:1;
			uint32_t HOST_WAKE:1;
			uint32_t AUTO_ADC:1;
			uint32_t EXT_WU_EN:4;
			uint32_t EXT_WU_EDGE:4;
			uint32_t EXT_WU_POL:4;
			uint32_t TIMER_WU_IEN:1;
			uint32_t RTC_WU_IEN:1;
			uint32_t :2;
			uint32_t EXT_WU_IEN:4;
			uint32_t :4;
			uint32_t EXT_OUT_POL:4;
		} WU_CNTLbits;
	};
	union {
		uint32_t SLEEP_CNTL;
		struct CRM_SLEEP_CNTL {
			uint32_t HIB:1;
			uint32_t DOZE:1;
			uint32_t :2;
			uint32_t RAM_RET:2;
			uint32_t MCU_RET:1;
			uint32_t DIG_PAD_EN:1;
			uint32_t :24;
		} SLEEP_CNTLbits;
	};
	union {
		uint32_t BS_CNTL;
		struct CRM_BS_CNTL {
			uint32_t BS_EN:1;
			uint32_t WAIT4IRQ:1;
			uint32_t BS_MAN_EN:1;
			uint32_t :2;
			uint32_t ARM_OFF_TIME:6;
			uint32_t :18;
		} BS_CNTLbits;
	};
	union {
		uint32_t COP_CNTL;
		struct CRM_COP_CNTL {
			uint32_t COP_EN:1;
			uint32_t COP_OUT:1;
			uint32_t COP_WP:1;
			uint32_t :5;
			uint32_t COP_TIMEOUT:7;
			uint32_t :1;
			uint32_t COP_COUNT:7;
			uint32_t :9;
		} COP_CNTLbits;
	};
	uint32_t COP_SERVICE;
	union {
		uint32_t STATUS;
		struct CRM_STATUS {
			uint32_t SLEEP_SYNC:1;
			uint32_t HIB_WU_EVT:1;
			uint32_t DOZE_WU_EVT:1;
			uint32_t RTC_WU_EVT:1;
			uint32_t EXT_WU_EVT:4;
			uint32_t :1;
			uint32_t CAL_DONE:1;
			uint32_t COP_EVT:1;
			uint32_t :6;
			uint32_t VREG_BUCK_RDY:1;
			uint32_t VREG_1P8V_RDY:1;
			uint32_t VREG_1P5V_RDY:1;
			uint32_t :12;
		} STATUSbits;
	};
	union {
		uint32_t MOD_STATUS;
		struct CRM_MOD_STATUS {
			uint32_t ARM_EN:1;
			uint32_t MACA_EN:1;
			uint32_t ASM_EN:1;
			uint32_t SPI_EN:1;
			uint32_t GPIO_EN:1;
			uint32_t UART1_EN:1;
			uint32_t UART2_EN:1;
			uint32_t TMR_EN:1;
			uint32_t RIF_EN:1;
			uint32_t I2C_EN:1;
			uint32_t SSI_EN:1;
			uint32_t SPIF_EN:1;
			uint32_t ADC_EN:1;
			uint32_t :1;
			uint32_t JTA_EN:1;
			uint32_t NEX_EN:1;
			uint32_t :1;
			uint32_t AIM_EN:1;
			uint32_t :14;
		} MOD_STATUSbits;
	};
	uint32_t WU_COUNT;
	uint32_t WU_TIMEOUT;
	uint32_t RTC_COUNT;
	uint32_t RTC_TIMEOUT;
	uint32_t reserved1;
	union {
		uint32_t CAL_CNTL;
		struct CRM_CAL_CNTL {
			uint32_t CAL_TIMEOUT:16;
			uint32_t CAL_EN:1;
			uint32_t CAL_IEN:1;
			uint32_t :14;
		} CAL_CNTLbits;
	};
	uint32_t CAL_COUNT;
	union {
		uint32_t RINGOSC_CNTL;
		struct CRM_RINGOSC_CNTL {
			uint32_t ROSC_EN:1;
			uint32_t :3;
			uint32_t ROSC_FTUNE:5;
			uint32_t ROSC_CTUNE:4;
			uint32_t :19;
		} RINGOSC_CNTLbits;
	};
	union {
		uint32_t XTAL_CNTL;
		struct CRM_XTAL_CNTL {
			uint32_t :8;
			uint32_t XTAL_IBIAS_SEL:4;
			uint32_t :4;
			uint32_t XTAL_FTUNE:5;
			uint32_t XTAL_CTUNE:5;
			uint32_t :6;
		} XTAL_CNTLbits;
	};
	union {
		uint32_t XTAL32_CNTL;
		struct CRM_XTAL32_CNTL {
			uint32_t XTAL32_EN:1;
			uint32_t :3;
			uint32_t XTAL32_GAIN:2;
			uint32_t :26;
		} XTAL32_CNTLbits;
	};
	union {
		uint32_t VREG_CNTL;
		struct CRM_VREG_CNTL {
			uint32_t BUCK_EN:1;
			uint32_t BUCK_SYNC_REC_EN:1;
			uint32_t BUCK_BYPASS_EN:1;
			uint32_t VREG_1P5V_EN:2;
			uint32_t VREG_1P5V_SEL:2;
			uint32_t VREG_1P8V_EN:1;
			uint32_t BUCK_CLKDIV:4;
			uint32_t :20;
		} VREG_CNTLbits;
	};
	uint32_t reserved2;
	uint32_t SW_RST;
	uint32_t reserved3;
	uint32_t reserved4;
	uint32_t reserved5;
	uint32_t reserved6;
};

static volatile struct CRM_struct * const CRM = (void *) (CRM_BASE);

/* COP watchdog timer helpers */

/* set the cop timout in milliseconds */
#define cop_timeout_ms(x) (CRM->COP_CNTLbits.COP_TIMEOUT = x/87) 
#define cop_service() (CRM->COP_SERVICE = 0xc0de5afe)

/* Old register definitions, for compatibility */
#ifndef REG_NO_COMPAT

static volatile uint32_t * const CRM_SYS_CNTL   = ((volatile uint32_t *) (CRM_BASE+0x00));
static volatile uint32_t * const CRM_WU_CNTL    = ((volatile uint32_t *) (CRM_BASE+0x04));
static volatile uint32_t * const CRM_SLEEP_CNTL = ((volatile uint32_t *) (CRM_BASE+0x08));
static volatile uint32_t * const CRM_BS_CNTL    = ((volatile uint32_t *) (CRM_BASE+0x0c));
static volatile uint32_t * const CRM_COP_CNTL   = ((volatile uint32_t *) (CRM_BASE+0x10));
static volatile uint32_t * const CRM_COP_SERVICE= ((volatile uint32_t *) (CRM_BASE+0x14));
static volatile uint32_t * const CRM_STATUS     = ((volatile uint32_t *) (CRM_BASE+0x18));
static volatile uint32_t * const CRM_MOD_STATUS = ((volatile uint32_t *) (CRM_BASE+0x1c));
static volatile uint32_t * const CRM_WU_COUNT   = ((volatile uint32_t *) (CRM_BASE+0x20));
static volatile uint32_t * const CRM_WU_TIMEOUT = ((volatile uint32_t *) (CRM_BASE+0x24));
static volatile uint32_t * const CRM_RTC_COUNT  = ((volatile uint32_t *) (CRM_BASE+0x28));
static volatile uint32_t * const CRM_RTC_TIMEOUT= ((volatile uint32_t *) (CRM_BASE+0x2c));
static volatile uint32_t * const CRM_CAL_CNTL   = ((volatile uint32_t *) (CRM_BASE+0x34));
static volatile uint32_t * const CRM_CAL_COUNT  = ((volatile uint32_t *) (CRM_BASE+0x38));
static volatile uint32_t * const CRM_RINGOSC_CNT= ((volatile uint32_t *) (CRM_BASE+0x3c));
static volatile uint32_t * const CRM_XTAL_CNTL  = ((volatile uint32_t *) (CRM_BASE+0x40));
static volatile uint32_t * const CRM_XTAL32_CNTL= ((volatile uint32_t *) (CRM_BASE+0x44));
static volatile uint32_t * const CRM_VREG_CNTL  = ((volatile uint32_t *) (CRM_BASE+0x48));
static volatile uint32_t * const CRM_SW_RST     = ((volatile uint32_t *) (CRM_BASE+0x50));

/* CRM_SYS_CNTL bit locations */
static const int XTAL32_EXISTS = 5;

/* CRM_WU_CNTL bit locations */
static const int EXT_WU_IEN =   20;      /* 4 bits */
static const int EXT_WU_EN =    4;       /* 4 bits */
static const int EXT_WU_EDGE =  8;       /* 4 bits */
static const int EXT_WU_POL =   12;      /* 4 bits */
static const int TIMER_WU_EN =  0;
static const int RTC_WU_EN =    1;
static const int TIMER_WU_IEN =  16;
static const int RTC_WU_IEN =    17;

/* CRM_STATUS bit locations */
static const int EXT_WU_EVT = 4;       /* 4 bits, rw1c */
static const int RTC_WU_EVT = 3;      /* rw1c */

/* RINGOSC_CNTL bit locations */
static const int ROSC_CTUNE = 9;       /* 4 bits */
static const int ROSC_FTUNE = 4;       /* 4 bits */
static const int ROSC_EN =    0;

#define ring_osc_on() (CRM->RINGOSC_CNTLbits.ROSC_EN = 1)
#define ring_osc_off() (CRM->RINGOSC_CNTLbits.ROSC_EN = 0)

#define REF_OSC 24000000UL          /* reference osc. frequency */
#define NOMINAL_RING_OSC_SEC 2000 /* nominal ring osc. frequency */
extern uint32_t cal_rtc_secs;      /* calibrated 2khz rtc seconds */

/* XTAL32_CNTL bit locations */
static const int XTAL32_GAIN = 4;      /* 2 bits */
static const int XTAL32_EN =   0;

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

#endif /* REG_NO_COMPAT */

#endif

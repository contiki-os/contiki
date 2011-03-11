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

#ifndef _MACA_H_
#define _MACA_H_

#include <packet.h>
#include <stdint.h>
#include <utils.h>

/* maca initialization and on off routines */
void maca_init(void);
void maca_off(void);
void maca_on(void);

/* run periodically to make sure the maca is still doing right */
void check_maca(void);

/* maca configuration interface */
void set_power(uint8_t power);
void set_channel(uint8_t chan);

extern uint8_t (*get_lqi)(void);

#define DEMOD_DCD 1 /* -96dBm, 22.2mA */
#define DEMOD_NCD 0 /* -100dBm, 24.2mA */
void set_demodulator_type(uint8_t demod);

/* set_fcs_mode(NO_FCS) to disable checksum filtering */
extern volatile uint8_t fcs_mode;
#define set_fcs_mode(x) fcs_mode = (x)

/* set_prm_mode(PROMISC) to disable address filtering */
/* set_prm_mode(AUTOACK) to enable address filtering AND autoack */
extern volatile uint8_t prm_mode;
#define set_prm_mode(x) prm_mode = (x)

/* maca packet interface */
void tx_packet(volatile packet_t *p);
volatile packet_t* rx_packet(void);
volatile packet_t* get_free_packet(void);
void free_packet(volatile packet_t *p);
void free_all_packets(void);

extern volatile packet_t *rx_head, *tx_head;
extern volatile uint32_t maca_entry;

extern void maca_rx_callback(volatile packet_t *p) __attribute__((weak));
extern void maca_tx_callback(volatile packet_t *p) __attribute__((weak));

/* maca lowlevel routines */
/* most applications won't need to use them */
void reset_maca(void);
void init_phy(void);
void flyback_init(void);
void ResumeMACASync(void);
void radio_init(void);
uint32_t init_from_flash(uint32_t addr);

#define MAX_PACKET_SIZE (MAX_PAYLOAD_SIZE + 2) /* packet includes 2 bytes of checksum */

/* maca register and field defines */

#define MACA_BASE       (0x80004000)
#define MACA_RESET      ((volatile uint32_t *) (MACA_BASE+0x04))
#define MACA_RANDOM     ((volatile uint32_t *) (MACA_BASE+0x08))
#define MACA_CONTROL    ((volatile uint32_t *) (MACA_BASE+0x0c))

/* MACA_CONTROL bits and fields */
#define ISM             20
#define PRECOUNT        16                   /* preamble reapeat counter       */
#define PRECOUNT_MASK   bit_mask(4,PRECOUNT) 
#define RTSO            15                   /* reset slot counter             */
#define ROLE            13                   /* set if PAN coordinator         */
#define NOFC            12                   /* set to disable FCS             */
enum {
	USE_FCS = 0,
	NO_FCS  = 1,
};
#define PRM             11                   /* set for promiscuous mode       */          
enum {
	AUTOACK  = 0,
	PROMISC = 1,
};
#define REL             10                   /* 1 for relative, 0 for absolute */
#define ASAP            9                    /* 1 start now, 0 timer start     */
#define BCN             8                    /* 1 beacon only, 0 for a         */
#define AUTO            7                    /* 1 continuous rx, rx only once  */
#define LFSR            6                    /* 1 use polynomial for Turbolink */
#define TM              5

#define MACA_MODE       3
#define MODE_MASK       bit_mask(2,MACA_MODE)
#define NO_CCA          0
#define NO_SLOT_CCA     1
#define SLOT_CCA        2

#define SEQUENCE        0
#define SEQUENCE_MASK   bit_mask(3,SEQUENCE)
/* end of MACA_CONTROL bits and fields */

#define MACA_STATUS     ((volatile uint32_t *) (MACA_BASE+0x10))
/* MACA_STATUS bits and fields */
#define STATUS_TIMEOUT   15
#define CRC       14
#define BUSY      13
#define OVR       12
#define CODE       0 
#define CODE_MASK bit_mask(4,CODE)
/* status codes */
#define SUCCESS          0 
#define CODE_TIMEOUT     1
#define CHANNEL_BUSY     2
#define CRC_FAILED       3
#define ABORTED          4
#define NO_ACK           5
#define NO_DATA          6
#define LATE_START       7
#define EXT_TIMEOUT      8
#define EXT_PND_TIMEOUT  9
#define PLL_UNLOCK      12
#define EXTERNAL_ABORT  13
#define NOT_COMPLETED   14
#define DMA_BUS_ERROR   15
/* end of MACA_CONTROL bits and fields */

#define MACA_FRMPND     ((volatile uint32_t *) (MACA_BASE+0x14))
#define MACA_TMREN      ((volatile uint32_t *) (MACA_BASE+0x40))
#define MACA_TMRDIS     ((volatile uint32_t *) (MACA_BASE+0x44))
#define MACA_CLK        ((volatile uint32_t *) (MACA_BASE+0x48))
#define MACA_STARTCLK   ((volatile uint32_t *) (MACA_BASE+0x4c))
#define MACA_CPLCLK     ((volatile uint32_t *) (MACA_BASE+0x50))
#define MACA_SFTCLK     ((volatile uint32_t *) (MACA_BASE+0x54))
#define MACA_CLKOFFSET  ((volatile uint32_t *) (MACA_BASE+0x58))
#define MACA_RELCLK     ((volatile uint32_t *) (MACA_BASE+0x5c))
#define MACA_CPLTIM     ((volatile uint32_t *) (MACA_BASE+0x60))
#define MACA_SLOTOFFSET ((volatile uint32_t *) (MACA_BASE+0x64))
#define MACA_TIMESTAMP  ((volatile uint32_t *) (MACA_BASE+0x68))
#define MACA_DMARX      ((volatile uint32_t *) (MACA_BASE+0x80))
#define MACA_DMATX      ((volatile uint32_t *) (MACA_BASE+0x84))
#define MACA_DMAPOLL    ((volatile uint32_t *) (MACA_BASE+0x88))
#define MACA_TXLEN      ((volatile uint32_t *) (MACA_BASE+0x8c))
#define MACA_TXSEQNR    ((volatile uint32_t *) (MACA_BASE+0x90))
#define MACA_SETRXLVL   ((volatile uint32_t *) (MACA_BASE+0x94))
#define MACA_GETRXLVL   ((volatile uint32_t *) (MACA_BASE+0x98))
#define MACA_IRQ        ((volatile uint32_t *) (MACA_BASE+0xc0))
#define MACA_CLRIRQ     ((volatile uint32_t *) (MACA_BASE+0xc4))
#define MACA_SETIRQ     ((volatile uint32_t *) (MACA_BASE+0xc8))
#define MACA_MASKIRQ    ((volatile uint32_t *) (MACA_BASE+0xcc))
#define MACA_MACPANID   ((volatile uint32_t *) (MACA_BASE+0x100))
#define MACA_MAC16ADDR  ((volatile uint32_t *) (MACA_BASE+0x104))
#define MACA_MAC64HI    ((volatile uint32_t *) (MACA_BASE+0x108))
#define MACA_MAC64LO    ((volatile uint32_t *) (MACA_BASE+0x10c))
#define MACA_FLTREJ     ((volatile uint32_t *) (MACA_BASE+0x110))
#define MACA_CLKDIV     ((volatile uint32_t *) (MACA_BASE+0x114))
#define MACA_WARMUP     ((volatile uint32_t *) (MACA_BASE+0x118))
#define MACA_PREAMBLE   ((volatile uint32_t *) (MACA_BASE+0x11c))
#define MACA_WHITESEED  ((volatile uint32_t *) (MACA_BASE+0x120))
#define MACA_FRAMESYNC0 ((volatile uint32_t *) (MACA_BASE+0x124))
#define MACA_FRAMESYNC1 ((volatile uint32_t *) (MACA_BASE+0x128))
#define MACA_TXACKDELAY ((volatile uint32_t *) (MACA_BASE+0x140))
#define MACA_RXACKDELAY ((volatile uint32_t *) (MACA_BASE+0x144))
#define MACA_EOFDELAY   ((volatile uint32_t *) (MACA_BASE+0x148))
#define MACA_CCADELAY   ((volatile uint32_t *) (MACA_BASE+0x14c))
#define MACA_RXEND      ((volatile uint32_t *) (MACA_BASE+0x150))
#define MACA_TXCCADELAY ((volatile uint32_t *) (MACA_BASE+0x154))
#define MACA_KEY3       ((volatile uint32_t *) (MACA_BASE+0x158))
#define MACA_KEY2       ((volatile uint32_t *) (MACA_BASE+0x15c))
#define MACA_KEY1       ((volatile uint32_t *) (MACA_BASE+0x160))
#define MACA_KEY0       ((volatile uint32_t *) (MACA_BASE+0x164))
#define MACA_OPTIONS    ((volatile uint32_t *) (MACA_BASE+0x180))


/******************************************************************************/
/* everything under this comment is messy, needs cleaning, and will           */
/* probably change in the future                                              */
/******************************************************************************/

#define control_pre_count (7<<16)   /* preamble reapeat counter       */
#define control_rst_slot  (1<<15)   /* reset slot counter             */
#define control_role      (1<<13)   /* set if PAN coordinator         */
#define control_nofc      (1<<12)   /* set to disable FCS             */
#define control_prm       (1<<11)   /* set for promiscuous mode       */
#define control_relative  (1<<10)   /* 1 for relative, 0 for absolute */
#define control_asap      (1<<9)    /* 1 start now, 0 timer start     */
#define control_bcn       (1<<8)    /* 1 beacon only, 0 for a         */
#define control_auto      (1<<7)    /* 1 continuous rx, rx only once  */
#define control_lfsr      (1<<6)    /* 1 use polynomial for Turbolink */

#define gMACA_Clock_DIV_c      95
  
//rom_base_adr      equ 0x00000000   ; rom  base address
//ram_base_adr      equ 0x00400000   ; ram  base address
//ram0_base_adr     equ 0x00400000   ; ram0 base address (2K words =  8K
//bytes)
//ram1_base_adr     equ 0x00402000   ; ram1 base address (6K words = 24K
//bytes)
//ram2_base_adr     equ 0x00408000   ; ram2 base address (8K words = 32K
//bytes)
//ram3_base_adr     equ 0x00410000   ; ram3 base address (8K words
enum {
   cc_success          = 0,
   cc_timeout          = 1,
   cc_channel_busy     = 2,
   cc_crc_fail         = 3,
   cc_aborted          = 4,
   cc_no_ack           = 5,
   cc_no_data          = 6,
   cc_late_start       = 7,
   cc_ext_timeout      = 8,
   cc_ext_pnd_timeout  = 9,
   cc_nc1              = 10,
   cc_nc2              = 11,
   cc_nc3              = 12,
   cc_cc_external_abort= 13,
   cc_not_completed    = 14,
   cc_bus_error        = 15
};
//control codes for mode bits

enum {
   control_mode_no_cca      = 0,
   control_mode_non_slotted = (1<<3),
   control_mode_slotted     = (1<<4)
};
//control codes for sequence bits
enum {
    control_seq_nop    = 0,
    control_seq_abort  = 1,
    control_seq_wait   = 2,
    control_seq_tx     = 3,
    control_seq_rx     = 4,
    control_seq_txpoll = 5,
    control_seq_cca    = 6,
    control_seq_ed     = 7
};  
  
#define maca_version    (*((volatile uint32_t *)(0x80004000)))      
#define maca_reset      (*((volatile uint32_t *)(0x80004004)))    
#define maca_random     (*((volatile uint32_t *)(0x80004008)))    
#define maca_control    (*((volatile uint32_t *)(0x8000400c)))    
#define maca_status     (*((volatile uint32_t *)(0x80004010)))    
#define maca_frmpnd     (*((volatile uint32_t *)(0x80004014)))    

#define maca_edvalue    (*((volatile uint32_t *)(0x8000401c)))    
#define maca_tmren      (*((volatile uint32_t *)(0x80004040)))    
#define maca_tmrdis     (*((volatile uint32_t *)(0x80004044)))    
#define maca_clk        (*((volatile uint32_t *)(0x80004048)))    
#define maca_startclk   (*((volatile uint32_t *)(0x8000404c)))    
#define maca_cplclk     (*((volatile uint32_t *)(0x80004050)))    
#define maca_sftclk     (*((volatile uint32_t *)(0x80004054)))    
#define maca_clkoffset  (*((volatile uint32_t *)(0x80004058))) 
#define maca_relclk     (*((volatile uint32_t *)(0x8000405c))) 
#define maca_cpltim     (*((volatile uint32_t *)(0x80004060))) 
#define maca_slotoffset (*((volatile uint32_t *)(0x80004064))) 
#define maca_timestamp  (*((volatile uint32_t *)(0x80004068))) 
#define maca_dmarx      (*((volatile uint32_t *)(0x80004080))) 
#define maca_dmatx      (*((volatile uint32_t *)(0x80004084))) 
#define maca_dmatxpoll  (*((volatile uint32_t *)(0x80004088)))
#define maca_txlen      (*((volatile uint32_t *)(0x8000408c))) 
#define maca_txseqnr    (*((volatile uint32_t *)(0x80004090))) 
#define maca_setrxlvl   (*((volatile uint32_t *)(0x80004094))) 
#define maca_getrxlvl   (*((volatile uint32_t *)(0x80004098))) 
#define maca_irq        (*((volatile uint32_t *)(0x800040c0))) 
#define maca_clrirq     (*((volatile uint32_t *)(0x800040c4))) 
#define maca_setirq     (*((volatile uint32_t *)(0x800040c8))) 
#define maca_maskirq    (*((volatile uint32_t *)(0x800040cc))) 
#define maca_panid      (*((volatile uint32_t *)(0x80004100))) 
#define maca_addr16     (*((volatile uint32_t *)(0x80004104))) 
#define maca_maca64hi   (*((volatile uint32_t *)(0x80004108))) 
#define maca_maca64lo   (*((volatile uint32_t *)(0x8000410c))) 
#define maca_fltrej     (*((volatile uint32_t *)(0x80004110))) 
#define maca_divider    (*((volatile uint32_t *)(0x80004114))) 
#define maca_warmup     (*((volatile uint32_t *)(0x80004118))) 
#define maca_preamble   (*((volatile uint32_t *)(0x8000411c))) 
#define maca_whiteseed  (*((volatile uint32_t *)(0x80004120))) 
#define maca_framesync  (*((volatile uint32_t *)(0x80004124))) 
#define maca_framesync2 (*((volatile uint32_t *)(0x80004128))) 
#define maca_txackdelay (*((volatile uint32_t *)(0x80004140))) 
#define maca_rxackdelay (*((volatile uint32_t *)(0x80004144))) 
#define maca_eofdelay   (*((volatile uint32_t *)(0x80004148))) 
#define maca_ccadelay   (*((volatile uint32_t *)(0x8000414c))) 
#define maca_rxend      (*((volatile uint32_t *)(0x80004150))) 
#define maca_txccadelay (*((volatile uint32_t *)(0x80004154))) 
#define maca_key3       (*((volatile uint32_t *)(0x80004158))) 
#define maca_key2       (*((volatile uint32_t *)(0x80004158))) 
#define maca_key1       (*((volatile uint32_t *)(0x80004158))) 
#define maca_key0       (*((volatile uint32_t *)(0x80004158))) 


typedef union maca_version_reg_tag
{
  struct
  {
    uint32_t  MINOR:8;
    uint32_t  RESERVED1:8;
    uint32_t  MAJOR:8;
    uint32_t  RESERVED2:8;
  } Bits;
  uint32_t Reg;
} maca_version_reg_t;

#define maca_version_reg_st ((maca_version_reg_t)(maca_version))


typedef union maca_reset_reg_tag
{
  struct
  {
    uint32_t  RESERVED:30;
    uint32_t  CLK_ON:1;
    uint32_t  RST:1;
  } Bits;
  uint32_t Reg;
} maca_reset_reg_t;

#define maca_reset_reg_st ((maca_reset_reg_t)(maca_reset))


/* typedef union maca_ctrl_reg_tag */
/* { */
/*   struct */
/*   { */
/*     uint32_t  RESERVED:11; */
/*     uint32_t  ISM:1; */
/*     uint32_t  PRE_COUNT:4; */
/*     uint32_t  RSTO:1; */
/*     uint32_t  RSV:1; */
/*     uint32_t  ROLE:1; */
/*     uint32_t  NOFC:1; */
/*     uint32_t  PRM:1; */
/*     uint32_t  rel:1; */
/*     uint32_t  ASAP:1; */
/*     uint32_t  BCN:1; */
/*     uint32_t  AUTO:1; */
/*     uint32_t  LFSR:1; */
/*     uint32_t  TM:1; */
/*     uint32_t  MODE:2; */
/*     uint32_t  SEQUENCE:3; */
/*   } Bits; */
/*   uint32_t Reg; */
/* } maca_ctrl_reg_t; */

#define maca_control_ism     (1<<20)
#define maca_control_zigbee  (~maca_control_ism)

#define maca_ctrl_reg_st ((maca_ctrl_reg_t *)(&maca_reset))
#define _set_maca_test_mode(x) (maca_ctrl_reg_st->Bits.TM = x)
#define _set_maca_sequence(x)  (maca_ctrl_reg_st->Bits.SEQUENCE = x)
#define _set_maca_asap(x)      (maca_ctrl_reg_st->Bits.ASAP = x)


#define MACA_CTRL_ZIGBEE_MODE          (0)
#define MACA_CTRL_ISM_MODE             (1)
#define MACA_CTRL_PRM_NORMAL_MODE      (0)
#define MACA_CTRL_PRM_PROMISCUOUS_MODE (1)
#define MACA_CTRL_BCN_ALL              (0)
#define MACA_CTRL_BCN_BEACON           (1)
#define MACA_CTRL_TM_NORMAL            (0)
#define MACA_CTRL_TM_TEST              (1)
#define MACA_CTRL_MODE_NO_CCA          (0)
#define MACA_CTRL_MODE_NON_SLOTTED     (1)
#define MACA_CTRL_MODE_SLOTTED         (2)

typedef enum maca_freq_chann_tag
{
  SMAC_CHANN_11 = 0,
  SMAC_CHANN_12,
  SMAC_CHANN_13,
  SMAC_CHANN_14,
  SMAC_CHANN_15,
  SMAC_CHANN_16,
  SMAC_CHANN_17,
  SMAC_CHANN_18,
  SMAC_CHANN_19,
  SMAC_CHANN_20,
  SMAC_CHANN_21,
  SMAC_CHANN_22,
  SMAC_CHANN_23,
  SMAC_CHANN_24,
  SMAC_CHANN_25,
  SMAC_CHANN_26,
  MAX_SMAC_CHANNELS
} maca_freq_chann_t;


/* Sequence complete codes */
enum maca_complete_code {
   maca_cc_success          = 0,
   maca_cc_timeout          = 1,
   maca_cc_channel_busy     = 2,
   maca_cc_crc_fail         = 3,
   maca_cc_aborted          = 4,
   maca_cc_no_ack           = 5,
   maca_cc_no_data          = 6,
   maca_cc_late_start       = 7,
   maca_cc_ext_timeout      = 8,
   maca_cc_ext_pnd_timeout  = 9,
   maca_cc_nc1              = 10,
   maca_cc_nc2              = 11,
   maca_cc_nc3              = 12,
   maca_cc_cc_external_abort= 13,
   maca_cc_not_completed    = 14,
   maca_cc_bus_error        = 15
};

/* control sequence codes */
enum maca_ctrl_seq {
    maca_ctrl_seq_nop    = 0,
    maca_ctrl_seq_abort  = 1,
    maca_ctrl_seq_wait   = 2,
    maca_ctrl_seq_tx     = 3,
    maca_ctrl_seq_rx     = 4,
    maca_ctrl_seq_txpoll = 5,
    maca_ctrl_seq_cca    = 6,
    maca_ctrl_seq_ed     = 7
};  

/* transmission modes */
enum maca_ctrl_modes {
	maca_ctrl_mode_no_cca = 0,
	maca_ctrl_mode_non_slotted_csma_ca = 1,
	maca_ctrl_mode_slotted_csma_ca = 2,
};

/* MACA_CONTROL bits */
enum maca_ctrl_bits {
        maca_ctrl_seq    = 0,  /* 3 bits */
	maca_ctrl_mode   = 3,  /* 2 bits */
        maca_ctrl_tm     = 5, 
	maca_ctrl_lfsr   = 6,
        maca_ctrl_auto   = 7,
	maca_ctrl_bcn    = 8,
        maca_ctrl_asap   = 9,
        maca_ctrl_rel    = 10,
        maca_ctrl_prm    = 11,
        maca_ctrl_nofc   = 12,
        maca_ctrl_role   = 13,
        /* 14 reserved */
        maca_ctrl_rsto   = 15,
        maca_ctrl_pre_count = 16, /* 4 bits */
        maca_ctrl_ism    = 20,
};

/* MACA_IRQ bits */
enum maca_irqs {
	maca_irq_acpl     = 0, 
	maca_irq_poll     = 1,
	maca_irq_di       = 2,
	maca_irq_wu       = 3,
	maca_irq_rst      = 4,
	maca_irq_lvl      = 9,
	maca_irq_sftclk   = 10,
	maca_irq_flt      = 11,
	maca_irq_crc      = 12, 
	maca_irq_cm       = 13,
	maca_irq_sync     = 14,  
	maca_irq_strt     = 15,   
};

/* MACA_RESET bits */
enum maca_reset_bits {
	maca_reset_rst    = 0,
	maca_reset_clkon  = 1,
};

/* MACA_TMREN bits */
enum maca_tmren_bits {
	maca_tmren_strt   = 0,
	maca_tmren_cpl    = 1,
	maca_tmren_sft    = 2,
};

enum maca_status_bits {
	maca_status_ovr   = 12,
	maca_status_busy  = 13,
	maca_status_crc   = 14,
	maca_status_to    = 15,
};

#define action_complete_irq()     bit_is_set(*MACA_IRQ,maca_irq_acpl)
#define filter_failed_irq()       bit_is_set(*MACA_IRQ,maca_irq_flt)
#define checksum_failed_irq()     bit_is_set(*MACA_IRQ,maca_irq_crc)
#define data_indication_irq()     bit_is_set(*MACA_IRQ,maca_irq_di)
#define softclock_irq()           bit_is_set(*MACA_IRQ,maca_irq_sftclk)
#define poll_irq()                bit_is_set(*MACA_IRQ,maca_irq_poll)

#define status_is_not_completed() ((*MACA_STATUS & 0xffff) == maca_cc_not_completed)
#define status_is_success() ((*MACA_STATUS & 0xffff) == maca_cc_success)

#define SMAC_MACA_CNTL_INIT_STATE  ( control_prm | control_nofc | control_mode_non_slotted )

#define MACA_WRITE(reg, src) (reg = src)
#define MACA_READ(reg)  reg

#endif // _MACA_H_

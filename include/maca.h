#ifndef _MACA_H_
#define _MACA_H_

#include "embedded_types.h"

#define MACA_BASE       0x80004000
#define MACA_RESET      0x80004004
#define MACA_RANDOM     0x80004008
#define MACA_CONTROL    0x8000400c
#define MACA_STATUS     0x80004010
#define MACA_DMARX      0x80004080
#define MACA_DMATX      0x80004084
#define MACA_GETRXLVL   0x80004098
#define MACA_PREAMBLE   0x8000411c

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

#define maca_status_cc_mask           (0x0F)

#define maca_reset_rst                (1<<0)
#define maca_reset_cln_on             (1<<1)

#define maca_frmpnd_data_pending      (1<<0)
#define maca_frmpnd_no_data_pending   (0x00)

#define maca_txlen_max_rxlen          (127<<16)

#define max_rx_ackwnd_slotted_mode    (0xFFF<<16)
#define max_rx_ackwnd_normal_mode     (0xFFF)


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

#define maca_irq_strt     (1<<15)   /*
                                      STRT
                                      Bit 15
                                      Action Started Interrupt—An auto-sequence is started, either
                                      immediately or by timer trigger.
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */
#define maca_irq_sync     (1<<14)   /*
                                      SYNC
                                      Bit 14
                                      Sync Detected Interrupt—The modem has detected the beginning
                                      of a new packet
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */
#define maca_irq_cm       (1<<13)   /*
                                      CM
                                      Bit 13
                                      Complete Clock Interrupt—The complete clock has generated a
                                      trigger.
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */
#define maca_irq_crc      (1<<12)   /*
                                      CRC
                                      Bit 12
                                      Checksum Failed Interrupt—The checksum failed for the received
                                      packet.
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */
#define maca_irq_flt      (1<<11)   /*
                                      FLT
                                      Bit 11
                                      Filter Failed Interrupt—The receive header filter failed. 1 = Clear interrupt source
                                      0 = Leave source untouched
                                      SFT
                                      Bit 10
                                      Soft Complete Clock Interrupt—The soft complete clock has
                                      generated a trigger.
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */
#define maca_irq_sftclk   (1<<10)

#define maca_irq_lvl      (1<<9)    /*
                                      LVL
                                      Bit 9
                                      FIFO Level interrupt—The receive FIFO level is reached or
                                      exceeded.
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                      Bit 8-5 Reserved bits—Read as zero and written with zero for future
                                      compatibility. N/A
                                    */
#define maca_irq_rst      (1<<4)    /*
                                      RST
                                      Bit 4
                                      Reset Interrupt—A non maskable reset interrupt detected (TBD!!!) 1 = Clear interrupt source
                                      0 = Leave source untouched
                                      WU
                                      Bit 3
                                      Wake-up Interrupt—Low power mode has been exited (TBD in
                                      connection with CCM module).
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */
#define maca_irq_wu       (1<<3)

#define maca_irq_di       (1<<2)    /*
                                      DI
                                      Bit 2
                                      Data Indication Interrupt—During receive, a packet has been
                                      successfully received.
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */
#define maca_irq_poll     (1<<1)    /*
                                      POLL
                                      Bit 1
                                      Poll Indication Interrupt—Issued when data request received (and
                                      before ACK transmitted). MCU may then set MACA_FRMPND and
                                      prepare fast response. TBD: Shall this be skipped if
                                      MACA_FRMPND is clear?
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */
#define maca_irq_acpl     (1<<0)    /*
                                      ACPL
                                      Action Complete Interrupt—Marks the completion of a complete
                                      auto-sequence.
                                      1 = Clear interrupt source
                                      0 = Leave source untouched
                                    */


#define maca_start_clk          (1<<0)/*
                                        TMREN & TMRDIS enable/disable start clock
                                      */

#define maca_cpl_clk            (1<<1)/*
                                        TMREN & TMRDIS enable/disable complete clock
                                      */

#define maca_soft_clk           (1<<2)/*
                                        TMREN & TMRDIS enable/disable soft complete clock
                                      */

#define maca_abort_start_clk    (1<<3)/*
                                        TMRDIS abort start clock
                                      */

#define maca_abort_cpl_clk      (1<<4)/*
                                        TMRDIS abort complete clock
                                      */

#define maca_abort_soft_clk     (1<<5)/*
                                        TMRDIS abort soft complete clock
                                      */

  
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


typedef union maca_ctrl_reg_tag
{
  struct
  {
    uint32_t  RESERVED:11;
    uint32_t  ISM:1;
    uint32_t  PRE_COUNT:4;
    uint32_t  RSTO:1;
    uint32_t  RSV:1;
    uint32_t  ROLE:1;
    uint32_t  NOFC:1;
    uint32_t  PRM:1;
    uint32_t  rel:1;
    uint32_t  ASAP:1;
    uint32_t  BCN:1;
    uint32_t  AUTO:1;
    uint32_t  LFSR:1;
    uint32_t  TM:1;
    uint32_t  MODE:2;
    uint32_t  SEQUENCE:3;
  } Bits;
  uint32_t Reg;
} maca_ctrl_reg_t;

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


typedef union maca_status_reg_tag
{
  struct
  {
    uint32_t  RESERVED:16;
    uint32_t  TO:1;
    uint32_t  CRC:1;
    uint32_t  BUSY:1;
    uint32_t  OVR:1;
    uint32_t  zigbee:1;
    uint32_t  :7;
    uint32_t  COMPLETE_CODE:4;
  } Bits;
  uint32_t Reg;
} maca_status_reg_t;


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


typedef union maca_maskirq_reg_tag
{
  struct
  {
    uint32_t  RESERVED1:16;
    uint32_t  STRT:1;
    uint32_t  SYNC:1;
    uint32_t  CM:1;
    uint32_t  CRC:1;
    uint32_t  FLT:1;
    uint32_t  SFT:1;
    uint32_t  LVL:1;
    uint32_t  RESERVED0:4;
    uint32_t  NOT_USED1:1;
    uint32_t  NOT_USED0:1;
    uint32_t  DI:1;
    uint32_t  POLL:1;
    uint32_t  ACPL:1;
  } Bits;
  uint32_t Reg;
} maca_maskirq_reg_t;

#define SMAC_MACA_CNTL_INIT_STATE  ( control_prm | control_nofc | control_mode_non_slotted )

#define MACA_WRITE(reg, src) (reg = src)
#define MACA_READ(reg)  reg

void reset_maca(void);
void init_phy(void);
void ResumeMACASync(void);

/* Magic data 

better format

00402e10 <gRadioInit_RegReplacement_c>:
  402e10:	80004118 	.word	0x80004118
  402e14:	00180012 	.word	0x00180012
  402e18:	80009204 	.word	0x80009204
  402e1c:	00000605 	.word	0x00000605
  402e20:	80009208 	.word	0x80009208
  402e24:	00000504 	.word	0x00000504
  402e28:	8000920c 	.word	0x8000920c
  402e2c:	00001111 	.word	0x00001111
  402e30:	80009210 	.word	0x80009210
  402e34:	0fc40000 	.word	0x0fc40000
  402e38:	80009300 	.word	0x80009300
  402e3c:	20046000 	.word	0x20046000
  402e40:	80009304 	.word	0x80009304
  402e44:	4005580c 	.word	0x4005580c
  402e48:	80009308 	.word	0x80009308
  402e4c:	40075801 	.word	0x40075801
  402e50:	8000930c 	.word	0x8000930c
  402e54:	4005d801 	.word	0x4005d801
  402e58:	80009310 	.word	0x80009310
  402e5c:	5a45d800 	.word	0x5a45d800
  402e60:	80009314 	.word	0x80009314
  402e64:	4a45d800 	.word	0x4a45d800
  402e68:	80009318 	.word	0x80009318
  402e6c:	40044000 	.word	0x40044000
  402e70:	80009380 	.word	0x80009380
  402e74:	00106000 	.word	0x00106000
  402e78:	80009384 	.word	0x80009384
  402e7c:	00083806 	.word	0x00083806
  402e80:	80009388 	.word	0x80009388
  402e84:	00093807 	.word	0x00093807
  402e88:	8000938c 	.word	0x8000938c
  402e8c:	0009b804 	.word	0x0009b804
  402e90:	80009390 	.word	0x80009390
  402e94:	000db800 	.word	0x000db800
  402e98:	80009394 	.word	0x80009394
  402e9c:	00093802 	.word	0x00093802
  402ea0:	8000a008 	.word	0x8000a008
  402ea4:	00000015 	.word	0x00000015
  402ea8:	8000a018 	.word	0x8000a018
  402eac:	00000002 	.word	0x00000002
  402eb0:	8000a01c 	.word	0x8000a01c
  402eb4:	0000000f 	.word	0x0000000f
  402eb8:	80009424 	.word	0x80009424
  402ebc:	0000aaa0 	.word	0x0000aaa0
  402ec0:	80009434 	.word	0x80009434
  402ec4:	01002020 	.word	0x01002020
  402ec8:	80009438 	.word	0x80009438
  402ecc:	016800fe 	.word	0x016800fe
  402ed0:	8000943c 	.word	0x8000943c
  402ed4:	8e578248 	.word	0x8e578248
  402ed8:	80009440 	.word	0x80009440
  402edc:	000000dd 	.word	0x000000dd
  402ee0:	80009444 	.word	0x80009444
  402ee4:	00000946 	.word	0x00000946
  402ee8:	80009448 	.word	0x80009448
  402eec:	0000035a 	.word	0x0000035a
  402ef0:	8000944c 	.word	0x8000944c
  402ef4:	00100010 	.word	0x00100010
  402ef8:	80009450 	.word	0x80009450
  402efc:	00000515 	.word	0x00000515
  402f00:	80009460 	.word	0x80009460
  402f04:	00397feb 	.word	0x00397feb
  402f08:	80009464 	.word	0x80009464
  402f0c:	00180358 	.word	0x00180358
  402f10:	8000947c 	.word	0x8000947c
  402f14:	00000455 	.word	0x00000455
  402f18:	800094e0 	.word	0x800094e0
  402f1c:	00000001 	.word	0x00000001
  402f20:	800094e4 	.word	0x800094e4
  402f24:	00020003 	.word	0x00020003
  402f28:	800094e8 	.word	0x800094e8
  402f2c:	00040014 	.word	0x00040014
  402f30:	800094ec 	.word	0x800094ec
  402f34:	00240034 	.word	0x00240034
  402f38:	800094f0 	.word	0x800094f0
  402f3c:	00440144 	.word	0x00440144
  402f40:	800094f4 	.word	0x800094f4
  402f44:	02440344 	.word	0x02440344
  402f48:	800094f8 	.word	0x800094f8
  402f4c:	04440544 	.word	0x04440544
  402f50:	80009470 	.word	0x80009470
  402f54:	0ee7fc00 	.word	0x0ee7fc00
  402f58:	8000981c 	.word	0x8000981c
  402f5c:	00000082 	.word	0x00000082
  402f60:	80009828 	.word	0x80009828
  402f64:	0000002a 	.word	0x0000002a

appears to be   addr addr data data addr addr data data

e.g. 0x80004118 gets 00180012 (MACA_WARMUP)

00402e10 <gRadioInit_RegReplacement_c>:
  402e10:       4118 8000 0012 0018 9204 8000 0605 0000     .A..............
  402e20:       9208 8000 0504 0000 920c 8000 1111 0000     ................
  402e30:       9210 8000 0000 0fc4 9300 8000 6000 2004     .............`. 
  402e40:       9304 8000 580c 4005 9308 8000 5801 4007     .....X.@.....X.@
  402e50:       930c 8000 d801 4005 9310 8000 d800 5a45     .......@......EZ
  402e60:       9314 8000 d800 4a45 9318 8000 4000 4004     ......EJ.....@.@
  402e70:       9380 8000 6000 0010 9384 8000 3806 0008     .....`.......8..
  402e80:       9388 8000 3807 0009 938c 8000 b804 0009     .....8..........
  402e90:       9390 8000 b800 000d 9394 8000 3802 0009     .............8..
  402ea0:       a008 8000 0015 0000 a018 8000 0002 0000     ................
  402eb0:       a01c 8000 000f 0000 9424 8000 aaa0 0000     ........$.......
  402ec0:       9434 8000 2020 0100 9438 8000 00fe 0168     4...  ..8.....h.
  402ed0:       943c 8000 8248 8e57 9440 8000 00dd 0000     <...H.W.@.......
  402ee0:       9444 8000 0946 0000 9448 8000 035a 0000     D...F...H...Z...
  402ef0:       944c 8000 0010 0010 9450 8000 0515 0000     L.......P.......
  402f00:       9460 8000 7feb 0039 9464 8000 0358 0018     `.....9.d...X...
  402f10:       947c 8000 0455 0000 94e0 8000 0001 0000     |...U...........
  402f20:       94e4 8000 0003 0002 94e8 8000 0014 0004     ................
  402f30:       94ec 8000 0034 0024 94f0 8000 0144 0044     ....4.$.....D.D.
  402f40:       94f4 8000 0344 0244 94f8 8000 0544 0444     ....D.D.....D.D.
  402f50:       9470 8000 fc00 0ee7 981c 8000 0082 0000     p...............
  402f60:       9828 8000 002a 0000                         (...*...
*/

#endif // _MACA_H_


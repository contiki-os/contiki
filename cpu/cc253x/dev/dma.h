/**
 * \file
 *       Header file for the cc2430 DMA controller
 *
 * \author
 *         Original: Martti Huttunen <martti@sensinode.com>
 *         Port: Zach Shelby <zach@sensinode.com>
 *         Further Modifications:
 *               George Oikonomou <oikonomou@users.sourceforge.net>
 */

#ifndef __DMA_H
#define __DMA_H
#include "cc253x.h"
#include "sfr-bits.h"

/* DMA triggers */
#define DMA_T_NONE       0 /* None, DMAREQ.DMAREQx bits start transfer */
#define DMA_T_PREV       1 /* completion of previous channel */
#define DMA_T_T1_CH0     2 /* Timer 1, compare, channel 0 */
#define DMA_T_T1_CH1     3 /* Timer 1, compare, channel 1 */
#define DMA_T_T1_CH2     4 /* Timer 1, compare, channel 2 */
#define DMA_T_T2_COMP    5 /* Timer 2, compare */
#define DMA_T_T2_OVFL    6 /* Timer 2, overflow */
#define DMA_T_T3_CH0     7 /* Timer 3, compare, channel 0 */
#define DMA_T_T3_CH1     8 /* Timer 3, compare, channel 1 */
#define DMA_T_T4_CH0     9 /* Timer 4, compare, channel 0 */
#define DMA_T_T4_CH1    10 /* Timer 4, compare, channel 1 */
#define DMA_T_ST        11 /* Sleep Timer compare */
#define DMA_T_IOC_0     12 /* Port 0 I/O pin input transition */
#define DMA_T_IOC_1     13 /* Port 1 I/O pin input transition */
#define DMA_T_URX0      14 /* USART0 RX complete */
#define DMA_T_UTX0      15 /* USART0 TX complete */
#define DMA_T_URX1      16 /* USART1 RX complete */
#define DMA_T_UTX1      17 /* USART1 TX complete */
#define DMA_T_FLASH     18 /* Flash data write complete */
#define DMA_T_RADIO     19 /* RF packet byte received/transmit */
#define DMA_T_ADC_CHALL 20 /* ADC end of a conversion in a sequence */
#define DMA_T_ADC_CH11  21 /* ADC end of conversion channel 0 in sequence */
#define DMA_T_ADC_CH21  22 /* ADC end of conversion channel 1 in sequence */
#define DMA_T_ADC_CH32  23 /* ADC end of conversion channel 2 in sequence */
#define DMA_T_ADC_CH42  24 /* ADC end of conversion channel 3 in sequence */
#define DMA_T_ADC_CH53  25 /* ADC end of conversion channel 4 in sequence */
#define DMA_T_ADC_CH63  26 /* ADC end of conversion channel 5 in sequence */
#define DMA_T_ADC_CH74  27 /* ADC end of conversion channel 6 in sequence */
#define DMA_T_ADC_CH84  28 /* ADC end of conversion channel 7 in sequence */
#define DMA_T_ENC_DW    29 /* AES processor requests download input data */
#define DMA_T_ENC_UP    30 /* AES processor requests upload output data */

/* variable DMA length modes (VLEN) */
#define DMA_VLEN_LEN  (0 << 5) /* Use LEN for transfer count*/
/*
 * Transfer the number of bytes/words specified by first byte/word + 1
 * (up to a maximum specified by LEN).
 * Thus transfer count excludes length byte/word.
 */
#define DMA_VLEN_N1   (1 << 5)
 /*
  * Transfer the number of bytes/words specified by first byte/word
  * (up to a maximum specified by LEN).
  * Thus transfer count includes length byte/word.
  */
#define	DMA_VLEN_N    (2 << 5)
 /*
  * Transfer the number of bytes/words specified by first byte/word + 2
  * (up to a maximum specified by LEN).
  */
#define	DMA_VLEN_N2   (3 << 5)
 /*
  * Transfer the number of bytes/words specified by first byte/word + 3
  * (up to a maximum specified by LEN).
  */
#define	DMA_VLEN_N3   (4 << 5)
#define	DMA_VLEN_RES1 (5 << 5) /* reserved */
#define	DMA_VLEN_RES2 (6 << 5) /* reserved */
#define	DMA_VLEN_LEN2 (7 << 5) /* Use LEN for transfer count */

/* Transfer Types (Byte 6 [6:5]) */
#define DMA_SINGLE           0x00 /* Single */
#define DMA_BLOCK            0x20 /* Block */
#define DMA_RPT_SINGLE       0x40 /* Repeated single */
#define DMA_RPT_BLOCK        0x60 /* Repeated block */

/* Source Increment Modes (Byte 7 [7:6])*/
#define DMA_SRC_INC_NO       0x00 /* Source No increment */
#define DMA_SRC_INC_1        0x40 /* Source Increment 1 */
#define DMA_SRC_INC_2        0x80 /* Source Increment 2 */
#define DMA_SRC_DEC          0xC0 /* Source Decrement 1 */
/* Source Increment Modes (Byte 7 [5:4])*/
#define DMA_DST_INC_NO       0x00 /* DestinationNo increment */
#define DMA_DST_INC_1        0x10 /* Destination Increment 1 */
#define DMA_DST_INC_2        0x20 /* Destination Increment 2 */
#define DMA_DST_DEC          0x30 /* Destination Decrement 1 */

/* Descriptor Byte 7, Bits[3:0] */
#define DMA_IRQ_MASK_ENABLE  0x08
#define DMA_MODE_7_BIT       0x04
#define DMA_PRIO_HIGH        0x02
#define DMA_PRIO_ASSURED     0x01
#define DMA_PRIO_LOW         0x00

/** DMA configuration structure */
typedef struct dma_config {
  uint8_t src_h; /* source address high byte*/
  uint8_t src_l; /* source address low byte*/
  uint8_t dst_h; /* dest. address high byte*/
  uint8_t dst_l; /* dest. address low byte*/
  uint8_t len_h; /* [7:5] VLEN, [4:0] length high byte, 5 lowest bits*/
  uint8_t len_l; /* length low byte*/
  uint8_t wtt;   /* 7: wordsize, [6:5] transfer mode, [4:0] trigger */
  /* [7:6] src inc, [5:4] dst_inc, 3: IRQ, 2: M8(vlen), [1-0] prio */
  uint8_t inc_prio;
} dma_config_t;

#ifdef DMA_CONF_ON
#define DMA_ON DMA_CONF_ON
#else
#define DMA_ON 0
#endif

/* Number of DMA Channels and their Descriptors */
#if DMA_ON
#define DMA_CHANNEL_COUNT 2
extern dma_config_t dma_conf[DMA_CHANNEL_COUNT];
#endif

/* DMA-Related Macros */
#define DMA_ARM(c)      (DMAARM |= (1 << c)) /* Arm DMA Channel C */
#define DMA_TRIGGER(c)  (DMAREQ |= (1 << c)) /* Trigger DMA Channel C */
/*
 * Check Channel C for Transfer Status
 * 1: Complete, Pending Interrupt, 0: Incomplete
 */
#define DMA_STATUS(c)   (DMAIRQ &(1 << c))
/* Abort Ongoing DMA Transfers on Channel C */
#define DMA_ABORT(c)    (DMAARM = DMAARM_ABORT | (1 << c))
#define DMA_ABORT_ALL() (DMAARM = 0x9F) /* Abort ALL Ongoing DMA Transfers */

/* Functions Declarations */
void dma_init(void);
void dma_associate_process(struct process *p, uint8_t c);
void dma_reset(uint8_t c);

/* Only link the ISR when DMA_ON is .... on */
#if DMA_ON
void dma_isr(void) __interrupt(DMA_VECTOR);
#endif

#endif /*__DMA_H*/

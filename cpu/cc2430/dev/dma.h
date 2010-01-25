/**
 * \file
 *         DMA driver header
 * \author
 *         Original: Martti Huttunen <martti@sensinode.com>
 *         Port: Zach Shelby <zach@sensinode.com>
 */

#ifndef __DMA_H
#define __DMA_H
#include "banked.h"
#include "cc2430_sfr.h"

/** DMA triggers */
typedef enum dma_trigger_t
{
	DMA_T_NONE=0, /*!<DMA No trigger, setting DMAREQ.DMAREQx bit starts transfer*/
	DMA_T_PREV=1, /*!<DMA DMA channel is triggered by completion of previous channel*/
	DMA_T_T1_CH0=2, /*!<Timer 1 Timer 1, compare, channel 0*/
	DMA_T_T1_CH1=3, /*!<Timer 1 Timer 1, compare, channel 1*/
	DMA_T_T1_CH2=4, /*!<Timer 1 Timer 1, compare, channel 2*/
	DMA_T_T2_COMP=5, /*!<Timer 2 Timer 2, compare*/
	DMA_T_T2_OVFL=6, /*!<Timer 2 Timer 2, overflow*/
	DMA_T_T3_CH0=7, /*!<Timer 3 Timer 3, compare, channel 0*/
	DMA_T_T3_CH1=8, /*!<Timer 3 Timer 3, compare, channel 1*/
	DMA_T_T4_CH0=9, /*!<Timer 4 Timer 4, compare, channel 0*/
	DMA_T_T4_CH1=10, /*!<Timer 4 Timer 4, compare, channel 1*/
	DMA_T_ST=11, /*!<Sleep Timer Sleep Timer compare*/
	DMA_T_IOC_0=12, /*!<IO Controller Port 0 I/O pin input transition*/
	DMA_T_IOC_1=13, /*!<IO Controller Port 1 I/O pin input transition*/
	DMA_T_URX0=14, /*!<USART0 USART0 RX complete*/
	DMA_T_UTX0=15, /*!<USART0 USART0 TX complete*/
	DMA_T_URX1=16, /*!<USART1 USART1 RX complete*/
	DMA_T_UTX1=17, /*!<USART1 USART1 TX complete*/
	DMA_T_FLASH=18, /*!<Flash controller Flash data write complete*/
	DMA_T_RADIO=19, /*!<Radio RF packet byte received/transmit*/
	DMA_T_ADC_CHALL=20, /*!<ADC ADC end of a conversion in a sequence, sample ready*/
	DMA_T_ADC_CH11=21, /*!<ADC ADC end of conversion channel 0 in sequence, sample ready*/
	DMA_T_ADC_CH21=22, /*!<ADC ADC end of conversion channel 1 in sequence, sample ready*/
	DMA_T_ADC_CH32=23, /*!<ADC ADC end of conversion channel 2 in sequence, sample ready*/
	DMA_T_ADC_CH42=24, /*!<ADC ADC end of conversion channel 3 in sequence, sample ready*/
	DMA_T_ADC_CH53=25, /*!<ADC ADC end of conversion channel 4 in sequence, sample ready*/
	DMA_T_ADC_CH63=26, /*!<ADC ADC end of conversion channel 5 in sequence, sample ready*/
	DMA_T_ADC_CH74=27, /*!<ADC ADC end of conversion channel 6 in sequence, sample ready*/
	DMA_T_ADC_CH84=28, /*!<ADC ADC end of conversion channel 7 in sequence, sample ready*/
	DMA_T_ENC_DW=29, /*!<AES AES encryption processor requests download input data*/
	DMA_T_ENC_UP=30, /*!<AES AES encryption processor requests upload output data*/
	DMA_T_RES=31
}dma_trigger_t;

/** variable DMA length modes */
typedef enum dma_vlen_t
{
	DMA_VLEN_LEN  = (0 << 5),/*!<Use LEN for transfer count*/
	DMA_VLEN_N1   = (1 << 5),/*!<Transfer the number of bytes/words specified by first byte/word + 1 (up to a maximum specified by LEN). Thus transfer count excludes length byte/word.*/
	DMA_VLEN_N    = (2 << 5),/*!<Transfer the number of bytes/words specified by first byte/word     (up to a maximum specified by LEN). Thus transfer count includes length byte/word.*/
	DMA_VLEN_N2   = (3 << 5),/*!<Transfer the number of bytes/words specified by first byte/word + 2 (up to a maximum specified by LEN).*/
	DMA_VLEN_N3   = (4 << 5),/*!<Transfer the number of bytes/words specified by first byte/word + 3 (up to a maximum specified by LEN).*/
	DMA_VLEN_RES1 = (5 << 5),/*!<reserved*/
	DMA_VLEN_RES2 = (6 << 5),/*!<reserved*/
	DMA_VLEN_LEN2 = (7 << 5) /*!<Use LEN for transfer count*/
}dma_vlen_t;

/** address increment modes */
typedef enum dma_inc_t
{
	DMA_NOINC = 0,	/*!<No increment*/
	DMA_INC = 1,	/*!<Increment*/
	DMA_INC2 = 2,	/*!<Increment 2*/
	DMA_DEC = 3	/*!<Decrement*/
}dma_inc_t;

/** transfer types */
typedef enum dma_type_t
{
	DMA_SINGLE = 0,	/*!<Single*/
	DMA_BLOCK = 1,	/*!<Block*/
	DMA_RPT = 2,	/*!<Repeated single*/
	DMA_BLOCK_RPT=3	/*!<Repeated block*/
}dma_type_t;

/** DMA configuration structure */
typedef struct dma_config_t
{
	uint8_t src_h; /*!<source address high byte*/
	uint8_t src_l; /*!<source address low byte*/
	uint8_t dst_h; /*!<dest. address high byte*/
	uint8_t dst_l; /*!<dest. address low byte*/
	uint8_t len_h; /*!<transfer mode in high 3 bits, length high byte, 4 lowest bits*/
	uint8_t len_l; /*!<length low byte*/
	uint8_t t_mode; /*!<transfer mode: bit7=word mode, 6-5=block/single 4-0=trigger */
	uint8_t addr_mode; /*!<address mode: 7-6=src inc, 5-4=dst_inc, 3=IRQ, 2=M8(vlen) 1-0:priority*/

}dma_config_t;

extern void dma_init(void) __banked;
typedef void (*dma_func)(void *);

extern dma_config_t dma_conf[4];

#ifdef HAVE_DMA
typedef uint8_t xDMAHandle;

#define dma_config(channel, src, src_inc, dst, dst_inc, length, vlen_mode, t_mode, trigger, proc) dma_config2(channel,src,src_inc, dst, dst_inc, length, 0, vlen_mode, t_mode, trigger, proc)
/* 
 extern xDMAHandle dma_config(uint8_t channel, void *src, dma_inc_t src_inc, void *dst, dma_inc_t dst_inc, 
                             uint16_t length, dma_vlen_t vlen_mode, dma_type_t t_mode,
			     dma_trigger_t trigger, struct process * p);
*/
extern xDMAHandle dma_config2(uint8_t channel, void *src, dma_inc_t src_inc, void *dst, dma_inc_t dst_inc,
                             uint16_t length, uint8_t word_mode, dma_vlen_t vlen_mode, dma_type_t t_mode,
			     dma_trigger_t trigger, struct process * p) __banked;
extern uint8_t dma_arm(xDMAHandle channel) __banked;
extern uint8_t dma_abort(xDMAHandle channel) __banked;
extern uint8_t dma_trigger(xDMAHandle channel) __banked;
extern uint8_t dma_state(xDMAHandle channel) __banked;
void dma_config_print(xDMAHandle channel) __banked;
#endif

extern void dma_ISR( void ) __interrupt (DMA_VECTOR);

#endif /*__DMA_H*/

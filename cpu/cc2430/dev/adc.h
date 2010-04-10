/**
 * \file
 *   Header file for ADC.
 * \author
 *   Anthony "Asterisk" Ambuehl
 *
 */

#ifndef __ADC_H
#define __ADC_H
#define ADC_DMA_CONFIG_CHANNEL 1
#define ADC_CHANNELS 8
#include "cc2430_sfr.h"
#include "dma.h"
#include "banked.h"

typedef struct adc_result
{
  uint16_t adc:14;
  uint16_t unused:2;
} adc_result_t;

typedef enum adc_stsel_t
{
	EXT = 0, /* externally triggered by P2_0 */
	CONTINUOUS = 1, /* continuous full speed conversion */
	TIMER1 = 2, /* Timer 1 channel 0 compare event  */
	ST = 3 /* ADCCON1.ST = 1  */
} adc_stsel_t;

extern void adc_init(void) __banked;
extern void adc_single_shot(void) __banked;
extern int16_t adc_convert_result(int16_t ptr) __banked;
extern int16_t adc_get_last_conv() __banked;
extern void adc_dma_callback(void) __banked;
#ifdef HAVE_DMA
extern xDMAHandle adc_dma;
extern unsigned int *adc_dma_dest;
#endif

#endif /*__ADC_H*/

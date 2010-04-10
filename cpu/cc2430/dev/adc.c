/**
 * \file
 *         ADC functions
 * \author
 * 	Anthony "Asterisk" Ambuehl
 *
 *  ADC initialization routine, trigger and result conversion routines.
 *
 */

#include <stdio.h>
#include "banked.h"
#include "contiki.h"
#include "sys/clock.h"

#include "cc2430_sfr.h"
#include "dev/adc.h"
#include "dev/dma.h"

#ifdef HAVE_DMA
xDMAHandle adc_dma=0xff;
unsigned int *adc_dma_dest;
#endif

/*---------------------------------------------------------------------------*/
void adc_init(void) __banked
{
  unsigned char jj;
  while (!SLEEP&(HFRC_STB)) {}
  /* power both 32MHz crystal and 15MHz RC */
  SLEEP &= ~(OSC_PD); 
  /* printf("SLEEP 1 %x\n",SLEEP); */
  /* then wait for it to stabilize */
  while (!SLEEP&(XOSC_STB)) {}
  /* printf("SLEEP 2 %x\n",SLEEP); */
  /* then wait 64uS more */
  clock_delay(150);
  /* switch to 32MHz clock */
  /* printf("switch to 32MHz %x\n",CLKCON); */
  CLKCON &= ~(OSC);
  /* printf("switched to 32MHz %x\n",CLKCON); */
  /* power down 15MHz RC clock */
  SLEEP |= OSC_PD; 
  /* printf("pwr down hfrc\n",SLEEP);  */
#ifdef HAVE_DMA
  /* preconfigure adc_dma before calling adc_init if a different dma type is desired. */
  if (adc_dma==0xff) {
    dma_init();
    /*  config DMA channel to copy results to single location */
    adc_dma=dma_config2(ADC_DMA_CONFIG_CHANNEL, &ADC_SHADOW, DMA_NOINC, adc_dma_dest, DMA_NOINC, 1, 1, DMA_VLEN_LEN, DMA_RPT, DMA_T_ADC_CHALL, 0);
  }
#endif
}
/* single sample trigger */
void adc_single_shot(void) __banked
{
  ADCCON1 |= 0x73;
}
/* convert adc results */
int16_t adc_convert_result(int16_t data) __banked {
  data = (0xfffc&data)>>2;
  return data; 
}
/* read/convert last conversion result */
int16_t adc_get_last_conv() __banked {
  int16_t result;
  result = (ADCH<<8)|(ADCL);
  result  = (0xfffc&result)>>2;
  return result; 
}

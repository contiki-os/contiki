/**
 * \file
 *         DMA driver
 * \author
 *         Original: Martti Huttunen <martti@sensinode.com>
 *         Port: Zach Shelby <zach@sensinode.com>
 */

#include <stdio.h>

#include "contiki.h"

#include "dev/dma.h"
#include "cc2430_sfr.h"

dma_config_t dma_conf[4];
struct process * dma_callback[4];

/*---------------------------------------------------------------------------*/
void
dma_init(void)
{
  uint16_t tmp_ptr;
  
  memset(dma_conf, 0, 4*sizeof(dma_config_t));
  for(tmp_ptr = 0; tmp_ptr < 4; tmp_ptr++) {
    dma_callback[tmp_ptr] = 0;
  }
  tmp_ptr = (uint16_t) &(dma_conf[0]);

  DMA1CFGH = tmp_ptr >> 8;
  DMA1CFGL = tmp_ptr;
  IEN1_DMAIE = 1;	/*enable DMA interrupts*/
}
/*---------------------------------------------------------------------------*/
#ifdef HAVE_DMA
/**
 * Configure a DMA channel.
 *
 * \param channel channel ID;
 * \param src source address;
 * \param src_inc source increment mode;
 * \param dst dest address;
 * \param dst_inc dest increment mode;
 * \param length maximum length;
 * \param vlen_mode variable length mode;
 * \param t_mode DMA transfer mode;
 * \param trigger DMA trigger;
 * \param proc process that is upon interrupt;
 *
 * \return Handle to DMA channel
 * \return 0 invalid channel
 */
xDMAHandle
dma_config(uint8_t channel, void *src, dma_inc_t src_inc, void *dst, dma_inc_t dst_inc,
	   uint16_t length, dma_vlen_t vlen_mode, dma_type_t t_mode, dma_trigger_t trigger,
	   struct process * proc)
{
  if((!channel) || (channel > 4)) {
    return 0;
  }

  DMAIRQ &= ~(1 << channel);
  
  channel--;
  
  dma_conf[channel].src_h = ((uint16_t) src) >> 8;
  dma_conf[channel].src_l = ((uint16_t) src);
  dma_conf[channel].dst_h = ((uint16_t) dst) >> 8;
  dma_conf[channel].dst_l = ((uint16_t) dst);
  dma_conf[channel].len_h = vlen_mode + (length >> 8);
  dma_conf[channel].len_l = length;
  dma_conf[channel].t_mode = (t_mode << 5) + trigger;
  dma_conf[channel].addr_mode = (src_inc << 6) + (dst_inc << 4) + 2; /*DMA has priority*/
  
  /*Callback is defined*/
  if(proc) {
    dma_conf[channel].addr_mode |= 8;	/*set IRQMASK*/
    IEN1_DMAIE = 1;	/*enable DMA interrupts*/
  }
  dma_callback[channel] = proc;
  
  return (xDMAHandle)channel + 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Arm a DMA channel.
 *
 * \param channel channel handle;
 *
 * \return pdTRUE
 * \return pdFALSE	semaphore creation failed
 */
uint8_t
dma_arm(xDMAHandle channel)
{
  uint8_t ch_id = ((uint8_t)channel);
  if(!ch_id || (ch_id > 4)) {
    return 0;
  }
  DMAARM |= (1 << ch_id);
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Stop a DMA channel.
 *
 * \param channel channel handle;
 *
 * \return pdTRUE
 * \return pdFALSE	semaphore creation failed
 */
uint8_t
dma_abort(xDMAHandle channel)
{
  uint8_t ch_id = ((uint8_t) channel);
  if(!ch_id || (ch_id > 4)) {
    return 0;
  }
  DMAARM = 0x80 + (1 << ch_id);	/*ABORT + channel bit*/
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Trigger a DMA channel.
 *
 * \param channel channel handle;
 *
 * \return pdTRUE
 * \return pdFALSE	semaphore creation failed
 */
uint8_t
dma_trigger(xDMAHandle channel)
{
  uint8_t ch_id = ((uint8_t) channel);
  if(!ch_id || (ch_id > 4)) {
    return 0;
  }
  DMAREQ |= (1 << ch_id);
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * Get DMA state.
 *
 * \param channel channel handle;
 *
 * \return pdTRUE	active
 * \return pdFALSE	not active
 */
uint8_t
dma_state(xDMAHandle channel)
{
  uint8_t ch_id = ((uint8_t)channel);
  if(!ch_id || (ch_id > 4)) {
    return 0;
  }
  if((DMAIRQ &(1 << ch_id)) == 0) {
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
dma_config_print(xDMAHandle channel)
{
  uint8_t ch_id = channel - 1;
  
  if(ch_id > 4) {
    return;
  }
  
  printf("DMA channel %d @ %x %x ", ch_id, (uint16_t) &(dma_conf[ch_id]) >> 8, (uint16_t) &(dma_conf[ch_id]) & 0xFF);
  {
    uint8_t i;
    uint8_t *ptr = (uint8_t *)&(dma_conf[ch_id]);
    for(i = 0; i< 8; i++) {
      if(i != 0) {
	printf(":%x", *ptr++);
      }
    }
    printf("\n");
  }
}
#endif
/*---------------------------------------------------------------------------*/
#ifdef HAVE_RF_DMA
extern void rf_dma_callback_isr(void);
#endif
#ifdef SPI_DMA_RX
extern void spi_rx_dma_callback(void);
#endif
/*---------------------------------------------------------------------------*/
/**
 * DMA interrupt service routine.
 *
 * if callback defined a poll is made to that process
 */
void
dma_ISR(void) __interrupt (DMA_VECTOR)
{
#ifdef HAVE_DMA
  uint8_t i;
#endif
  EA=0;
#ifdef HAVE_RF_DMA
  if((DMAIRQ & 1) != 0) {
    DMAIRQ &= ~1;
    DMAARM=0x81;
    rf_dma_callback_isr();
  }
#endif
#ifdef SPI_DMA_RX
  if((DMAIRQ & 0x08) != 0) {
    DMAIRQ &= ~(1 << 3);
    spi_rx_dma_callback();
  }
#endif
  
#ifdef HAVE_DMA
  for(i = 0; i < 4; i++) {
    if((DMAIRQ & (1 << i + 1)) != 0) {
      DMAIRQ &= ~(1 << i+1);
      if(dma_callback[i] != 0) {
	process_poll(dma_callback[i]);
      }
    }
  }
#endif
  IRCON_DMAIF = 0;
  EA = 1;
}
/*---------------------------------------------------------------------------*/

/**
 * \file
 *         DMA driver
 * \author
 *         Original: Martti Huttunen <martti@sensinode.com>
 *         Port: Zach Shelby <zach@sensinode.com>
 *
 *         bankable DMA functions
 */

#include <stdio.h>

#include "contiki.h"
#include "banked.h"

#include "dev/dma.h"
#include "cc2430_sfr.h"

dma_config_t dma_conf[4];
struct process * dma_callback[4];

/*---------------------------------------------------------------------------*/
void
dma_init(void) __banked
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
 * Configure a DMA channel except word mode.
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
/*  IMPLEMENTED dma_config as macro to reduce stack/code space
xDMAHandle
dma_config(uint8_t channel, void *src, dma_inc_t src_inc, void *dst, dma_inc_t dst_inc,
	   uint16_t length, dma_vlen_t vlen_mode, dma_type_t t_mode, dma_trigger_t trigger,
	   struct process * proc) __banked
{
  return dma_config2(channel,src,src_inc, dst, dst_inc, length, 0, vlen_mode, t_mode, trigger, proc);
}
*/
/*---------------------------------------------------------------------------*/
/**
 * Configure a DMA channel.
 *
 * \param channel channel ID;
 * \param src source address;
 * \param src_inc source increment mode;
 * \param dst dest address;
 * \param dst_inc dest increment mode;
 * \param length maximum length;
 * \param word_mode set to 1 for 16-bits per transfer;
 * \param vlen_mode variable length mode;
 * \param t_mode DMA transfer mode;
 * \param trigger DMA trigger;
 * \param proc process that is upon interrupt;
 *
 * \return Handle to DMA channel
 * \return 0 invalid channel
 */
xDMAHandle
dma_config2(uint8_t channel, void *src, dma_inc_t src_inc, void *dst, dma_inc_t dst_inc,
	   uint16_t length, uint8_t word_mode, dma_vlen_t vlen_mode, dma_type_t t_mode, dma_trigger_t trigger,
	   struct process * proc) __banked
{
  unsigned char jj;
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
  dma_conf[channel].t_mode = ((word_mode&0x1)<<7) | (t_mode << 5) | trigger;
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
dma_arm(xDMAHandle channel) __banked
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
dma_abort(xDMAHandle channel) __banked
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
dma_trigger(xDMAHandle channel) __banked
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
dma_state(xDMAHandle channel) __banked
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
dma_config_print(xDMAHandle channel) __banked
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
	printf(":%02x", *ptr++);
      }
    }
    printf("\n");
  }
}
#endif

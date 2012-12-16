/**
 * \file
 *         Driver for the cc2430 DMA controller. Can be assigned to any bank
 *
 * \author
 *         Original: Martti Huttunen <martti@sensinode.com>
 *         Port: Zach Shelby <zach@sensinode.com>
 *         Further Modifications:
 *               George Oikonomou <oikonomou@users.sourceforge.net>
 *
 */

#include "contiki.h"
#include "dev/dma.h"
#include "cc2430_sfr.h"

#if DMA_ON
struct dma_config dma_conf[DMA_CHANNEL_COUNT]; /* DMA Descriptors */
struct process *dma_callback[DMA_CHANNEL_COUNT];
/*---------------------------------------------------------------------------*/
void
dma_init(void)
{
  uint16_t tmp_ptr;

  memset(dma_conf, 0, 4 * sizeof(dma_config_t));

  for(tmp_ptr = 0; tmp_ptr < DMA_CHANNEL_COUNT; tmp_ptr++) {
    dma_callback[tmp_ptr] = 0;
  }

  /* The address of the descriptor for Channel 0 is configured separately */
  tmp_ptr = (uint16_t)&(dma_conf[0]);
  DMA0CFGH = tmp_ptr >> 8;
  DMA0CFGL = tmp_ptr;

  /*
   * Descriptors for Channels 1-4 must be consecutive in RAM.
   * We write the address of the 1st one to the register and the rest are
   * derived by the SoC
   */
#if (DMA_CHANNEL_COUNT > 1)
  tmp_ptr = (uint16_t)&(dma_conf[1]);
  DMA1CFGH = tmp_ptr >> 8;
  DMA1CFGL = tmp_ptr;
#endif

  IEN1_DMAIE = 1;	/* Enable DMA interrupts */
}
/*---------------------------------------------------------------------------*/
/*
 * Associate process p with DMA channel c. When a transfer on that channel
 * completes, the ISR will poll this process.
 */
void
dma_associate_process(struct process *p, uint8_t c)
{
  if((!c) || (c >= DMA_CHANNEL_COUNT)) {
    return;
  }

  if(p) {
    dma_conf[c].inc_prio |= 8; /* Enable interrupt generation */
    IEN1_DMAIE = 1; /* Make sure DMA interrupts are acknowledged */
  }
  dma_callback[c] = p;
}
/*---------------------------------------------------------------------------*/
#endif

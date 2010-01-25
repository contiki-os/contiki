/**
 * \file
 *         DMA driver ISRs
 * \author
 *         Original: Martti Huttunen <martti@sensinode.com>
 *         Port: Zach Shelby <zach@sensinode.com>
 *
 *         DMA interrupt routines, must be stored in HOME bank
 */

#include <stdio.h>

#include "contiki.h"

#include "dev/dma.h"
#include "cc2430_sfr.h"
#include "banked.h"

extern struct process * dma_callback[4];

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

/**
 * \file
 *         CC2430 RF driver header file
 * \author
 *         Zach Shelby <zach@sensinode.com>
 */

#ifndef __CC2430_RF_H__
#define __CC2430_RF_H__

#include "contiki.h"
#include "dev/radio.h"
#include "cc2430_sfr.h"
#if HAVE_RF_DMA
#include "dev/dma.h"
#endif

/* Constants */
typedef enum rf_address_mode_t
{
	RF_DECODER_NONE = 0,
	RF_DECODER_COORDINATOR,
	RF_SOFTACK_MONITOR,
	RF_MONITOR,
	RF_SOFTACK_CLIENT,
	RF_DECODER_ON
}rf_address_mode_t;

/*CSP command set*/
#define SSTOP		  0xDF
/*this is not a real command but a way of having rf_command
  wait until the script is done*/
#define SSTART		0xDE

#define SNOP		  0xC0
#define STXCALN 	0xC1
#define SRXON		  0xC2
#define STXON		  0xC3
#define STXONCCA	0xC4
#define SRFOFF		0xC5
#define SFLUSHRX	0xC6
#define SFLUSHTX	0xC7
#define SACK		  0xC8
#define SACKPEND	0xC9

#define ISTXCALN 	0xE1
#define ISRXON		0xE2
#define ISTXON		0xE3
#define ISTXONCCA	0xE4
#define ISRFOFF		0xE5
#define ISFLUSHRX	0xE6
#define ISFLUSHTX	0xE7
#define ISACK		  0xE8
#define ISACKPEND	0xE9

#define ISSTOP		0xFF
#define ISSTART		0xFE

#define MAC_IFS (1200/128)

#define CC2430_MAX_PACKET_LEN      127
#define CC2430_MIN_PACKET_LEN      4

#define CC2430_CCA_CLEAR  1
#define CC2430_CCA_BUSY   0

#ifdef CC2430_CONF_RFERR_INTERRUPT
#define CC2430_RFERR_INTERRUPT CC2430_CONF_RFERR_INTERRUPT
#else
#define CC2430_RFERR_INTERRUPT 0
#endif

extern const struct radio_driver cc2430_rf_driver;

void cc2430_rf_command(uint8_t command);
int8_t cc2430_rf_channel_set(uint8_t channel);
uint8_t cc2430_rf_channel_get();
uint8_t cc2430_rf_power_set(uint8_t new_power);
void cc2430_rf_set_addr(unsigned pan, unsigned addr, const uint8_t *ieee_addr);

#if !NETSTACK_CONF_SHORTCUTS
extern void cc2430_rf_ISR( void ) __interrupt (RF_VECTOR);
#endif
#if CC2430_RFERR_INTERRUPT
extern void cc2430_rf_error_ISR( void ) __interrupt (RFERR_VECTOR);
#endif

#ifdef HAVE_RF_DMA
void rf_dma_callback_isr(void);
#endif

#endif /* __CC2430_RF_H__ */

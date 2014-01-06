/*
 * Copyright (c) 2013, KTH, Royal Institute of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "emac.h"
#include "emac-driver.h"
#include "lpc17xx_emac.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>

/* Example group ----------------------------------------------------------- */
/** @defgroup EMAC_uIP	uIP
 * @ingroup EMAC_Examples
 * @{
 */

#define DB  _DBG((uint8_t *)db)
char db[64];

/* Init the LPC17xx ethernet */
BOOL_8
tapdev_init(void)
{
  /* EMAC configuration type */
  EMAC_CFG_Type emac_config;
  /* pin configuration */
  PINSEL_CFG_Type pincfg;
  /* EMAC address */
  uint8_t emac_addr[] =
  { EMAC_ADDR0, EMAC_ADDR1, EMAC_ADDR2, EMAC_ADDR3, EMAC_ADDR4, EMAC_ADDR5 };

#if AUTO_NEGOTIATION_ENA != 0
  emac_config.Mode = EMAC_MODE_AUTO;
#else
#if (FIX_SPEED == SPEED_100)
#if (FIX_DUPLEX == FULL_DUPLEX)
  emac_config.Mode = EMAC_MODE_100M_FULL;
#elif (FIX_DUPLEX == HALF_DUPLEX)
  emac_config.Mode = EMAC_MODE_100M_HALF;
#else
#error Does not support this duplex option
#endif
#elif (FIX_SPEED == SPEED_10)
#if (FIX_DUPLEX == FULL_DUPLEX)
  emac_config.Mode = EMAC_MODE_10M_FULL;
#elif (FIX_DUPLEX == HALF_DUPLEX)
  emac_config.Mode = EMAC_MODE_10M_HALF;
#else
#error Does not support this duplex option
#endif
#else
#error Does not support this speed option
#endif
#endif

  /*
   * Enable P1 Ethernet Pins:
   * P1.0 - ENET_TXD0
   * P1.1 - ENET_TXD1
   * P1.4 - ENET_TX_EN
   * P1.8 - ENET_CRS
   * P1.9 - ENET_RXD0
   * P1.10 - ENET_RXD1
   * P1.14 - ENET_RX_ER
   * P1.15 - ENET_REF_CLK
   * P1.16 - ENET_MDC
   * P1.17 - ENET_MDIO
   */
  pincfg.Funcnum = 1;
  pincfg.OpenDrain = 0;
  pincfg.Pinmode = 0;
  pincfg.Portnum = 1;

  pincfg.Pinnum = 0;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 1;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 4;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 8;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 9;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 10;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 14;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 15;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 16;
  PINSEL_ConfigPin(&pincfg);
  pincfg.Pinnum = 17;
  PINSEL_ConfigPin(&pincfg);

  /* printf("Init EMAC module\n"); */
  /* printf("MAC addr: %X-%X-%X-%X-%X-%X \n", emac_addr[0], emac_addr[1], */
  /*    emac_addr[2], emac_addr[3], emac_addr[4], emac_addr[5]); */

  emac_config.Mode = EMAC_MODE_AUTO;
  emac_config.pbEMAC_Addr = emac_addr;
  /* Initialize EMAC module with given parameter */
  if(EMAC_Init(&emac_config) == ERROR) {
    return FALSE;
    /* Disable the TX_DONE interrupt enabled by EMAC_Init */
  }
  EMAC_IntCmd(EMAC_INT_TX_DONE, DISABLE);
  /* Set interrupt priority to 1, the second-highest */
  NVIC_SetPriority(ENET_IRQn, 1);
  /* Enable the ENET interruption */
  NVIC_EnableIRQ(ENET_IRQn);

  /* printf("Init EMAC complete\n"); */

  return TRUE;
}
/* receive an Ethernet frame from MAC/DMA controller */
UNS_32
tapdev_read(void *packet_ptr)
{
  UNS_32 packet_size = EMAC_MAX_PACKET_SIZE;
  UNS_32 in_size;
  EMAC_PACKETBUF_Type packet_rx;

  /* Check Receive status */
  if(EMAC_CheckReceiveIndex() == FALSE) {
    return 0;
  }
  /* Get size of receive data */
  in_size = EMAC_GetReceiveDataSize() + 1;

  packet_size = MIN(packet_size, in_size);

  /* Setup Rx packet */
  packet_rx.pbDataBuf = (uint32_t *)packet_ptr;
  packet_rx.ulDataLen = packet_size;
  EMAC_ReadPacketBuffer(&packet_rx);

  /* update receive status */
  EMAC_UpdateRxConsumeIndex();
  return packet_size;
}
/* transmit an Ethernet frame to MAC/DMA controller */
BOOL_8
tapdev_send(void *packet_ptr, UNS_32 size)
{
  EMAC_PACKETBUF_Type packet_tx;

  /* Check size */
  if(size == 0) {
    return TRUE;
  }
  /* check Tx Slot is available */
  if(EMAC_CheckTransmitIndex() == FALSE) {
    return FALSE;
  }
  size = MIN(size, EMAC_MAX_PACKET_SIZE);

  /* Setup Tx Packet buffer */
  packet_tx.ulDataLen = size;
  packet_tx.pbDataBuf = (uint32_t *)packet_ptr;
  EMAC_WritePacketBuffer(&packet_tx);
  EMAC_UpdateTxProduceIndex();

  return TRUE;
}
/* Interrupt function for the Ethernet module */
/* We only enable the RX_DONE interrupt */
void
ENET_IRQHandler(void)
{
  /* Check which interrupt source brought us here and clear the flag */
  /* If a packet arrived, we inform the Ethernet driver */

  /* Receive Done */
  if(EMAC_IntGetStatus(EMAC_INT_RX_DONE)) {
    poll_eth_driver();
  }
}
/*
 * @}
 */

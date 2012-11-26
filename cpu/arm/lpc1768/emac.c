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

#define DB	_DBG((uint8_t *)db)
char db[64];

/* Init the LPC17xx ethernet */
BOOL_8
tapdev_init(void)
{
  /* EMAC configuration type */
  EMAC_CFG_Type Emac_Config;
  /* pin configuration */
  PINSEL_CFG_Type PinCfg;
  /* EMAC address */
  uint8_t EMACAddr[] =
    { EMAC_ADDR0, EMAC_ADDR1, EMAC_ADDR2, EMAC_ADDR3, EMAC_ADDR4, EMAC_ADDR5 };

#if AUTO_NEGOTIATION_ENA != 0
  Emac_Config.Mode = EMAC_MODE_AUTO;
#else
#if (FIX_SPEED == SPEED_100)
#if (FIX_DUPLEX == FULL_DUPLEX)
  Emac_Config.Mode = EMAC_MODE_100M_FULL;
#elif (FIX_DUPLEX == HALF_DUPLEX)
  Emac_Config.Mode = EMAC_MODE_100M_HALF;
#else
#error Does not support this duplex option
#endif
#elif (FIX_SPEED == SPEED_10)
#if (FIX_DUPLEX == FULL_DUPLEX)
  Emac_Config.Mode = EMAC_MODE_10M_FULL;
#elif (FIX_DUPLEX == HALF_DUPLEX)
  Emac_Config.Mode = EMAC_MODE_10M_HALF;
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
  PinCfg.Funcnum = 1;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Portnum = 1;

  PinCfg.Pinnum = 0;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 1;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 4;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 8;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 9;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 10;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 14;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 15;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 16;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 17;
  PINSEL_ConfigPin(&PinCfg);

  printf("Init EMAC module\n");
  printf("MAC addr: %X-%X-%X-%X-%X-%X \n", EMACAddr[0], EMACAddr[1],
      EMACAddr[2], EMACAddr[3], EMACAddr[4], EMACAddr[5]);

  Emac_Config.Mode = EMAC_MODE_AUTO;
  Emac_Config.pbEMAC_Addr = EMACAddr;
  // Initialize EMAC module with given parameter
  if (EMAC_Init(&Emac_Config) == ERROR)
    {
      return (FALSE);
    }

  printf("Init EMAC complete\n");

  return (TRUE);
}

/* receive an Ethernet frame from MAC/DMA controller */
UNS_32
tapdev_read(void * pPacket)
{
  UNS_32 Size = EMAC_MAX_PACKET_SIZE;
  UNS_32 in_size;
  EMAC_PACKETBUF_Type RxPack;

  // Check Receive status
  if (EMAC_CheckReceiveIndex() == FALSE)
    {
      return (0);
    }

  // Get size of receive data
  in_size = EMAC_GetReceiveDataSize() + 1;

  Size = MIN(Size,in_size);

  // Setup Rx packet
  RxPack.pbDataBuf = (uint32_t *) pPacket;
  RxPack.ulDataLen = Size;
  EMAC_ReadPacketBuffer(&RxPack);

  // update receive status
  EMAC_UpdateRxConsumeIndex();
  return (Size);
}

/* transmit an Ethernet frame to MAC/DMA controller */
BOOL_8
tapdev_send(void *pPacket, UNS_32 size)
{
  EMAC_PACKETBUF_Type TxPack;

  // Check size
  if (size == 0)
    {
      return (TRUE);
    }

  // check Tx Slot is available
  if (EMAC_CheckTransmitIndex() == FALSE)
    {
      return (FALSE);
    }

  size = MIN(size,EMAC_MAX_PACKET_SIZE);

  // Setup Tx Packet buffer
  TxPack.ulDataLen = size;
  TxPack.pbDataBuf = (uint32_t *) pPacket;
  EMAC_WritePacketBuffer(&TxPack);
  EMAC_UpdateTxProduceIndex();

  return (TRUE);
}

//Interrupt function for the Ethernet module
void
ENET_IRQHandler(void)
{
  //Check which interrupt source brought us here and clear the flag
  //If a packet arrived, we inform the Ethernet driver

  //Receive buffer overrun
  if (EMAC_IntGetStatus(EMAC_INT_RX_OVERRUN))
    {

    }

  //Receive error
  if (EMAC_IntGetStatus(EMAC_INT_RX_ERR))
    {

    }

  //Receive Descriptor Finish
  if (EMAC_IntGetStatus(EMAC_INT_RX_FIN))
    {

    }

  //Receive Done
  if (EMAC_IntGetStatus(EMAC_INT_RX_DONE))
    {
      poll_eth_driver();
    }

  //Transmit Under-run
  if (EMAC_IntGetStatus(EMAC_INT_TX_UNDERRUN))
    {

    }

  //Transmit Error
  if (EMAC_IntGetStatus(EMAC_INT_TX_ERR))
    {

    }

  //Transmit Descriptor Finish
  if (EMAC_IntGetStatus(EMAC_INT_TX_FIN))
    {

    }

  //Transmit Done
  if (EMAC_IntGetStatus(EMAC_INT_TX_FIN))
    {
      poll_eth_driver();
    }

  //Software Interrupt
  if (EMAC_IntGetStatus(EMAC_INT_SOFT_INT))
    {

    }

  //Wakeup interrupt
  if (EMAC_IntGetStatus(EMAC_INT_WAKEUP))
    {

    }
}

/*
 * @}
 */

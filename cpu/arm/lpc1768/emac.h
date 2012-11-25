#ifndef __EMAC_H
#define __EMAC_H

#include "lpc_types.h"
#include "lpc17xx_emac.h"
#include "contiki-conf.h"

#define EMAC_MAX_PACKET_SIZE (UIP_CONF_BUFFER_SIZE + 16)	// 1536 bytes
#define ENET_DMA_DESC_NUMB   	3
#define AUTO_NEGOTIATION_ENA 	1  		// Enable PHY Auto-negotiation
#define PHY_TO               	200000  // ~10sec
#define RMII                    1		// If zero, it's a MII interface

/* Configurable macro ---------------------- */
#define SPEED_100               1
#define SPEED_10                0
#define FULL_DUPLEX             1
#define HALF_DUPLEX             0

#define FIX_SPEED               SPEED_100
#define FIX_DUPLEX              FULL_DUPLEX

BOOL_8
tapdev_init(void);
UNS_32
tapdev_read(void * pPacket);
BOOL_8
tapdev_send(void *pPacket, UNS_32 size);

#endif

#ifndef __RTL8019DEV_H__
#define __RTL8019DEV_H__

/*****************************************************************************
*  Module Name:       Realtek 8019AS Driver Interface for uIP-AVR Port
*
*  Created By:        Louis Beaudoin (www.embedded-creations.com)
*
*  Original Release:  September 21, 2002
*
*  Module Description:
*  Provides three functions to interface with the Realtek 8019AS driver
*  These functions can be called directly from the main uIP control loop
*  to send packets from uip_buf and uip_appbuf, and store incoming packets to
*  uip_buf
*
*  September 30, 2002 - Louis Beaudoin
*    Modifications required to handle the packet receive function changes in
*      rtl8019.c.  There is no longer a need to poll for an empty buffer or
*      an overflow.
*    Added support for the Imagecraft Compiler
*
*****************************************************************************/

#include "net/uip.h"
#include "dev/rtl8019.h"

/*****************************************************************************
*  RTL8019dev_init()
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Power-up initialization of the RTL8019
*****************************************************************************/
void RTL8019dev_init(void);


/*****************************************************************************
*  RTL8019dev_send()
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Sends the packet contained in uip_buf and uip_appdata over
*                 the network
*****************************************************************************/
void RTL8019dev_send(void);


/*****************************************************************************
*  unsigned char/int RTL8019dev_poll()
*  Returns:     Length of the packet retreived, or zero if no packet retreived
*  Created By:  Louis Beaudoin
*  Date:        September 21, 2002
*  Description: Polls the RTL8019 looking for an overflow condition or a new
*                 packet in the receive buffer.  If a new packet exists and
*                 will fit in uip_buf, it is retreived, and the length is
*                 returned.  A packet bigger than the buffer is discarded
*****************************************************************************/
unsigned int RTL8019dev_poll(void);


/*****************************************************************************
*  RTL8019dev_exit()
*  Created By:  -
*  Date:        -
*  Description: Final shutdown of the RTL8019
*****************************************************************************/
void RTL8019dev_exit(void);


#endif /* __RTL8019DEV_H__ */

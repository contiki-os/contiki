/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file rndis_task.h *********************************************************
 *
 * \brief
 *      This file manages the RNDIS task
 *
 * \addtogroup usbstick
 *
 * \author
 *      Colin O'Flynn
 *
 ******************************************************************************/
/* Copyright (c) 2008  ATMEL Corporation
   Copyright (c) 2008 Colin O'Flynn
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef RNDIS_TASK_H_
#define RNDIS_TASK_H_

/**
 \addtogroup RNDIS
 @{
 */

//_____ I N C L U D E S ____________________________________________________


#include "config.h"
#include "../conf_usb.h"

//_____ M A C R O S ________________________________________________________

#define USB_ETH_MTU (UIP_BUFSIZE + 4)


/*! Hook Documentation
**	USB_ETH_HOOK_RX_START()
**	USB_ETH_HOOK_RX_END()
**	USB_ETH_HOOK_RX_ERROR(string_reason)
**
**	USB_ETH_HOOK_TX_START()
**	USB_ETH_HOOK_TX_END()
**	USB_ETH_HOOK_TX_ERROR(string_reason)
**
**	USB_ETH_HOOK_INITIALIZED()
**	USB_ETH_HOOK_UNINITIALIZED()
**
**	USB_ETH_HOOK_INIT()
**
**	USB_ETH_HOOK_SET_PROMISCIOUS_MODE(bool)
**
**	USB_ETH_HOOK_HANDLE_INBOUND_PACKET(buffer,len)
**	USB_ETH_HOOK_IS_READY_FOR_INBOUND_PACKET()
*/



#ifndef USB_ETH_HOOK_RX_START
void rx_start_led(void);
#define USB_ETH_HOOK_RX_START()	rx_start_led()
#endif

#ifndef USB_ETH_HOOK_TX_END
void tx_end_led(void);
#define USB_ETH_HOOK_TX_END()	tx_end_led()
#endif



#ifndef USB_ETH_HOOK_TX_ERROR
#define USB_ETH_HOOK_TX_ERROR(string)	do { } while(0)
#endif

#ifndef USB_ETH_HOOK_RX_ERROR
#define USB_ETH_HOOK_RX_ERROR(string)	do { } while(0)
#endif

//_____ D E C L A R A T I O N S ____________________________________________


uint8_t usb_eth_send(uint8_t * senddata, uint16_t sendlen, uint8_t led);
uint8_t usb_eth_set_active(uint8_t active);
uint8_t usb_eth_ready_for_next_packet();

void sof_action(void);

extern uint8_t usb_eth_is_active;

// TIP: Avoid using usb_ethernet_addr directly and use the get/set mac_address functions below.
extern uint64_t usb_ethernet_addr;
void usb_eth_get_mac_address(uint8_t dest[6]);
void usb_eth_set_mac_address(const uint8_t src[6]);

PROCESS_NAME(usb_eth_process);

/** @} */

#endif /*RNDIS_TASK_H_ */


/**
 * \file rndis.c
 *         RNDIS Functions for mounting USB device as network interface
 *
 * \author
 *         Colin O'Flynn <coflynn@newae.com>
 *
 * \addtogroup usbstick
 */
/* Copyright (c) 2008  Colin O'Flynn

   The CDC code which this is based on is Copyright (c) Atmel Corporation 2008
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

/*
RNDIS Status Information:

802.3 Support:
    More or less working	
 
802.11 Support:
	Incomplete, would just error out if you tried probably


*/

/**
  \addtogroup RNDIS
  @{
  */
 

//_____ I N C L U D E S ____________________________________________________

#include <stdbool.h>
#include "radio.h"
#include "contiki.h"
#include "config.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include "rndis/rndis_protocol.h"
#include "rndis/rndis_task.h"
#include "uip.h"
#include "serial/uart_usb_lib.h"
#include "sicslow_ethernet.h"
#include <avr/pgmspace.h>
#include <string.h>
#if RF230BB
#include "rf230bb.h"
#endif

#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N ________________________________________________

//_____ P R I V A T E   D E C L A R A T I O N ______________________________

extern PGM_VOID_P pbuffer;
extern U8   data_to_transfer;

/**
 * \brief List of supported RNDIS OID's
 */
const uint32_t OIDSupportedList[] PROGMEM = {
		/* Required General */
        OID_GEN_SUPPORTED_LIST            ,
        OID_GEN_HARDWARE_STATUS           ,
        OID_GEN_MEDIA_SUPPORTED           ,
        OID_GEN_MEDIA_IN_USE              ,
        OID_GEN_MAXIMUM_FRAME_SIZE        ,
        OID_GEN_LINK_SPEED                ,
        OID_GEN_TRANSMIT_BLOCK_SIZE       ,
        OID_GEN_RECEIVE_BLOCK_SIZE        ,
        OID_GEN_VENDOR_ID                 ,
        OID_GEN_VENDOR_DESCRIPTION        ,
        OID_GEN_CURRENT_PACKET_FILTER     ,
        OID_GEN_MAXIMUM_TOTAL_SIZE        ,
        OID_GEN_MEDIA_CONNECT_STATUS      ,
        OID_GEN_VENDOR_DRIVER_VERSION     ,
        OID_GEN_PHYSICAL_MEDIUM			  ,

		/* Required Statistical */
        OID_GEN_XMIT_OK                   ,
        OID_GEN_RCV_OK                    ,
        OID_GEN_XMIT_ERROR                ,
        OID_GEN_RCV_ERROR                 ,
        OID_GEN_RCV_NO_BUFFER             ,

		/* Please configure us        */
		OID_GEN_RNDIS_CONFIG_PARAMETER    ,


		/* IEEE 802.3 (Ethernet) OIDs */
        OID_802_3_PERMANENT_ADDRESS       ,
        OID_802_3_CURRENT_ADDRESS         ,
        OID_802_3_MULTICAST_LIST          ,
        OID_802_3_MAXIMUM_LIST_SIZE       ,
        OID_802_3_MAC_OPTIONS             ,
        OID_802_3_RCV_ERROR_ALIGNMENT     ,
        OID_802_3_XMIT_ONE_COLLISION      ,
        OID_802_3_XMIT_MORE_COLLISIONS    ,

/*802.11 OID's not fully implemented yet. Hence do not say we
  support them */
#if USB_ETH_EMULATE_WIFI
		/* 802.11 OIDs */
        OID_802_11_BSSID                  ,
        OID_802_11_SSID                   ,
        OID_802_11_NETWORK_TYPE_IN_USE    ,
        OID_802_11_RSSI                   ,
        OID_802_11_BSSID_LIST             ,
        OID_802_11_BSSID_LIST_SCAN        ,
        OID_802_11_INFRASTRUCTURE_MODE    ,
        OID_802_11_SUPPORTED_RATES        ,
        OID_802_11_CONFIGURATION          ,
        OID_802_11_ADD_WEP                ,
        OID_802_11_WEP_STATUS             ,
        OID_802_11_REMOVE_WEP             ,
        OID_802_11_DISASSOCIATE           ,
        OID_802_11_AUTHENTICATION_MODE    ,
        OID_802_11_RELOAD_DEFAULTS        ,
#endif

		/* Minimum power managment needed for USB */

		OID_PNP_CAPABILITIES              ,
		OID_PNP_QUERY_POWER               ,
		OID_PNP_SET_POWER                 ,

		OID_PNP_ENABLE_WAKE_UP            ,
		OID_PNP_ADD_WAKE_UP_PATTERN       ,
		OID_PNP_REMOVE_WAKE_UP_PATTERN
};

#define OID_LIST_LENGTH sizeof(OIDSupportedList)/sizeof(*OIDSupportedList)


rndis_state_t rndis_state;

usb_eth_stat_t usb_eth_stat;

uint8_t schedule_interrupt = 0;

//_____ D E C L A R A T I O N ______________________________________________


void rndis_packetFilter(uint32_t newfilter);

/******** RNDIS ********/

#define ENC_BUF_SIZE    (4*OID_LIST_LENGTH + 32)

// Command buffer
U8 encapsulated_buffer[ENC_BUF_SIZE];

//Do we have data to send back?
U8 data_to_send = 0x00;

/**
 * \brief Handles a "SEND ENCAPSULATED COMMAND" message.
 *
 * \return True on success, false on failure.
 */
uint8_t rndis_send_encapsulated_command(uint16_t wLength)
{
	U8 i = 0;

	//Received setup message OK
	Usb_ack_receive_setup();


	//Crude bounds check
	if (wLength > ENC_BUF_SIZE)
		wLength = ENC_BUF_SIZE;

	//Read in all the bytes...

	uint8_t nb_counter;

    //Clear NAK bit
    Usb_ack_nak_in();

    while (wLength) {
        nb_counter = EP_CONTROL_LENGTH;

        //Wait for data to come in or nak
        while((!Is_usb_receive_out()) & (!Is_usb_receive_nak_in()));

        //Received OUT
        if (Is_usb_receive_out()) {
            while(nb_counter && wLength) {
                encapsulated_buffer[i] = Usb_read_byte();
                i++;
                wLength--;
                nb_counter--;
            }

            Usb_ack_receive_out();

            //Received NAK, no more data
            } else  {
                Usb_ack_nak_in();
                break;
            }
    }

	Usb_send_control_in();


	switch(((rndis_generic_msg_t *)encapsulated_buffer)->MessageType)
		{
		/* Requests remote intilization. Respond with complete,
		   eventually should probably do something */
		case REMOTE_NDIS_INITIALIZE_MSG:
				{				
				rndis_initialize_cmplt_t * m;
				m = ((rndis_initialize_cmplt_t *)encapsulated_buffer);
	
				//m->MessageID is same as before
				m->MessageType = REMOTE_NDIS_INITIALIZE_CMPLT;
				m->MessageLength = sizeof(rndis_initialize_cmplt_t);
				m->MajorVersion = RNDIS_MAJOR_VERSION;
				m->MinorVersion = RNDIS_MAJOR_VERSION;
				m->Status = RNDIS_STATUS_SUCCESS;
				m->DeviceFlags = RNDIS_DF_CONNECTIONLESS;
#if USB_ETH_EMULATE_WIFI
				m->Medium = NDIS_MEDIUM_WIRELESS_LAN;
#else
				m->Medium = RNDIS_MEDIUM_802_3;
#endif // USB_ETH_EMULATE_WIFI
				m->MaxPacketsPerTransfer = 1;
				m->MaxTransferSize = USB_ETH_MTU + 58; /* Space for 1280 IP buffer, Ethernet Header, 
				                              RNDIS messages */
				m->PacketAlignmentFactor = 3;
				m->AfListOffset = 0;
				m->AfListSize = 0;

				rndis_state = rndis_initialized;

				data_to_send = m->MessageLength;
				}
			break;
		case REMOTE_NDIS_HALT_MSG:

			Led0_on();
			Led1_on();
			Led2_on();
			Led3_on();


			Usb_send_control_in();

			while(1);

			
			break;

		case REMOTE_NDIS_QUERY_MSG:
				rndis_query_process();
			break;

		case REMOTE_NDIS_SET_MSG:
				{
				rndis_set_process();
				}
			break;
	
		case REMOTE_NDIS_RESET_MSG:
				{
				rndis_reset_cmplt_t * m;
				m = ((rndis_reset_cmplt_t *)encapsulated_buffer);
	
				rndis_state = rndis_uninitialized;

				m->MessageType = REMOTE_NDIS_RESET_CMPLT;
				m->MessageLength = sizeof(rndis_reset_cmplt_t);
				m->Status = RNDIS_STATUS_SUCCESS;
				m->AddressingReset = 1; /* Make it look like we did something */
			//	m->AddressingReset = 0; //Windows halts if set to 1 for some reason
				data_to_send = m->MessageLength;				
				}
			break;

		case REMOTE_NDIS_KEEPALIVE_MSG:
				{
				rndis_keepalive_cmplt_t * m;
				m = (rndis_keepalive_cmplt_t *)encapsulated_buffer;
				m->MessageType = REMOTE_NDIS_KEEPALIVE_CMPLT;
				m->MessageLength = sizeof(rndis_keepalive_cmplt_t);
				m->Status = RNDIS_STATUS_SUCCESS;

				//We have data to send back
				data_to_send = m->MessageLength;
			break;
			}

		default:
			Led2_on();
			return FALSE;
			break;
		}

		while(!(Is_usb_read_control_enabled()));

		if (Is_usb_receive_out()) Usb_ack_receive_out();

		rndis_send_interrupt();

		return TRUE;
}

/**
 * \brief Send an interrupt over the interrupt endpoint to the host.
 */
void rndis_send_interrupt(void)
	{
	
	//Schedule the interrupt to take place next
	//time USB task is run
	schedule_interrupt = 1;	
	}

#define INFBUF ((uint32_t *)(encapsulated_buffer + sizeof(rndis_query_cmplt_t)))

uint32_t oid_packet_filter = 0x0000000;

#if USB_ETH_EMULATE_WIFI
uint16_t panid = 0xbaad;
#endif

/**
 * \brief Function to handle a RNDIS "QUERY" command in the encapsulated_buffer
 */
void rndis_query_process(void)
	{
	rndis_query_msg_t * m;
	rndis_query_cmplt_t * c;
	rndis_Status_t status = RNDIS_STATUS_SUCCESS;

	m = (rndis_query_msg_t *)encapsulated_buffer;
	c = (rndis_query_cmplt_t *)encapsulated_buffer;

	/* We set up packet for sending one 4-byte response, which a lot of
	   these will do. If you need more or less just change the defaults in
	   the specific case */

	c->MessageType = REMOTE_NDIS_QUERY_CMPLT;
	//c->Status DO NOT SET YET - as m->Oid resides in this space. We still need it...
	c->InformationBufferLength = 4;
	c->InformationBufferOffset = 16;	

	switch (m->Oid) {

		/**** GENERAL ****/
        case OID_GEN_SUPPORTED_LIST:
			c->InformationBufferLength = 4 * OID_LIST_LENGTH;

			//Copy data to SRAM
			memcpy_P(INFBUF, OIDSupportedList, 4*OID_LIST_LENGTH);
			break;

        case OID_GEN_HARDWARE_STATUS:       
			*INFBUF = 0x00000000; /* Ready and Willing */  
			break;

        case OID_GEN_MEDIA_SUPPORTED:           
        case OID_GEN_MEDIA_IN_USE: 
        case OID_GEN_PHYSICAL_MEDIUM:			
#if USB_ETH_EMULATE_WIFI
			*INFBUF = NDIS_MEDIUM_WIRELESS_LAN;
#else
			*INFBUF = NDIS_MEDIUM_802_3;
#endif
			break;	
		
        case OID_GEN_MAXIMUM_FRAME_SIZE:
			*INFBUF = (uint32_t) USB_ETH_MTU-14; //1280 //102; /* Assume 25 octet header on 15.4 */
			break;

        case OID_GEN_LINK_SPEED:            
			*INFBUF = (uint32_t) 250000/100; /* in 100 bytes/sec units.. this is kinda a lie */
			break; 

        case OID_GEN_TRANSMIT_BLOCK_SIZE:       
        case OID_GEN_RECEIVE_BLOCK_SIZE:   
			*INFBUF = (uint32_t) 102; 
			break;
			     
        case OID_GEN_VENDOR_ID:
			*INFBUF = 0xFFFFFF; /* No vendor ID ! */
			break;
			                 
        case OID_GEN_VENDOR_DESCRIPTION:        
			c->InformationBufferLength = 8;
			memcpy_P(INFBUF, PSTR("Atmel\0\0\0\0"), 8);
			break;

        case OID_GEN_CURRENT_PACKET_FILTER:     
			*INFBUF =  oid_packet_filter;
			break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:        
			*INFBUF = (uint32_t) USB_ETH_MTU; //127; 
			break;

        case OID_GEN_MEDIA_CONNECT_STATUS: 
			*INFBUF = usb_eth_is_active?NDIS_MEDIA_STATE_CONNECTED:NDIS_MEDIA_STATE_DISCONNECTED;
			break;
			     
        case OID_GEN_VENDOR_DRIVER_VERSION:     
			*INFBUF = 0x00001000;
			break;


		case OID_GEN_CURRENT_LOOKAHEAD:
			*INFBUF = (uint32_t) USB_ETH_MTU-14; //102;

//		case OID_GEN_RNDIS_CONFIG_PARAMETER:
//			break;

		/******* 802.3 (Ethernet) *******/

		/*The address of the NIC encoded in the hardware.*/
       	case OID_802_3_PERMANENT_ADDRESS:
        case OID_802_3_CURRENT_ADDRESS: 		

			//Clear unused bytes
            *(INFBUF + 1) = 0;

			//get address
			usb_eth_get_mac_address((uint8_t*)INFBUF);
			
			/*4+2 = 6 Bytes of eth address */
			c->InformationBufferLength += 2;
			break;

		/* The multicast address list on the NIC enabled for packet reception. */
        case OID_802_3_MULTICAST_LIST:  			
			*INFBUF = 0xE000000;
			break;
			        
		/* The maximum number of multicast addresses the NIC driver can manage. */
        case OID_802_3_MAXIMUM_LIST_SIZE:
			*INFBUF = 1;
			break;

		/* Features supported by the underlying driver, which could be emulating Ethernet. */
        case OID_802_3_MAC_OPTIONS:      
			*INFBUF = 0;
			break;

		/* Frames received with alignment error */
        case OID_802_3_RCV_ERROR_ALIGNMENT:
		/* Frames transmitted with one collision */
        case OID_802_3_XMIT_ONE_COLLISION:    
		/* Frames transmitted with more than one collision */
        case OID_802_3_XMIT_MORE_COLLISIONS:
			*INFBUF = 0;
			break;

#if USB_ETH_EMULATE_WIFI
		/*** 802.11 OIDs ***/
        case OID_802_11_BSSID:   
			*INFBUF = (uint32_t)panid;
			*(INFBUF + 1) = 0;
			
			/*4+2 = 6 Bytes of eth address */
			c->InformationBufferLength += 2;
			break;

        case OID_802_11_SSID:       
			/* Our SSID is *always* "PANID: 0xXXXX", length = 13 */
			*INFBUF = 13;

			strncpy_P((char*)(INFBUF + 1), PSTR("PANID: 0xBAAD"), 13);
			break;
		            
        case OID_802_11_NETWORK_TYPE_IN_USE:    
			*INFBUF = 0; /* Ndis802_11FH - it's all lies anyway */
			break;

        case OID_802_11_RSSI:
			*((int32_t *) INFBUF) = -20; //-20 dBm
			break;

        case OID_802_11_BSSID_LIST:  
			break;

		/* todo: */
        case OID_802_11_INFRASTRUCTURE_MODE:
        case OID_802_11_SUPPORTED_RATES:   
        case OID_802_11_CONFIGURATION: 
        case OID_802_11_WEP_STATUS:   
        case OID_802_11_AUTHENTICATION_MODE:
			break;
#endif //USB_ETH_EMULATE_WIFI
  
		/*** Statistical ***/

		/* Frames transmitted without errors */
		case OID_GEN_XMIT_OK:    
			*INFBUF = usb_eth_stat.txok;
			break;
		
		/* Frames received without errors */              
		case OID_GEN_RCV_OK:               
			*INFBUF = usb_eth_stat.rxok;
			break;
		
		/* Frames received with errors */             
		case OID_GEN_RCV_ERROR:            
			*INFBUF = usb_eth_stat.rxbad;
			break;

		/* Frames transmitted with errors */
		case OID_GEN_XMIT_ERROR: 
		    *INFBUF = usb_eth_stat.txbad;  
			break;

		/* Frames dropped due to lack of buffer space */
		case OID_GEN_RCV_NO_BUFFER:

			*INFBUF = 0; /* Lies! */
			break;

		/*** Power Managment ***/
		case OID_PNP_CAPABILITIES:
			c->InformationBufferLength = sizeof(struct NDIS_PM_WAKE_UP_CAPABILITIES);

			//Sorry, I don't play ball. Power managment is for hippies
			memset((char *)INFBUF, 0, sizeof(struct NDIS_PM_WAKE_UP_CAPABILITIES));
			break;

		case OID_PNP_QUERY_POWER:
			c->InformationBufferLength = 0;
			break;		

		case OID_PNP_ENABLE_WAKE_UP:
			*INFBUF = 0; /* Nothing Supported */
			break;

		default:
			status = RNDIS_STATUS_FAILURE;
			c->InformationBufferLength = 0;
			break;


	}

	//Set Status now that we are done with Oid
	c->Status = status;

	//Calculate message size
	c->MessageLength = sizeof (rndis_query_cmplt_t) + c->InformationBufferLength;

	//Check if we are sending no information buffer
	if (c->InformationBufferLength == 0) {
		c->InformationBufferOffset = 0;
	}
	
	//Set it up
	data_to_send = c->MessageLength;
	}


#undef INFBUF
#define INFBUF ((uint32_t *)((uint8_t *)&(m->RequestId) + m->InformationBufferOffset))
#define CFGBUF ((rndis_config_parameter_t *) INFBUF)
#define PARMNAME  ((uint8_t *)CFGBUF + CFGBUF->ParameterNameOffset)
#define PARMVALUE ((uint8_t *)CFGBUF + CFGBUF->ParameterValueOffset)
#define PARMVALUELENGTH	CFGBUF->ParameterValueLength
#define PARM_NAME_LENGTH 25 /* Maximum parameter name length */

void
rndis_handle_config_parm(const char* parmname,const uint8_t* parmvalue,size_t parmlength) {
	if (strncmp_P(parmname, PSTR("rawmode"), 7) == 0) {
		if (parmvalue[0] == '0') {
			usbstick_mode.raw = 0;
		} else {
			usbstick_mode.raw = 1;
		}
	}

}

/**
 * \brief Function to deal with a RNDIS "SET" command present in the
 *        encapsulated_buffer
 */
void rndis_set_process(void)
	{
	rndis_set_cmplt_t * c;
	rndis_set_msg_t * m;

	c = ((rndis_set_cmplt_t *)encapsulated_buffer);
	m = ((rndis_set_msg_t *)encapsulated_buffer);

	//Never have longer parameter names than PARM_NAME_LENGTH
	char parmname[PARM_NAME_LENGTH];
	
	uint8_t i;
	int8_t parmlength;

	/* The parameter name seems to be transmitted in uint16_t, but
	   we want this in uint8_t. Hence have to throw out some info...  */
	if (CFGBUF->ParameterNameLength > (PARM_NAME_LENGTH*2)) {
		parmlength = PARM_NAME_LENGTH * 2;
	} else {
		parmlength = CFGBUF->ParameterNameLength;
	}

	i = 0;
	while(parmlength > 0) {
		//Convert from uint16_t to char array. 
		parmname[i] = (char)*(PARMNAME + 2*i);
		parmlength -= 2;
		i++;
	}
	

	switch(m->Oid) {

	/* Parameters set up in 'Advanced' tab */
		case OID_GEN_RNDIS_CONFIG_PARAMETER:
			/* Parameter name: rawmode
			   Parameter desc: Enables or disable raw capture of 802.15.4 Packets
			   Parameter type: single octet
			   Parameter values: '0' = disabled, '1' = enabled
			*/
			rndis_handle_config_parm(parmname,PARMVALUE,PARMVALUELENGTH);


			break;

		/* Mandatory general OIDs */
		case OID_GEN_CURRENT_PACKET_FILTER:
			oid_packet_filter = *INFBUF;

			if (oid_packet_filter) {

				rndis_packetFilter(oid_packet_filter);

				rndis_state = rndis_data_initialized;
			} else {
				rndis_state = rndis_initialized;
			}

			break;

		case OID_GEN_CURRENT_LOOKAHEAD:
			break;

		case OID_GEN_PROTOCOL_OPTIONS:
			break;

		/* Mandatory 802_3 OIDs */
		case OID_802_3_MULTICAST_LIST:
			break;

#if USB_ETH_EMULATE_WIFI
		/* Mandatory 802.11 OIDs */
		case OID_802_11_BSSID: 
			panid = *INFBUF;
			break;

		case OID_802_11_SSID:
			break;
			//TODO: rest of 802.11
#endif // USB_ETH_EMULATE_WIFI

		/* Power Managment: fails for now */
		case OID_PNP_ADD_WAKE_UP_PATTERN:
		case OID_PNP_REMOVE_WAKE_UP_PATTERN:
		case OID_PNP_ENABLE_WAKE_UP:

		default:
			//c->MessageID is same as before
			c->MessageType = REMOTE_NDIS_SET_CMPLT;
			c->MessageLength = sizeof(rndis_set_cmplt_t);
			c->Status = RNDIS_STATUS_FAILURE;
			data_to_send = c->MessageLength;
			return;

			break;
	}
	
	//c->MessageID is same as before
	c->MessageType = REMOTE_NDIS_SET_CMPLT;
	c->MessageLength = sizeof(rndis_set_cmplt_t);
	c->Status = RNDIS_STATUS_SUCCESS;
	data_to_send = c->MessageLength;
	return;
	}

/**
 * \brief Handle "GET ENCAPSULATED COMMAND"
 *
 * \return True on success, false on failure.
 *
 *  This function assumes the message has already set up in
 * the "encapsulated_buffer" variable. This will be done by
 * the "SEND ENCAPSULATED COMMAND" message, which will trigger
 * and interrupt on the host so it knows data is ready.
 */
uint8_t rndis_get_encapsulated_command(void)
	{
	U8 nb_byte, zlp, i;

	//We assume this is already set up...

	//Received setup message OK	
	Usb_ack_receive_setup();

     if ((data_to_send % EP_CONTROL_LENGTH) == 0) { zlp = TRUE; }
      else { zlp = FALSE; }                   //!< no need of zero length packet


	i = 0;
   while((data_to_send != 0) && (!Is_usb_receive_out()))
   {
      while(!Is_usb_read_control_enabled());

      nb_byte=0;
      while(data_to_send != 0)        //!< Send data until necessary
      {
         if(nb_byte++==EP_CONTROL_LENGTH) //!< Check endpoint 0 size
         {
            break;
         }
         Usb_write_byte(encapsulated_buffer[i]);
		 i++;
         data_to_send--;

      }
      Usb_send_control_in();
   }

   if(Is_usb_receive_out()) { Usb_ack_receive_out(); return TRUE; } //!< abort from Host
   
   if(zlp == TRUE)
   {
     while(!Is_usb_read_control_enabled());
     Usb_send_control_in();
   }

   while(!Is_usb_receive_out());
   Usb_ack_receive_out();

   return TRUE;
   }


/**
 * \brief Send a status packet back to the host
 *
 * \return Sucess or Failure
 * \retval 1 Success
 * \retval 0 Failure
 */
uint8_t rndis_send_status(rndis_Status_t stat)
	{
	uint8_t i;

	if(Is_usb_read_control_enabled() && !data_to_send) {
		
			rndis_indicate_status_t * m;
			m = (rndis_indicate_status_t *)encapsulated_buffer;
	
			m->MessageType = REMOTE_NDIS_INDICATE_STATUS_MSG;
			m->MessageLength = sizeof(rndis_indicate_status_t);
			m->Status = stat;

			for(i = 0; i < sizeof(rndis_indicate_status_t); i++) {
				Usb_write_byte(encapsulated_buffer[i]);
			}

			Usb_send_control_in();
			while(!(Is_usb_read_control_enabled()));

    		while(!Is_usb_receive_out());
    		Usb_ack_receive_out();

			return 1;
	}


	return 0;
	}

void rndis_configure_endpoints() {
	usb_configure_endpoint(INT_EP,      \
						 TYPE_INTERRUPT,     \
						 DIRECTION_IN,  \
						 SIZE_8,       \
						 ONE_BANK,     \
						 NYET_ENABLED);
	usb_configure_endpoint(TX_EP,      \
						 TYPE_BULK,  \
						 DIRECTION_IN,  \
						 SIZE_64,     \
						 TWO_BANKS,     \
						 NYET_ENABLED);

	usb_configure_endpoint(RX_EP,      \
						 TYPE_BULK,     \
						 DIRECTION_OUT,  \
						 SIZE_64,       \
						 TWO_BANKS,     \
						 NYET_ENABLED);
	Usb_reset_endpoint(INT_EP);
	Usb_reset_endpoint(TX_EP);
	Usb_reset_endpoint(RX_EP);
}


extern char usb_busy;
extern uint8_t usb_eth_data_buffer[64];
#define PBUF ((rndis_data_packet_t *) usb_eth_data_buffer)


uint8_t rndis_process(void)
{
	uint8_t bytecounter, headercounter;
	uint16_t i, dataoffset;
	
	if(rndis_state != rndis_data_initialized) { //Enumeration processs OK ?
		return 0;
	}
	
#ifdef USB_ETH_HOOK_INIT
	static uint8_t doInit = 1;
	if (doInit) {
		USB_ETH_HOOK_INIT();
		doInit = 0;
	}
#endif

	//Connected!
	Led0_on();

	Usb_select_endpoint(RX_EP);

	//If we have data and a free buffer
	if(Is_usb_receive_out() && (uip_len == 0)) {
	
		//Read how much (endpoint only stores up to 64 bytes anyway)
		bytecounter = Usb_byte_counter_8();

		//Try and read the header in
		headercounter = sizeof(rndis_data_packet_t);

		uint8_t fail = 0;

		//Hmm.. what's going on here
		if (bytecounter < headercounter) {
			Usb_ack_receive_out();
			fail = 1;
		}

		i = 0;
		while (headercounter) {
			usb_eth_data_buffer[i] = Usb_read_byte();
			bytecounter--;
			headercounter--;
			i++;
		}

		//This is no good. Probably lost syncronization... just drop it for now
		if(PBUF->MessageType != REMOTE_NDIS_PACKET_MSG) {
			Usb_ack_receive_out();
			fail = 1;
		}

		//802.3 does not have OOB data, and we don't care about per-packet data
		//so that just leave regular packet data...
		if (PBUF->DataLength && (fail == 0)) {			
		
			//Looks like we've got a live one
#ifdef USB_ETH_HOOK_RX_START
			USB_ETH_HOOK_RX_START();
#endif

			//Get offset
			dataoffset = PBUF->DataOffset;

			//Make it offset from start of message, not DataOffset field
			dataoffset += (sizeof(rndis_MessageType_t) + sizeof(rndis_MessageLength_t));

			//Subtract what we already took
			dataoffset -= sizeof(rndis_data_packet_t);

			//Clear this flag
			Usb_ack_nak_out();

			//Read to the start of data
			while(dataoffset) {
				Usb_read_byte();
				dataoffset--;
				bytecounter--;

				//If endpoint is done
				if (bytecounter == 0) {	
		
					Usb_ack_receive_out();		
			

					//Wait for new data
					while (!Is_usb_receive_out() && (!Is_usb_receive_nak_out()));

					//Check for NAK
					// TODO: darco: What in the world is this for?
					if (Is_usb_receive_nak_out()) {
						Usb_ack_nak_out();
						break;
					}

					bytecounter = Usb_byte_counter_8();

					//ZLP?
					if (bytecounter == 0)
						break;
				}

			}
	
			//Read the data itself in
			uint8_t * uipdata = uip_buf;
			uint16_t datalen = PBUF->DataLength;
	
			while(datalen) {
				*uipdata++ = Usb_read_byte();
				datalen--;
				bytecounter--;

				//If endpoint is done
				if (bytecounter == 0) {
					//Might be everything we need!
					if (datalen) {
						Usb_ack_receive_out();
						//Wait for new data
						if(usb_endpoint_wait_for_receive_out()!=0) {
							USB_ETH_HOOK_RX_ERROR("Timeout: receive out");
							goto bail;
						}
						bytecounter = Usb_byte_counter_8();
					}
				}

			}

			//Ack final data packet
			Usb_ack_receive_out();					

#ifdef USB_ETH_HOOK_RX_END
			USB_ETH_HOOK_RX_END();
#endif

			//Send data over RF or to local stack
			if(PBUF->DataLength <= USB_ETH_MTU) {
				USB_ETH_HOOK_HANDLE_INBOUND_PACKET(uip_buf,PBUF->DataLength);
			} else {
				USB_ETH_HOOK_RX_ERROR("Oversized packet");
			}
		} //if (PBUF->DataLength)
	}
bail:	
	return 1;
}

/**
 \brief Send data over RNDIS interface, data is in uipbuf and length is uiplen
 */
uint8_t rndis_send(uint8_t * senddata, uint16_t sendlen, uint8_t led)
{


	uint16_t i;

	//Setup Header
	PBUF->MessageType = REMOTE_NDIS_PACKET_MSG;
	PBUF->DataOffset = sizeof(rndis_data_packet_t) - sizeof(rndis_MessageType_t) - sizeof(rndis_MessageLength_t);
	PBUF->DataLength = sendlen;
	PBUF->OOBDataLength = 0;
	PBUF->OOBDataOffset = 0;
	PBUF->NumOOBDataElements = 0;
	PBUF->PerPacketInfoOffset = 0;
	PBUF->PerPacketInfoLength = 0;
	PBUF->DeviceVcHandle = 0;
	PBUF->Reserved = 0;
	PBUF->MessageLength = sizeof(rndis_data_packet_t) + PBUF->DataLength;

	//Send Data
	Usb_select_endpoint(TX_EP);
	Usb_send_in();

	//Wait for ready
	if(usb_endpoint_wait_for_write_enabled()!=0) {
		USB_ETH_HOOK_TX_ERROR("Timeout: write enabled");
		return 0;
	}

#ifdef USB_ETH_HOOK_TX_START
	USB_ETH_HOOK_TX_START();
#endif

	//Setup first part of transfer...
	for(i = 0; i < sizeof(rndis_data_packet_t); i++) {
		Usb_write_byte(usb_eth_data_buffer[i]);
	}
	
	//Send packet
	while(sendlen) {
		Usb_write_byte(*senddata);
		senddata++;
		sendlen--;

		//If endpoint is full, send data in
		//And then wait for data to transfer
		if (!Is_usb_write_enabled()) {
			Usb_send_in();

			if(usb_endpoint_wait_for_write_enabled()!=0) {
				USB_ETH_HOOK_TX_ERROR("Timeout: write enabled");
				return 0;
			}
		}

	}

	Usb_send_in();

#ifdef USB_ETH_HOOK_TX_END
	USB_ETH_HOOK_TX_END();
#endif

    //Wait for ready
	if(usb_endpoint_wait_for_IN_ready()!=0) {
		USB_ETH_HOOK_TX_ERROR("Timeout: IN ready");
		return 0;
	}

	return 1;
}


/****************** Radio Interface ****************/

/**
 * \brief Set the packet filter - currently distinguishes
 *        between promiscuous mode and normal mode
 */
void rndis_packetFilter(uint32_t newfilter)
{
	if (newfilter & NDIS_PACKET_TYPE_PROMISCUOUS) {
		USB_ETH_HOOK_SET_PROMISCIOUS_MODE(true);
	} else {
		USB_ETH_HOOK_SET_PROMISCIOUS_MODE(false);
	}
}

/** @} */

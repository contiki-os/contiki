/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *      USB Protocol-Specific Requests
 *
 * \addtogroup usbtask
 *
 * \author
 *      Colin O'Flynn <coflynn@newae.com>
 *
 ******************************************************************************/
/* Copyright (c) 2008  Colin O'Flynn
   Copyright (c) Atmel Corporation 2008
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


//_____ I N C L U D E S ____________________________________________________

#include "radio.h"
#include "contiki.h"
#include "config.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include "rndis/rndis_protocol.h"
#include "rndis/cdc_ecm.h"
#include "rndis/rndis_task.h"
#include "serial/uart_usb_lib.h"
#include "storage/ctrl_access.h"
#include "uip.h"
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <string.h>

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N ________________________________________________
#if USB_CONF_STORAGE
bit  ms_multiple_drive;
#endif
//_____ P R I V A T E   D E C L A R A T I O N ______________________________

extern PGM_VOID_P pbuffer;
extern U8   data_to_transfer;

//_____ D E C L A R A T I O N ______________________________________________



//! This function is called by the standard usb read request function when
//! the Usb request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard usb read request function.
//!
//! @param type Not used
//! @param request Read request type
//!
//! @retval FALSE if unknown read type
//! @retval TRUE if request type is processed
//!
Bool usb_user_read_request(U8 type, U8 request)
{	
	U16 wLength;

	switch(request)
	{
		case SEND_ENCAPSULATED_COMMAND:			
				Usb_read_byte();
				Usb_read_byte();
				Usb_read_byte();//wIndex LSB
				Usb_read_byte();//wIndex MSB

				LSB(wLength) = Usb_read_byte();
				MSB(wLength) = Usb_read_byte();
				if((usb_configuration_nb==USB_CONFIG_RNDIS)||(usb_configuration_nb==USB_CONFIG_RNDIS_DEBUG))
					return rndis_send_encapsulated_command(wLength);
				else
					return FALSE;
      			break;

  		case GET_ENCAPSULATED_COMMAND:
				Usb_read_byte();
				Usb_read_byte();
				Usb_read_byte();//wIndex LSB
				Usb_read_byte();//wIndex MSB

				LSB(wLength) = Usb_read_byte();
				MSB(wLength) = Usb_read_byte();
				if((usb_configuration_nb==USB_CONFIG_RNDIS)||(usb_configuration_nb==USB_CONFIG_RNDIS_DEBUG))
					return rndis_get_encapsulated_command();
				else
					return FALSE;
      			break;

		case SET_ETHERNET_PACKET_FILTER:
				if((usb_configuration_nb==USB_CONFIG_ECM) || (usb_configuration_nb==USB_CONFIG_ECM_DEBUG)) {
					cdc_ecm_set_ethernet_packet_filter();
					return TRUE;
				} else
					return FALSE;
				break;

#if USB_CONF_STORAGE
        case MASS_STORAGE_RESET:
         		Usb_ack_receive_setup();
         		Usb_send_control_in();
         		return TRUE;
         		break;


		case GET_MAX_LUN:
         		Usb_ack_receive_setup();
         		Usb_write_byte( (get_nb_lun()-1) );
         		Usb_send_control_in();
         		ms_multiple_drive = 1;
         		return TRUE;
         		break;
#endif /* USB_CONF_STORAGE */

#if USB_CONF_SERIAL	
	/* We don't have a real serial port - so these aren't applicable. We
	   advertise that we support nothing, so shouldn't get them anyway */
		case GET_LINE_CODING:
				Usb_read_byte();
				Usb_read_byte();
				if(USB_CONFIG_HAS_DEBUG_PORT(usb_configuration_nb)) {
					cdc_get_line_coding();
					return TRUE;
				} else {
					return FALSE;
				}
      			break;

  		case SET_LINE_CODING:
				Usb_read_byte();
				Usb_read_byte();
				if(USB_CONFIG_HAS_DEBUG_PORT(usb_configuration_nb)) {
					cdc_set_line_coding();
					return TRUE;
				} else {
					return FALSE;
				}
      			break;

		case SET_CONTROL_LINE_STATE:
				if(USB_CONFIG_HAS_DEBUG_PORT(usb_configuration_nb)) {
					cdc_set_control_line_state();
					return TRUE;
				} else {
					return FALSE;
				}
      			break;
#endif /* USB_CONF_SERIAL */
     	default:
				break;

	}
	 
  	return FALSE;
}

//! usb_user_get_descriptor.
//!
//! @brief This function returns the size and the pointer on a user information
//! structure
//!
//! @param type descriptor type
//! @param string descriptor ID
//!
//! @retval FALSE
//!
Bool usb_user_get_descriptor(U8 type, U8 string)
{ 
	switch(type)
	{
		case STRING_DESCRIPTOR:
      	switch (string)
      	{
        		case LANG_ID:
          		data_to_transfer = sizeof (usb_user_language_id);
          		pbuffer = &(usb_user_language_id.bLength);
					return TRUE;
          		break;
        		default:
          		return FALSE;
			}
		default:
			return FALSE;
	}

	return FALSE;
}
static char itoh(unsigned char i) {
	char ret;
	ret = pgm_read_byte_near(PSTR("0123456789abcdef")+(i&0xF));
	return ret;
}
const char* usb_user_get_string_sram(U8 string_type) {
	static char serial[13];
	uint8_t i;
	
	switch (string_type)
	{
		case USB_STRING_SERIAL:
		case USB_STRING_MAC_ADDRESS:
			{
				uint8_t mac_address[6];
				usb_eth_get_mac_address(mac_address);
				
				for(i=0;i<6;i++) {
					serial[i*2] = itoh(mac_address[i]>>4);
					serial[i*2+1] = itoh(mac_address[i]);
				}
			}
			break;
		default:
			serial[0] = 0;
			break;
	}
	
	return serial;
}

PGM_P usb_user_get_string(U8 string_type) {
	switch (string_type)
	{
		case USB_STRING_MAN:
			return PSTR("Atmel");

		case USB_STRING_PRODUCT:
			return PSTR("Jackdaw 6LoWPAN Adaptor");

		case USB_STRING_CONFIG_COMPOSITE:
			return PSTR("RNDIS+Debug");

		case USB_STRING_CONFIG_RNDIS:
		case USB_STRING_INTERFACE_RNDIS:
			return PSTR("RNDIS");

		case USB_STRING_CONFIG_EEM:
		case USB_STRING_INTERFACE_EEM:
			return PSTR("CDC-EEM");

		case USB_STRING_CONFIG_ECM:
		case USB_STRING_INTERFACE_ECM:
			return PSTR("CDC-ECM");

		case USB_STRING_CONFIG_ECM_DEBUG:
			return PSTR("CDC-ECM+DEBUG");

		case USB_STRING_INTERFACE_SERIAL:
			return PSTR("Debug Port");
/*
		case USB_STRING_INTERFACE_ECM_ATTACHED:
			return PSTR("Attached");
		case USB_STRING_INTERFACE_ECM_DETACHED:
			return PSTR("Detached");
*/			
#if USB_CONF_STORAGE
		case USB_STRING_CONFIG_MS:
		case USB_STRING_INTERFACE_MS:
			return PSTR("Mass Storage");
#endif

/*
		// This is now handled by usb_user_get_string_sram.
		case USB_STRING_SERIAL:
			return PSTR("JD01");

		case USB_STRING_MAC_ADDRESS:
			return PSTR("021213141516");

*/


		default:
			return NULL;
	}
}


//! usb_user_endpoint_init.
//!
//! @brief This function configures the endpoints.
//!
//! @param conf_nb Not used
void usb_user_endpoint_init(U8 conf_nb)
{
	if(USB_CONFIG_HAS_DEBUG_PORT(conf_nb)) {
		uart_usb_configure_endpoints();
	}

	switch(conf_nb) {
		case USB_CONFIG_ECM:
		case USB_CONFIG_ECM_DEBUG:
			cdc_ecm_configure_endpoints();
			break;

		case USB_CONFIG_RNDIS_DEBUG:
		case USB_CONFIG_RNDIS:
			rndis_configure_endpoints();
			break;	
		case USB_CONFIG_EEM:
			cdc_ecm_configure_endpoints();

			break;
#if USB_CONF_STORAGE
		case USB_CONFIG_MS:
			usb_configure_endpoint(VCP_INT_EP,      \
								 TYPE_INTERRUPT,     \
								 DIRECTION_IN,  \
								 SIZE_32,       \
								 ONE_BANK,     \
								 NYET_ENABLED);

			usb_configure_endpoint(VCP_TX_EP,      \
								 TYPE_BULK,     \
								 DIRECTION_IN,  \
								 SIZE_32,       \
								 TWO_BANKS,     \
								 NYET_ENABLED);

			usb_configure_endpoint(VCP_RX_EP,      \
								 TYPE_BULK,     \
								 DIRECTION_OUT,  \
								 SIZE_32,       \
								 TWO_BANKS,     \
								 NYET_ENABLED);
			Usb_reset_endpoint(VCP_INT_EP);
			Usb_reset_endpoint(VCP_TX_EP);
			Usb_reset_endpoint(VCP_RX_EP);

			break;	
#endif
	}
	Led0_on();
}

#if USB_CONF_SERIAL
/******************** Virtual Serial Port ************************/

extern S_line_coding   line_coding;

//! cdc_get_line_coding.
//!
//! @brief This function manages reception of line coding parameters (baudrate...).
//!
void cdc_get_line_coding(void)
{
  	Usb_ack_receive_setup();
  	Usb_write_byte(LSB0(line_coding.dwDTERate));
  	Usb_write_byte(LSB1(line_coding.dwDTERate));
  	Usb_write_byte(LSB2(line_coding.dwDTERate));
  	Usb_write_byte(LSB3(line_coding.dwDTERate));
  	Usb_write_byte(line_coding.bCharFormat);
  	Usb_write_byte(line_coding.bParityType);
  	Usb_write_byte(line_coding.bDataBits);

    Usb_send_control_in();
}


//! cdc_set_line_coding.
//!
//! @brief This function manages reception of line coding parameters (baudrate...).
//!
void cdc_set_line_coding (void)
{
	Usb_ack_receive_setup();
	if(usb_endpoint_wait_for_receive_out()==0) {
		LSB0(line_coding.dwDTERate) = Usb_read_byte();
		LSB1(line_coding.dwDTERate) = Usb_read_byte();
		LSB2(line_coding.dwDTERate) = Usb_read_byte();
		LSB3(line_coding.dwDTERate) = Usb_read_byte();
		line_coding.bCharFormat = Usb_read_byte();
		line_coding.bParityType = Usb_read_byte();
		line_coding.bDataBits = Usb_read_byte();
		Usb_ack_receive_out();

		Usb_send_control_in();                // send a ZLP for STATUS phase
		usb_endpoint_wait_for_read_control_enabled();
	}
}

//! cdc_set_control_line_state.
//!
//! @brief This function manages the SET_CONTROL_LINE_LINE_STATE CDC request.
//!
//! Note: Can manage hardware flow control here...
//!
void cdc_set_control_line_state (void)
{
	U8 controlLineState = Usb_read_byte();
//	U8 dummy = Usb_read_byte();  //Compiler warning
    if (Usb_read_byte()) {};
	U8 interface = Usb_read_byte();

  	Usb_ack_receive_setup();
	Usb_send_control_in();
	usb_endpoint_wait_for_read_control_enabled();

	if(interface == INTERFACE2_NB) {
		uart_usb_set_control_line_state(controlLineState);		
	}
}
#endif /* USB_CONF_SERIAL */

Bool  usb_user_set_alt_interface(U8 interface, U8 alt_setting) {
	return FALSE;
	if((interface==ECM_INTERFACE0_NB) && ((usb_configuration_nb==USB_CONFIG_ECM) || (usb_configuration_nb==USB_CONFIG_ECM_DEBUG))) {
		// The alt_setting in this case corresponds to
		// if the interface is enabled or not.
		usb_eth_set_active(alt_setting);
	}
	return TRUE;
}


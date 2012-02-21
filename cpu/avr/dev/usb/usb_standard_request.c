/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *      This file contains the USB endpoint 0 management routines corresponding to
 *      the standard enumeration process (refer to chapter 9 of the USB
 *      specification.
 *      This file calls routines of the usb_specific_request.c file for non-standard
 *      request management.
 *      The enumeration parameters (descriptor tables) are contained in the
 *      usb_descriptors.c file.
 *
 * \addtogroup usbdriver
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 ******************************************************************************/
/* Copyright (c) 2008  ATMEL Corporation
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

#include "config.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "usb_specific_request.h"
#include <string.h>

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N ________________________________________________
#define PRINTF printf
#define PRINTF_P printf_P

//_____ P R I V A T E   D E C L A R A T I O N ______________________________

static  void    usb_get_descriptor(   void);
static  void    usb_set_address(      void);
static  void    usb_set_configuration(void);
static  void    usb_clear_feature(    void);
static  void    usb_set_feature(      void);
static  void    usb_get_status(       void);
static  void    usb_get_configuration(void);
static  void    usb_get_interface (void);
static  void    usb_set_interface (void);


//_____ D E C L A R A T I O N ______________________________________________

static  bit  zlp;
static  U8   endpoint_status[NB_ENDPOINTS];

#ifdef AVRGCC
        PGM_VOID_P pbuffer;
#else
        U8   FLASH *pbuffer;
#endif
        U8   data_to_transfer;

        U16  wInterface;

static  U8   bmRequestType;

        U8   usb_configuration_nb;

		usb_mode_t usb_mode = rndis_debug;

extern  bit     usb_connected;
extern  FLASH    S_usb_device_descriptor             usb_user_device_descriptor_network;
extern  FLASH    S_usb_user_configuration_descriptor_network usb_user_configuration_descriptor_network;
extern  FLASH    S_usb_device_descriptor             usb_user_device_descriptor_composite;
extern  FLASH    S_usb_user_configuration_descriptor_composite usb_user_configuration_descriptor_composite;

//! usb_process_request.
//!
//! @brief This function reads the SETUP request sent to the default control endpoint
//! and calls the appropriate function. When exiting of the usb_read_request
//! function, the device is ready to manage the next request.
//!
//! @note list of supported requests:
//! GET_DESCRIPTOR
//! GET_CONFIGURATION
//! SET_ADDRESS
//! SET_CONFIGURATION
//! CLEAR_FEATURE
//! SET_FEATURE
//! GET_STATUS
//!
void usb_process_request(void)
{
   U8  bmRequest;

   bmRequestType = Usb_read_byte();
   bmRequest     = Usb_read_byte();

   switch (bmRequest)
   {
    case GET_DESCRIPTOR:
         if (0x80 == bmRequestType) { usb_get_descriptor(); }
         else goto user_read;
         break;

    case GET_CONFIGURATION:
         if (0x80 == bmRequestType) { usb_get_configuration(); }
         else goto user_read;
         break;

    case SET_ADDRESS:
         if (0x00 == bmRequestType) { usb_set_address(); }
         else goto user_read;
         break;

    case SET_CONFIGURATION:
         if (0x00 == bmRequestType) { usb_set_configuration(); }
         else goto user_read;
         break;

    case CLEAR_FEATURE:
         if (0x02 >= bmRequestType) { usb_clear_feature(); }
         else goto user_read;
         break;

    case SET_FEATURE:
         if (0x02 >= bmRequestType) { usb_set_feature(); }
         else goto user_read;
         break;

    case GET_STATUS:
         if ((0x7F < bmRequestType) & (0x82 >= bmRequestType))
                                    { usb_get_status(); }
         else goto user_read;
         break;

    case GET_INTERFACE:
          if (bmRequestType == 0x81) { usb_get_interface(); }
		  else goto user_read;
          break;


    case SET_INTERFACE:
      if (bmRequestType == 0x01) {usb_set_interface();}
      break;

    case SET_DESCRIPTOR:
    case SYNCH_FRAME:
    default: //!< un-supported request => call to user read request
	user_read:
         usb_user_read_request(bmRequestType, bmRequest);
         break;
  }
	
	Usb_select_endpoint(EP_CONTROL);

	// If the receive setup flag hasn't been cleared
	// by this point then we can assume that we didn't
	// support this request and should stall.
	if(Is_usb_receive_setup())
		Usb_enable_stall_handshake();
	
	// Clear some flags.
	Usb_ack_receive_setup();
	Usb_ack_receive_out();
	Usb_ack_in_ready();
}

//! usb_set_address.
//!
//! This function manages the SET ADDRESS request. When complete, the device
//! will filter the requests using the new address.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_set_address(void)
{
   Usb_configure_address(Usb_read_byte());

   Usb_ack_receive_setup();

   Usb_send_control_in();                    //!< send a ZLP for STATUS phase
   while(!Is_usb_in_ready());                //!< waits for status phase done
                                             //!< before using the new address
   Usb_enable_address();
}

//! usb_set_configuration.
//!
//! This function manages the SET CONFIGURATION request. If the selected
//! configuration is valid, this function call the usb_user_endpoint_init()
//! function that will configure the endpoints following the configuration
//! number.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_set_configuration( void )
{
U8 configuration_number;

   configuration_number = Usb_read_byte();

   // TODO: Verify configuration_number!
   Usb_ack_receive_setup();
   usb_configuration_nb = configuration_number;

   Usb_send_control_in();                    //!< send a ZLP for STATUS phase
   while(!Is_usb_in_ready());

   usb_user_endpoint_init(usb_configuration_nb);  //!< endpoint configuration
   Usb_set_configuration_action();
}





void usb_get_string_descriptor_sram(U8  string_type) {
	U16 requested_length;
	U8  dummy;
	const char* user_str;

	user_str = usb_user_get_string_sram(string_type);
	
	dummy = Usb_read_byte();                     //!< don't care of wIndex field
	dummy = Usb_read_byte();
    if (dummy) {;}                                   //avoid gcc unused variable warning
	requested_length = Usb_read_byte();              //!< read wLength
	requested_length |= Usb_read_byte()<<8;
	
	if(!user_str)
		return;
	
	const U8 actual_descriptor_size = 2+strlen(user_str)*2;

	if (requested_length > actual_descriptor_size) {
		zlp = ((actual_descriptor_size % EP_CONTROL_LENGTH) == 0);
		requested_length = actual_descriptor_size;
	}
	
	Usb_ack_receive_setup() ;                  //!< clear the receive setup flag

	if(usb_endpoint_wait_for_read_control_enabled()!=0) {
		Usb_enable_stall_handshake();
		return;
	}

	// Output the length
	Usb_write_byte(actual_descriptor_size);
	
	// Output the type
	Usb_write_byte(STRING_DESCRIPTOR);

	requested_length -= 2;
	U8  nb_byte = 2;

	if(!requested_length) {
		Usb_send_control_in();
	}
		
   while((requested_length != 0) && (!Is_usb_receive_out()))
   {
		if(usb_endpoint_wait_for_read_control_enabled()!=0) {
			Usb_enable_stall_handshake();
			break;
		}

      while(requested_length != 0)        //!< Send data until necessary
      {
         if(nb_byte==EP_CONTROL_LENGTH) //!< Check endpoint 0 size
         {
            nb_byte=0;
            break;
         }

         Usb_write_byte(*user_str);
         Usb_write_byte(0);
		 user_str++;
         requested_length -=2;
		 nb_byte+=2;
      }
      Usb_send_control_in();
   }

//bail:
	if(Is_usb_receive_out()) {
		//! abort from Host
		Usb_ack_receive_out();
		return;
	} 

	if(zlp == TRUE) {
		if(usb_endpoint_wait_for_read_control_enabled()!=0) {
			Usb_enable_stall_handshake();
			return;
		}
		Usb_send_control_in();
	}

	usb_endpoint_wait_for_receive_out();
	Usb_ack_receive_out();
}


void usb_get_string_descriptor(U8  string_type) {
	U16 requested_length;
	U8  dummy;
	PGM_P user_str;

	user_str = usb_user_get_string(string_type);

	if(!user_str) {
		usb_get_string_descriptor_sram(string_type);
		return;
	}
	
	dummy = Usb_read_byte();                     //!< don't care of wIndex field
	dummy = Usb_read_byte();
    if (dummy) {;}  
	requested_length = Usb_read_byte();              //!< read wLength
	requested_length |= Usb_read_byte()<<8;
	
	
	const U8 actual_descriptor_size = 2+strlen_P(user_str)*2;

	if (requested_length > actual_descriptor_size) {
		zlp = ((actual_descriptor_size % EP_CONTROL_LENGTH) == 0);
		requested_length = actual_descriptor_size;
	}
	
	Usb_ack_receive_setup() ;                  //!< clear the receive setup flag

	if(usb_endpoint_wait_for_read_control_enabled()!=0) {
		Usb_enable_stall_handshake();
		return;
	}
	U8  nb_byte = 0;

	// Output the length
	if(requested_length) {
		Usb_write_byte(actual_descriptor_size);
		requested_length--;
		nb_byte++;
	}
	
	// Output the type
	if(requested_length) {
		Usb_write_byte(STRING_DESCRIPTOR);
		requested_length--;
		nb_byte++;
	}
	
	if(!requested_length) {
		Usb_send_control_in();
	}
		
	while((requested_length != 0) && (!Is_usb_receive_out()))
	{
		if(usb_endpoint_wait_for_read_control_enabled()!=0) {
			Usb_enable_stall_handshake();
			break;
		}

		while(requested_length != 0)        //!< Send data until necessary
		{
			if(nb_byte==EP_CONTROL_LENGTH) { //!< Check endpoint 0 size
				nb_byte=0;
				break;
			}

			Usb_write_byte(pgm_read_byte_near((unsigned int)user_str++));
			requested_length--;
			nb_byte++;
			if(requested_length) {
				Usb_write_byte(0);
				requested_length--;
				nb_byte++;
			}
		}
		Usb_send_control_in();
	}

//bail:

	if(Is_usb_receive_out()) {
		//! abort from Host
		Usb_ack_receive_out();
		return;
	} 

	if(zlp == TRUE) {
		if(usb_endpoint_wait_for_read_control_enabled()!=0) {
			Usb_enable_stall_handshake();
			return;
		}
		Usb_send_control_in();
	}

	usb_endpoint_wait_for_receive_out();
	Usb_ack_receive_out();
}


//! usb_get_descriptor.
//!
//! This function manages the GET DESCRIPTOR request. The device descriptor,
//! the configuration descriptor and the device qualifier are supported. All
//! other descriptors must be supported by the usb_user_get_descriptor
//! function.
//! Only 1 configuration is supported.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_get_descriptor(void)
{
	U8  LSBwLength, MSBwLength;
	U8  descriptor_type ;
	U8  string_type     ;
	U8  dummy;
	U8  byteswereread;

	zlp             = FALSE;                  /* no zero length packet */
	string_type     = Usb_read_byte();        /* read LSB of wValue    */
	descriptor_type = Usb_read_byte();        /* read MSB of wValue    */
	byteswereread   = 0;

	switch (descriptor_type)
	{
	case DEVICE_DESCRIPTOR:
	  data_to_transfer = Usb_get_dev_desc_length(); //!< sizeof (usb_user_device_descriptor);
	  pbuffer          = Usb_get_dev_desc_pointer();
	  break;
	case CONFIGURATION_DESCRIPTOR:
		data_to_transfer = Usb_get_conf_desc_length(string_type); //!< sizeof (usb_user_configuration_descriptor);
		pbuffer          = Usb_get_conf_desc_pointer(string_type);
	  break;
#if 1 
	case STRING_DESCRIPTOR:
	  if(string_type!=LANG_ID) {
		usb_get_string_descriptor(string_type);
		return;
	  }
#endif
	default:
		dummy = Usb_read_byte();
		dummy = Usb_read_byte();
        if (dummy) {;}  
		LSBwLength = Usb_read_byte();
		MSBwLength = Usb_read_byte();
		byteswereread=1;
		if( usb_user_get_descriptor(descriptor_type, string_type)==FALSE ) {
		    Usb_enable_stall_handshake(); //TODO:is this necessary, Win7 flaky without?
			Usb_ack_receive_setup();
			return;
		}
	  break;
	}
	if (byteswereread==0) {
		dummy = Usb_read_byte();                     //!< don't care of wIndex field
		dummy = Usb_read_byte();
		LSBwLength = Usb_read_byte();              //!< read wLength
		MSBwLength = Usb_read_byte();
	}

	Usb_ack_receive_setup() ;                  //!< clear the receive setup flag

	if ((LSBwLength > data_to_transfer) || (MSBwLength)) {
		if ((data_to_transfer % EP_CONTROL_LENGTH) == 0) { zlp = TRUE; }
		else { zlp = FALSE; }                   //!< no need of zero length packet

		LSBwLength = data_to_transfer;
		MSBwLength = 0x00;
	} else {
		data_to_transfer = LSBwLength;         //!< send only requested number of data
	}
	
	while((data_to_transfer != 0) && (!Is_usb_receive_out())) {
		U8  nb_byte = 0;
		if(usb_endpoint_wait_for_read_control_enabled()!=0) {
			Usb_enable_stall_handshake();
			break;
		}

        //! Send data until necessary
		while(data_to_transfer != 0) {
//			if(Is_usb_write_enabled()) //!< Check endpoint 0 size
			if(nb_byte++==EP_CONTROL_LENGTH) //!< Check endpoint 0 size
				break;

			Usb_write_byte(pgm_read_byte_near((unsigned int)pbuffer++));
			data_to_transfer --;

		}
		Usb_send_control_in();
	}

	if(Is_usb_receive_out()) {
		//! abort from Host
		Usb_ack_receive_out();
		return;
	}
	
	if(zlp == TRUE) {
		if(usb_endpoint_wait_for_read_control_enabled()!=0) {
			Usb_enable_stall_handshake();
			return;
		}
		Usb_send_control_in();
	}

	usb_endpoint_wait_for_receive_out();
	Usb_ack_receive_out();
}

//! usb_get_configuration.
//!
//! This function manages the GET CONFIGURATION request. The current
//! configuration number is returned.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_get_configuration(void)
{
   Usb_ack_receive_setup();

   Usb_write_byte(usb_configuration_nb);
   Usb_send_control_in();

   usb_endpoint_wait_for_receive_out();
   Usb_ack_receive_out();
}

//! usb_get_status.
//!
//! This function manages the GET STATUS request. The device, interface or
//! endpoint status is returned.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_get_status(void)
{
U8 wIndex;
U8 dummy;

   dummy    = Usb_read_byte();                 //!< dummy read
   dummy    = Usb_read_byte();                 //!< dummy read
   if (dummy) {;}  
   wIndex = Usb_read_byte();

   switch(bmRequestType)
   {
    case REQUEST_DEVICE_STATUS:    Usb_ack_receive_setup();
                                   Usb_write_byte(DEVICE_STATUS);
                                   break;

    case REQUEST_INTERFACE_STATUS: Usb_ack_receive_setup();
                                   Usb_write_byte(INTERFACE_STATUS);
                                   break;

    case REQUEST_ENDPOINT_STATUS:  Usb_ack_receive_setup();
                                   wIndex = wIndex & MSK_EP_DIR;
                                   Usb_write_byte(endpoint_status[wIndex]);
                                   break;
    default:
                                   Usb_enable_stall_handshake();
                                   Usb_ack_receive_setup();
                                   return;
   }

   Usb_write_byte(0x00);
   Usb_send_control_in();

   usb_endpoint_wait_for_receive_out();
   Usb_ack_receive_out();
}

//! usb_set_feature.
//!
//! This function manages the SET FEATURE request. The USB test modes are
//! supported by this function.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_set_feature(void)
{
U8 wValue;
U8 wIndex;
U8 dummy;

   if (bmRequestType == INTERFACE_TYPE)
   {
      return;
   }
   else if (bmRequestType == ENDPOINT_TYPE)
   {
      wValue = Usb_read_byte();
      dummy    = Usb_read_byte();                //!< dummy read
      if (dummy) {;}  

      if (wValue == FEATURE_ENDPOINT_HALT)
      {
         wIndex = (Usb_read_byte() & MSK_EP_DIR);

         if (wIndex == EP_CONTROL)
         {
            return;
         }

         Usb_select_endpoint(wIndex);
         if(Is_usb_endpoint_enabled())
         {
            Usb_enable_stall_handshake();
            Usb_select_endpoint(EP_CONTROL);
            endpoint_status[wIndex] = 0x01;
            Usb_ack_receive_setup();
            Usb_send_control_in();
         }
         else
         {
            Usb_select_endpoint(EP_CONTROL);
            return;
         }
      }
      else
      {
         return;
      }
   }
}

//! usb_clear_feature.
//!
//! This function manages the SET FEATURE request.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_clear_feature(void)
{
U8 wValue;
U8 wIndex;
U8 dummy;

   if (bmRequestType == ZERO_TYPE)
   {
      return;
   }
   else if (bmRequestType == INTERFACE_TYPE)
   {
      return;
   }
   else if (bmRequestType == ENDPOINT_TYPE)
   {
      wValue = Usb_read_byte();
      dummy  = Usb_read_byte();                //!< dummy read
      if (dummy) {;}

      if (wValue == FEATURE_ENDPOINT_HALT)
      {
         wIndex = (Usb_read_byte() & MSK_EP_DIR);

         Usb_select_endpoint(wIndex);
         if(Is_usb_endpoint_enabled())
         {
            if(wIndex != EP_CONTROL)
            {
               Usb_disable_stall_handshake();
               Usb_reset_endpoint(wIndex);
               Usb_reset_data_toggle();
            }
            Usb_select_endpoint(EP_CONTROL);
            endpoint_status[wIndex] = 0x00;
            Usb_ack_receive_setup();
            Usb_send_control_in();

         }
         else
         {
            return;
         }
      }
      else
      {
         return;
      }
   }
}

//! usb_get_interface.
//!
//! TThis function manages the GET_INTERFACE request.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_get_interface (void)
{
	// Not yet implemented.
}

//! usb_set_interface.
//!
//! TThis function manages the SET_INTERFACE request.
//!
//! @warning Code:xx bytes (function code length)
//!
void usb_set_interface (void)
{
	U8 alt_setting;
	U8 dummy;
	U8 interface;
	
	alt_setting = Usb_read_byte();
	dummy    = Usb_read_byte();
    if (dummy) {;}  
	interface = Usb_read_byte();
	
	if(usb_user_set_alt_interface(interface, alt_setting)) {
		Usb_ack_receive_setup();
		Usb_send_control_in();                    //!< send a ZLP for STATUS phase
		while(!Is_usb_in_ready());

		usb_endpoint_wait_for_receive_out();
		Usb_ack_receive_out();
	}

}

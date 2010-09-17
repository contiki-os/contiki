/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file usb_drv.c************************************************************
 *
 * \brief
 *      This file contains the USB driver routines.
 *
 * \addtogroup usbstick
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

/**
  \addtogroup usbdriver
  @{
  */

//_____ I N C L U D E S ____________________________________________________

#include "config.h"
#include "conf_usb.h"
#include "usb_drv.h"
#include <util/delay.h>

//_____ M A C R O S ________________________________________________________

//_____ D E C L A R A T I O N ______________________________________________

#if (USB_DEVICE_FEATURE==DISABLED && USB_HOST_FEATURE==DISABLED)
   #error at least one of  USB_DEVICE_FEATURE or USB_HOST_FEATURE should be unabled
#endif

#if (USB_DEVICE_FEATURE == ENABLED)

#define USB_ENDPOINT_WAIT_TIMEOUT	250

U8 usb_endpoint_wait_for_write_enabled() {
#if USB_ENDPOINT_WAIT_TIMEOUT
	U16 timeout=USB_ENDPOINT_WAIT_TIMEOUT;	
	while(timeout) {
		if(Is_usb_sof()) {
			Usb_ack_sof();
			timeout--;
		}
#else
	while(1) {
#endif
		if(Is_usb_write_enabled())
			return 0;
		if((!Is_usb_enabled()) || Is_usb_detached())
			return 1;
		if(!Is_usb_endpoint_enabled())
			return 2;
		if(Is_usb_endpoint_stall_requested())
			return 3;
	}
	return 10;
}

U8 usb_endpoint_wait_for_read_control_enabled() {
#if USB_ENDPOINT_WAIT_TIMEOUT
	U16 timeout=USB_ENDPOINT_WAIT_TIMEOUT;	
	while(timeout) {
		if(Is_usb_sof()) {
			Usb_ack_sof();
			timeout--;
		}
#else
	while(1) {
#endif
		if(Is_usb_read_control_enabled())
			return 0;
		if((!Is_usb_enabled()) || Is_usb_detached())
			return 1;
		if(!Is_usb_endpoint_enabled())
			return 2;
		if(Is_usb_endpoint_stall_requested())
			return 3;
	}
	return 10;
}


U8 usb_endpoint_wait_for_IN_ready() {
#if USB_ENDPOINT_WAIT_TIMEOUT
	U16 timeout=USB_ENDPOINT_WAIT_TIMEOUT;	
	while(timeout) {
		if(Is_usb_sof()) {
			Usb_ack_sof();
			timeout--;
		}
#else
	while(1) {
#endif
		if(Is_usb_in_ready())
			return 0;
		if((!Is_usb_enabled()) || Is_usb_detached())
			return 1;
		if(!Is_usb_endpoint_enabled())
			return 2;
		if(Is_usb_endpoint_stall_requested())
			return 3;
	}
	return 10;
}


U8 usb_endpoint_wait_for_receive_out() {
#if USB_ENDPOINT_WAIT_TIMEOUT
	U16 timeout=USB_ENDPOINT_WAIT_TIMEOUT;	
	while(timeout) {
		if(Is_usb_sof()) {
			Usb_ack_sof();
			timeout--;
		}
#else
	while(1) {
#endif
		if(Is_usb_receive_out())
			return 0;
		if((!Is_usb_enabled()) || Is_usb_detached())
			return 1;
		if(!Is_usb_endpoint_enabled())
			return 2;
		if(Is_usb_endpoint_stall_requested())
			return 3;
	}
	return 10;
}

//! usb_configure_endpoint.
//!
//!  This function configures an endpoint with the selected type.
//!
//! @param config0
//! @param config1
//!
//! @return Is_endpoint_configured().
//!
U8 usb_config_ep(U8 config0, U8 config1)
{
    Usb_enable_endpoint();
    UECFG0X = config0;
    UECFG1X = (UECFG1X & (1<<ALLOC)) | config1;
    Usb_allocate_memory();
    return (Is_endpoint_configured());
}

//! usb_select_endpoint_interrupt.
//!
//! This function select the endpoint where an event occurs and returns the
//! number of this endpoint. If no event occurs on the endpoints, this
//! function returns 0.
//!
//! @return 0 endpoint number.
//!
U8 usb_select_enpoint_interrupt(void)
{
U8 interrupt_flags;
U8 ep_num;

   ep_num = 0;
   interrupt_flags = Usb_interrupt_flags();

   while(ep_num < MAX_EP_NB)
   {
      if (interrupt_flags & 1)
      {
         return (ep_num);
      }
      else
      {
         ep_num++;
         interrupt_flags = interrupt_flags >> 1;
      }
   }
   return 0;
}

//! usb_send_packet.
//!
//! This function moves the data pointed by tbuf to the selected endpoint fifo
//! and sends it through the USB.
//!
//!
//! @param ep_num       number of the addressed endpoint
//! @param tbuf        address of the first data to send
//! @param data_length  number of bytes to send
//!
//! @return remaining_length address of the next U8 to send.
//!
//! Example:
//! usb_send_packet(3,&first_data,0x20);    // send packet on the endpoint #3
//! while(!(Usb_tx_complete));              // wait packet ACK'ed by the Host
//! Usb_clear_tx_complete();                     // acknowledge the transmit
//!
//! Note:
//! tbuf is incremented of 'data_length'.
//!
U8 usb_send_packet(U8 ep_num, U8* tbuf, U8 data_length)
{
U8 remaining_length;

   remaining_length = data_length;
   Usb_select_endpoint(ep_num);
   while(Is_usb_write_enabled() && (0 != remaining_length))
   {
      Usb_write_byte(*tbuf);
      remaining_length--;
      tbuf++;
   }
   return remaining_length;
}

//! usb_read_packet.
//!
//! This function moves the data stored in the selected endpoint fifo to
//! the address specified by *rbuf.
//!
//!
//! @param ep_num       number of the addressed endpoint
//! @param rbuf        aaddress of the first data to write with the USB data
//! @param data_length  number of bytes to read
//!
//! @return remaining_length address of the next U8 to send.
//!
//! Example:
//! while(!(Usb_rx_complete));                      // wait new packet received
//! usb_read_packet(4,&first_data,usb_get_nb_byte); // read packet from ep 4
//! Usb_clear_rx();                                 // acknowledge the transmit
//!
//! Note:
//! rbuf is incremented of 'data_length'.
//!
U8 usb_read_packet(U8 ep_num, U8* rbuf, U8  data_length)
{
U8 remaining_length;

   remaining_length = data_length;
   Usb_select_endpoint(ep_num);

   while(Is_usb_read_enabled() && (0 != remaining_length))
   {
      *rbuf = Usb_read_byte();
      remaining_length--;
      rbuf++;
   }
   return remaining_length;
}

//! usb_halt_endpoint.
//!
//! This function sends a STALL handshake for the next Host request. A STALL
//! handshake will be send for each next request untill a SETUP or a Clear Halt
//! Feature occurs for this endpoint.
//!
//! @param ep_num number of the addressed endpoint
//!
void usb_halt_endpoint (U8 ep_num)
{
   Usb_select_endpoint(ep_num);
   Usb_enable_stall_handshake();
}

//! usb_init_device.
//!
//! This function initializes the USB device controller and
//! configures the Default Control Endpoint.
//!
//! @retval FALSE if not Is_usb_id_device() returns FALSE
//! @return usb_configure_endpoint() status
//!
U8 usb_init_device (void)
{
   U8 rv = FALSE;

   Usb_select_device();
   if(Is_usb_id_device())
   {
      Usb_select_endpoint(EP_CONTROL);
      if(!Is_usb_endpoint_enabled())
      {
#if (USB_LOW_SPEED_DEVICE==DISABLE)
         rv = usb_configure_endpoint(EP_CONTROL,    \
                                TYPE_CONTROL,  \
                                DIRECTION_OUT, \
                                SIZE_64,       \
                                ONE_BANK,      \
                                NYET_DISABLED);
#else
         rv = usb_configure_endpoint(EP_CONTROL,    \
                                TYPE_CONTROL,  \
                                DIRECTION_OUT, \
                                SIZE_8,       \
                                ONE_BANK,      \
                                NYET_DISABLED);
#endif

      }
   }
   return rv;
}

#endif

//! ---------------------------------------------------------
//! ------------------ HOST ---------------------------------
//! ---------------------------------------------------------

#if (USB_HOST_FEATURE == ENABLED)

//! usb_configure_pipe.
//!
//!  This function configures a pipe with the selected type.
//!
//! @param config0
//! @param config1
//!
//! @return Is_endpoint_configured().
U8 host_config_pipe(U8 config0, U8 config1)
{
    Host_enable_pipe();
    UPCFG0X = config0;
    UPCFG1X = config1;
    Host_allocate_memory();
    return (Is_pipe_configured());
}

//! host_determine_pipe_size.
//!
//!  This function returns the size configuration register value according
//!  to the endpint size detected inthe device enumeration process.
//!
//! @retval SIZE_8 pipe size register value.
//! @retval SIZE_16 pipe size register value.
//! @retval SIZE_32 pipe size register value.
//! @retval SIZE_64 pipe size register value.
//! @retval SIZE_128 pipe size register value.
//! @retval SIZE_256 pipe size register value.
//! @retval SIZE_512 pipe size register value.
//! @retval SIZE_1024 pipe size register value.
//!
U8 host_determine_pipe_size(U16 size)
{
        if(size <= 8  ) {return (SIZE_8   );}
   else if(size <= 16 ) {return (SIZE_16  );}
   else if(size <= 32 ) {return (SIZE_32  );}
   else if(size <= 64 ) {return (SIZE_64  );}
   else if(size <= 128) {return (SIZE_128 );}
   else if(size <= 256) {return (SIZE_256 );}
   else if(size <= 512) {return (SIZE_512 );}
   else                 {return (SIZE_1024);}

}

//! host_disable_all_pipe.
//!
//!  This function disable all pipes for the host controller
//!  Usefull to execute upon device disconnection.
//!
void host_disable_all_pipe(void)
{
U8 i;
   for (i=0;i<7;i++)
   {
      Host_reset_pipe(i);
      Host_select_pipe(i);
      Host_unallocate_memory();
      Host_disable_pipe();
   }
}

//! @brief Returns the pipe number that generates a USB communication interrupt
//!
//! This function sould be called only when an interrupt has been detected. Otherwize
//! the return value is incorect
//!
//! @retval MAX_EP_NB + 1 - pipe_number
//!
U8 usb_get_nb_pipe_interrupt(void)
{
U8 interrupt_flags;
U8 i;

   interrupt_flags = Host_get_pipe_interrupt();
   for(i=0;i< MAX_EP_NB;i++)
   {
      if (interrupt_flags & (1<<i))
      {
         return (i);
      }
   }
   // This return should never occurs ....
   return MAX_EP_NB+1;
}


#endif   // USB_HOST_FEATURE == ENABLED

/** @} */

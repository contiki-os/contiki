/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file usb_task.c *********************************************************************
 *
 * \brief
 *      This file manages the USB storage.
 *
 * \addtogroup usbstick
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 ******************************************************************************/
/* 
   Copyright (c) 2004  ATMEL Corporation
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
 \addtogroup usbstorage
 @{
*/

//_____  I N C L U D E S ___________________________________________________



#include "contiki.h"
#include "usb_drv.h"
#include "storage/storage_task.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "storage/scsi_decoder.h"
#include "storage/ctrl_access.h"


//_____ M A C R O S ________________________________________________________





//_____ D E F I N I T I O N S ______________________________________________



//_____ D E C L A R A T I O N S ____________________________________________



extern  U8   usb_configuration_nb;


static bit ms_data_direction;
static  U8  dCBWTag[4];

extern  U8  g_scsi_status;
extern  U32 g_scsi_data_remaining;
extern bit ms_multiple_drive;
extern  U8  g_scsi_command[16];


U8 usb_LUN;


void usb_mass_storage_cbw (void);
void usb_mass_storage_csw (void);

#define Usb_set_ms_data_direction_in()  (ms_data_direction = 1)
#define Usb_set_ms_data_direction_out() (ms_data_direction = 0)
#define Is_usb_ms_data_direction_in()   (ms_data_direction == 1)

static struct etimer et;

PROCESS(storage_process, "Storage process");

/**
 * \brief USB Mass Storage Class Process
 *
 *   This is the link between USB and the "good stuff". In this routine data
 *   is received and processed by USB Storage Class
 */
PROCESS_THREAD(storage_process, ev, data_proc)
{
   PROCESS_BEGIN();

   while(1) {

    
	 if (usb_mode == mass_storage) {

		 if (Is_device_enumerated()) {
	         Usb_select_endpoint(MS_OUT_EP);
	         
			 if (Is_usb_receive_out()) {
	           usb_mass_storage_cbw();
	           usb_mass_storage_csw();
	         }
	     }

	}

	if (usb_mode == mass_storage) {
		etimer_set(&et, CLOCK_SECOND/250 + 1);
	} else {
		etimer_set(&et, CLOCK_SECOND);
	}

	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

  } //while(1)

  PROCESS_END();
}




//! @brief USB Command Block Wrapper (CBW) management
//!
//! This function decodes the CBW command and stores the SCSI command
//!
//! @warning Code:?? bytes (function code length)
//!
void usb_mass_storage_cbw (void)
{
   bit cbw_error;
   uint8_t  c;
   uint8_t dummy;

   cbw_error = FALSE;
   Usb_select_endpoint(MS_OUT_EP);           //! check if dCBWSignature is correct
   if (0x55 != Usb_read_byte())
      { cbw_error = TRUE; } //! 'U'
   if (0x53 != Usb_read_byte())
      { cbw_error = TRUE; } //! 'S'
   if (0x42 != Usb_read_byte())
      { cbw_error = TRUE; } //! 'B'
   if (0x43 != Usb_read_byte())
      { cbw_error = TRUE; } //! 'C'

   dCBWTag[0] = Usb_read_byte();             //! Store CBW Tag to be repeated in CSW
   dCBWTag[1] = Usb_read_byte();
   dCBWTag[2] = Usb_read_byte();
   dCBWTag[3] = Usb_read_byte();
	
   LSB0(g_scsi_data_remaining) = Usb_read_byte();
   LSB1(g_scsi_data_remaining) = Usb_read_byte();
   LSB2(g_scsi_data_remaining) = Usb_read_byte();
   LSB3(g_scsi_data_remaining) = Usb_read_byte();


   if (Usb_read_byte() != 0x00)              //! if (bmCBWFlags.bit7 == 1) {direction = IN}
   {
      Usb_set_ms_data_direction_in();
      if (cbw_error)
      {
         Usb_ack_receive_out();
         Usb_select_endpoint(MS_IN_EP);
         Usb_enable_stall_handshake();
         return;
      }
   }
   else
   {
      Usb_set_ms_data_direction_out();
      if (cbw_error)
      {
         Usb_enable_stall_handshake();
         Usb_ack_receive_out();
         return;
      }
   }

   usb_LUN = Usb_read_byte();

   if (!ms_multiple_drive)
   {
      usb_LUN = get_cur_lun();
   }

   dummy      = Usb_read_byte();                //! dummy CBWCBLength read


   for (c=0; c<16; c++)                         // store scsi_command
   {
      g_scsi_command[c] = Usb_read_byte();
   }

   Usb_ack_receive_out();

   if (Is_usb_ms_data_direction_in())
   {
      Usb_select_endpoint(MS_IN_EP);
   }

   if (TRUE != scsi_decode_command())
   {
      if (g_scsi_data_remaining != 0)
      {
         Usb_enable_stall_handshake();
      }
   }
}

//! @brief USB Command Status Wrapper (CSW) management
//!
//! This function sends the status in relation with the last CBW
//!
void usb_mass_storage_csw (void)
{
   Usb_select_endpoint(MS_IN_EP);
   while (Is_usb_endpoint_stall_requested())
   {
      Usb_select_endpoint(EP_CONTROL);
      if (Is_usb_receive_setup())       { usb_process_request(); }
      Usb_select_endpoint(MS_IN_EP);
   }

   Usb_select_endpoint(MS_OUT_EP);
   while (Is_usb_endpoint_stall_requested())
   {
      Usb_select_endpoint(EP_CONTROL);
      if (Is_usb_receive_setup())       { usb_process_request(); }
      Usb_select_endpoint(MS_OUT_EP);
   }

   Usb_select_endpoint(MS_IN_EP);
   while(!Is_usb_write_enabled());
                                                         //! write CSW Signature
   Usb_write_byte(0x55);                                 //! 'U'
   Usb_write_byte(0x53);                                 //! 'S'
   Usb_write_byte(0x42);                                 //! 'B'
   Usb_write_byte(0x53);                                 //! 'S'
                                                         //! write stored CBW Tag
   Usb_write_byte(dCBWTag[0]);
   Usb_write_byte(dCBWTag[1]);
   Usb_write_byte(dCBWTag[2]);
   Usb_write_byte(dCBWTag[3]);
                                                         //! write data residue value
   Usb_write_byte( ((Byte*)&g_scsi_data_remaining)[3] );
   Usb_write_byte( ((Byte*)&g_scsi_data_remaining)[2] );
   Usb_write_byte( ((Byte*)&g_scsi_data_remaining)[1] );
   Usb_write_byte( ((Byte*)&g_scsi_data_remaining)[0] );

   //! write command status
    Usb_write_byte(g_scsi_status);                        //! 0 -> PASS, 1 -> FAIL
    Usb_send_in();
}

/** @} */

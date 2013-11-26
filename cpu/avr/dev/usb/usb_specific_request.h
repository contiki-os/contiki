/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief
 *      This file contains the user callback functions corresponding to the
 *      application.
 *
 * \addtogroup usbtask
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

#ifndef USB_SPECIFIC_REQUEST_H_
#define USB_SPECIFIC_REQUEST_H_

/*_____ I N C L U D E S ____________________________________________________*/

#include "config.h"

/*_____ M A C R O S ________________________________________________________*/


#define SEND_ENCAPSULATED_COMMAND 0x00
#define GET_ENCAPSULATED_COMMAND  0x01
#define GET_LINE_CODING           0x21
#define SET_LINE_CODING           0x20
#define SET_CONTROL_LINE_STATE    0x22
#define SEND_BREAK                0x23
#define MASS_STORAGE_RESET        0xFF
#define GET_MAX_LUN               0xFE

extern  FLASH S_usb_device_descriptor usb_dev_desc_composite;
extern  FLASH S_usb_device_descriptor usb_dev_desc_network;
extern  FLASH S_usb_user_configuration_descriptor_composite usb_conf_desc_composite;
extern  FLASH S_usb_user_configuration_descriptor_network usb_conf_desc_network;
extern  FLASH S_usb_device_qualifier_descriptor usb_qual_desc;
extern  FLASH S_usb_language_id usb_user_language_id;



/*_____ D E F I N I T I O N ________________________________________________*/
Bool  usb_user_read_request(U8, U8);
Bool  usb_user_get_descriptor(U8 , U8);
void  usb_user_endpoint_init(U8);
Bool  usb_user_set_alt_interface(U8 interface, U8 alt_setting);

PGM_P usb_user_get_string(U8 string_type);
const char* usb_user_get_string_sram(U8 string_type);

void  cdc_get_line_coding();
void  cdc_set_line_coding();
void 	cdc_set_control_line_state (void);

extern usb_mode_t usb_mode;

// ____ T Y P E  D E F I N I T I O N _______________________________________

#endif //USB_SPECIFIC_REQUEST_H_


/* This file has been prepared for Doxygen automatic documentation generation.*/


/*! \file usb_task.h *********************************************************************
 *
 * \brief
 *      This file manages the USB task either device/host or both.
 *
 *      The USB task selects the correct USB task (usb_device task or usb_host task
 *      to be executed depending on the current mode available.
 *
 *      According to USB_DEVICE_FEATURE and USB_HOST_FEATURE value (located in conf_usb.h file)
 *      The usb_task can be configured to support USB DEVICE mode or USB Host mode or both
 *      for a dual role device application.
 *
 *      This module also contains the general USB interrupt subroutine. This subroutine is used
 *      to detect asynchronous USB events.
 *
 *      Note:
 *        - The usb_task belongs to the scheduler, the usb_device_task and usb_host do not, they are called
 *          from the general usb_task
 *        - See conf_usb.h file for more details about the configuration of this module
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

#ifndef _USB_TASK_H_
#define _USB_TASK_H_

/**
   \ingroup usbstick
   \defgroup usbdriver USB Driver
   @{
*/

//_____ I N C L U D E S ____________________________________________________


//_____ M A C R O S ________________________________________________________

//! \name USB Events
//! @{

//! Send event
#define Usb_send_event(x)               (g_usb_event |= (1<<x))
//! Ack event processed
#define Usb_ack_event(x)                (g_usb_event &= ~(1<<x))
//! Clear all events
#define Usb_clear_all_event()           (g_usb_event = 0)
//! Check for USB event
#define Is_usb_event(x)                 ((g_usb_event & (1<<x)) ? TRUE : FALSE)
//! Check for USB event NOT occuring
#define Is_not_usb_event(x)             ((g_usb_event & (1<<x)) ? FALSE: TRUE)
//! Check if USB is device
#define Is_usb_device()                 (g_usb_mode==USB_MODE_DEVICE ? TRUE : FALSE)


//! USB Event: USB plugged
#define EVT_USB_POWERED               1         
//! USB Event: USB un-plugged
#define EVT_USB_UNPOWERED             2         
//! USB Event: USB in device
#define EVT_USB_DEVICE_FUNCTION       3         
//! USB Event: USB in host
#define EVT_USB_HOST_FUNCTION         4         
//! USB Event: USB suspend
#define EVT_USB_SUSPEND               5         
//! USB Event: USB wake up
#define EVT_USB_WAKE_UP               6         
//! USB Event: USB resume
#define EVT_USB_RESUME                7         
//! USB Event: USB reset
#define EVT_USB_RESET                 8         
//! USB Event: USB setup received
#define EVT_USB_SETUP_RX              9         
//! @}

//! \name Standard requests defines
//! @{

   #define GET_STATUS                     0x00
   #define GET_DEVICE                     0x01
   #define CLEAR_FEATURE                  0x01           //!< see FEATURES below
   #define GET_STRING                     0x03
   #define SET_FEATURE                    0x03           //!< see FEATURES below
   #define SET_ADDRESS                    0x05
   #define GET_DESCRIPTOR                 0x06
   #define SET_DESCRIPTOR                 0x07
   #define GET_CONFIGURATION              0x08
   #define SET_CONFIGURATION              0x09
   #define GET_INTERFACE                  0x0A
   #define SET_INTERFACE                  0x0B
   #define SYNCH_FRAME                    0x0C

   #define GET_DEVICE_DESCRIPTOR             1
   #define GET_CONFIGURATION_DESCRIPTOR      4

   #define REQUEST_DEVICE_STATUS          0x80
   #define REQUEST_INTERFACE_STATUS       0x81
   #define REQUEST_ENDPOINT_STATUS        0x82
   #define ZERO_TYPE                      0x00
   #define INTERFACE_TYPE                 0x01
   #define ENDPOINT_TYPE                  0x02

                     // Descriptor Types
   #define DEVICE_DESCRIPTOR                     0x01
   #define CONFIGURATION_DESCRIPTOR              0x02
   #define STRING_DESCRIPTOR                     0x03
   #define INTERFACE_DESCRIPTOR                  0x04
   #define ENDPOINT_DESCRIPTOR                   0x05
   #define DEVICE_QUALIFIER_DESCRIPTOR           0x06
   #define OTHER_SPEED_CONFIGURATION_DESCRIPTOR  0x07



                    // Standard Features
   #define FEATURE_DEVICE_REMOTE_WAKEUP   0x01
   #define FEATURE_ENDPOINT_HALT          0x00

   #define TEST_J                         0x01
   #define TEST_K                         0x02
   #define TEST_SEO_NAK                   0x03
   #define TEST_PACKET                    0x04
   #define TEST_FORCE_ENABLE              0x05


                     // Device Status
   #define BUS_POWERED                       0
   #define SELF_POWERED                      1

   //! @}

#define USB_MODE_UNDEFINED            0x00
#define USB_MODE_HOST                 0x01
#define USB_MODE_DEVICE               0x02


typedef enum {
	rndis_only,
	rndis_debug,
	mass_storage,
	eem
} usb_mode_t;

//_____ D E C L A R A T I O N S ____________________________________________

extern volatile uint16_t g_usb_event;
extern uint8_t g_usb_mode;


PROCESS_NAME(usb_process);

extern volatile uint8_t private_sof_counter;

void usb_start_device  (void);
void usb_device_task   (void);


//! @}

#endif /* _USB_TASK_H_ */

/** @} */

/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file usb_task.c *********************************************************************
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
 ******************************************************************************/
/* Copyright (c) 2008  Colin O'Flynn
   Copyright (c) 2008  ATMEL Corporation
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
   \ingroup usbstick
   \defgroup usbtask USB Contiki Task
   @{
*/

//_____  I N C L U D E S ___________________________________________________

#include "contiki.h"
#include "config.h"
#include "conf_usb.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "pll_drv.h"
#include "usb_task.h"
#include "rndis/rndis_protocol.h"
#include "rndis/rndis_task.h"

PROCESS(usb_process, "USB process");

#ifndef  USE_USB_PADS_REGULATOR
   #error "USE_USB_PADS_REGULATOR" should be defined as ENABLE or DISABLE in conf_usb.h file
#endif
#include <avr/sleep.h>

//_____ M A C R O S ________________________________________________________

#ifndef LOG_STR_CODE
#define LOG_STR_CODE(str)
#else
U8 code log_device_disconnect[]="Device Disconnected";
U8 code log_id_change[]="Pin Id Change";
#endif

#define USB_EVENT 0x2F /* Contiki event number - I just made this one up?*/

//_____ D E F I N I T I O N S ______________________________________________

//!
//! Public : U16 g_usb_event
//! usb_connected is used to store USB events detected upon
//! USB general interrupt subroutine
//! Its value is managed by the following macros (See usb_task.h file)
//! Usb_send_event(x)
//! Usb_ack_event(x)
//! Usb_clear_all_event()
//! Is_usb_event(x)
//! Is_not_usb_event(x)
volatile uint16_t g_usb_event=0;


//!
//! Public : (bit) usb_connected
//! usb_connected is set to TRUE when VBUS has been detected
//! usb_connected is set to FALSE otherwise
//! Used with USB_DEVICE_FEATURE == ENABLED only
//!/
bit   usb_connected;

//!
//! Public : (U8) usb_configuration_nb
//! Store the number of the USB configuration used by the USB device
//! when its value is different from zero, it means the device mode is enumerated
//! Used with USB_DEVICE_FEATURE == ENABLED only
//!/
extern U8    usb_configuration_nb;


//_____ D E C L A R A T I O N S ____________________________________________



/**
 *   \brief Spare function to handle sleep mode.
 */
extern void suspend_action(void)
{
   Enable_interrupt();
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
   sleep_mode();
}

/**
  *    \brief This function initializes the USB device controller
  *
  * This function enables the USB controller and init the USB interrupts.
  * The aim is to allow the USB connection detection in order to send
  * the appropriate USB event to the operating mode manager.
  * Start device function is executed once VBUS connection has been detected
  * either by the VBUS change interrupt either by the VBUS high level
  */
void usb_start_device (void)
{
   Pll_start_auto();
   Wait_pll_ready();
   Usb_unfreeze_clock();
   Usb_enable_vbus_interrupt();
   Usb_enable_reset_interrupt();
   usb_init_device();         // configure the USB controller EP0
   Usb_attach();
}


/**
  *  \brief USB Poll Handler
  *
  * This routine is repetively called, and deals with things such as new SETUP transfers
  * on the control endpoint
  */
static void pollhandler(void) 
{
	/* Check for setup packets */	
	Usb_select_endpoint(EP_CONTROL);
	if (Is_usb_receive_setup()) {
	  usb_process_request();
	}

	/* The previous call might have requested we send
		out something to the RNDIS interrupt endpoint */
	if (schedule_interrupt) {
		Usb_select_endpoint(INT_EP);

		//Linux is a bunch of lies, and won't read
		//the interrupt endpoint. Hence if this isn't ready just exit
		//while(!Is_usb_write_enabled());

		 if (Is_usb_write_enabled()) {

			// Only valid interrupt is:
			//   0x00000001 0x00000000
			//
			Usb_write_byte(0x01);
			Usb_write_byte(0x00);
			Usb_write_byte(0x00);
			Usb_write_byte(0x00);
			Usb_write_byte(0x00);
			Usb_write_byte(0x00);
			Usb_write_byte(0x00);
			Usb_write_byte(0x00);

			//Send back
			Usb_send_in();
			
			schedule_interrupt = 0;
		}
	}	 

	/* Continue polling */
	process_poll(&usb_process);

}
/**
  *  \brief USB Process
  *
  * The actual USB process, deals with USB events such as resets, and being plugged in
  * or unplugged. A seperate polling routine is setup, which continously checks for 
  * things such as SETUP packets on the control interface. They must be responded to
  * very quickly, hence the need for a polling process.
  */
PROCESS_THREAD(usb_process, ev, data_proc)
{

PROCESS_POLLHANDLER(pollhandler());

PROCESS_BEGIN();

   
   /*** USB initilization ***/

   #if (USE_USB_PADS_REGULATOR==ENABLE)  // Otherwise assume USB PADs regulator is not used
   Usb_enable_regulator();
   #endif

   Usb_force_device_mode();

	/* Init USB controller */
   Enable_interrupt();
   Usb_disable();
   Usb_enable();
   Usb_select_device();
#if (USB_LOW_SPEED_DEVICE==ENABLE)
   Usb_low_speed_mode();
#endif
   Usb_enable_vbus_interrupt();
   Enable_interrupt();

   
   /* Ensure pollhandler is called to start it off */
   process_poll(&usb_process);


   /*** Begin actual USB process ***/
   while(1)
	{

  if (Is_usb_vbus_high()&& usb_connected==FALSE)
   {
      usb_connected = TRUE;
      usb_start_device();
      Usb_vbus_on_action();
   }

   if(Is_usb_event(EVT_USB_RESET))
   {
      Usb_ack_event(EVT_USB_RESET);
      Usb_reset_endpoint(0);
      usb_configuration_nb=0;
   }


	PROCESS_WAIT_EVENT_UNTIL(ev == USB_EVENT);
	}//while(1)

PROCESS_END();

}


//! @brief USB general interrupt subroutine
//!
//! This function is called each time a USB interrupt occurs.
//! The following USB DEVICE events are taken in charge:
//! - VBus On / Off
//! - Start Of Frame
//! - Suspend
//! - Wake-Up
//! - Resume
//! - Reset
//! - Start of frame
//!
//! For each event, the user can launch an action by completing
//! the associate define (See conf_usb.h file to add action upon events)
//!
//! Note: Only interrupts events that are enabled are processed
//!

ISR(USB_GEN_vect)
{ 

	process_post(&usb_process, USB_EVENT, NULL);

  //- VBUS state detection
   if (Is_usb_vbus_transition() && Is_usb_vbus_interrupt_enabled())
   {
      Usb_ack_vbus_transition();
      if (Is_usb_vbus_high())
      {
         usb_connected = TRUE;
         Usb_vbus_on_action();
         Usb_send_event(EVT_USB_POWERED);
			Usb_enable_reset_interrupt();
         usb_start_device();
			Usb_attach();
      }
      else
      {
         Usb_vbus_off_action();
         usb_connected = FALSE;
         usb_configuration_nb = 0;
         Usb_send_event(EVT_USB_UNPOWERED);
      }
   }
  // - Device start of frame received
   if (Is_usb_sof() && Is_sof_interrupt_enabled())
   {
      Usb_ack_sof();
      Usb_sof_action();
   }
  // - Device Suspend event (no more USB activity detected)
   if (Is_usb_suspend() && Is_suspend_interrupt_enabled())
   {
      Usb_ack_suspend();
      Usb_enable_wake_up_interrupt();
      Usb_ack_wake_up();                 // clear wake up to detect next event
      Usb_freeze_clock();
      Usb_send_event(EVT_USB_SUSPEND);
      Usb_suspend_action();
   }
  // - Wake up event (USB activity detected): Used to resume
   if (Is_usb_wake_up() && Is_swake_up_interrupt_enabled())
   {
      Usb_unfreeze_clock();
      Usb_ack_wake_up();
      Usb_disable_wake_up_interrupt();
      Usb_wake_up_action();
      Usb_send_event(EVT_USB_WAKE_UP);
   }
  // - Resume state bus detection
   if (Is_usb_resume() && Is_resume_interrupt_enabled())
   {
      Usb_disable_wake_up_interrupt();
      Usb_ack_resume();
      Usb_disable_resume_interrupt();
      Usb_resume_action();
      Usb_send_event(EVT_USB_RESUME);
   }
  // - USB bus reset detection
   if (Is_usb_reset()&& Is_reset_interrupt_enabled())
   {
      Usb_ack_reset();
      usb_init_device();
      Usb_reset_action();
      Usb_send_event(EVT_USB_RESET);
   }

}

/** @} */


/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file conf_usb.h **********************************************************
 *
 * \brief
 *      This file contains the possible external configuration of the USB.
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

#ifndef CONF_USB_H_
#define CONF_USB_H_



/**
   \ingroup usbstick
   \defgroup usbconf USB Configuration
   @{
*/


   // _________________ USB MODE CONFIGURATION ____________________________
   //! @ingroup usbconf
   //! @defgroup USB_op_mode USB operating modes configuration
   //! defines to enable device or host usb operating modes
   //! supported by the application
   //! @{

      //! @brief ENABLE to activate the host software library support
      //!
      //! Possible values ENABLE or DISABLE
      #define USB_HOST_FEATURE            DISABLED

      //! @brief ENABLE to activate the device software library support
      //!
      //! Possible values ENABLE or DISABLE
      #define USB_DEVICE_FEATURE          ENABLED

   //! @}

   // _________________ USB REGULATOR CONFIGURATION _______________________
   //! @ingroup usbconf
   //! @defgroup USB_reg_mode USB regulator configuration
   //! @{

   //! @brief Enable the internal regulator for USB pads
   //!
   //! When the application voltage is lower than 3.5V, to optimize power consumption
   //! the internal USB pads regulatr can be disabled.
#ifndef USE_USB_PADS_REGULATOR
   #define USE_USB_PADS_REGULATOR   ENABLE      // Possible values ENABLE or DISABLE
#endif
   //! @}

// _________________ DEVICE MODE CONFIGURATION __________________________

   //! @ingroup usbconf
   //! @defgroup USB_device_mode_cfg USB device operating mode configuration
   //!
   //! @{

/** USB RNDIS / Virtual com port setup **/

#ifndef CDC_ECM_USES_INTERRUPT_ENDPOINT
#define CDC_ECM_USES_INTERRUPT_ENDPOINT	0
#endif

#ifndef USB_ETH_EMULATE_WIFI
#define USB_ETH_EMULATE_WIFI	0
#endif

#define NB_ENDPOINTS        7  //!  number of endpoints in the application including control endpoint
#define VCP_RX_EP			0x06
#define VCP_TX_EP			0x05
#define VCP_INT_EP			0x04
#define TX_EP				0x01
#define RX_EP				0x02
#define INT_EP              0x03

/** USB Mass Storage Setup **/

#define NB_ENDPOINTS_MS     3 //!  number of endpoints in the application including control endpoint
#define MS_IN_EP            0x01
#define MS_OUT_EP           0x02

#define USB_LOW_SPEED_DEVICE DISABLE


#define Usb_unicode(a)			((U16)(a))

   //! @ingroup usbconf
   //! @defgroup device_cst_actions USB device custom actions
   //!
   //! @{
   // write here the action to associate to each USB event
   // be carefull not to waste time in order not disturbing the functions
#define Usb_sof_action()         /* sof_action(); */
#define Usb_wake_up_action()
#define Usb_resume_action()
#define Usb_suspend_action()     suspend_action();
#define Usb_reset_action()
#define Usb_vbus_on_action()
#define Usb_vbus_off_action()
#define Usb_set_configuration_action()


// write here the action to associate to each SCSI event
// be carefull not to waste time in order not disturbing the functions
#define Scsi_start_read_action()    Led1_on()
#define Scsi_stop_read_action()     Led1_off()
#define Scsi_start_write_action()   Led2_on()
#define Scsi_stop_write_action()    Led2_off()

   //! @}

extern void sof_action(void);
extern void suspend_action(void);
   //! @}


/** @}  */

#endif //CONF_USB_H_

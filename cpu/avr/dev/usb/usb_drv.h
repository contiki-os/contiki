/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
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

#ifndef _USB_DRV_H_
#define _USB_DRV_H_

#include "compiler.h"

/**
  * \addtogroup usbdriver
  * @{
  */

//_____ I N C L U D E S ____________________________________________________


typedef enum endpoint_parameter{ep_num, ep_type, ep_direction, ep_size, ep_bank, nyet_status} t_endpoint_parameter;


//_____ M A C R O S ________________________________________________________

#define MAX_EP_NB             7

#define EP_CONTROL            0
#define EP_1                  1
#define EP_2                  2
#define EP_3                  3
#define EP_4                  4
#define EP_5                  5
#define EP_6                  6
#define EP_7                  7

#define PIPE_CONTROL          0
#define PIPE_0                0
#define PIPE_1                1
#define PIPE_2                2
#define PIPE_3                3
#define PIPE_4                4
#define PIPE_5                5
#define PIPE_6                6
#define PIPE_7                7

// USB EndPoint
#define MSK_EP_DIR            0x7F
#define MSK_UADD              0x7F
#define MSK_EPTYPE            0xC0
#define MSK_EPSIZE            0x70
#define MSK_EPBK              0x0C
#define MSK_DTSEQ             0x0C
#define MSK_NBUSYBK           0x03
#define MSK_CURRBK            0x03
#define MSK_DAT               0xFF  // UEDATX
#define MSK_BYCTH             0x07  // UEBCHX
#define MSK_BYCTL             0xFF  // UEBCLX
#define MSK_EPINT             0x7F  // UEINT
#define MSK_HADDR             0xFF  // UHADDR

// USB Pipe
#define MSK_PNUM              0x07  // UPNUM
#define MSK_PRST              0x7F  // UPRST
#define MSK_PTYPE             0xC0  // UPCFG0X
#define MSK_PTOKEN            0x30
#define MSK_PEPNUM            0x0F
#define MSK_PSIZE             0x70  // UPCFG1X
#define MSK_PBK               0x0C

#define MSK_NBUSYBK           0x03

#define MSK_ERROR             0x1F

#define MSK_PTYPE             0xC0  // UPCFG0X
#define MSK_PTOKEN            0x30
#define MSK_TOKEN_SETUP       0x30
#define MSK_TOKEN_IN          0x10
#define MSK_TOKEN_OUT         0x20
#define MSK_PEPNUM            0x0F

#define MSK_PSIZE             0x70  // UPCFG1X
#define MSK_PBK               0x0C


// Parameters for endpoint configuration
// These define are the values used to enable and configure an endpoint.
#define TYPE_CONTROL             0
#define TYPE_ISOCHRONOUS         1
#define TYPE_BULK                2
#define TYPE_INTERRUPT           3
 //typedef enum ep_type {TYPE_CONTROL, TYPE_BULK, TYPE_ISOCHRONOUS, TYPE_INTERRUPT} e_ep_type;

#define DIRECTION_OUT            0
#define DIRECTION_IN             1
 //typedef enum ep_dir {DIRECTION_OUT, DIRECTION_IN} e_ep_dir;

#define SIZE_8                   0
#define SIZE_16                  1
#define SIZE_32                  2
#define SIZE_64                  3
#define SIZE_128                 4
#define SIZE_256                 5
#define SIZE_512                 6
#define SIZE_1024                7
 //typedef enum ep_size {SIZE_8,   SIZE_16,  SIZE_32,  SIZE_64,
 //                      SIZE_128, SIZE_256, SIZE_512, SIZE_1024} e_ep_size;

#define ONE_BANK                 0
#define TWO_BANKS                1
 //typedef enum ep_bank {ONE_BANK, TWO_BANKS} e_ep_bank;

#define NYET_ENABLED             0
#define NYET_DISABLED            1
 //typedef enum ep_nyet {NYET_DISABLED, NYET_ENABLED} e_ep_nyet;

#define TOKEN_SETUP              0
#define TOKEN_IN                 1
#define TOKEN_OUT                2

#define Is_ep_addr_in(x)         (  (x&0x80)?   TRUE : FALSE)


//! @ingroup usbdriver
//! @defgroup Endpoints_configuration Configuration macros for endpoints
//! List of the standard macro used to configure pipes and endpoints
//! @{
#define Usb_build_ep_config0(type, dir, nyet)     ((type<<6) /*| (nyet<<1)*/ | (dir))
#define Usb_build_ep_config1(size, bank     )     ((size<<4) | (bank<<2)        )
#define usb_configure_endpoint(num, type, dir, size, bank, nyet)             \
                                    ( Usb_select_endpoint(num),              \
                                      usb_config_ep(Usb_build_ep_config0(type, dir, nyet),\
                                                    Usb_build_ep_config1(size, bank)    ))

#define Host_build_pipe_config0(type, token, ep_num)     ((type<<6) | (token<<4) | (ep_num))
#define Host_build_pipe_config1(size, bank     )         ((size<<4) | (bank<<2)        )
#define host_configure_pipe(num, type, token,ep_num, size, bank, freq)             \
                                    ( Host_select_pipe(num),              \
                                      Host_set_interrupt_frequency(freq), \
                                      host_config_pipe(Host_build_pipe_config0(type, token, ep_num),\
                                                       Host_build_pipe_config1(size, bank)    ))
//! @}

//! @ingroup usbdriver
//! @defgroup USB_regulator USB Pads Regulator drivers
//! Turns ON/OFF USB pads regulator
//! @{
   //! Enable internal USB pads regulator
#define Usb_enable_regulator()          (UHWCON |= (1<<UVREGE))
   //! Disable internal USB pads regulator
#define Usb_disable_regulator()         (UHWCON &= ~(1<<UVREGE))
   //! Check regulator enable bit
#define Is_usb_regulator_enabled()      ((UHWCON &  (1<<UVREGE))  ? TRUE : FALSE)
//! @}

//! @ingroup usbdriver
//! @defgroup gen_usb USB common management drivers
//! These macros manage the USB controller
//! @{
   //! Enable external UID pin
#define Usb_enable_uid_pin()            (UHWCON |= (1<<UIDE))
   //! Disable external UID pin
#define Usb_disable_uid_pin()           (UHWCON &= ~(1<<UIDE))
   //! Disable external UID pin and force device mode
#define Usb_force_device_mode()         (Usb_disable_uid_pin(), UHWCON |= (1<<UIMOD))
   //! Disable external UID pin and force host mode
#define Usb_force_host_mode()           (Usb_disable_uid_pin(), UHWCON &= ~(1<<UIMOD))
   //! Enable external UVCON pin
#define Usb_enable_uvcon_pin()          (UHWCON |= (1<<UVCONE))
   //! Enable external UVCON pin
#define Usb_disable_uvcon_pin()         (UHWCON &= ~(1<<UVCONE))
   //! Use device full speed mode (default mode)
#define Usb_full_speed_mode()           (UDCON &= ~(1<<LSM))
   //! For device mode, force low speed mode
#define Usb_low_speed_mode()            (UDCON |= (1<<LSM))

   //! Enable both USB interface and Vbus pad
#define Usb_enable()                  (USBCON |= ((1<<USBE) | (1<<OTGPADE)))
   //! Disable both USB interface and Vbus pad
#define Usb_disable()                 (USBCON &= ~((1<<USBE) | (1<<OTGPADE)))
#define Is_usb_enabled()              ((USBCON  &   (1<<USBE))   ? TRUE : FALSE)

   //! Enable VBUS pad
#define Usb_enable_vbus_pad()         (USBCON |= (1<<OTGPADE))
   //! Disable VBUS pad
#define Usb_disable_vbus_pad()        (USBCON &= ~(1<<OTGPADE))

#define Usb_select_device()           (USBCON  &= ~(1<<HOST))
#define Usb_select_host()             (USBCON  |=  (1<<HOST))
#define Is_usb_host_enabled()         ((USBCON  &   (1<<HOST))   ? TRUE : FALSE)

   //! Stop internal USB clock in interface (freeze the interface register)
#define Usb_freeze_clock()            (USBCON  |=  (1<<FRZCLK))
#define Usb_unfreeze_clock()          (USBCON  &= ~(1<<FRZCLK))
#define Is_usb_clock_freezed()        ((USBCON  &   (1<<FRZCLK)) ? TRUE : FALSE)

#define Usb_enable_id_interrupt()     (USBCON  |=  (1<<IDTE))
#define Usb_disable_id_interrupt()    (USBCON  &= ~(1<<IDTE))
#define Is_usb_id_interrupt_enabled() ((USBCON &  (1<<IDTE))     ? TRUE : FALSE)
#define Is_usb_id_device()            ((USBSTA &   (1<<ID))      ? TRUE : FALSE)
#define Usb_ack_id_transition()       (USBINT  = ~(1<<IDTI))
#define Is_usb_id_transition()        ((USBINT &   (1<<IDTI))    ? TRUE : FALSE)

#define Usb_enable_vbus_interrupt()   (USBCON  |=  (1<<VBUSTE))
#define Usb_disable_vbus_interrupt()  (USBCON  &= ~(1<<VBUSTE))
#define Is_usb_vbus_interrupt_enabled() ((USBCON &  (1<<VBUSTE))     ? TRUE : FALSE)
#define Is_usb_vbus_high()            ((USBSTA &   (1<<VBUS))    ? TRUE : FALSE)
#define Is_usb_vbus_low()             ((USBSTA &   (1<<VBUS))    ? FALSE : TRUE)
#define Usb_ack_vbus_transition()     (USBINT  = ~(1<<VBUSTI))
#define Is_usb_vbus_transition()      ((USBINT &   (1<<VBUSTI))  ? TRUE : FALSE)

   //! returns the USB general interrupts (interrupt enabled)
#define Usb_get_general_interrupt()      (USBINT & (USBCON & MSK_IDTE_VBUSTE))
   //! acks the general interrupts (interrupt enabled)
#define Usb_ack_all_general_interrupt()  (USBINT = ~(USBCON & MSK_IDTE_VBUSTE))
#define Usb_ack_cache_id_transition(x)   ((x)  &= ~(1<<IDTI))
#define Usb_ack_cache_vbus_transition(x) ((x)  &= ~(1<<VBUSTI))
#define Is_usb_cache_id_transition(x)    (((x) &   (1<<IDTI))  )
#define Is_usb_cache_vbus_transition(x)  (((x) &   (1<<VBUSTI)))

   //! returns the USB Pad interrupts (interrupt enabled)
#define Usb_get_otg_interrupt()            (OTGINT & OTGIEN)
   //! acks the USB Pad interrupts (interrupt enabled)
#define Usb_ack_all_otg_interrupt()        (OTGINT = ~OTGIEN)
#define Is_otg_cache_bconnection_error(x)  (((x) &   MSK_BCERRI))
#define Usb_ack_cache_bconnection_error(x) ((x)  &= ~MSK_BCERRI)

#define Usb_enter_dpram_mode()        (UDPADDH =  (1<<DPACC))
#define Usb_exit_dpram_mode()         (UDPADDH =  (U8)~(1<<DPACC))
#define Usb_set_dpram_address(addr)   (UDPADDH =  (1<<DPACC) + ((Uint16)addr >> 8), UDPADDL = (Uchar)addr)
#define Usb_write_dpram_byte(val)     (UEDATX=val)
#define Usb_read_dpram_byte()			  (UEDATX)

   //! requests for VBus activation
#define Usb_enable_vbus()             (OTGCON  |=  (1<<VBUSREQ))
   //! requests for VBus desactivation
#define Usb_disable_vbus()            (OTGCON  |=  (1<<VBUSRQC))
   //! Manually request VBUS without UVCON signal from USB interface
#define Usb_enable_manual_vbus()      (PORTE|=0x80,DDRE|=0x80,Usb_disable_uvcon_pin())

   //! initiates a Host Negociation Protocol
#define Usb_device_initiate_hnp()     (OTGCON  |=  (1<<HNPREQ))
   //! accepts a Host Negociation Protocol
#define Usb_host_accept_hnp()         (OTGCON  |=  (1<<HNPREQ))
   //! rejects a Host Negociation Protocol
#define Usb_host_reject_hnp()         (OTGCON  &= ~(1<<HNPREQ))
   //! initiates a Session Request Protocol
#define Usb_device_initiate_srp()     (OTGCON  |=  (1<<SRPREQ))
   //! selects VBus as SRP method
#define Usb_select_vbus_srp_method()  (OTGCON  |=  (1<<SRPSEL))
   //! selects data line as SRP method
#define Usb_select_data_srp_method()  (OTGCON  &= ~(1<<SRPSEL))
   //! enables hardware control on VBus
#define Usb_enable_vbus_hw_control()  (OTGCON  &= ~(1<<VBUSHWC))
   //! disables hardware control on VBus
#define Usb_disable_vbus_hw_control() (OTGCON  |=  (1<<VBUSHWC))
   //! tests if VBus has been requested
#define Is_usb_vbus_enabled()         ((OTGCON &   (1<<VBUSREQ)) ? TRUE : FALSE)
   //! tests if a HNP occurs
#define Is_usb_hnp()                  ((OTGCON &   (1<<HNPREQ))  ? TRUE : FALSE)
   //! tests if a SRP from device occurs
#define Is_usb_device_srp()           ((OTGCON &   (1<<SRPREQ))  ? TRUE : FALSE)

   //! enables suspend time out interrupt
#define Usb_enable_suspend_time_out_interrupt()   (OTGIEN  |=  (1<<STOE))
   //! disables suspend time out interrupt
#define Usb_disable_suspend_time_out_interrupt()  (OTGIEN  &= ~(1<<STOE))
#define Is_suspend_time_out_interrupt_enabled()   ((OTGIEN &  (1<<STOE))   ? TRUE : FALSE)
   //! acks suspend time out interrupt
#define Usb_ack_suspend_time_out_interrupt()      (OTGINT  &= ~(1<<STOI))
   //! tests if a suspend time out occurs
#define Is_usb_suspend_time_out_interrupt()       ((OTGINT &   (1<<STOI))    ? TRUE : FALSE)

   //! enables HNP error interrupt
#define Usb_enable_hnp_error_interrupt()          (OTGIEN  |=  (1<<HNPERRE))
   //! disables HNP error interrupt
#define Usb_disable_hnp_error_interrupt()         (OTGIEN  &= ~(1<<HNPERRE))
#define Is_hnp_error_interrupt_enabled()          ((OTGIEN &  (1<<HNPERRE))   ? TRUE : FALSE)
   //! acks HNP error interrupt
#define Usb_ack_hnp_error_interrupt()             (OTGINT  &= ~(1<<HNPERRI))
   //! tests if a HNP error occurs
#define Is_usb_hnp_error_interrupt()              ((OTGINT &   (1<<HNPERRI)) ? TRUE : FALSE)

   //! enables role exchange interrupt
#define Usb_enable_role_exchange_interrupt()      (OTGIEN  |=  (1<<ROLEEXE))
   //! disables role exchange interrupt
#define Usb_disable_role_exchange_interrupt()     (OTGIEN  &= ~(1<<ROLEEXE))
#define Is_role_exchange_interrupt_enabled()      ((OTGIEN &  (1<<ROLEEXE))   ? TRUE : FALSE)
   //! acks role exchange interrupt
#define Usb_ack_role_exchange_interrupt()         (OTGINT  &= ~(1<<ROLEEXI))
   //! tests if a role exchange occurs
#define Is_usb_role_exchange_interrupt()          ((OTGINT &   (1<<ROLEEXI)) ? TRUE : FALSE)

   //! enables B device connection error interrupt
#define Usb_enable_bconnection_error_interrupt()  (OTGIEN  |=  (1<<BCERRE))
   //! disables B device connection error interrupt
#define Usb_disable_bconnection_error_interrupt() (OTGIEN  &= ~(1<<BCERRE))
#define Is_bconnection_error_interrupt_enabled()  ((OTGIEN &  (1<<BCERRE))   ? TRUE : FALSE)
   //! acks B device connection error interrupt
#define Usb_ack_bconnection_error_interrupt()     (OTGINT  &= ~(1<<BCERRI))
   //! tests if a B device connection error occurs
#define Is_usb_bconnection_error_interrupt()      ((OTGINT &   (1<<BCERRI))  ? TRUE : FALSE)

   //! enables VBus error interrupt
#define Usb_enable_vbus_error_interrupt()         (OTGIEN  |=  (1<<VBERRE))
   //! disables VBus error interrupt
#define Usb_disable_vbus_error_interrupt()        (OTGIEN  &= ~(1<<VBERRE))
#define Is_vbus_error_interrupt_enabled()         ((OTGIEN &  (1<<VBERRE))   ? TRUE : FALSE)
   //! acks VBus error interrupt
#define Usb_ack_vbus_error_interrupt()            (OTGINT  &= ~(1<<VBERRI))
   //! tests if a VBus error occurs
#define Is_usb_vbus_error_interrupt()             ((OTGINT &   (1<<VBERRI))  ? TRUE : FALSE)

   //! enables SRP interrupt
#define Usb_enable_srp_interrupt()                (OTGIEN  |=  (1<<SRPE))
   //! disables SRP interrupt
#define Usb_disable_srp_interrupt()               (OTGIEN  &= ~(1<<SRPE))
#define Is_srp_interrupt_enabled()                ((OTGIEN &  (1<<SRPE))   ? TRUE : FALSE)
   //! acks SRP interrupt
#define Usb_ack_srp_interrupt()                   (OTGINT  &= ~(1<<SRPI))
   //! tests if a SRP occurs
#define Is_usb_srp_interrupt()                    ((OTGINT &   (1<<SRPI))    ? TRUE : FALSE)
//! @}

//! @ingroup usbdriver
//! @defgroup USB_device_driver USB device controller drivers
//! These macros manage the USB Device controller.
//! @{
   //! initiates a remote wake-up
   #define Usb_initiate_remote_wake_up()             (UDCON   |=  (1<<RMWKUP))
   //! detaches from USB bus
   #define Usb_detach()                              (UDCON   |=  (1<<DETACH))
   //! attaches to USB bus
   #define Usb_attach()                              (UDCON   &= ~(1<<DETACH))
   //! test if remote wake-up still running
   #define Is_usb_pending_remote_wake_up()           ((UDCON & (1<<RMWKUP)) ? TRUE : FALSE)
   //! test if the device is detached
   #define Is_usb_detached()                         ((UDCON & (1<<DETACH)) ? TRUE : FALSE)

   //! returns the USB device interrupts (interrupt enabled)
   #define Usb_get_device_interrupt()                (UDINT   &   (1<<UDIEN))
   //! acks the USB device interrupts (interrupt enabled)
   #define Usb_ack_all_device_interrupt()            (UDINT   =  ~(1<<UDIEN))

   //! enables remote wake-up interrupt
   #define Usb_enable_remote_wake_up_interrupt()     (UDIEN   |=  (1<<UPRSME))
   //! disables remote wake-up interrupt
   #define Usb_disable_remote_wake_up_interrupt()    (UDIEN   &= ~(1<<UPRSME))
#define Is_remote_wake_up_interrupt_enabled()     ((UDIEN &  (1<<UPRSME))   ? TRUE : FALSE)
   //! acks remote wake-up
#define Usb_ack_remote_wake_up_start()            (UDINT   = ~(1<<UPRSMI))
   //! tests if remote wake-up still running
#define Is_usb_remote_wake_up_start()             ((UDINT &   (1<<UPRSMI))  ? TRUE : FALSE)

   //! enables resume interrupt
#define Usb_enable_resume_interrupt()             (UDIEN   |=  (1<<EORSME))
   //! disables resume interrupt
#define Usb_disable_resume_interrupt()            (UDIEN   &= ~(1<<EORSME))
#define Is_resume_interrupt_enabled()             ((UDIEN &  (1<<EORSME))   ? TRUE : FALSE)
   //! acks resume
#define Usb_ack_resume()                          (UDINT   = ~(1<<EORSMI))
   //! tests if resume occurs
#define Is_usb_resume()                           ((UDINT &   (1<<EORSMI))  ? TRUE : FALSE)

   //! enables wake-up interrupt
#define Usb_enable_wake_up_interrupt()            (UDIEN   |=  (1<<WAKEUPE))
   //! disables wake-up interrupt
#define Usb_disable_wake_up_interrupt()           (UDIEN   &= ~(1<<WAKEUPE))
#define Is_swake_up_interrupt_enabled()           ((UDIEN &  (1<<WAKEUPE))   ? TRUE : FALSE)
   //! acks wake-up
#define Usb_ack_wake_up()                         (UDINT   = ~(1<<WAKEUPI))
   //! tests if wake-up occurs
#define Is_usb_wake_up()                          ((UDINT &   (1<<WAKEUPI)) ? TRUE : FALSE)

   //! enables USB reset interrupt
#define Usb_enable_reset_interrupt()              (UDIEN   |=  (1<<EORSTE))
   //! disables USB reset interrupt
#define Usb_disable_reset_interrupt()             (UDIEN   &= ~(1<<EORSTE))
#define Is_reset_interrupt_enabled()              ((UDIEN &  (1<<EORSTE))   ? TRUE : FALSE)
   //! acks USB reset
#define Usb_ack_reset()                           (UDINT   = ~(1<<EORSTI))
   //! tests if USB reset occurs
#define Is_usb_reset()                            ((UDINT &   (1<<EORSTI))  ? TRUE : FALSE)

   //! enables Start Of Frame Interrupt
#define Usb_enable_sof_interrupt()                (UDIEN   |=  (1<<SOFE))
   //! disables Start Of Frame Interrupt
#define Usb_disable_sof_interrupt()               (UDIEN   &= ~(1<<SOFE))
#define Is_sof_interrupt_enabled()                ((UDIEN &  (1<<SOFE))   ? TRUE : FALSE)
   //! acks Start Of Frame
#define Usb_ack_sof()                             (UDINT   = ~(1<<SOFI))
   //! tests if Start Of Frame occurs
#define Is_usb_sof()                              ((UDINT &   (1<<SOFI))    ? TRUE : FALSE)

   //! enables suspend state interrupt
#define Usb_enable_suspend_interrupt()            (UDIEN   |=  (1<<SUSPE))
   //! disables suspend state interrupt
#define Usb_disable_suspend_interrupt()           (UDIEN   &= ~(1<<SUSPE))
#define Is_suspend_interrupt_enabled()            ((UDIEN &  (1<<SUSPE))   ? TRUE : FALSE)
   //! acks Suspend
#define Usb_ack_suspend()                         (UDINT   = ~(1<<SUSPI))
   //! tests if Suspend state detected
#define Is_usb_suspend()                          ((UDINT &   (1<<SUSPI))   ? TRUE : FALSE)

   //! enables USB device address
#define Usb_enable_address()                      (UDADDR  |=  (1<<ADDEN))
   //! disables USB device address
#define Usb_disable_address()                     (UDADDR  &= ~(1<<ADDEN))
   //! sets the USB device address
#define Usb_configure_address(addr)               (UDADDR  =   (UDADDR & (1<<ADDEN)) | ((U8)addr & MSK_UADD))

   //! returns the last frame number
#define Usb_frame_number()                        ((U16)((((U16)UDFNUMH) << 8) | ((U16)UDFNUML)))
   //! tests if a crc error occurs in frame number
#define Is_usb_frame_number_crc_error()           ((UDMFN & (1<<FNCERR)) ? TRUE : FALSE)
//! @}



//! @ingroup usbdriver
//! @defgroup usb_gen_ep USB endpoint drivers
//! These macros manage the common features of the endpoints.
//! @{
   //! selects the endpoint number to interface with the CPU
#define Usb_select_endpoint(ep)                   (UENUM = (U8)ep )

   //! get the currently selected endpoint number
#define Usb_get_selected_endpoint()               (UENUM )

   //! resets the selected endpoint
#define Usb_reset_endpoint(ep)                    (UERST   =   1 << (U8)ep, UERST  =  0)

   //! enables the current endpoint
#define Usb_enable_endpoint()                     (UECONX  |=  (1<<EPEN))
   //! enables the STALL handshake for the next transaction
#define Usb_enable_stall_handshake()              (UECONX  |=  (1<<STALLRQ))
   //! resets the data toggle sequence
#define Usb_reset_data_toggle()                   (UECONX  |=  (1<<RSTDT))
   //! disables the current endpoint
#define Usb_disable_endpoint()                    (UECONX  &= ~(1<<EPEN))
   //! disables the STALL handshake
#define Usb_disable_stall_handshake()             (UECONX  |=  (1<<STALLRQC))
   //! selects endpoint interface on CPU
#define Usb_select_epnum_for_cpu()                (UECONX  &= ~(1<<EPNUMS))
   //! tests if the current endpoint is enabled
#define Is_usb_endpoint_enabled()                 ((UECONX & (1<<EPEN))    ? TRUE : FALSE)
   //! tests if STALL handshake request is running
#define Is_usb_endpoint_stall_requested()         ((UECONX & (1<<STALLRQ)) ? TRUE : FALSE)

   //! configures the current endpoint
#define Usb_configure_endpoint_type(type)         (UECFG0X =   (UECFG0X & ~(MSK_EPTYPE)) | ((U8)type << 6))
   //! configures the current endpoint direction
#define Usb_configure_endpoint_direction(dir)     (UECFG0X =   (UECFG0X & ~(1<<EPDIR))  | ((U8)dir))

   //! configures the current endpoint size
#define Usb_configure_endpoint_size(size)         (UECFG1X =   (UECFG1X & ~MSK_EPSIZE) | ((U8)size << 4))
   //! configures the current endpoint number of banks
#define Usb_configure_endpoint_bank(bank)         (UECFG1X =   (UECFG1X & ~MSK_EPBK)   | ((U8)bank << 2))
   //! allocates the current configuration in DPRAM memory
#define Usb_allocate_memory()                     (UECFG1X |=  (1<<ALLOC))
   //! un-allocates the current configuration in DPRAM memory
#define Usb_unallocate_memory()                   (UECFG1X &= ~(1<<ALLOC))

   //! acks endpoint overflow interrupt
#define Usb_ack_overflow_interrupt()              (UESTA0X &= ~(1<<OVERFI))
   //! acks endpoint underflow memory
#define Usb_ack_underflow_interrupt()             (UESTA0X &= ~(1<<UNDERFI))
   //! acks Zero Length Packet received
#define Usb_ack_zlp()                             (UESTA0X &= ~(1<<ZLPSEEN))
   //! returns data toggle
#define Usb_data_toggle()                         ((UESTA0X&MSK_DTSEQ) >> 2)
   //! returns the number of busy banks
#define Usb_nb_busy_bank()                        (UESTA0X &   MSK_NBUSYBK)
   //! tests if at least one bank is busy
#define Is_usb_one_bank_busy()                    ((UESTA0X &  MSK_NBUSYBK) == 0 ? FALSE : TRUE)
   //! tests if current endpoint is configured
#define Is_endpoint_configured()                  ((UESTA0X &  (1<<CFGOK))   ? TRUE : FALSE)
   //! tests if an overflows occurs
#define Is_usb_overflow()                         ((UESTA0X &  (1<<OVERFI))  ? TRUE : FALSE)
   //! tests if an underflow occurs
#define Is_usb_underflow()                        ((UESTA0X &  (1<<UNDERFI)) ? TRUE : FALSE)
   //! tests if a ZLP has been detected
#define Is_usb_zlp()                              ((UESTA0X &  (1<<ZLPSEEN)) ? TRUE : FALSE)

   //! returns the control direction
#define Usb_control_direction()                   ((UESTA1X &  (1<<CTRLDIR)) >> 2)
   //! returns the number of the current bank
#define Usb_current_bank()                        ( UESTA1X & MSK_CURRBK)

   //! clears FIFOCON bit
#define Usb_ack_fifocon()                         (UEINTX &= ~(1<<FIFOCON))
   //! acks NAK IN received
#define Usb_ack_nak_in()                          (UEINTX &= ~(1<<NAKINI))
   //! acks NAK OUT received
#define Usb_ack_nak_out()                         (UEINTX &= ~(1<<NAKOUTI))
   //! acks receive SETUP
#define Usb_ack_receive_setup()                   (UEINTX &= ~(1<<RXSTPI))
//! tests if NAK IN received
#define Is_usb_receive_nak_in()                   (UEINTX &(1<<NAKINI))
//! tests if NAK OUT received
#define Is_usb_receive_nak_out()                  (UEINTX &(1<<NAKOUTI))
   //! acks reveive OUT
#define Usb_ack_receive_out()                     (UEINTX &= ~(1<<RXOUTI), Usb_ack_fifocon())
   //! acks STALL sent
#define Usb_ack_stalled()                         (MSK_STALLEDI=   0)
   //! acks IN ready
#define Usb_ack_in_ready()                        (UEINTX &= ~(1<<TXINI), Usb_ack_fifocon())
   //! Kills last bank
#define Usb_kill_last_in_bank()                   (UENTTX |= (1<<RXOUTI))
   //! tests if endpoint read allowed
#define Is_usb_read_enabled()                     (UEINTX&(1<<RWAL))
   //! tests if endpoint write allowed
#define Is_usb_write_enabled()                    (UEINTX&(1<<RWAL))
   //! tests if read allowed on control endpoint
#define Is_usb_read_control_enabled()             (UEINTX&(1<<TXINI))
   //! tests if SETUP received
#define Is_usb_receive_setup()                    (UEINTX&(1<<RXSTPI))
   //! tests if OUT received
#define Is_usb_receive_out()                      (UEINTX&(1<<RXOUTI))
   //! tests if IN ready
#define Is_usb_in_ready()                         (UEINTX&(1<<TXINI))
   //! sends IN
#define Usb_send_in()                             (UEINTX &= ~(1<<FIFOCON))
   //! sends IN on control endpoint
#define Usb_send_control_in()                     (UEINTX &= ~(1<<TXINI))
   //! frees OUT bank
#define Usb_free_out_bank()                       (UEINTX &= ~(1<<FIFOCON))
   //! acks OUT on control endpoint
#define Usb_ack_control_out()                     (UEINTX &= ~(1<<RXOUTI))

   //! enables flow error interrupt
#define Usb_enable_flow_error_interrupt()         (UEIENX  |=  (1<<FLERRE))
   //! enables NAK IN interrupt
#define Usb_enable_nak_in_interrupt()             (UEIENX  |=  (1<<NAKINE))
   //! enables NAK OUT interrupt
#define Usb_enable_nak_out_interrupt()            (UEIENX  |=  (1<<NAKOUTE))
   //! enables receive SETUP interrupt
#define Usb_enable_receive_setup_interrupt()      (UEIENX  |=  (1<<RXSTPE))
   //! enables receive OUT interrupt
#define Usb_enable_receive_out_interrupt()        (UEIENX  |=  (1<<RXOUTE))
   //! enables STALL sent interrupt
#define Usb_enable_stalled_interrupt()            (UEIENX  |=  (1<<STALLEDE))
   //! enables IN ready interrupt
#define Usb_enable_in_ready_interrupt()           (UEIENX  |=  (1<<TXINE))
   //! disables flow error interrupt
#define Usb_disable_flow_error_interrupt()        (UEIENX  &= ~(1<<FLERRE))
   //! disables NAK IN interrupt
#define Usb_disable_nak_in_interrupt()            (UEIENX  &= ~(1<<NAKINE))
   //! disables NAK OUT interrupt
#define Usb_disable_nak_out_interrupt()           (UEIENX  &= ~(1<<NAKOUTE))
   //! disables receive SETUP interrupt
#define Usb_disable_receive_setup_interrupt()     (UEIENX  &= ~(1<<RXSTPE))
   //! disables receive OUT interrupt
#define Usb_disable_receive_out_interrupt()       (UEIENX  &= ~(1<<RXOUTE))
   //! disables STALL sent interrupt
#define Usb_disable_stalled_interrupt()           (UEIENX  &= ~(1<<STALLEDE))
   //! disables IN ready interrupt
#define Usb_disable_in_ready_interrupt()          (UEIENX  &= ~(1<<TXIN))

   //! returns FIFO byte for current endpoint
#define Usb_read_byte()                           (UEDATX)
   //! writes byte in FIFO for current endpoint
#define Usb_write_byte(byte)                      (UEDATX  =   (U8)byte)

   //! returns number of bytes in FIFO current endpoint (16 bits)
#define Usb_byte_counter()                        ((((U16)UEBCHX) << 8) | (UEBCLX))
   //! returns number of bytes in FIFO current endpoint (8 bits)
#define Usb_byte_counter_8()                      ((U8)UEBCLX)

   //! tests the general endpoint interrupt flags
#define Usb_interrupt_flags()                     (UEINT)
   //! tests the general endpoint interrupt flags
#define Is_usb_endpoint_event()                   (Usb_interrupt_flags() != 0x00)
//! @}


//! @ingroup usbdriver
//! @defgroup host_management USB host controller drivers
//! These macros manage the USB Host controller.
//! @{
   //! allocates the current configuration in DPRAM memory
   #define Host_allocate_memory()                 (UPCFG1X |=  (1<<ALLOC))
   //! un-allocates the current configuration in DPRAM memory
   #define Host_unallocate_memory()               (UPCFG1X &= ~(1<<ALLOC))

   //! enables USB Host function
   #define Host_enable()                          (USBCON |= (1<<HOST))

   #ifndef    SOFEN
   #define    SOFEN           0       //For AVRGCC, SOFEN bit missing in default sfr file
   #endif
   //! enables SOF generation
   #define Host_enable_sof()                      (UHCON |= (1<<SOFEN))
   //! disables SOF generation
   #define Host_disable_sof()                     (UHCON &= ~(1<<SOFEN))
   //! sends a USB Reset to the device
   #define Host_send_reset()                      (UHCON |= (1<<RESET))
   //! tests if USB Reset running
   #define Host_is_reset()                        ((UHCON & (1<<RESET)) ? TRUE : FALSE)
   //! sends a USB Resume to the device
   #define Host_send_resume()                     (UHCON |= (1<<RESUME))
   //! tests if USB Resume running
   #define Host_is_resume()                       ((UHCON & (1<<RESUME)) ? TRUE : FALSE)

   //! enables host start of frame interrupt
   #define Host_enable_sof_interrupt()            (UHIEN |= (1<<HSOFE))
   //! enables host start of frame interrupt
   #define Host_disable_sof_interrupt()           (UHIEN &= ~(1<<HSOFE))
#define Is_host_sof_interrupt_enabled()        ((UHIEN &  (1<<HSOFE))   ? TRUE : FALSE)
   //! tests if SOF detected
#define Host_is_sof()                          ((UHINT & (1<<HSOFI)) ? TRUE : FALSE)
#define Is_host_sof()                          ((UHINT & (1<<HSOFI)) ? TRUE : FALSE)
#define Host_ack_sof()                         (UHINT &= ~(1<<HSOFI))

   //! enables host wake up interrupt detection
#define Host_enable_hwup_interrupt()            (UHIEN |= (1<<HWUPE))
   //! disables host wake up interrupt detection
#define Host_disable_hwup_interrupt()           (UHIEN &= ~(1<<HWUPE))
#define Is_host_hwup_interrupt_enabled()        ((UHIEN &  (1<<HWUPE))   ? TRUE : FALSE)
   //! tests if host wake up detected
#define Host_is_hwup()                          ((UHINT & (1<<HWUPI)) ? TRUE : FALSE)
   //! Ack host wake up detection
#define Is_host_hwup()                          ((UHINT & (1<<HWUPI)) ? TRUE : FALSE)
#define Host_ack_hwup()                         (UHINT &= ~(1<<HWUPI))

   //! enables host down stream rsm sent interrupt detection
#define Host_enable_down_stream_resume_interrupt()            (UHIEN |= (1<<RSMEDE))
   //! disables host down stream rsm sent interrupt detection
#define Host_disable_down_stream_resume_interrupt()           (UHIEN &= ~(1<<RSMEDE))
#define Is_host_down_stream_resume_interrupt_enabled()        ((UHIEN &  (1<<RSMEDE))   ? TRUE : FALSE)
   //! Ack host down stream resume sent
#define Is_host_down_stream_resume()                          ((UHINT & (1<<RSMEDI)) ? TRUE : FALSE)
#define Host_ack_down_stream_resume()                         (UHINT &= ~(1<<RSMEDI))

   //! enables host remote wake up interrupt detection
#define Host_enable_remote_wakeup_interrupt()         (UHIEN |= (1<<RXRSME))
   //! disables host remote wake up interrupt detection
#define Host_disable_remote_wakeup_interrupt()        (UHIEN &= ~(1<<RXRSME))
#define Is_host_remote_wakeup_interrupt_enabled()     ((UHIEN &  (1<<RXRSME))   ? TRUE : FALSE)
   //! tests if host wake up detected
#define Host_is_remote_wakeup()                       ((UHINT & (1<<RXRSMI)) ? TRUE : FALSE)
   //! Ack host wake up detection
#define Is_host_remote_wakeup()                       ((UHINT & (1<<RXRSMI)) ? TRUE : FALSE)
#define Host_ack_remote_wakeup()                      (UHINT &= ~(1<<RXRSMI))

   //! enables host device connection interrupt
#define Host_enable_device_connection_interrupt()        (UHIEN |= (1<<DCONNE))
   //! disables USB device connection interrupt
#define Host_disable_device_connection_interrupt()    (UHIEN &= ~(1<<DCONNE))
#define Is_host_device_connection_interrupt_enabled()    ((UHIEN &  (1<<DCONNE))   ? TRUE : FALSE)
   //! tests if a USB device has been detected
#define Is_device_connection()                 (UHINT & (1<<DCONNI))
   //! acks device connection
#define Host_ack_device_connection()           (UHINT = ~(1<<DCONNI))

   //! enables host device disconnection interrupt
#define Host_enable_device_disconnection_interrupt()     (UHIEN |= (1<<DDISCE))
   //! disables USB device connection interrupt
#define Host_disable_device_disconnection_interrupt()    (UHIEN &= ~(1<<DDISCE))
#define Is_host_device_disconnection_interrupt_enabled() ((UHIEN &  (1<<DDISCE))   ? TRUE : FALSE)
   //! tests if a USB device has been removed
#define Is_device_disconnection()              (UHINT & (1<<DDISCI)   ? TRUE : FALSE)
   //! acks device disconnection
#define Host_ack_device_disconnection()        (UHINT = ~(1<<DDISCI))

   //! enables host USB reset interrupt
#define Host_enable_reset_interrupt()          (UHIEN   |=  (1<<RSTE))
   //! disables host USB reset interrupt
#define Host_disable_reset_interrupt()         (UHIEN   &= ~(1<<RSTE))
#define Is_host_reset_interrupt_enabled()      ((UHIEN &  (1<<RSTE))   ? TRUE : FALSE)
   //! acks host USB reset sent
#define Host_ack_reset()                       (UHINT   = ~(1<<RSTI))
   //! tests if USB reset has been sent
#define Is_host_reset()                        Host_is_reset()


   //! switches on VBus
#define Host_vbus_request()                    (OTGCON |= (1<<VBUSREQ))
   //! switches off VBus
#define Host_clear_vbus_request()              (OTGCON |= (1<<VBUSRQC))
   //! configures the address to use for the device
#define Host_configure_address(addr)           (UHADDR = addr & MSK_HADDR)

   //! Get connected device speed, returns TRUE when in full speed mode
#define Is_host_full_speed()                   ((USBSTA &  (1<<SPEED))   ? TRUE : FALSE)
//! @}


//! @ingroup usbdriver
//! @defgroup general_pipe USB pipe drivers
//! These macros manage the common features of the pipes.
//! @{
   //! selects pipe for CPU interface
#define Host_select_pipe(p)                    (UPNUM = (U8)p)

   //! get the currently selected pipe number
#define Host_get_selected_pipe()              (UPNUM )

   //! enables pipe
#define Host_enable_pipe()                     (UPCONX |= (1<<PEN))
   //! disables pipe
#define Host_disable_pipe()                    (UPCONX &= ~(1<<PEN))

   //! sets SETUP token
#define Host_set_token_setup()                 (UPCFG0X =  UPCFG0X & ~MSK_TOKEN_SETUP)
   //! sets IN token
#define Host_set_token_in()                    (UPCFG0X = (UPCFG0X & ~MSK_TOKEN_SETUP) | MSK_TOKEN_IN)
   //! sets OUT token
#define Host_set_token_out()                   (UPCFG0X = (UPCFG0X & ~MSK_TOKEN_SETUP) | MSK_TOKEN_OUT)

   //! returns the number of the endpoint associated to the current pipe
#define Host_get_endpoint_number()             (UPCFG0X & (MSK_PEPNUM))

   //! returns pipe interrupt register
#define Host_get_pipe_interrupt()              (UPINT)

   //! sets the interrupt frequency
#define Host_set_interrupt_frequency(frq)      (UPCFG2X = (U8)frq)

   //! tests if current pipe is configured
#define Is_pipe_configured()                   (UPSTAX  &  (1<<CFGOK))
   //! tests if at least one bank is busy
#define Is_host_one_bank_busy()                ((UPSTAX &  (1<<MSK_NBUSYBK)) != 0)
   //! returns the number of busy banks
#define Host_number_of_busy_bank()             (UPSTAX &  (1<<MSK_NBUSYBK))

   //! resets the pipe
#define Host_reset_pipe(p)                     (UPRST = 1<<p , UPRST = 0)

   //! writes a byte into the pipe FIFO
#define Host_write_byte(dat)                   (UPDATX = dat)
   //! reads a byte from the pipe FIFO
#define Host_read_byte()                       (UPDATX)

   //! freezes the pipe
#define Host_freeze_pipe()                     (UPCONX |=  (1<<PFREEZE))
   //! un-freezees the pipe
#define Host_unfreeze_pipe()                   (UPCONX &= ~(1<<PFREEZE))
   //! tests if the current pipe is frozen
#define Is_host_pipe_freeze()                  (UPCONX &   (1<<PFREEZE))

   //! resets data toggle
#define Host_reset_pipe_data_toggle()          (UPCONX |=  (1<<RSTDT)  )

   //! tests if SETUP has been sent
#define Is_host_setup_sent()                   ((UPINTX & (1<<TXSTPI))    ? TRUE : FALSE)
   //! tests if control IN has been received
#define Is_host_control_in_received()          ((UPINTX & (1<<RXINI))    ? TRUE : FALSE)
   //! tests if control OUT has been sent
#define Is_host_control_out_sent()             ((UPINTX & (1<<TXOUTI))    ? TRUE : FALSE)
   //! tests if a STALL has been received
#define Is_host_stall()                        ((UPINTX & (1<<RXSTALLI))    ? TRUE : FALSE)
   //! tests if an error occurs on current pipe
#define Is_host_pipe_error()                   ((UPINTX & (1<<PERRI))    ? TRUE : FALSE)
   //! sends a setup
#define Host_send_setup()                      (UPINTX  &= ~(1<<FIFOCON))
   //! sends a control IN
#define Host_send_control_in()                 (UPINTX  &= ~(1<<FIFOCON))
   //! sends a control OUT
#define Host_send_control_out()                (UPINTX  &= ~(1<<FIFOCON))
   //! acks control OUT
#define Host_ack_control_out()                 (UPINTX  &= ~(1<<TXOUTI))
   //! acks control IN
#define Host_ack_control_in()                  (UPINTX  &= ~(1<<RXINI))
   //! acks setup
#define Host_ack_setup()                       (UPINTX  &= ~(1<<TXSTPI))
   //! acks STALL reception
#define Host_ack_stall()                       (UPINTX  &= ~(1<<RXSTALLI))

   //! sends a OUT
#define Host_send_out()                        (UPINTX &= ~(1<<FIFOCON))
   //! tests if OUT has been sent
#define Is_host_out_sent()                     ((UPINTX & (1<<TXOUTI))    ? TRUE : FALSE)
   //! acks OUT sent
#define Host_ack_out_sent()                    (UPINTX &= ~(1<<TXOUTI))
   //! tests if IN received
#define Is_host_in_received()                  ((UPINTX & (1<<RXINI))    ? TRUE : FALSE)
   //! acks IN reception
#define Host_ack_in_received()                 (UPINTX &= ~(1<<RXINI))
   //! sends a IN
#define Host_send_in()                         (UPINTX &= ~(1<<FIFOCON))
   //! tests if nak handshake has been received
#define Is_host_nak_received()                 ((UPINTX & (1<<NAKEDI))    ? TRUE : FALSE)
   //! acks NAk received sent
#define Host_ack_nak_received()                (UPINTX &= ~(1<<NAKEDI))



   //! tests if endpoint read allowed
#define Is_host_read_enabled()                 (UPINTX&(1<<RWAL))
   //! tests if endpoint read allowed
#define Is_host_write_enabled()                 (UPINTX&(1<<RWAL))

   //! sets IN in standard mode
#define Host_standard_in_mode()                (UPCONX &= ~(1<<INMODE))
   //! sets IN in continuous mode
#define Host_continuous_in_mode()              (UPCONX |=  (1<<INMODE))

   //! sets number of IN requests to perform before freeze
#define Host_in_request_number(in_num)         (UPINRQX = (U8)in_num)
   //! returns number of remaining IN requests
#define Host_get_in_request_number()           (UPINRQX)

   //! returns number of bytes (8 bits)
#define Host_data_length_U8()                  (UPBCLX)
   //! returns number of bytes (16 bits)
#define Host_data_length_U16()                 ((((U16)UPBCHX)<<8) | UPBCLX)
   //! for device compatibility
#define Host_byte_counter()                    Host_data_length_U16()
   //! for device compatibility
#define Host_byte_counter_8()                  Host_data_length_U8()

   //! returns the size of the current pipe
#define Host_get_pipe_length()                 ((U16)0x08 << ((UPCFG1X & MSK_PSIZE)>>4))

   //! returns the type of the current pipe
#define Host_get_pipe_type()                   (UPCFG0X>>6)

   //! tests if error occurs on pipe
#define Host_error_status()                    (UPERRX & MSK_ERROR)
   //! acks all pipe error
#define Host_ack_all_errors()                  (UPERRX = 0x00)

   //! Enable pipe end transmission interrupt
#define Host_enable_transmit_interrupt()       (UPIENX |= (1<<TXOUTE))
   //! Disable pipe end transmission interrupt
#define Host_disable_transmit_interrupt()      (UPIENX &= ~(1<<TXOUTE))

   //! Enable pipe reception interrupt
#define Host_enable_receive_interrupt()        (UPIENX |= (1<<RXINE))
   //! Disable pipe recption interrupt
#define Host_disable_receive_interrupt()       (UPIENX &= ~(1<<RXINE))

   //! Enable pipe stall interrupt
#define Host_enable_stall_interrupt()        (UPIENX |= (1<<RXSTALLE))
   //! Disable pipe stall interrupt
#define Host_disable_stall_interrupt()       (UPIENX &= ~(1<<RXSTALLE))

   //! Enable pipe error interrupt
#define Host_enable_error_interrupt()        (UPIENX |= (1<<PERRE))
   //! Disable pipe error interrupt
#define Host_disable_error_interrupt()       (UPIENX &= ~(1<<PERRE))

   //! Enable pipe NAK interrupt
#define Host_enable_nak_interrupt()        (UPIENX |= (1<<NAKEDE))
   //! Disable pipe NAK interrupt
#define Host_disable_nak_interrupt()       (UPIENX &= ~(1<<NAKEDE))

#define Get_pipe_token(x)                 ((x & (0x80)) ? TOKEN_IN : TOKEN_OUT)

//! @}

//! wSWAP
//! This macro swaps the U8 order in words.
//!
//! @param x        (U16) the 16 bit word to swap
//!
//! @return         (U16) the 16 bit word x with the 2 bytes swaped

#define wSWAP(x)        \
   (   (((x)>>8)&0x00FF) \
   |   (((x)<<8)&0xFF00) \
   )


//! Usb_write_word_enum_struc
//! This macro help to fill the U16 fill in USB enumeration struct.
//! Depending on the CPU architecture, the macro swap or not the nibbles
//!
//! @param x        (U16) the 16 bit word to be written
//!
//! @return         (U16) the 16 bit word written
#if !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)
	#error YOU MUST Define the Endian Type of target: LITTLE_ENDIAN or BIG_ENDIAN
#endif
#ifdef LITTLE_ENDIAN
	#define Usb_write_word_enum_struc(x)	(x)
#else //BIG_ENDIAN
	#define Usb_write_word_enum_struc(x)	(wSWAP(x))
#endif


//_____ D E C L A R A T I O N ______________________________________________

U8      usb_config_ep                (U8, U8);
U8      usb_select_enpoint_interrupt (void);
U8      usb_send_packet              (U8 , U8*, U8);
U8      usb_read_packet              (U8 , U8*, U8);
void    usb_halt_endpoint            (U8);
U8      usb_init_device              (void);

U8      host_config_pipe             (U8, U8);
U8      host_determine_pipe_size     (U16);
void    host_disable_all_pipe        (void);
U8      usb_get_nb_pipe_interrupt    (void);

U8 usb_endpoint_wait_for_read_control_enabled();
U8 usb_endpoint_wait_for_write_enabled();
U8 usb_endpoint_wait_for_receive_out();
U8 usb_endpoint_wait_for_IN_ready();

#define usb_endpoint_wait_for_read_enabled usb_endpoint_wait_for_write_enabled

#endif  // _USB_DRV_H_

/** @} */

/**
 * \file
 *
 * \brief USB host driver for Vendor interface.
 *
 * Copyright (c) 2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef _UHI_VENDOR_H_
#define _UHI_VENDOR_H_

#include "conf_usb_host.h"
#include "usb_protocol.h"
#include "usb_protocol_vendor.h"
#include "uhi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup uhi_vendor_group
 * \defgroup uhi_vendor_group_uhc Interface with USB Host Core (UHC)
 *
 * Define and functions required by UHC.
 *
 * @{
 */

//! Global define which contains standard UHI API for UHC
//! It must be added in USB_HOST_UHI define from conf_usb_host.h file.
#define UHI_VENDOR { \
	.install = uhi_vendor_install, \
	.enable = uhi_vendor_enable, \
	.uninstall = uhi_vendor_uninstall, \
	.sof_notify = NULL, \
}

/**
 * \name Functions required by UHC
 * @{
 */
extern uhc_enum_status_t uhi_vendor_install(uhc_device_t* dev);
extern void uhi_vendor_enable(uhc_device_t* dev);
extern void uhi_vendor_uninstall(uhc_device_t* dev);
//@}
//@}

/**
 * \ingroup uhi_group
 * \defgroup uhi_vendor_group UHI for Vendor Class
 *
 * Common APIs used by high level application to use this USB host class.
 * 
 * This Vendor Class implementation supports one endpoint for all endpoint
 * types on all directions: Control IN, control OUT, interrupt IN,
 * interrupt OUT, bulk IN, bulk OUT, isochronous IN, isochronous OUT.
 *
 * This implementation is an example and can be a base to create another Vendor Class
 * which support more endpoint as two bulk IN endpoints.
 *
 * See \ref uhi_vendor_quickstart.
 * @{
 */

/**
 * \brief Start a transfer on control IN
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback
 * is called.
 * The \a callback returns the transfer status and eventually the number of byte
 * transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool uhi_vendor_control_in_run(uint8_t * buf, iram_size_t buf_size,
		uhd_callback_setup_end_t callback);

/**
 * \brief Start a transfer on control OUT
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback
 * is called.
 * The \a callback returns the transfer status and eventually the number of byte
 * transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool uhi_vendor_control_out_run(uint8_t * buf, iram_size_t buf_size,
		uhd_callback_setup_end_t callback);

/**
 * \brief Start a transfer on bulk IN
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback
 * is called.
 * The \a callback returns the transfer status and eventually the number of byte
 * transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool uhi_vendor_bulk_in_run(uint8_t * buf, iram_size_t buf_size,
		uhd_callback_trans_t callback);

/**
 * \brief Start a transfer on bulk OUT
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback
 * is called.
 * The \a callback returns the transfer status and eventually the number of byte
 * transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool uhi_vendor_bulk_out_run(uint8_t * buf, iram_size_t buf_size,
		uhd_callback_trans_t callback);

/**
 * \brief Start a transfer on interrupt IN
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback
 * is called.
 * The \a callback returns the transfer status and eventually the number of byte
 * transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool uhi_vendor_int_in_run(uint8_t * buf, iram_size_t buf_size,
		uhd_callback_trans_t callback);

/**
 * \brief Start a transfer on interrupt OUT
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback
 * is called.
 * The \a callback returns the transfer status and eventually the number of byte
 * transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 *
 */
bool uhi_vendor_int_out_run(uint8_t * buf, iram_size_t buf_size,
		uhd_callback_trans_t callback);

/**
 * \brief Start a transfer on ISO IN
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback
 * is called.
 * The \a callback returns the transfer status and eventually the number of byte
 * transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool uhi_vendor_iso_in_run(uint8_t * buf, iram_size_t buf_size,
		uhd_callback_trans_t callback);

/**
 * \brief Start a transfer on ISO OUT
 *
 * When the transfer is finished or aborted (stall, reset, ...), the \a callback
 * is called.
 * The \a callback returns the transfer status and eventually the number of byte
 * transfered.
 *
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param callback      NULL or function to call at the end of transfer
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 *
 */
bool uhi_vendor_iso_out_run(uint8_t * buf, iram_size_t buf_size,
		uhd_callback_trans_t callback);

/**
 * \brief Check if a transfer on BULK is possible
 *
 * \return \c 1 if possible, otherwise \c 0.
 */
bool uhi_vendor_bulk_is_available(void);

/**
 * \brief Check if a transfer on INTERRUPT is possible
 *
 * \return \c 1 if possible, otherwise \c 0.
 */
bool uhi_vendor_int_is_available(void);

/**
 * \brief Check if a transfer on ISO is possible
 *
 * \return \c 1 if possible, otherwise \c 0.
 */
bool uhi_vendor_iso_is_available(void);

// Modified control out function to upload firmware
bool uhi_vendor_control_out_firmware_upload(const uint8_t * buf, iram_size_t buf_size, uint8_t bRequest, le16_t wValue, uhd_callback_setup_end_t callback); // JOHN: Changed lolo's implementation to wValue from wIndex
//@}


/**
 * \page uhi_vendor_quickstart Quick start guide for USB host vendor module (UHI vendor)
 *
 * This is the quick start guide for the \ref uhi_vendor_group 
 * "USB host vendor module (UHI vendor)" with step-by-step instructions on 
 * how to configure and use the modules in a selection of use cases.
 *
 * The use cases contain several code fragments. The code fragments in the
 * steps for setup can be copied into a custom initialization function, while
 * the steps for usage can be copied into, e.g., the main application function.
 *
 * \section uhi_vendor_basic_use_case Basic use case
 * In this basic use case, the "USB Vendor (Single Class support)" module is
 * used.
 *
 * The "USB Vendor (Composite)" module usage is described in
 * \ref uhi_vendor_use_cases "Advanced use cases".
 *
 * \section uhi_vendor_basic_use_case_setup Setup steps
 * \subsection uhi_vendor_basic_use_case_setup_prereq Prerequisites
 * \copydetails uhc_basic_use_case_setup_prereq
 * \subsection uhi_vendor_basic_use_case_setup_code Example code
 * \copydetails uhc_basic_use_case_setup_code
 * \subsection uhi_vendor_basic_use_case_setup_flow Workflow
 * \copydetails uhc_basic_use_case_setup_flow
 *
 * \section uhi_vendor_basic_use_case_usage Usage steps
 *
 * \subsection uhi_vendor_basic_use_case_usage_code Example code
 * Content of conf_usb_host.h:
 * \code
 * #define USB_HOST_UHI        UHI_VENDOR
 * #define UHI_VENDOR_CHANGE(dev, b_plug) my_callback_vendor_change(dev, b_plug)
 * extern void my_callback_vendor_change(uhc_device_t* dev, bool b_plug);
 * #define UHI_VENDOR_VID_PID_LIST {USB_VID_ATMEL, USB_PID_ATMEL_ASF_VENDOR_CLASS}
 * #include "uhi_vendor.h" // At the end of conf_usb_host.h file
 * \endcode
 *
 * Add to application C-file:
 * \code
 * static bool my_flag_vendor_test_start = false;
 * void my_callback_vendor_change(uhc_device_t* dev, bool b_plug)
 * {
 *    // USB Device Vendor connected
 *    my_flag_vendor_test_start = b_plug;
 * }
 *
 * static void my_callback_bulk_in_done (usb_add_t add,
 *         usb_ep_t ep, uhd_trans_status_t status, iram_size_t nb_transfered)
 * {
 *   if (status != UHD_TRANS_NOERROR) {
 *     return; // Error during transfer
 *   }
 *   // Data received then restart test
 *   my_flag_vendor_test_start = true;
 * }
 *
 * #define MESSAGE "Hello bulk"
 * #define HELLO_SIZE 5
 * #define HELLO_BULK_SIZE 10
 * uint8_t my_out_buffer[MESSAGE_SIZE+1] = MESSAGE;
 * uint8_t my_in_buffer[MESSAGE_SIZE+1];
 * void my_task(void)
 * {
 *    if (!my_flag_vendor_test_start) {
 *      return;
 *    }
 *    my_flag_vendor_test_start = false;
 *
 *    // Send data through control endpoint
 *    uhi_vendor_control_out_run(my_out_buffer, HELLO_SIZE, NULL);
 *
 *    // Check if bulk endpoints are available
 *    if (uhi_vendor_bulk_is_available()) {
 *      // Send data through bulk OUT endpoint
 *      uhi_vendor_bulk_out_run(my_out_buffer, HELLO_BULK_SIZE, NULL);
 *      // Receive data through bulk IN endpoint
 *      uhi_vendor_bulk_in_run(my_in_buffer, sizeof(my_in_buffer),
 *              my_callback_bulk_in_done);
 *    }
 * }
 * \endcode
 *
 * \subsection uhi_vendor_basic_use_case_setup_flow Workflow
 * -# Ensure that conf_usb_host.h is available and contains the following
 * configuration which is the USB host vendor configuration:
 *   - \code #define USB_HOST_UHI   UHI_HID_VENDOR \endcode
 *     \note It defines the list of UHI supported by USB host.
 *   - \code #define UHI_VENDOR_CHANGE(dev, b_plug) my_callback_vendor_change(dev, b_plug)
 * extern bool my_callback_vendor_change(uhc_device_t* dev, bool b_plug); \endcode
 *     \note This callback is called when a USB device vendor is plugged or unplugged.
 *   - \code #define UHI_VENDOR_VID_PID_LIST {USB_VID_ATMEL, USB_PID_ATMEL_ASF_VENDOR_CLASS} \endcode
 *     \note It defines the list of devices supported by USB host (defined by VID and PID).
 * -# The Vendor data transfert functions are described in \ref uhi_vendor_group.
 *   - \code uhi_vendor_control_out_run(), uhi_vendor_bulk_out_run(),... \endcode
 *
 * \section uhi_vendor_use_cases Advanced use cases
 * - \subpage uhc_use_case_1
 * - \subpage uhc_use_case_2
 * - \subpage uhc_use_case_3
 */


#ifdef __cplusplus
}
#endif
#endif // _UHI_VENDOR_H_

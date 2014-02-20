/**
 * \file
 *
 * \brief Common API for USB Host Drivers (UHD)
 *
 * Copyright (C) 2011 - 2012 Atmel Corporation. All rights reserved.
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

#ifndef _UHD_H_
#define _UHD_H_

#include "usb_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup usb_host_group
 * \defgroup uhd_group USB Host Driver (UHD)
 *
 * The UHD driver provides a low-level abstraction of the host
 * controller hardware. Most events coming from the hardware such as
 * interrupts may cause the UHD performing function call in UHC and UHI.
 *
 * @{
 */

//! \brief Device speed
typedef enum {
	UHD_SPEED_LOW  = 0,
	UHD_SPEED_FULL = 1,
	UHD_SPEED_HIGH = 2,
} uhd_speed_t;

/**
 * \brief Endpoint transfer status
 * The status field is updated after each transaction attempt,
 * whether successful or not.
 */
typedef enum {
	//! Transaction is successful
	UHD_TRANS_NOERROR = 0,

	//! Device is disconnected
	UHD_TRANS_DISCONNECT,

	//! CRC error in data packet
	UHD_TRANS_CRC,

	//! Data toggle PID did not match the expected value
	UHD_TRANS_DT_MISMATCH,

	//! The endpoint returned a STALL PID
	UHD_TRANS_STALL,

	//! Device did not respond to token (IN)
	//! or did not provide a handshake (OUT)
	UHD_TRANS_NOTRESPONDING,

	//! Check bits on PID from endpoint failed
	UHD_TRANS_PIDFAILURE,

	//! Data transmission not completed before timeout
	UHD_TRANS_TIMEOUT,

	//! Data transmission has been aborted
	UHD_TRANS_ABORTED,
} uhd_trans_status_t;

/**
 * \brief End of reset callback function type.
 * Registered by uhd_send_reset()
 * Callback called when reset event is completed.
 */
typedef void (*uhd_callback_reset_t)(void);

/**
 * \brief Data setup transfer callback function type.
 * Registered by uhd_setup_request()
 * Called during DATA phase when the (payload) buffer is full or empty.
 * Then the setup request is halted.
 * A new buffer can be provided to continue the DATA phase or
 * abort DATA phase.
 *
 * \param add           USB address of the setup request
 * \param payload       To return the next buffer address
 * \param payload_size  To return the size of next buffer
 *
 * \return \c true, if a new buffer is available, otherwise stop the request.
 */
typedef bool (*uhd_callback_setup_run_t)(
		usb_add_t add,
		uint8_t **payload,
		uint16_t *payload_size );

/**
 * \brief End of setup callback function type.
 * Registered by uhd_setup_request()
 * Called when the setup request is completed.
 *
 * \param add           USB address of the setup request
 * \param status        Transfer status
 * \param payload_trans Number of data transfered during DATA phase
 */
typedef void (*uhd_callback_setup_end_t)(
		usb_add_t add,
		uhd_trans_status_t status,
		uint16_t payload_trans);

/**
 * \brief End of transfer callback function type.
 * Registered by uhd_ep_run()
 * Callback called by USB interrupt after data transfer or abort (reset,...).
 *
 * \param add           USB address used by the transfer
 * \param status        Transfer status
 * \param nb_transfered Number of data transfered
 */
typedef void (*uhd_callback_trans_t) (
		usb_add_t add,
		usb_ep_t ep,
		uhd_trans_status_t status,
		iram_size_t nb_transfered);

/**
 * \brief Enables the USB host mode
 * Start the ID pin management if the ID pin is available.
 */
void uhd_enable(void);

/**
 * \brief Disables the USB host mode
 *
 * \param b_id_stop  Stop ID pin management, if true.
 */
void uhd_disable(bool b_id_stop);

/**
 * \brief Returns the speed of connected device
 *
 * \return Device speed
 */
uhd_speed_t uhd_get_speed(void);

/**
 * \brief Returns the current Start Of Frame (SOF) number
 *
 * \return current start of frame number.
 */
uint16_t uhd_get_frame_number(void);

/**
 * \brief Returns the current micro start of frame number
 *
 * \return current micro start of frame number required in high speed mode.
 */
uint16_t uhd_get_microframe_number(void);

/**
 * \brief Enables the Reset state on the USB line.
 *
 * \param callback  Callback when reset sequence is finished
 */
void uhd_send_reset(uhd_callback_reset_t callback );

/**
 * \brief Enables the suspend state on the USB line.
 * The SUSPEND state is enable when SOF are disabled on USB line.
 */
void uhd_suspend(void);

/**
 * \brief Test if the suspend state is enabled on the USB line.
 * \return USB line in SUSPEND state, if true
 */
bool uhd_is_suspend(void);

/**
 * \brief Enables the IDLE state on the USB line.
 * The IDLE state is enable when SOF are present on USB line.
 * A “Downstream Resume” signal can be sent.
 */
void uhd_resume(void);

/**
 * \brief Add a setup request in the control endpoint setup queue.
 * Note: Request timeout is 5s.
 *
 * \param add           USB address of control endpoint
 * \param req           Setup request definition
 * \param payload       Buffer to use in setup DATA phase
 * \param payload_size  Size of buffer used in DATA phase
 * \param callback_run  Callback to call if buffer is empty or full
 * \param callback_end  Callback to call when request is finish
 *
 * \return \c true if the request has been accepted, otherwise \c false.
 * Note: The swap of "req.wValues" from uin16_t to le16_t is done by UHD.
 */
bool uhd_setup_request(
		usb_add_t add,
		usb_setup_req_t *req,
		uint8_t *payload,
		uint16_t payload_size,
		uhd_callback_setup_run_t callback_run,
		uhd_callback_setup_end_t callback_end);



/**
 * \name Endpoint Management
 *
 * The following functions allow drivers to create and remove
 * endpoints, as well as set, clear and query their "halted" and
 * "wedged" states.
 */
//@{

/**
 * \brief Configures and enables a control endpoint 0
 *
 * \param add              USB address of endpoint
 * \param ep_size          Endpoint control maximum size
 *
 * \return \c 1 if the endpoint is enabled, otherwise \c 0.
 */
bool uhd_ep0_alloc(usb_add_t add, uint8_t ep_size);

/**
 * \brief Configures and enables an endpoint
 *
 * \param add              USB address of endpoint
 * \param ep_desc          Endpoint descriptor
 *
 * \return \c 1 if the endpoint is enabled, otherwise \c 0.
 */
bool uhd_ep_alloc(usb_add_t add, usb_ep_desc_t *ep_desc);

/**
 * \brief Disables an endpoint or all endpoint of a device
 *
 * \param add              USB address of endpoint
 * \param endp             Endpoint number and direction (USB_EP_DIR_IN/OUT).
 *                         Remove all endpoints of USB address, if 0xFF.
 */
void uhd_ep_free(usb_add_t add, usb_ep_t endp);

/**
 * \brief Allows to receive or send data on an endpoint
 *
 * If a USB DMA is available, the driver uses it to transfer endpoint data
 * from or to internal RAM.
 * When the transfer is finished or aborted (stall, reset, ...),
 * the \a callback is called. This callback returns the transfer status
 * and eventually the number of byte transfered.
 * Note: The control endpoint is not authorized.
 *
 * \param add           USB address of endpoint
 * \param endp          Endpoint number
 * \param b_shortpacket Enabled automatic short packet
 * \param buf           Buffer on Internal RAM to send or fill.
 *                      It must be align, then use COMPILER_WORD_ALIGNED.
 * \param buf_size      Buffer size to send or fill
 * \param timeout       Transfer timeout (ms)
 * \param callback      NULL or function to call at the end of transfer
 *
 * \warning About \a b_shortpacket, for OUT endpoint it means that
 * a short packet or a Zero Length Packet must be sent to the USB line
 * to properly close the usb transfer at the end of the data transfer.
 * For Bulk and Interrupt IN endpoint, it will automatically stop the transfer
 * at the end of the data transfer (received short packet).
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool uhd_ep_run(
		usb_add_t add,
		usb_ep_t endp,
		bool b_shortpacket,
		uint8_t *buf,
		iram_size_t buf_size,
		uint16_t timeout,
		uhd_callback_trans_t callback);

/**
 * \brief Aborts an on-going transfer on an endpoint
 *
 * If a transfer is on going, then it is stopped and
 * the callback registered is called to signal the end of transfer.
 * Note: The control endpoint is not authorized.
 *
 * \param add           USB address of endpoint
 * \param endp          Endpoint to abort
 *
 */
void uhd_ep_abort(usb_add_t add, usb_ep_t endp);

//@}


/**
 * \name High speed test mode management
 *
 * The following functions allow the host to jump to a specific test mode
 * required in high speed mode.
 */
//@{
void uhd_test_mode_j(void);
void uhd_test_mode_k(void);
void uhd_test_mode_se0_nak(void);
void uhd_test_mode_packet(void);
//@}


/**
 * \name UHC callbacks to provide for UHD
 *
 * The following callbacks are used by UHD.
 */
//@{

#ifndef _UHC_H_ /* uhc.h is not included before */
/**
 * \brief Starts the host mode
 */
extern void uhc_start(void);

/**
 * \brief Stops the host mode
 */
extern void uhc_stop(bool b_id_stop);
#endif

/**
 * \brief Notify device connection or disconnection
 *
 * \param b_plug  Device connection, if true
 */
extern void uhc_notify_connection(bool b_plug);

/**
 * \brief Notify each start of frame sent by driver
 *
 * \param b_micro  It is a micro start of frame, if true
 */
extern void uhc_notify_sof(bool b_micro);

/**
 * \brief Notify that a resume bus occurs
 * A resume can occur after a downstream or an upstream resume.
 */
extern void uhc_notify_resume(void);

//@}

//@}

#ifdef __cplusplus
}
#endif
#endif // _UHD_H_

/**
 * \file
 *
 * \brief Common API for USB Host Interface
 *
 * Copyright (C) 2011 Atmel Corporation. All rights reserved.
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

#ifndef _UHI_H_
#define _UHI_H_

#include "conf_usb_host.h"
#include "usb_protocol.h"
#include "uhc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup usb_host_group
 * \defgroup uhi_group USB Host Interface (UHI)
 * The UHI provides a common API for all classes,
 * and this is used by UHC for the main control of USB host interface.
 * @{
 */

/**
 * \brief UHI API.
 *
 * The callbacks within this structure are called only by
 * USB Host Controller (UHC)
 */
typedef struct {
	/**
	 * \brief Install interface
	 * Allocate interface endpoints if supported.
	 *
	 * \param uhc_device_t    Device to request
	 *
	 * \return status of the install
	 */
	uhc_enum_status_t (*install)(uhc_device_t*);

	/**
	 * \brief Enable the interface.
	 *
	 * Enable a USB interface corresponding to UHI.
	 *
	 * \param uhc_device_t    Device to request
	 */
	void (*enable)(uhc_device_t*);

	/**
	 * \brief Uninstall the interface (if installed)
	 *
	 * \param uhc_device_t    Device to request
	 */
	void (*uninstall)(uhc_device_t*);

	/**
	 * \brief Signal that a SOF has occurred
	 */
	void (*sof_notify)(bool b_micro);
} uhi_api_t;

//@}

#ifdef __cplusplus
}
#endif
#endif // _UHI_H_

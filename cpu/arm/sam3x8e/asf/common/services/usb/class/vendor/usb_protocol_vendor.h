/**
 * \file
 *
 * \brief USB Vendor class protocol definitions.
 *
 * Copyright (c) 2011 Atmel Corporation. All rights reserved.
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

#ifndef _USB_PROTOCOL_VENDOR_H_
#define _USB_PROTOCOL_VENDOR_H_

/**
 * \ingroup usb_protocol_group
 * \defgroup usb_vendor_protocol USB Vendor Class definitions
 *
 * @{
 */

/**
 * \name Vendor class values
 */
//@{
#define  VENDOR_CLASS                  0x08
#define  VENDOR_SUBCLASS               0x02
#define  VENDOR_PROTOCOL               0x50

#define  VENDOR_CLASS_2                  0xFF
#define  VENDOR_SUBCLASS_2               0x00
#define  VENDOR_PROTOCOL_2               0x00

#define  VENDOR_CLASS_3                  0xFF
#define  VENDOR_SUBCLASS_3               0xFF
#define  VENDOR_PROTOCOL_3               0xFF
//@}

#define USB_ATHEROS_VID			0x057c
#define USB_ATHEROS_PID_WLAN	0x8401
#define USB_ATHEROS_PID_MSC		0x84ff
//@}

#endif // _USB_PROTOCOL_VENDOR_H_

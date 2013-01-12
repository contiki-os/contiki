/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-usb
 * @{
 *
 * \file
 * Header file for cc2538's UART-like I/O over USB
 */
#ifndef USB_SERIAL_H_
#define USB_SERIAL_H_

#include "contiki.h"

/**
 * \brief Initialise the Serial-over-USB process
 */
void usb_serial_init(void);

/**
 * \brief Write a byte over USB
 * \param b The byte
 * \sa usb_serial_flush()
 *
 * USB-Serial output is buffered. The buffer is actually submitted to the USB
 * controller for transmission when:
 *  - It is full or
 *  - The code explicitly calls usb_serial_flush()
 */
void usb_serial_writeb(uint8_t b);

/**
 * \brief Set an input hook for bytes received over USB
 * \param input A pointer to a function to be called when a byte is received
 */
void usb_serial_set_input(int (* input)(unsigned char c));

/**
 * \brief Immediately transmit the content of Serial-over-USB TX buffers
 * \sa usb_serial_writeb()
 */
void usb_serial_flush(void);

#endif /* USB_SERIAL_H_ */

/** @} */

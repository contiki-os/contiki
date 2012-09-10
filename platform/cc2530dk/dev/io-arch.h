/*
 * Copyright (c) 2012, George Oikonomou (oikonomou@users.sourceforge.net)
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
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         Platform-specific header file which switches between UART and USB
 *         input/output, depending on whether we are building for the
 *         cc2531 USB dongle or the SmartRF
 *
 *         These configuration directives are hardware-specific and you
 *         normally won't have to modify anything in this file.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef IO_ARCH_H_
#define IO_ARCH_H_

#include "contiki.h"
/*---------------------------------------------------------------------------*/
#if MODEL_CC2531
#include "dev/usb-serial.h"
#define IO_ARCH_PREFIX usb_serial

/*
 * When using the cc2531 USB stick, we have the option to enable buffering
 * for USB-serial output. In this case, the driver will implicitly flush the
 * output when the TX buffer is full or when it encounters SLIP_END or '\n'.
 * We can trigger an explicit flush by calling io_arch_flush(). Naturally, this
 * does not make sense when our output uses the UART nor when buffering is
 * disabled, in which cases the calls are simply pre-processed to nothingness
 */
#define io_arch_flush() usb_serial_flush()
#else
#include "dev/uart0.h"
#define IO_ARCH_PREFIX uart0
#define io_arch_flush()
#endif
/*---------------------------------------------------------------------------*/
/* Expands to uart0_init(), usb_serial_init() */
#define io_arch_init() io_arch_init_x(IO_ARCH_PREFIX)
#define io_arch_writeb(b) io_arch_writeb_x(IO_ARCH_PREFIX, b)
#define io_arch_set_input(f) io_arch_set_input_x(IO_ARCH_PREFIX, f)
/*---------------------------------------------------------------------------*/
/* Second round of macro substitutions. You can stop reading here */
#define io_arch_init_x(prefix) io_arch_init_x_x(prefix)
#define io_arch_writeb_x(prefix, b) io_arch_writeb_x_x(prefix, b)
#define io_arch_set_input_x(prefix, f) io_arch_set_input_x_x(prefix, f)
/*---------------------------------------------------------------------------*/
#define io_arch_init_x_x(prefix) prefix##_init()
#define io_arch_writeb_x_x(prefix, b) prefix##_writeb(b)
#define io_arch_set_input_x_x(prefix, f) prefix##_set_input(f)

#endif /* IO_ARCH_H_ */

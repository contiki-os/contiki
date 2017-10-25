/*
 * Copyright (c) 2016, George Oikonomou - http://www.spd.gr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
/*---------------------------------------------------------------------------*/
#ifndef JN516X_IO_H_
#define JN516X_IO_H_
/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "dev/uart0.h"
#include "dev/uart1.h"
/*---------------------------------------------------------------------------*/
#ifdef JN516X_IO_CONF_USE_UART1
#define JN516X_IO_USE_UART1 JN516X_IO_CONF_USE_UART1
#else
#define JN516X_IO_USE_UART1 0
#endif
/*---------------------------------------------------------------------------*/
#if JN516X_IO_USE_UART1
#define sensniff_io_byte_out(b)  uart1_writeb(b)
#define sensniff_io_flush()
#define sensniff_io_set_input(f) uart1_set_input(f)
#else
#define sensniff_io_byte_out(b)  uart0_writeb(b)
#define sensniff_io_flush()
#define sensniff_io_set_input(f) uart0_set_input(f)
#endif
/*---------------------------------------------------------------------------*/
#endif /* JN516X_IO_H_ */
/*---------------------------------------------------------------------------*/

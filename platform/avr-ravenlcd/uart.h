/*
 *  Copyright (c) 2008  Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *
 * \brief
 *      Handles the control of the USART for communication with the ATmega1284p
 *      for sending commands.
 *
 * \author
 *      Mike Vidales mavida404@gmail.com
 *
 */

#ifndef __UART_H__
#define __UART_H__   1

#include <inttypes.h>

/** \name ASCII characters defined */
/** \{ */
#define SOF_CHAR            (0x01)  /**< Start-of-frame character. */
#define EOF_CHAR            (0x04)  /**< End-of-frame character. */
/** \} */

/* Macros & Defines */

#define BUFSIZE 80
#define BAUD_RATE_38400     (12)

/** \brief Circular buffer structure */
typedef struct {
    volatile uint8_t head;  /**< Index to last available character in buffer. */
    volatile uint8_t tail;  /**< Index to first available character in buffer. */
    uint8_t buf[BUFSIZE];   /**< The actual buffer used for storing characters. */
} tcirc_buf;

extern tcirc_buf rxbuf;
#define rx_char_ready() (rxbuf.head != rxbuf.tail)

/* Serial port functions */
void uart_init(void);
void uart_deinit(void);
uint8_t uart_circ_buf_has_char(tcirc_buf *cbuf);
void uart_clear_rx_buf(void);
uint8_t uart_get_char_rx(void);
void uart_send_byte(uint8_t byte);
void uart_serial_send_frame(uint8_t cmd, uint8_t payload_length, uint8_t *payload);
void uart_serial_rcv_frame(uint8_t wait_for_it);

#endif /* __UART_H__ */

/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Buffered, interrupt driven serial I/O
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.1 $
 *
 * DATED:              $Date: 2008/10/17 10:17:56 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             LJM
 *
 * DESCRIPTION:
 * Just some simple common uart functions (header file)
 *
 * CHANGE HISTORY:
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

/* Integrated into Contiki by Beshr Al Nahas */

#ifndef  UARTDRIVER_H_INCLUDED
#define  UARTDRIVER_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

#include <jendefs.h>
#include "contiki-conf.h"

void uart_driver_init(uint8_t uart_dev, uint8_t br, uint8_t *txbuf_data, uint16_t txbuf_size, uint8_t *rxbuf_data, uint16_t rxbuf_size, int (*uart_input_function)(unsigned char c));
void uart_driver_write_buffered(uint8_t uart_dev, uint8_t ch);
void uart_driver_write_with_deadline(uint8_t uart_dev, uint8_t c);
uint8_t uart_driver_read(uint8_t uart_dev, uint8_t *data);
void uart_driver_write_direct(uint8_t uart_dev, uint8_t ch);
void uart_driver_set_input(uint8_t u8Uart, int (*uart_input_function)(unsigned char c));

void uart_driver_rx_handler(uint8_t uart_dev);
void uart_driver_enable_interrupts(uint8_t uart_dev);
void uart_driver_disable_interrupts(uint8_t uart_dev);
int8_t uart_driver_interrupt_is_enabled(uint8_t uart_dev);
void uart_driver_store_interrupts(uint8_t uart_dev);
void uart_driver_restore_interrupts(uint8_t uart_dev);

uint8_t uart_driver_tx_in_progress(uint8_t uart_dev);

#ifdef UART_EXTRAS
void uart_driver_flush(uint8_t uart_dev);
#endif


#if defined __cplusplus
}
#endif

#endif  /* UARTDRIVER_H_INCLUDED */

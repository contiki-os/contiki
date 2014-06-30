/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Jim Paris <jim.paris@rigado.com>
 */

#include <aducrf101-include.h>

static int (*uart_input_handler)(unsigned char c);
static int stdout_enabled;

void
uart_init(int baud)
{
  /* P1.0 is UARTRXD, P1.1 is UARTTXD */
  pADI_GP1->GPCON &= ~(GP1CON_CON0_MSK | GP1CON_CON1_MSK);
  pADI_GP1->GPCON |= GP1CON_CON0_UART0RXD | GP1CON_CON1_UART0TXD;

  /* Set P1.1 as output */
  GP1OEN_OEN1_BBA = 1;

  /* Set baudrate */
  int div = (F_CPU / 32) / baud;
  pADI_UART->COMDIV = div;
  pADI_UART->COMFBR = 0x8800 | ((((64 * F_CPU) / div) / baud) - 2048);
  pADI_UART->COMIEN = 0;
  pADI_UART->COMLCR = 3;

  /* Set up RX IRQ */
  pADI_UART->COMIEN = COMIEN_ERBFI_EN;
  NVIC_EnableIRQ(UART_IRQn);
  __enable_irq();

  uart_input_handler = NULL;
  stdout_enabled = 1;
}
/*---------------------------------------------------------------------------*/
void
uart_put(unsigned char x)
{
  while(!(pADI_UART->COMLSR & COMLSR_THRE))
    continue;
  pADI_UART->COMTX = x;
}
/*---------------------------------------------------------------------------*/
void
UART_Int_Handler(void)
{
  if(pADI_UART->COMIIR & COMIIR_STA_RXBUFFULL) {
    unsigned char x = pADI_UART->COMRX;
    if(uart_input_handler) {
      uart_input_handler(x);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
uart_set_input(int (*input)(unsigned char c))
{
  uart_input_handler = input;
}
void
uart_enable_stdout(int enabled)
{
  stdout_enabled = enabled;
}
/*---------------------------------------------------------------------------*/
/* Connect newlib's _write function to the UART. */
int
_write(int fd, const void *buf, size_t len)
{
  if(stdout_enabled == 0) {
    return -1;
  }

  if(fd == 1 || fd == 2) {
    int n = len;
    const unsigned char *p = buf;
    while(n--)
      uart_put(*p++);
    return len;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
#ifdef __ICCARM__
/* Connect IAR's __write function to the UART. */
size_t
__write(int fd, const unsigned char *buf, size_t count)
{
  return _write(fd, buf, count);
}
#endif

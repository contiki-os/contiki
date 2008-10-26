/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file uart_usb_lib.c *********************************************************************
 *
 * \brief
 *      This file controls the UART USB functions.
 *
 * \addtogroup usbstick
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 ******************************************************************************/
/* Copyright (c) 2008  ATMEL Corporation
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/*_____ I N C L U D E S ____________________________________________________*/

#include "config.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "serial/cdc_task.h"
#include "serial/uart_usb_lib.h"
#include <stdio.h>

/**
 \addtogroup cdctask
 @{
 */

/*_____ M A C R O S ________________________________________________________*/

/*_____ D E F I N I T I O N ________________________________________________*/

Uchar tx_counter;
Uchar rx_counter;
S_line_coding   line_coding;

/*_____ D E C L A R A T I O N ______________________________________________*/


int usb_stdout_putchar(char c, FILE *stream)
	{
    // send to USB port
    // don't send anything if USB can't accept chars
     Usb_select_endpoint(TX_EP);
     if (!uart_usb_tx_ready())
			return 0;
	        
	 // turn on LED
	 vcptx_end_led();

	uart_usb_putchar(c);
	 
	return 0;
	}

static FILE usb_stdout = FDEV_SETUP_STREAM(usb_stdout_putchar,
					     NULL,
					     _FDEV_SETUP_WRITE);

/**
  * @brief Initializes the uart_usb library
  */
void uart_usb_init(void)
{
  tx_counter = 0;
  rx_counter = 0;
}

void uart_usb_set_stdout(void)
{
  stdout = &usb_stdout;
}


/**
  * @brief This function checks if the USB emission buffer is ready to accept at
  * at least 1 byte
  *
  * @retval TRUE if the firmware can write a new byte to transmit.
  * @retval FALSE otherwise
  */
bit uart_usb_tx_ready(void)
{
  if (!Is_usb_write_enabled())
  {
    return FALSE;
  }
  return TRUE;
}

/**
  * @brief This function fills the USB transmit buffer with the new data. This buffer
  * is sent if complete. To flush this buffer before waiting full, launch
  * the uart_usb_flush() function.
  *
  * @param data_to_send Data to send
  *
  * @return data_to_send Data that was sent
  */
int uart_usb_putchar(int data_to_send)
{
  	Usb_select_endpoint(VCP_TX_EP);
    
	if(!uart_usb_tx_ready()) return -1;
	
	Usb_write_byte(data_to_send);
	tx_counter++;
  	if(!Is_usb_write_enabled()) //If Endpoint full -> flush
  	{
   	uart_usb_flush();
  	}
	return data_to_send;
}

/** 
  * @brief This function checks if a character has been received on the USB bus.
  * 
  * @return bit (true if a byte is ready to be read)
  */
bit uart_usb_test_hit(void)
{
  if (!rx_counter)
  {
    Usb_select_endpoint(VCP_RX_EP);
    if (Is_usb_receive_out())
    {
      rx_counter = Usb_byte_counter();
      if (!rx_counter)
		{
        Usb_ack_receive_out();
		}
    }
  }
  return (rx_counter!=0);
}

/** 
  * @brief This function reads one byte from the USB bus
  *
  * If one byte is present in the USB fifo, this byte is returned. If no data
  * is present in the USB fifo, this function waits for USB data.
  * 
  * @return U8 byte received
  */
char uart_usb_getchar(void)
{
  register Uchar data_rx;

  Usb_select_endpoint(VCP_RX_EP);
  if (!rx_counter) while (!uart_usb_test_hit());
  data_rx=Usb_read_byte();
  rx_counter--;
  if (!rx_counter) Usb_ack_receive_out();
  
  //Local echo
  uart_usb_putchar(data_rx);
  
  return data_rx;
}


/**
  * @brief This function sends the data stored in the USB transmit buffer.
  * This function does nothing if there is no data in the buffer.
  */
void uart_usb_flush (void)
{
   Usb_select_endpoint(VCP_TX_EP);
   Usb_send_in();
	tx_counter = 0;
}

/** @} */

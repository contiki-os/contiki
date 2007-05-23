/*
 * Copyright (c) 2003, Groepaz/Hitmen.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * rs232silversurfer.h
 *
 * Groepaz/Hitmen, 16.12.2001
 *
 * This defines for the SilverSurver (16c550 UART) what Ullrichs rs232 module
 * defines for the Swithlink/Turbo232
 *
 * this driver operates in polling mode only atm !
 *
 */

#ifndef _RS232silversurfer_H
#define _RS232silversurfer_H

/*****************************************************************************/
/*                   Data                                                    */
/*****************************************************************************/

/* Baudrate settings */
#define RS_BAUD_50                0x00
#define RS_BAUD_110               0x10
#define RS_BAUD_134_5             0x20
#define RS_BAUD_300               0x30
#define RS_BAUD_600               0x40
#define RS_BAUD_1200              0x50
#define RS_BAUD_2400              0x60
#define RS_BAUD_4800              0x70
#define RS_BAUD_9600              0x80
#define RS_BAUD_19200             0x90
#define RS_BAUD_38400             0xa0
#define RS_BAUD_57600             0xb0
#define RS_BAUD_115200            0xc0
#define RS_BAUD_230400            0xd0

/* Stop bit settings */
#define RS_STOP_1      	       		0x00
#define RS_STOP_2      	       		0x04

/* Data bit settings */
#define RS_BITS_5      	       		0x00
#define RS_BITS_6      	       		0x01
#define RS_BITS_7      	       		0x02
#define RS_BITS_8      	       		0x03

/* Parity settings */
#define RS_PAR_NONE    	       		0x00
#define RS_PAR_ODD     	       		0x28
#define RS_PAR_EVEN    	       		0x38
#define RS_PAR_MARK    	       		0x48
#define RS_PAR_SPACE   	       		0x48

/* Bit masks to mask out things from the status returned by rs232_status */
#define RS_STATUS_IRQ                   0x01    /* (iir) IRQ condition */
#define RS_STATUS_OVERRUN               0x02    /* (lsr) Overrun error */
#define RS_STATUS_PE                    0x04    /* (lsr) Parity error */
#define RS_STATUS_FE                    0x08    /* (lsr) Framing error */
#define RS_STATUS_DSR                   0x10    /* (msr>>1) NOT data set ready */
#define RS_STATUS_THRE                  0x20    /* (lsr) Transmit holding reg. empty */
#define RS_STATUS_DCD                   0x40    /* (msr>>1) NOT data carrier detect */
#define RS_STATUS_RDRF                  0x80    /* Receiver data register full */

/* Error codes returned by all functions */
#define RS_ERR_OK                       0x00    /* Not an error - relax */
#define RS_ERR_NOT_INITIALIZED 		0x01   	/* Module not initialized */
#define RS_ERR_BAUD_TOO_FAST            0x02    /* Cannot handle baud rate */
#define RS_ERR_BAUD_NOT_AVAIL           0x03    /* Baud rate not available */
#define RS_ERR_NO_DATA                  0x04    /* Nothing to read */
#define RS_ERR_OVERFLOW                 0x05    /* No room in send buffer */

/*****************************************************************************/
/*				     Code			                                               	     */
/*****************************************************************************/

unsigned char __fastcall__ rs232_init (char hacked);
/* Initialize the serial port, install the interrupt handler. The parameter
 * has no effect for now and should be set to 0.
 */

unsigned char __fastcall__ rs232_params (unsigned char params, unsigned char parity);
/* Set the port parameters. Use a combination of the #defined values above. */

unsigned char __fastcall__ rs232_done (void);
/* Close the port, deinstall the interrupt hander. You MUST call this function
 * before terminating the program, otherwise the machine may crash later. If
 * in doubt, install an exit handler using atexit(). The function will do
 * nothing, if it was already called.
 */

unsigned char __fastcall__ rs232_get (char* b);
/* Get a character from the serial port. If no characters are available, the
 * function will return RS_ERR_NO_DATA, so this is not a fatal error.
 */

unsigned char __fastcall__ rs232_put (char b);
/* Send a character via the serial port. There is a transmit buffer, but
 * transmitting is not done via interrupt. The function returns
 * RS_ERR_OVERFLOW if there is no space left in the transmit buffer.
 */

unsigned char __fastcall__ rs232_pause (void);
/* Assert flow control and disable interrupts. */

unsigned char __fastcall__ rs232_unpause (void);
/* Re-enable interrupts and release flow control */

unsigned char __fastcall__ rs232_status (unsigned char* status,
					 unsigned char* errors);
/* Return the serial port status. */

/* End of rs232silversurfer.h */
#endif




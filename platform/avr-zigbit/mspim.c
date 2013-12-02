/* Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *
 *\brief
 *	Functions to control the USART0 how SPI master Mode.
 * \author
 *	Juan Carlos Abdala jcabdala@insus.com.ar
 */

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "mspim.h"


void mspim0Init(void)
{
	DDRE |= (1 << 1) | (1 << 2);

	 // clear baud rate
	 UBRR0 = 0;

	 // Set mode of operation
	 UCSR0C = (1 << UMSEL01) | (1 << UMSEL00) | (0 << UCPOL0) | (1 << UCSZ00);

	 // Enable Tx and Rx
	 UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	 // set baud rate (MCK / 2), must be done after tx enable
	 UBRR0 = 0;
}

void mspim0SendByte(uint8_t u8Data)
{
	//wait while previous byte is completed
	while (!(UCSR0A & (1 << UDRE0)))
	{
	};
	// Transmit data
	UDR0 = u8Data;
}

signed int mspim0Receive2Byte()
{
	signed int temp;
	int msb = 0, lsb = 0;
	uint8_t first = 0;

	//	Wait for byte to be received

	for (;;)
	{
		if ((UCSR0A & (1 << RXC0))) //finalize 8bit recive
		{

			if (first == 0) //frist time recive 8bit msb
			{
				msb = UDR0;
				printf("msb %x\n", msb);
				mspim0SendByte(0);
				first = 1;
			}
			else //second time recive 8bit lsb
			{
				lsb = UDR0;
				printf("lsb %x\n", lsb);
				break;
			}
		}
	}

	//save in temp 16bits of convertion
	temp = ((msb) << 8);
	temp = temp | lsb;

	// Return received data
	return temp;
}

uint8_t mspim0ReceiveByte()
{
    uint8_t Byte;
    mspim0SendByte(0);

    while(!(UCSR0A & (1 << RXC0)))
	{

	}

    Byte = UDR0;

    return Byte;
}

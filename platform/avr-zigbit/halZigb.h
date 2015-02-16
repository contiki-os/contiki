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
 *	Mapping ZIGBIT PORTS FROM ATMEGA1281
 * \author
 *	Juan Pablo Leal Licudis jplicudis@insus.com.ar
 */

#ifndef HAL_ZIGB_H
#define HAL_ZIGB_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

/**Mapping ZIGBIT PORTS FROM ATMEGA1281
* It was implemented based on the information found in this link.
* http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=67368&start=0
* Explanation
* Code:
* #define REGISTER_BIT(rg,bt) ((volatile _io_reg*)&rg)->bit##bt 
* The above macro works like this: 
* #define is the start of a macro definition 
* REGISTER_BIT is the name of the macro 
* (tg,bt) are the parameters to the macro 
* The C keyword volatile is accurately described in the AVR-libc Manual Frequently Asked Questions, in the FAQ item #1. It is also described * in the article Introduction to the "Volatile" Keyword. 
* _io_reg is the struct type that is typedef'ed right above the macro, and thus 
* _io_reg* is a pointer to such a struct, and so 
* (volatile _io_reg*) is a typecast to "a volatile pointer to a _io_reg struct" 
* & is the "address of" operator, and thus 
* &rg is the address of rg, ie the address of the first parameter to the macro 
* The -> is the "member by pointer operator", in other words it "refers" to one of the fields in the struct that the pointer points to. 
* ## is the token pasting operator (of the C preprocessor). The preprocessor concatenates the two operands together so that the compiler *  
* sees one token, in this case "bit" and whatever value the bt parameter to the macro has. 
**/

// Macro Definition 
typedef struct
{
  unsigned int bit0:1;
  unsigned int bit1:1;
  unsigned int bit2:1;
  unsigned int bit3:1;
  unsigned int bit4:1;
  unsigned int bit5:1;
  unsigned int bit6:1;
  unsigned int bit7:1;
} _io_reg;

#define REGISTER_BIT(rg,bt) ((volatile _io_reg*)&rg)->bit##bt

#define GPIO0  REGISTER_BIT(PORTB,5)
#define GPIO1  REGISTER_BIT(PORTB,6)
#define GPIO2  REGISTER_BIT(PORTB,7)
#define GPIO3  REGISTER_BIT(PORTG,0)
#define GPIO4  REGISTER_BIT(PORTG,1)
#define GPIO5  REGISTER_BIT(PORTG,2)
#define GPIO6  REGISTER_BIT(PORTD,6)
#define GPIO7  REGISTER_BIT(PORTD,7)
#define GPIO8  REGISTER_BIT(PORTE,3)

#define DDRGPIO0  REGISTER_BIT(DDRB,5)
#define DDRGPIO1  REGISTER_BIT(DDRB,6)
#define DDRGPIO2  REGISTER_BIT(DDRB,7)
#define DDRGPIO3  REGISTER_BIT(DDRG,0)
#define DDRGPIO4  REGISTER_BIT(DDRG,1)
#define DDRGPIO5  REGISTER_BIT(DDRG,2)
#define DDRGPIO6  REGISTER_BIT(DDRD,6)
#define DDRGPIO7  REGISTER_BIT(DDRD,7)
#define DDRGPIO8  REGISTER_BIT(DDRE,3)

#define SPI_CLK   REGISTER_BIT(PORTB,1)
#define SPI_MISO  REGISTER_BIT(PORTB,3)
#define SPI_MOSI  REGISTER_BIT(PORTB,2)

#define DDRSPI_CLK    REGISTER_BIT(DDRB,1)
#define DDRSPI_MISO   REGISTER_BIT(DDRB,3)
#define DDRSPI_MOSI   REGISTER_BIT(DDRB,2)

#define OSC32K_OUT    REGISTER_BIT(PORTG,3)
#define DDROSC32K_OUT REGISTER_BIT(DDRG,3)

#define I2C_CLK       REGISTER_BIT(PORTD,0)
#define I2C_DATA      REGISTER_BIT(PORTD,1)

#define DDRI2C_CLK    REGISTER_BIT(DDRD,0)
#define DDRI2C_DATA   REGISTER_BIT(DDRD,1)

#define UART_TXD  REGISTER_BIT(PORTD,2)
#define UART_RXD  REGISTER_BIT(PORTD,3)
#define UART_RTS  REGISTER_BIT(PORTD,4)
#define UART_CTS  REGISTER_BIT(PORTD,5)
#define UART_DTR  REGISTER_BIT(PORTE,4)

#define DDRUART_TXD  REGISTER_BIT(DDRD,2)
#define DDRUART_RXD  REGISTER_BIT(DDRD,3)
#define DDRUART_RTS  REGISTER_BIT(DDRD,4)
#define DDRUART_CTS  REGISTER_BIT(DDRD,5)
#define DDRUART_DTR  REGISTER_BIT(DDRE,4)

#define ADC_INPUT_1  REGISTER_BIT(PORTF,1)
#define ADC_INPUT_2  REGISTER_BIT(PORTF,2)
#define ADC_INPUT_3  REGISTER_BIT(PORTF,3)

#define DDRADC_INPUT_1  REGISTER_BIT(DDRF,1)
#define DDRADC_INPUT_2  REGISTER_BIT(DDRF,2)
#define DDRADC_INPUT_3  REGISTER_BIT(DDRF,3)

#define BAT             REGISTER_BIT(PORTF,0)
#define DDRBAT          REGISTER_BIT(DDRF,0)

#define GPIO_1WR        REGISTER_BIT(PORTG,5)
#define DDRGPIO_1WR     REGISTER_BIT(DDRG,5)

#define USART0_RXD      REGISTER_BIT(PORTE,0)
#define USART0_TXD      REGISTER_BIT(PORTE,1)
#define USART0_EXTCLK   REGISTER_BIT(PORTE,2)

#define DDRUSART0_RXD      REGISTER_BIT(DDRE,0)
#define DDRUSART0_TXD      REGISTER_BIT(DDRE,1)
#define DDRUSART0_EXTCLK   REGISTER_BIT(DDRE,2)

#define IRQ7   REGISTER_BIT(PORTE,7)
#define IRQ6   REGISTER_BIT(PORTE,6)

#define DDRIRQ7   REGISTER_BIT(DDRE,7)
#define DDRIRQ6   REGISTER_BIT(DDRE,6)

#endif

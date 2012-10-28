/*
 * Copyright (c) 2011, Hedde Bosman <heddebosman@incas3.eu>
 *
 * I2C communication device drivers for mc1322x
 *
 * Redistribution and use in source and binary forms, with or without
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
 */

#include "i2c.h"

#include <stdio.h>

static int8_t tx_byte_ctr;
static int8_t rx_byte_ctr;

static uint8_t* tx_buf_ptr;
static uint8_t* rx_buf_ptr;


volatile unsigned int i;	// volatile to prevent optimization

static inline void i2c_send_byte(void) {
	*I2CDR = *(tx_buf_ptr++); // set new byte, MCF is automatically cleared
	tx_byte_ctr--;
}
static inline void i2c_recv_byte(void) {
	*(rx_buf_ptr++) = *I2CDR;
	rx_byte_ctr--;
}

//------------------------------------------------------------------------------
// void i2c_receiveinit(uint8_t slave_address, 
//                              uint8_t prescale)
//
// This function initializes the USCI module for master-receive operation. 
//
// IN:   uint8_t slave_address   =>  Slave Address
//       uint8_t prescale        =>  SCL clock adjustment 
//-----------------------------------------------------------------------------
//static volatile uint8_t rx_byte_tot = 0;
void i2c_receiveinit(uint8_t slave_address, uint8_t byte_ctr, uint8_t *rx_buf) {
	// wait for bus to be free before setting MSTA (assuming we're in a multi-master environment or still sending something else)
	while(i2c_busy()) /* wait */;

	// assert: rx_byte_ctr <= 0
	// assert: tx_byte_ctr <= 0
	tx_buf_ptr = 0;
	tx_byte_ctr = 0; // indicate that nothing is to be received
	rx_byte_ctr = byte_ctr;
	rx_buf_ptr  = rx_buf;

	// clockdiv
	//*I2CFDR = 0x20; // 150 khz for redbee econotag
	
	// assume being master, thus no addres has to be set
	*I2CCR = I2C_MEN |
#ifdef I2C_NON_BLOCKING
		I2C_MIEN | 
#endif
		I2C_MSTA | I2C_MTX | I2C_RXAK; // start condition is triggered
	
	// write out address of slave
	*I2CDR = (slave_address & 0x7f) <<1 | 0x01;

#ifndef I2C_NON_BLOCKING
	i2c_receive();
#endif	
}

//------------------------------------------------------------------------------
// void i2c_transmitinit(uint8_t slave_address, 
//                               uint8_t prescale)
//
// Initializes USCI for master-transmit operation. 
//
// IN:   uint8_t slave_address   =>  Slave Address
//       uint8_t prescale        =>  SCL clock adjustment 
//------------------------------------------------------------------------------
//static volatile uint8_t tx_byte_tot = 0;
void i2c_transmitinit(uint8_t slave_address, uint8_t byte_ctr, uint8_t *tx_buf) {
	// wait for bus to be free before setting MSTA (assuming we're in a multi-master environment or still sending something else)
	while(i2c_busy()) /* wait */;

	// assert: rx_byte_ctr <= 0
	// assert: tx_byte_ctr <= 0
	rx_buf_ptr = 0;
	rx_byte_ctr = 0; // indicate that nothing is to be received
	tx_byte_ctr = byte_ctr;
	tx_buf_ptr  = tx_buf;

	// clockdiv
	//*I2CFDR = 0x20; // 150 khz for redbee econotag
	
	// assume being master, thus no addres has to be set
	*I2CCR = I2C_MEN | 
#ifdef I2C_NON_BLOCKING
		I2C_MIEN | 
#endif
		I2C_MSTA | I2C_MTX ; // start condition is triggered

	// write out address of slave
	*I2CDR = (slave_address & 0x7f) <<1;

#ifndef I2C_NON_BLOCKING
	i2c_transmit();
#endif	
}

/*----------------------------------------------------------------------------*/
/*- blocking counterparts of interrupt hanlder function  ---------------------*/
/*----------------------------------------------------------------------------*/
#ifndef I2C_NON_BLOCKING
/*----------------------------------------------------------------------------*/
uint8_t i2c_receive() {
	while(rx_byte_ctr > 0) {
		// busy wait
		while(!(*I2CSR & I2C_MCF) || !(*I2CSR & I2C_MIF)) /*wait*/;

		if (rx_byte_ctr == 1) { // receiving next-to-last byte, thus turn off auto-ack for stop condition
			*I2CCR |= I2C_TXAK;
		}
		
		if (*I2CSR & I2C_MCF) {
			i2c_recv_byte(); // read new byte
		}

		if (*I2CSR & I2C_MAL) {
			*I2CSR &= ~I2C_MAL; // should be cleared in software
			printf("*** ERROR I2C: Arbitration lost\n");
			// Arbitration lost; ERROR?
		}
	}

	while(!(*I2CSR & I2C_MCF) || !(*I2CSR & I2C_MIF)) /*wait*/;
	if (*I2CSR & I2C_RXAK) {
		// NO acknoledge byte received
		printf("*** ERROR I2C: No ack received\n");
	}
	if (*I2CSR & I2C_MAL) {
		*I2CSR &= ~I2C_MAL; // should be cleared in software
		printf("*** ERROR I2C: Arbitration lost\n");
		// Arbitration lost; ERROR?
	}

	*I2CCR &= ~I2C_MSTA; // stop condition
}
/*----------------------------------------------------------------------------*/
void    i2c_transmit() {
	while(tx_byte_ctr > 0) {
		// busy wait
		while(!(*I2CSR & I2C_MCF) || !(*I2CSR & I2C_MIF)) /*wait*/;

		if (*I2CSR & I2C_RXAK) {
			// NO acknoledge byte received
			printf("*** ERROR I2C: No ack received\n");
		}
		
		if (*I2CSR & I2C_MCF) {
			i2c_send_byte();
		}
		
		if (*I2CSR & I2C_MAL) {
			*I2CSR &= ~I2C_MAL; // should be cleared in software
			printf("*** ERROR I2C: Arbitration lost\n");
			// Arbitration lost; ERROR?
		}
		
		// clear MIF
		*I2CSR &= ~I2C_MIF;
	}
	
	while(!(*I2CSR & I2C_MCF) || !(*I2CSR & I2C_MIF)) /*wait*/;
	if (*I2CSR & I2C_RXAK) {
		// NO acknoledge byte received
		printf("*** ERROR I2C: No ack received\n");
	}
	if (*I2CSR & I2C_MAL) {
		*I2CSR &= ~I2C_MAL; // should be cleared in software
		printf("*** ERROR I2C: Arbitration lost\n");
		// Arbitration lost; ERROR?
	}
	
	*I2CCR &= ~I2C_MSTA; // stop condition
}
#endif
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* force SCL to become bus master when sda is still low (can occur after sys reset */
/*----------------------------------------------------------------------------*/
void i2c_force_reset(void) {
	uint8_t tmp;
	*I2CCR = 0x20;
	*I2CCR = 0xA0;
	tmp = *I2CDR;
	// return to module state Master?
}


/*----------------------------------------------------------------------------*/
/*-  check if we're still in the process of sending something  ----------------*/
/*----------------------------------------------------------------------------*/
uint8_t i2c_transferred(void) {
	return (!i2c_busy() && rx_byte_ctr == 0 && tx_byte_ctr == 0);
}


/*----------------------------------------------------------------------------*/
/* check if there is communication in progress on the i2c bus.                */
/*----------------------------------------------------------------------------*/
uint8_t i2c_busy(void) {
  return ((*I2CSR & I2C_MBB) > 0); // bit 5 high = busy
}


/*----------------------------------------------------------------------------*/
/* Setup ports and pins for I2C use. */
/*----------------------------------------------------------------------------*/
void i2c_enable(void) {
	// enable clock signal to i2c module
	*I2CCKER = I2C_CKEN; // enable

	enable_irq(I2C);

	*I2CFDR = 0x20; // clockdiv: 150 khz for redbee econotag
	*I2CADR = 0x01; // our slave address (not used; we're master)
	// then enable i2c module
	*I2CCR |= I2C_MEN | // module-enable, auto-ack = on
#ifdef I2C_NON_BLOCKING
		I2C_MIEN | //module-interrupt-enable
#endif
		0;

	// then switch gpio pins to i2c
	*GPIO_FUNC_SEL0 	|= (0x01 << (I2C_SCL*2)) | (0x01 << (I2C_SDA*2)); // GPIO 12, 13 to i2c
	// and enable pull-up resistors
	*GPIO_PAD_PU_EN0 	|= (0x01 << I2C_SCL) | (0x01 << I2C_SDA); // Activate internal pull-up/-down resistors
	*GPIO_PAD_PU_SEL0 	|= (0x01 << I2C_SCL) | (0x01 << I2C_SDA); // select pull-up resistors for ports
}

/*----------------------------------------------------------------------------*/
/* Reset ports and pins for GPIO use. */
/*----------------------------------------------------------------------------*/
void i2c_disable(void) {
	// all control values are set off
	*I2CCR = 0;
	// clock is turned off
	*I2CCKER = ~I2C_CKEN;

	// then switch gpio pins to gpio
	*GPIO_FUNC_SEL0 	&= ~(0x01 << (I2C_SCL*2)) | (0x01 << (I2C_SDA*2)); // GPIO 12, 13 to i2c
	// and disable resistors
	*GPIO_PAD_PU_EN0 	&= ~(0x01 << I2C_SCL) | (0x01 << I2C_SDA); // Deactivate internal pull-up/-down resistors

	disable_irq(I2C);
}


/*----------------------------------------------------------------------------*/
/*-  i2c interrupt handler  --------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#ifdef I2C_NON_BLOCKING
void i2c_isr (void) {
	uint8_t dummy;
	if (*I2CSR & I2C_MIF) { // interrupt is from i2c
		if (*I2CSR & I2C_MCF) { // one byte transferred/received. will be cleared automatically when I2CDR is written or I2CSR read
			if (tx_buf_ptr != 0) { // we're sending
				if (*I2CSR & I2C_RXAK) {
					// NO acknoledge byte received
					printf("*** ERROR I2C: No ack received\n");
				}

				if (tx_byte_ctr > 0) { // tx
					i2c_send_byte(); // set new byte, MCF is automatically cleared
				} else {
					*I2CCR &= ~I2C_MSTA; // generate stop condition
				}
			} else { //if (rx_buf_ptr != 0) { // receive
				if (rx_byte_ctr == 1) { // receiving next-to-last byte, thus turn off auto-ack for stop condition
					*I2CCR |= I2C_TXAK;
				}
				if (*I2CCR & I2C_MTX) { // address byte was just sent
					*I2CCR &= ~I2C_MTX; // switch to receive mode
					dummy = *I2CDR; // dummy read to throw away the address from register
					
				} else if (rx_byte_ctr > 0) {
					i2c_recv_byte(); // read new byte
				} else {
					*I2CCR &= ~I2C_MSTA; // generate stop condition
				}
				
			}
		}
		if (*I2CSR & I2C_MAL) {
			*I2CSR &= ~I2C_MAL; // should be cleared in software
			printf("*** ERROR I2C: Arbitration lost\n");
			// Arbitration lost; reset..
			rx_byte_ctr = tx_byte_ctr = 0;
			*I2CCR &= ~I2C_MSTA; // generate stop condition
		}
		
		// clear MIF
		*I2CSR &= ~I2C_MIF;
	}
}
#endif

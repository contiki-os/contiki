/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *         I2C communication device drivers for Zolertia Z1 sensor node.
 * \author
 *         Enric M. Calvo, Zolertia <ecalvo@zolertia.com>
 *         Marcus Lundén, SICS <mlunden@sics.se>
 */

#include "i2cmaster.h"
#include "isr_compat.h"

signed   char tx_byte_ctr, rx_byte_ctr;
unsigned char rx_buf[2];
unsigned char* tx_buf_ptr;
unsigned char* rx_buf_ptr;
unsigned char receive_data;
unsigned char transmit_data1;
unsigned char transmit_data2;
volatile unsigned int i;	// volatile to prevent optimization

//------------------------------------------------------------------------------
// void i2c_receiveinit(unsigned char slave_address, 
//                              unsigned char prescale)
//
// This function initializes the USCI module for master-receive operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//       unsigned char prescale        =>  SCL clock adjustment 
//-----------------------------------------------------------------------------
void
i2c_receiveinit(uint8_t slave_address) {
  UCB1CTL1 = UCSWRST;                    // Enable SW reset
  UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;  // I2C Master, synchronous mode
  UCB1CTL1 = UCSSEL_2 | UCSWRST;         // Use SMCLK, keep SW reset
  UCB1BR0  = I2C_PRESC_400KHZ_LSB;       // prescaler for 400 kHz data rate
  UCB1BR1  = I2C_PRESC_400KHZ_MSB;
  UCB1I2CSA = slave_address;	         // set slave address

  UCB1CTL1 &= ~UCTR;		         // I2C Receiver 

  UCB1CTL1 &= ~UCSWRST;	                 // Clear SW reset, resume operation
  UCB1I2CIE = UCNACKIE;
#if I2C_RX_WITH_INTERRUPT
  UC1IE = UCB1RXIE;                      // Enable RX interrupt if desired
#endif
}

//------------------------------------------------------------------------------
// void i2c_transmitinit(unsigned char slave_address, 
//                               unsigned char prescale)
//
// Initializes USCI for master-transmit operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//       unsigned char prescale        =>  SCL clock adjustment 
//------------------------------------------------------------------------------
void
i2c_transmitinit(uint8_t slave_address) {
  UCB1CTL1 |= UCSWRST;		           // Enable SW reset
  UCB1CTL0 |= (UCMST | UCMODE_3 | UCSYNC); // I2C Master, synchronous mode
  UCB1CTL1  = UCSSEL_2 + UCSWRST;          // Use SMCLK, keep SW reset
  UCB1BR0   = I2C_PRESC_400KHZ_LSB;        // prescaler for 400 kHz data rate
  UCB1BR1   = I2C_PRESC_400KHZ_MSB;
  UCB1I2CSA = slave_address;	           // Set slave address

  UCB1CTL1 &= ~UCSWRST;                    // Clear SW reset, resume operation
  UCB1I2CIE = UCNACKIE;
  UC1IE = UCB1TXIE;		           // Enable TX ready interrupt
}

//------------------------------------------------------------------------------
// void i2c_receive_n(unsigned char byte_ctr, unsigned char * rx_buf)
// This function is used to start an I2C communication in master-receiver mode WITHOUT INTERRUPTS
// for more than 1 byte
// IN:   unsigned char byte_ctr   =>  number of bytes to be read
// OUT:  unsigned char rx_buf     =>  receive data buffer
// OUT:  int n_received           =>  number of bytes read
//------------------------------------------------------------------------------
static volatile uint8_t rx_byte_tot = 0;
uint8_t
i2c_receive_n(uint8_t byte_ctr, uint8_t *rx_buf) {

  rx_byte_tot = byte_ctr;
  rx_byte_ctr = byte_ctr;
  rx_buf_ptr  = rx_buf;

  while ((UCB1CTL1 & UCTXSTT) || (UCB1STAT & UCNACKIFG))	// Slave acks address or not?
    PRINTFDEBUG ("____ UCTXSTT not clear OR NACK received\n");

#if I2C_RX_WITH_INTERRUPT
  PRINTFDEBUG(" RX Interrupts: YES \n");

  // SPECIAL-CASE: Stop condition must be sent while receiving the 1st byte for 1-byte only read operations
  if(rx_byte_tot == 1){                 // See page 537 of slau144e.pdf
    dint();
    UCB1CTL1 |= UCTXSTT;		// I2C start condition
    while(UCB1CTL1 & UCTXSTT)           // Waiting for Start bit to clear
      PRINTFDEBUG ("____ STT clear wait\n");
    UCB1CTL1 |= UCTXSTP;		// I2C stop condition
    eint();
  }
  else{                                 // all other cases
    UCB1CTL1 |= UCTXSTT;		// I2C start condition
  }
  return 0;

#else
  uint8_t n_received = 0;

  PRINTFDEBUG(" RX Interrupts: NO \n");

  UCB1CTL1 |= UCTXSTT;		// I2C start condition

  while (rx_byte_ctr > 0){
    if (UC1IFG & UCB1RXIFG) {   // Waiting for Data
      rx_buf[rx_byte_tot - rx_byte_ctr] = UCB1RXBUF;
      rx_byte_ctr--;
      UC1IFG &= ~UCB1RXIFG;     // Clear USCI_B1 RX int flag      
      n_received++;
    }
  }
  UCB1CTL1 |= UCTXSTP;		// I2C stop condition
  return n_received;
#endif
}


//------------------------------------------------------------------------------
// uint8_t i2c_busy()
//
// This function is used to check if there is communication in progress. 
//
// OUT:  unsigned char  =>  0: I2C bus is idle, 
//                          1: communication is in progress
//------------------------------------------------------------------------------
uint8_t
i2c_busy(void) {
  return (UCB1STAT & UCBBUSY);
}

/*----------------------------------------------------------------------------*/
/* Setup ports and pins for I2C use. */

void
i2c_enable(void) {
  I2C_PxSEL |= (I2C_SDA | I2C_SCL);    // Secondary function (USCI) selected
  I2C_PxSEL2 |= (I2C_SDA | I2C_SCL);   // Secondary function (USCI) selected
  I2C_PxDIR |= I2C_SCL;	               // SCL is output (not needed?)
  I2C_PxDIR &= ~I2C_SDA;	       // SDA is input (not needed?)
  I2C_PxREN |= (I2C_SDA | I2C_SCL);    // Activate internal pull-up/-down resistors
  I2C_PxOUT |= (I2C_SDA | I2C_SCL);    // Select pull-up resistors
}

void
i2c_disable(void) {
  I2C_PxSEL &= ~(I2C_SDA | I2C_SCL);    // GPIO function selected
  I2C_PxSEL2 &= ~(I2C_SDA | I2C_SCL);   // GPIO function selected
  I2C_PxREN &= ~(I2C_SDA | I2C_SCL);    // Deactivate internal pull-up/-down resistors
  I2C_PxOUT &= ~(I2C_SDA | I2C_SCL);    // Select pull-up resistors
}

/*----------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
// void i2c_transmit_n(unsigned char byte_ctr, unsigned char *field)
//
// This function is used to start an I2C communication in master-transmit mode. 
//
// IN:   unsigned char byte_ctr   =>  number of bytes to be transmitted
//       unsigned char *tx_buf    =>  Content to transmit. Read and transmitted from [0] to [byte_ctr]
//------------------------------------------------------------------------------
static volatile uint8_t tx_byte_tot = 0;
void
i2c_transmit_n(uint8_t byte_ctr, uint8_t *tx_buf) {
  tx_byte_tot = byte_ctr;
  tx_byte_ctr = byte_ctr;
  tx_buf_ptr  = tx_buf;
  UCB1CTL1 |= UCTR + UCTXSTT;	   // I2C TX, start condition
}

/*----------------------------------------------------------------------------*/
ISR(USCIAB1TX, i2c_tx_interrupt)
{
  // TX Part
  if (UC1IFG & UCB1TXIFG) {        // TX int. condition
    if (tx_byte_ctr == 0) {
      UCB1CTL1 |= UCTXSTP;	   // I2C stop condition
      UC1IFG &= ~UCB1TXIFG;	   // Clear USCI_B1 TX int flag
    }
    else {
      UCB1TXBUF = tx_buf_ptr[tx_byte_tot - tx_byte_ctr];
      tx_byte_ctr--;
    }
  }
  // RX Part
#if I2C_RX_WITH_INTERRUPT
  else if (UC1IFG & UCB1RXIFG){    // RX int. condition
    if (rx_byte_ctr == 0){
      // Only for 1-byte transmissions, STOP is handled in receive_n_int
      if (rx_byte_tot != 1) 
        UCB1CTL1 |= UCTXSTP;       // I2C stop condition

      UC1IFG &= ~UCB1RXIFG;        // Clear USCI_B1 RX int flag. XXX Just in case, check if necessary
    }
    else {
      rx_buf_ptr[rx_byte_tot - rx_byte_ctr] = UCB1RXBUF;
      rx_byte_ctr--;
    }
  }
#endif
}

ISR(USCIAB1RX, i2c_rx_interrupt)
{
  if(UCB1STAT & UCNACKIFG) {
    PRINTFDEBUG("!!! NACK received in RX\n");
    UCB1CTL1 |= UCTXSTP;
    UCB1STAT &= ~UCNACKIFG;
  }
}

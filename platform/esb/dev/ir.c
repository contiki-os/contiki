


/* The software in this file is based on code from FU Berlin. */

/*
Copyright 2003/2004, Freie Universitaet Berlin. All rights reserved.

These sources were developed at the Freie Universität Berlin, Computer
Systems and Telematics group.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
- Neither the name of Freie Universitaet Berlin (FUB) nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

This software is provided by FUB and the contributors on an "as is"
basis, without any representations or warranties of any kind, express
or implied including, but not limited to, representations or
warranties of non-infringement, merchantability or fitness for a
particular purpose. In no event shall FUB or contributors be liable
for any direct, indirect, incidental, special, exemplary, or
consequential damages (including, but not limited to, procurement of
substitute goods or services; loss of use, data, or profits; or
business interruption) however caused and on any theory of liability,
whether in contract, strict liability, or tort (including negligence
or otherwise) arising in any way out of the use of this software, even
if advised of the possibility of such damage.

This implementation was developed by the CST group at the FUB.
Contributors: Thomas Pietsch, Bjoern Lichtblau

*/

/*  \file recir.c
 ** \ingroup Firmware
 ** \brief Receiving RC5 via IR Receiving Diode.
 **
 ** \code
 ** RC5: 1780 us bitlength (manchester encoded, so half bitlength of 890 us is important)
 ** Transferred packet (2 start + toggle bit + 5 address bits + 6 comand bits)): 
 **                                   | S | S | T | A4 | A3 | A2 | A1 | A0 | C5 | C4 | C3 | C2 | C1 | C0 |
 ** irdata format: | ? | ? | error  | newData | T | A4 | A3 | A2 | A1 | A0 | C5 | C4 | C3 | C2 | C1 | C0 |
 ** \endcode
 **
 ** <img src="../pics/rc5.jpg">
 ** See detailed description at <a href="http://users.pandora.be/davshomepage/rc5.htm">http://users.pandora.be/davshomepage/rc5.htm</a>
 **
 ** Some common addresses and commands:
 ** \code
 ** Address:          Device:          Command:
 **   0               TV1              0...9    Numbers 0...9 (channel select)
 **   1               TV2              12       Standby
 **   5               VCR1             16       Master Volume +
 **   6               VCR2             17       Master Volume -
 **  17               Tuner            18       Brightness +
 **  18               Audio Tape       19       Brightness -
 **  20               CD Player        50       Fast rewind
 **                                    52       Fast run forward
 **                                    53       Play
 **                                    54       Stop
 **                                    55       Recording
 ** \endcode
 **/

#include "contiki.h"
#include "dev/ir.h"

#include "dev/leds.h"
#include "dev/beep.h"
#include "isr_compat.h"

PROCESS(ir_process, "IR receiver");
process_event_t ir_event_received;
/*---------------------------------------------------------------------------*/
#define SIR1  (P1OUT |= 0x01) ///< MACRO: Puts IR sending diode high.
#define SIR0  (P1OUT &= 0xFE) ///< MACRO: Puts IR sending diode low.
#define BIT75 3282      ///< 3 quarters of a bit after start, 3282 cyc @ 2,4576Mhz = 1335us.
#define BIT50 2188      ///< Half of bit length, 2188 cyc @ 2,4576Mhz = 890 us.

/*---------------------------------------------------------------------------*/
/* Sends a logical one via IR, method is timed for the 2.4576Mhz SMCLK!!!
 */
static volatile void
send1bit(void)
{
  volatile int i;
  for(i = 0; i < 34; ++i) {
    SIR1; SIR1; SIR1; SIR1;
    SIR0; SIR0; SIR0; SIR0; 
    SIR0; SIR0; SIR0; SIR0; 
    SIR0; 
  }  
}
/*---------------------------------------------------------------------------*/
/* Sends a logical 0 via IR, method is timed for the 2.4576Mhz SMCLK!!!
 */
static volatile void
send0bit(void)
{
  volatile int i;
  for(i = 0; i < 34; ++i) {
    SIR0; SIR0; SIR0; SIR0; 
    SIR0; SIR0; SIR0; SIR0; 
    SIR0; SIR0; SIR0; SIR0; 
    SIR0; 
  }  
}
/*---------------------------------------------------------------------------*/
/* Sends the lower 12 bits of data via IR, turns interrupt off while
   it's sending.
 */
void
ir_send(unsigned short data)
{
  volatile unsigned short mask = 0x2000;
  data |= 0xF000;
  
  dint();
  while(mask != 0){
    if(!(mask & data)){
      send1bit();
      send0bit();
    } else {
      send0bit();
      send1bit();
    }
    mask /= 2;
  }
  eint();
}
/*---------------------------------------------------------------------------*/
/* Testroutine which repetedly sends two commands.
 */
/*void
ir_test_send(void)
{
  volatile unsigned int i;
  send12bits(0xF010);
  for(i=0; i<0xFFFF; i++) nop(); 
  send12bits(0xF011);
  for(i=0; i<0xFFFF; i++) nop();
}*/
/*---------------------------------------------------------------------------*/


static void setErrorBit(void);
static void clearErrorBit(void);
static void setDataAvailableBit(void);
static void clearDataAvailableBit(void);


/// \name Internal variables.
//@{
static unsigned int ir_pos;             ///< current position in frame
static unsigned int recvdata;            ///< here a received packet is saved
static unsigned int recvdatabuffer;      ///< temporary buffer for receiving
static unsigned char ir_temp;           ///< saves the first half of the manchester bit
//@}

/// \name Public functions. 
/// If ::recir_dataAvailable()==1 use the get* functions.
//@{
unsigned char recir_getCode(void){ return (recvdata & 0x003F); }
unsigned char recir_getAddress(void){ return ((recvdata & 0x07C0) >> 6); }
unsigned char recir_getToggle(void){  return ((recvdata & 0x0800) >> 11); }
unsigned char recir_getError(void){ return ((recvdata & 0x2000) >> 13); }

uint16_t
ir_data(void)
{
  return recvdata;
}

uint8_t
ir_poll(void)
{
  if(recvdata & 0x1000) {
    clearDataAvailableBit();
    return 1;
  } else {
    return 0;
  }
}


///\name Internal functions.
//@{
static void setErrorBit(void){ recvdata |= 0x2000; }
static void clearErrorBit(void) { recvdata &= 0xDFFF; }
static void setDataAvailableBit(void){ recvdata |= 0x1000; }
static void clearDataAvailableBit(void){ recvdata &= 0xEFFF; }


/// Timer B0 interrupt service routine
ISR(TIMERB1, Timer_B1) {

  /*P2OUT = (P2OUT & 0xf7) | (8 - (P2OUT & 0x08));*/
  
  if(ir_pos <= 25) {
      if(ir_pos % 2) {                             // odd position
        if(ir_temp && !(P1IN & 0x04)) {           // 1 - 0 -->  write 1 
          recvdatabuffer +=1;
          recvdatabuffer = recvdatabuffer << 1;  
        } else if(!ir_temp && (P1IN & 0x04)) {      // 0 - 1 -->  write 0 
          recvdatabuffer = recvdatabuffer << 1;  
        } else { 
          setErrorBit();
          if(P1IN & 0x04) {
	    recvdatabuffer += 1;
	  }
          recvdatabuffer = recvdatabuffer << 1;
        }
      } else {                                    // even position
        ir_temp = P1IN & 0x04;
      }
    }
    
    if(ir_pos == 25) {                          // end reached
      recvdatabuffer = recvdatabuffer >> 1; 

      if(!recir_getError() && ( (recvdatabuffer & 0x0FFF) != (recvdata & 0x0FFF) ) ){
        recvdata = recvdatabuffer;
        setDataAvailableBit();
      } else {
        _NOP();
      }      
    }
    
    if(ir_pos==27) {
      TBCCTL1 &= ~CCIE;  

      //GREENOFF;      
      // temporary debug output
      //sendRS232Address(recvdatabuffer);
      //if(recir_getError()) sendRS232('E');
      //sendRS232String("\r\n");
      if(!recir_getError()) beep_beep(20);
      
      // reenable interrupt for falling edge
      P1IFG &= ~(0x04);
      P1IE  |= 0x04;                    // enable interrupt for recir RC5
      leds_off(LEDS_RED);
    }
    
  ir_pos++;
  TBCCR1 += BIT50;        // set new interrupt
 
  TBCCTL1 &= ~CCIFG;
}


/** \brief IR Interrupt routine 
 **
 ** For the falling edge (start of RC5 packet)( mid of first start bit ), IRReceiver is on P12
 ** real interrupt routine, which calls this, is in sensors.c */
void
ir_irq(void)
{
  if(P1IN & 0x04) return; // high again, just a peak

  ir_pos = 0;
  recvdatabuffer = 0;  
  clearErrorBit();

  // the first timer interrupt will occur in the mid of the first half of the second start bit 
  TBCCR1 = TBR + BIT75;     // set first TBCCR1 IRQ to 75% of RC5 bitlength
  TBCCTL1 &= ~CCIFG;        // clear previous compare flag
  TBCCTL1 |= CCIE;          // CCR0 interrupt enabled, interrupt occurs when timer equals CCR0
  
  P1IE &= ~0x04;   // disable interrupt for P12 ( ReceiveIR )
  leds_on(LEDS_RED);
  //GREENON;
} 

//@}
/*---------------------------------------------------------------------*/
PROCESS_THREAD(ir_process, ev, data)
{
  PROCESS_BEGIN();

  // init TIMERB ccr0 to run continouslycreate the 5 ms interval
  // ccr1 is used for ir receiving (RC5)
  TBCTL = TBSSEL1 + TBCLR;         // select SMCLK (2.4576MHz), clear TBR
  TBCTL |= MC1;                         // Start Timer_A in continuous mode
  

  P1IES |= 0x04;  // Important for IR-RC5 receive to detect the first FALLING edge

  ir_event_received = process_alloc_event();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    
    if(ir_poll() == IR_DATA) {
      unsigned short irdata;
      irdata = ir_data() & 0x7ff;    
      process_post(PROCESS_BROADCAST, ir_event_received, (process_data_t)irdata);
    }
    
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------*/

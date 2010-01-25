/**
 * \file
 *
 *   uart initialization routines
 *
 * \author
 * 
 *   Anthony "Asterisk" Ambuehl
 *
 *   non-interrupt routines typically only called once, stored in any bank.
 *
 */
#include <stdlib.h>
#include <string.h>

#include "banked.h"
#include "cc2430_sfr.h"

#include "dev/leds.h"
#include "dev/uart.h"

/*---------------------------------------------------------------------------*/
void
uart0_init(uint32_t speed) __banked
{
  if(speed == 115200) {
    U0BAUD=216;	/*115200*/
    U0GCR =11; 	/*LSB first and 115200*/
  }
  else if(speed == 38400) {
    U0BAUD=59;	/*38400*/
    U0GCR =10; 	/*LSB first and 38400*/
  }
  else if(speed == 9600) {
    U0BAUD= 59;	/* 9600 */
    U0GCR = 8; 	/*LSB first and 9600*/
  }
  else { return; }

#ifdef UART0_ALTERNATIVE_2
  PERCFG |= U0CFG;	/*alternative port 2 = P1.5-2*/
#ifdef UART0_RTSCTS
  P1SEL |= 0x3C;		/*peripheral select for TX and RX, RTS, CTS*/
#else
  P1SEL |= 0x30;		/*peripheral select for TX and RX*/
  P1 &= ~0x08;		/*RTS down*/
#endif
  P1DIR |= 0x28;		/*RTS, TX out*/
  P1DIR &= ~0x14;		/*CTS & RX in*/
#else
  PERCFG &= ~U0CFG;	/*alternative port 1 = P0.5-2*/
#ifdef UART0_RTSCTS
  P0SEL |= 0x3C;		/*peripheral select for TX and RX, RTS, CTS*/
#else
  P0SEL |= 0x0C;		/*peripheral select for TX and RX*/
  P0 &= ~0x20;		/*RTS down*/
#endif
  P0DIR |= 0x28;		/*RTS & TX out*/
  P0DIR &= ~0x14;		/*CTS & RX in*/
#endif
  

#ifdef UART0_RTSCTS
  U0UCR = 0x42;		/*defaults: 8N1, RTS/CTS, high stop bit*/
#else
  U0UCR = 0x02;		/*defaults: 8N1, no flow control, high stop bit*/
#endif
  
  U0CSR = U_MODE | U_RE | U_TXB;  /*UART mode, receiver enable, TX done*/
  
  /*set priority group of group 3 to highest, so the UART won't miss bytes*/
  IP1 |= IP1_3;
  IP0 |= IP0_3;
  
  IEN0_URX0IE = 1;
}
/*---------------------------------------------------------------------------*/
/* UART1 initialization */
void
uart1_init(uint32_t speed) __banked
{
#ifdef UART1_ALTERNATIVE_1
  PERCFG &= ~U1CFG;	/*alternative port 1 = P0.5-2*/
#ifdef UART1_RTSCTS
  P0SEL |= 0x3C;		/*peripheral select for TX and RX, RTS, CTS*/
#else
  P0SEL |= 0x30;		/*peripheral select for TX and RX*/
  P0 &= ~0x08;		/*RTS down*/
#endif
  P0DIR |= 0x18;		/*RTS, TX out*/
  P0DIR &= ~0x24;		/*CTS, RX in*/
#else
  PERCFG |= U1CFG;	/*alternative port 2 = P1.7-4*/
#ifdef UART1_RTSCTS
  P1SEL |= 0xF0;		/*peripheral select for TX and RX*/
#else
  P1SEL |= 0xC0;		/*peripheral select for TX and RX*/
  P1 &= ~0x20;		/*RTS down*/
#endif
  P1DIR |= 0x60;		/*RTS, TX out*/
  P1DIR &= ~0x90;		/*CTS, RX in*/
#endif
  
  if(speed == 115200) {
    U1BAUD=216;	/*115200*/
    U1GCR =11; 	/*LSB first and 115200*/
  }

  if(speed == 38400) {
    U1BAUD=59;	/*38400*/
    U1GCR =10; 	/*LSB first and 38400*/
  }

  if(speed == 9600) {
    U1BAUD= 59;	/* 9600 */
    U1GCR = 8; 	/*LSB first and 9600*/
  }

#ifdef UART1_RTSCTS
  U1UCR = 0x42;	/*defaults: 8N1, RTS/CTS, high stop bit*/
#else
  U1UCR = 0x02;	/*defaults: 8N1, no flow control, high stop bit*/
#endif
  
  U1CSR = U_MODE | U_RE | U_TXB;  /*UART mode, receiver enable, TX done*/
  
  /*set priority group of group 3 to highest, so the UART won't miss bytes*/
  IP1 |= IP1_3;
  IP0 |= IP0_3;
  
  IEN0_URX1IE = 1;	/* Enable the RX interrupt */
}
/*---------------------------------------------------------------------------*/

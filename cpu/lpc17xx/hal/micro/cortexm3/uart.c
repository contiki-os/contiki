/** @file hal/micro/cortexm3/uart.c
 * @brief STM32W  uart drivers, supporting IAR's standard library
 *        IO routines.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#include PLATFORM_HEADER
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"
#include "uart.h"
#ifdef __GNUC__
#include <sys/stat.h>
#include <stdio.h>
#define _LLIO_STDIN ((int) stdin)
#define _LLIO_STDOUT ((int) stdout)
#define _LLIO_STDERR ((int) stderr)
#define _LLIO_ERROR  (-1)
#define __write _write
#define __read _read
#undef putchar
void __io_putchar( char c );
int putchar (int c)
{
  __io_putchar((char) c);
  return c;
}
#endif
#define RECEIVE_QUEUE_SIZE (128)

int8u rxQ[RECEIVE_QUEUE_SIZE];
int16u rxHead;
int16u rxTail;
int16u rxUsed;

//////////////////////////////////////////////////////////////////////////////
// Initialization

void uartInit(int32u baudrate, int8u databits, SerialParity parity, int8u stopbits)
{
  int32u tempcfg;
  int32u tmp;
  
  assert( (baudrate >= 300) && (baudrate <=921600) );
    
  tmp = (2*12000000L + baudrate/2) / baudrate;
  SC1_UARTFRAC = tmp & 1;
  SC1_UARTPER = tmp / 2;
  
  if(databits == 7) {
    tempcfg = 0;
  } else {
    tempcfg = SC_UART8BIT;
  }
  
  if (parity == PARITY_ODD) {
    tempcfg |= SC_UARTPAR | SC_UARTODD;
  } else if( parity == PARITY_EVEN ) {
    tempcfg |= SC_UARTPAR;
  }

  if ((stopbits & 0x0F) >= 2) {
    tempcfg |= SC_UART2STP;
  }
  SC1_UARTCFG = tempcfg;

  SC1_MODE = SC1_MODE_UART;

  rxHead=0;
  rxTail=0;
  rxUsed=0;

  halGpioConfig(PORTB_PIN(1),GPIOCFG_OUT_ALT);
  halGpioConfig(PORTB_PIN(2),GPIOCFG_IN);

  // Make the RX Valid interrupt level sensitive (instead of edge)
  SC1_INTMODE = SC_RXVALLEVEL;
  // Enable just RX interrupts; TX interrupts are controlled separately
  INT_SC1CFG |= (INT_SCRXVAL   |
                 INT_SCRXOVF   |
                 INT_SC1FRMERR |
                 INT_SC1PARERR);
  INT_SC1FLAG = 0xFFFF; // Clear any stale interrupts
  INT_CFGSET = INT_SC1;
}

//////////////////////////////////////////////////////////////////////////////
// Transmit

// IAR Standard library hook for serial output
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
  size_t nChars = 0;

  /* This template only writes to "standard out" and "standard err",
   * for all other file handles it returns failure. */
  if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR) {
    return _LLIO_ERROR;
  }

  if (buffer == 0) {
    // This means that we should flush internal buffers.  
    //spin until TX complete (TX is idle)
    while ((SC1_UARTSTAT&SC_UARTTXIDLE)!=SC_UARTTXIDLE) {}
    return 0;
  }
  
  // ensure port is configured for UART
  if(SC1_MODE != SC1_MODE_UART) {
    return _LLIO_ERROR;
  }

  while(size--) {
    //spin until data register has room for more data
    while ((SC1_UARTSTAT&SC_UARTTXFREE)!=SC_UARTTXFREE) {}
    SC1_DATA = *buffer;
    buffer++;

    ++nChars;
  }

  return nChars;
}
#ifdef __GNUC__
int fflush (FILE *f)
#endif
#ifdef __ICCARM__
size_t fflush(int handle)
#endif
{
  return __write(_LLIO_STDOUT, NULL, 0);
}

static void halInternalUart1TxIsr(void)
{
  // Nothing for now, as ouput is blocking from the __write function
}


//////////////////////////////////////////////////////////////////////////////
// Receive

// IAR Standard library hook for serial input
size_t __read(int handle, unsigned char * buffer, size_t size)
{
  int nChars = 0;

  /* This template only reads from "standard in", for all other file
   * handles it returns failure. */
  if (handle != _LLIO_STDIN)
  {
    return _LLIO_ERROR;
  }

  for(nChars = 0; (rxUsed>0) && (nChars < size); nChars++) {
    ATOMIC(
      *buffer++ = rxQ[rxTail];
      rxTail = (rxTail+1) % RECEIVE_QUEUE_SIZE;
      rxUsed--;
    )
  }

  return nChars;
}

static void halInternalUart1RxIsr(void)
{

  // At present we really don't care which interrupt(s)
  // occurred, just that one did.  Loop reading RXVALID
  // data, processing any errors noted
  // along the way.
  while ( SC1_UARTSTAT & SC_UARTRXVAL ) {
    int8u errors = SC1_UARTSTAT & (SC_UARTFRMERR |
                                   SC_UARTRXOVF  |
                                   SC_UARTPARERR );
    int8u incoming = (int8u) SC1_DATA;

    if ( (errors == 0) && (rxUsed < (RECEIVE_QUEUE_SIZE-1)) ) {
      rxQ[rxHead] = incoming;
      rxHead = (rxHead+1) % RECEIVE_QUEUE_SIZE;
      rxUsed++;
    } else {
      // IAR standard library doesn't allow for any error handling in the 
      //  case of rx errors or not having space in the receive queue, so the
      //  errors are just silently dropped.
    }
  } // end of while ( SC1_UARTSTAT & SC1_UARTRXVAL )
}


//////////////////////////////////////////////////////////////////////////////
// Interrupts

void halSc1Isr(void)
{
  int32u interrupt;

  //this read and mask is performed in two steps otherwise the compiler
  //will complain about undefined order of volatile access
  interrupt = INT_SC1FLAG;
  interrupt &= INT_SC1CFG;
  
  while (interrupt != 0) {
  
    INT_SC1FLAG = interrupt; // acknowledge the interrupts early

    // RX events
    if ( interrupt & (INT_SCRXVAL   | // RX has data
                      INT_SCRXOVF   | // RX Overrun error
                      INT_SC1FRMERR | // RX Frame error
                      INT_SC1PARERR ) // RX Parity error
       ) {
      halInternalUart1RxIsr();
    }
    
    // TX events
    if ( interrupt & (INT_SCTXFREE | // TX has room
                      INT_SCTXIDLE ) // TX idle (more room)
       ) {
      halInternalUart1TxIsr();
    }

    interrupt = INT_SC1FLAG;
    interrupt &= INT_SC1CFG;
  }
}

/*******************************************************************************
* Function Name  : __io_getcharNonBlocking
* Description    : Non blocking read 
* Input          : none
* Output         : dataByte: buffer containing the read byte if any
* Return         : TRUE if there is a data, FALSE otherwise
*******************************************************************************/
boolean __io_getcharNonBlocking(int8u *data)
{
  if (__read(_LLIO_STDIN,data,1))
    return TRUE;
  else
    return FALSE;
}/* end serialReadByte() */

void __io_putchar( char c )
{
  __write(_LLIO_STDOUT, (unsigned char *)&c, 1);
}

int __io_getchar()
{
  unsigned char c;
  __read(_LLIO_STDIN, &c, 1);
  return (int)(c);
}

void __io_flush( void )
{
  __write(_LLIO_STDOUT, NULL, 0);
}

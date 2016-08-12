/*
 * Copyright (c) 2015 NXP B.V.
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
 * 3. Neither the name of NXP B.V. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP B.V. AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NXP B.V. OR CONTRIBUTORS BE LIABLE
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
 * Author: Lee Mitchell
 * Integrated into Contiki by Beshr Al Nahas
 *
 */

#include <jendefs.h>

#ifdef DEBUG
#include <dbg.h>
#else
#define DBG_vPrintf(...)
#endif

#include "contiki-conf.h"
#include "uart-driver.h"
#include "sys/rtimer.h"
#include "watchdog.h"
#include <math.h>
#include <AppHardwareApi.h>

#if UART_XONXOFF_FLOW_CTRL

#include "sys/process.h"

#define TX_FIFO_SW_FLOW_LIMIT 8 /* Maximum allowed fill level for tx fifo */
#if TX_FIFO_SW_FLOW_LIMIT > 16
#undef TX_FIFO_SW_FLOW_LIMIT
#define TX_FIFO_SW_FLOW_LIMIT 16
#warning "TX_FIFO_SW_FLOW_LIMIT too big. Forced to 16."
#endif /* TX_FIFO_SW_FLOW_LIMIT > 16 */

#define XON   17
#define XOFF  19

extern volatile unsigned char xonxoff_state;

#endif /* UART_XONXOFF_FLOW_CTRL */

/***        Macro Definitions                                             ***/
#define BUSYWAIT_UNTIL(cond, max_time) \
  do { \
    rtimer_clock_t t0; \
    t0 = RTIMER_NOW(); \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) ; \
  } while(0)

#define DEBUG_UART_BUFFERED FALSE

#define CHAR_DEADLINE (uart_char_delay * 100)

/***        Local Function Prototypes                                     ***/
static void uart_driver_isr(uint32_t device_id, uint32_t item_bitmap);
#if !UART_XONXOFF_FLOW_CTRL
static int16_t uart_driver_get_tx_fifo_available_space(uint8_t uart_dev);
#endif /* !UART_XONXOFF_FLOW_CTRL */
static void uart_driver_set_baudrate(uint8_t uart_dev, uint8_t br);
static void uart_driver_set_high_baudrate(uint8_t uart_dev, uint32_t baud_rate);

/***        Local Variables                                               ***/
#define UART_NUM_UARTS  2
static uint16_t tx_fifo_size[UART_NUM_UARTS] = { 0 };
static uint8_t active_uarts[UART_NUM_UARTS] = { 0 };
/** slip input function pointer */
static int(*uart_input[UART_NUM_UARTS]) (unsigned char) = { 0 };
/* time in uSec for transmitting 1 char */
static uint16_t uart_char_delay = 0;
static volatile int8_t interrupt_enabled[UART_NUM_UARTS] = { 0 };
static volatile int8_t interrupt_enabled_saved[UART_NUM_UARTS] = { 0 };

/****************************************************************************
 *
 * NAME:       uart_driver_init
 *
 * DESCRIPTION:
 * Initializes the specified UART device.
 *
 * PARAMETERS:      Name             RW  Usage
 *                  uart_dev         R   UART to initialise, eg, E_AHI_UART_0
 *                  br               R   Baudrate to use (e.g. UART_RATE_115200)
 *                                       if br > UART_RATE_115200
 *                                       then uart_driver_set_baud_rate is called
 *                                       else vAHI_UartSetClockDivisor
 *                  txbuf_data       R   Pointer to a memory block to use
 *                  and rxbuf_data       as uart tx/rx fifo
 *                  txbuf_size       R   size of tx fifo (valid range: 16-2047)
 *                  txbuf_size       R   size of rx fifo (valid range: 16-2047)
 *                  uart_input_function  a function pointer to input uart rx bytes
 * RETURNS:
 * void
 *
 ****************************************************************************/
void
uart_driver_init(uint8_t uart_dev, uint8_t br, uint8_t *txbuf_data,
                 uint16_t txbuf_size, uint8_t *rxbuf_data, uint16_t rxbuf_size,
                 int (*uart_input_function)(unsigned char c))
{
#if !UART_HW_FLOW_CTRL
  /* Disable RTS/CTS */
  vAHI_UartSetRTSCTS(uart_dev, FALSE);
#endif

  tx_fifo_size[uart_dev] = txbuf_size;

  /* Configure the selected Uart */
  uint8_t uart_enabled = bAHI_UartEnable(uart_dev, txbuf_data, txbuf_size,
                                         rxbuf_data, rxbuf_size);
  /* fallback to internal buffers */
  if(!uart_enabled) {
    vAHI_UartEnable(uart_dev);
    tx_fifo_size[uart_dev] = 16; /* Fixed size */
  }
  /* Reset tx/rx fifos */
  vAHI_UartReset(uart_dev, TRUE, TRUE);
  vAHI_UartReset(uart_dev, FALSE, FALSE);

  uart_driver_set_baudrate(uart_dev, br);

  /* install interrupt service callback */
  if(uart_dev == E_AHI_UART_0) {
    vAHI_Uart0RegisterCallback((void *)uart_driver_isr);
  } else {
    vAHI_Uart1RegisterCallback((void *)uart_driver_isr);
    /* Enable RX interrupt */
  }
  uart_driver_enable_interrupts(uart_dev);
  uart_input[uart_dev] = uart_input_function;
  active_uarts[uart_dev] = 1;

#if UART_HW_FLOW_CTRL
  /* Configure HW flow control */
  vAHI_UartSetAutoFlowCtrl(uart_dev, E_AHI_UART_FIFO_ARTS_LEVEL_13, /* uint8 const u8RxFifoLevel,*/
                           FALSE, /* bool_t const bFlowCtrlPolarity,*/
                           TRUE, /* bool_t const bAutoRts, */
                           TRUE /* bool_t const bAutoCts */);
#endif

  DBG_vPrintf("UART %d init: using %s buffers %d\n", uart_dev,
         uart_enabled ? "external" : "internal", tx_fifo_size[uart_dev]);
}
void
uart_driver_enable_interrupts(uint8_t uart_dev)
{
  /* wait while char being tx is done */
  while((u8AHI_UartReadLineStatus(uart_dev) & E_AHI_UART_LS_THRE) == 0) ;

  vAHI_UartSetInterrupt(uart_dev, FALSE /*bEnableModemStatus*/,
                        FALSE /*bEnableRxLineStatus == Break condition */,
                        FALSE /*bEnableTxFifoEmpty*/,
                        TRUE /* bEnableRxData */, E_AHI_UART_FIFO_LEVEL_14);
  interrupt_enabled[uart_dev] = 1;
}
void
uart_driver_disable_interrupts(uint8_t uart_dev)
{
  /* wait while char being tx is done */
  while((u8AHI_UartReadLineStatus(uart_dev) & E_AHI_UART_LS_THRE) == 0) ;

  vAHI_UartSetInterrupt(uart_dev, FALSE /*bEnableModemStatus*/,
                        FALSE /*bEnableRxLineStatus == Break condition */,
                        FALSE /*bEnableTxFifoEmpty*/,
                        FALSE /* bEnableRxData */, E_AHI_UART_FIFO_LEVEL_14);
  interrupt_enabled[uart_dev] = 0;
}
void
uart_driver_store_interrupts(uint8_t uart_dev)
{
  interrupt_enabled_saved[uart_dev] = interrupt_enabled[uart_dev];
}
void
uart_driver_restore_interrupts(uint8_t uart_dev)
{
  if(interrupt_enabled_saved[uart_dev]) {
    uart_driver_enable_interrupts(uart_dev);
  } else {
    uart_driver_disable_interrupts(uart_dev);
  }
}
int8_t
uart_driver_interrupt_is_enabled(uint8_t uart_dev)
{
  return interrupt_enabled[uart_dev];
}
void
uart_driver_set_input(uint8_t uart_dev, int
                      (*uart_input_function)(unsigned char c))
{
  uart_input[uart_dev] = uart_input_function;
}
/****************************************************************************
 *
 * NAME:       uart_driver_read
 *
 * DESCRIPTION:
 * Reads 1 byte from the RX buffer. If there is no data in the
 * buffer, then return FALSE
 *
 * PARAMETERS:      Name            RW  Usage
 *                  uart_dev        R   UART to use, eg, E_AHI_UART_0
 *
 * RETURNS:
 * TRUE if a byte has been read from the queue
 *
 ****************************************************************************/
uint8_t
uart_driver_read(uint8_t uart_dev, uint8_t *data)
{
  if(data && u16AHI_UartReadRxFifoLevel(uart_dev) > 0) {
    *data = u8AHI_UartReadData(uart_dev);
    return TRUE;
  }
  return FALSE;
}
void
uart_driver_write_buffered(uint8_t uart_dev, uint8_t ch)
{
  uart_driver_write_with_deadline(uart_dev, ch);
}
/****************************************************************************
 *
 * NAME:       uart_driver_write_with_deadline
 *
 * DESCRIPTION:
 * Writes one byte to the specified uart for transmission
 *
 * PARAMETERS:      Name            RW  Usage
 *                  uart_dev        R   UART to use, eg, E_AHI_UART_0
 *                  ch              R   data to transmit
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void
uart_driver_write_with_deadline(uint8_t uart_dev, uint8_t ch)
{
#if UART_XONXOFF_FLOW_CTRL
  /* Block until host can receive data */
  /* Wait until there are less than N characters in TX FIFO */
  while(xonxoff_state != XON
        || u16AHI_UartReadTxFifoLevel(uart_dev) > TX_FIFO_SW_FLOW_LIMIT) {
    watchdog_periodic();
  }
  /* write to TX FIFO and return immediately */
  vAHI_UartWriteData(uart_dev, ch);
#else /* UART_XONXOFF_FLOW_CTRL */
  volatile int16_t write = 0;
  watchdog_periodic();
  /* wait until there is space in tx fifo */
  BUSYWAIT_UNTIL(write = (uart_driver_get_tx_fifo_available_space(uart_dev) > 0),
                 CHAR_DEADLINE);
  /* write only if there is space so we do not get stuck */
  if(write) {
    /* write to TX FIFO and return immediately */
    vAHI_UartWriteData(uart_dev, ch);
  }
#endif /* UART_XONXOFF_FLOW_CTRL */
}
void
uart_driver_write_direct(uint8_t uart_dev, uint8_t ch)
{
  /* Write character */
  vAHI_UartWriteData(uart_dev, ch);
  /* Wait for buffers to empty */
  while((u8AHI_UartReadLineStatus(uart_dev) & E_AHI_UART_LS_THRE) == 0) ;
  while((u8AHI_UartReadLineStatus(uart_dev) & E_AHI_UART_LS_TEMT) == 0) ;
}
/****************************************************************************
 *
 * NAME:       uart_driver_rx_handler
 *
 * DESCRIPTION:
 * Interrupt service callback for UART data reception. Reads a received
 * byte from the UART and writes it to the reception buffer if it is not
 * full.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  uart_dev        R   Uart to read from
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void
uart_driver_rx_handler(uint8_t uart_dev)
{
  /* optimization for high throughput: Read upto 32 bytes from RX fifo.
   * Disabled because it does not work with current slip_input_byte */

  /* Status from uart_input:
   *  0 means do not exit power saving mode
   * -1 means RX buffer overflow ==> stop reading
   *  1 means end of slip packet
   */
#if UART_XONXOFF_FLOW_CTRL
  /* save old status */
  int xonxoff_state_old = xonxoff_state;
#endif /* UART_XONXOFF_FLOW_CTRL */
  int status = 0;
  int c = 0;
  while(u16AHI_UartReadRxFifoLevel(uart_dev) > 0 && c++ < 32 && status == 0) {
    if(uart_input[uart_dev] != NULL) { /* read one char at a time */

      /* process received character */
      status = (uart_input[uart_dev])(u8AHI_UartReadData(uart_dev));

#if UART_XONXOFF_FLOW_CTRL
      /* Process XON-XOFF*/
      if(xonxoff_state == XOFF) {
        /* XXX do not set break condition as it corrupts one character, instead we block on TX */
        /* Instruct uart to stop TX */
        /* vAHI_UartSetBreak(uart_dev, TRUE); */
        break;
      } else if(xonxoff_state_old == XOFF && xonxoff_state == XON) {
        /* Instruct uart to resume TX if it was stopped */
        /* vAHI_UartSetBreak(uart_dev, FALSE); */
      }
#endif /* UART_XONXOFF_FLOW_CTRL */
    } else {
      /* no input handler, or no bytes to read: Discard byte. */
      u8AHI_UartReadData(uart_dev);
    }
  }
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#if !UART_XONXOFF_FLOW_CTRL
/* Returns the free space in tx fifo, i.e., how many characters we can put */
static int16_t
uart_driver_get_tx_fifo_available_space(uint8_t uart_dev)
{
  return tx_fifo_size[uart_dev] - u16AHI_UartReadTxFifoLevel(uart_dev);
}
#endif /* !UART_XONXOFF_FLOW_CTRL */
/* Initializes the specified UART with auto-selection of
   baudrate tuning method */
static void
uart_driver_set_baudrate(uint8_t uart_dev, uint8_t br)
{
  uint32_t high_br = 0;
  uint8_t low_br = 0;

  switch(br) {
  case UART_RATE_4800:
    low_br = E_AHI_UART_RATE_4800;
    uart_char_delay = 1667;
    break;
  case UART_RATE_9600:
    low_br = E_AHI_UART_RATE_9600;
    uart_char_delay = 834;
    break;
  case UART_RATE_19200:
    low_br = E_AHI_UART_RATE_19200;
    uart_char_delay = 417;
    break;
  case UART_RATE_38400:
    low_br = E_AHI_UART_RATE_38400;
    uart_char_delay = 209;
    break;
  case UART_RATE_76800:
    low_br = E_AHI_UART_RATE_76800;
    uart_char_delay = 105;
    break;
  case UART_RATE_115200:
    low_br = E_AHI_UART_RATE_115200;
    uart_char_delay = 69;
    break;
  case UART_RATE_230400:
    high_br = 230400UL;
    uart_char_delay = 35;
    break;
  case UART_RATE_460800:
    high_br = 460800UL;
    uart_char_delay = 18;
    break;
  case UART_RATE_500000:
    high_br = 500000UL;
    uart_char_delay = 16;
    break;
  case UART_RATE_576000:
    high_br = 576000UL;
    uart_char_delay = 14;
    break;
  case UART_RATE_921600:
    high_br = 921600UL;
    uart_char_delay = 9;
    break;
  case UART_RATE_1000000:
    high_br = 1000000UL;
    uart_char_delay = 8;
    break;
  default:
    high_br = 1000000UL;
    uart_char_delay = 8;
    break;
  }
  if(high_br == 0) {
    vAHI_UartSetClockDivisor(uart_dev, low_br);
  } else {
    uart_driver_set_high_baudrate(uart_dev, high_br);
  }
}
/****************************************************************************
 *
 * NAME:       uart_driver_set_high_baudrate
 *
 * DESCRIPTION:
 * Sets the baud rate for the specified uart
 *
 * PARAMETERS:      Name            RW  Usage
 *                  uart_dev        R   UART to initialise, eg, E_AHI_UART_0
 *                  baud_rate       R   Baudrate to use (bps eg 921600)
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void
uart_driver_set_high_baudrate(uint8_t uart_dev, uint32_t baud_rate)
{
  uint16 u16Divisor = 1;
  uint32_t u32Remainder;
  uint8_t u8ClocksPerBit = 16;

#if (ENABLE_ADVANCED_BAUD_SELECTION)
  /* Defining ENABLE_ADVANCED_BAUD_SELECTION in the Makefile
   * enables this code which searches for a clocks per bit setting
   * that gets closest to the configured rate.
   */
  uint32_t u32CalcBaudRate = 0;
  int32 i32BaudError = 0x7FFFFFFF;

  DBG_vPrintf(DEBUG_UART_BUFFERED, "Config uart=%d, baud=%d\n", uart_dev,
              baud_rate);

  while(ABS(i32BaudError) > (int32)(baud_rate >> 4)) { /* 6.25% (100/16) error */
    if(--u8ClocksPerBit < 3) {
      DBG_vPrintf(DEBUG_UART_BUFFERED,
                  "Could not calculate UART settings for target baud!");
      return;
    }
#endif /* ENABLE_ADVANCED_BAUD_SELECTION */

  /* Calculate Divisor register = 16MHz / (16 x baud rate) */
  u16Divisor = (uint16)(16000000UL / ((u8ClocksPerBit + 1) * baud_rate));

  /* Correct for rounding errors */
  u32Remainder =
    (uint32_t)(16000000UL % ((u8ClocksPerBit + 1) * baud_rate));

  if(u32Remainder >= (((u8ClocksPerBit + 1) * baud_rate) / 2)) {
    u16Divisor += 1;
  }
#if (ENABLE_ADVANCED_BAUD_SELECTION)
  DBG_vPrintf(DEBUG_UART_BUFFERED, "Divisor=%d, cpb=%d\n", u16Divisor,
              u8ClocksPerBit);

  u32CalcBaudRate = (16000000UL / ((u8ClocksPerBit + 1) * u16Divisor));

  DBG_vPrintf(DEBUG_UART_BUFFERED, "Calculated baud=%d\n", u32CalcBaudRate);

  i32BaudError = (int32)u32CalcBaudRate - (int32)baud_rate;

  DBG_vPrintf(DEBUG_UART_BUFFERED, "Error baud=%d\n", i32BaudError);
}
DBG_vPrintf(DEBUG_UART_BUFFERED, "Config uart=%d: Divisor=%d, cpb=%d\n",
            uart_dev, u16Divisor, u8ClocksPerBit);

/* Set the calculated clocks per bit */
vAHI_UartSetClocksPerBit(uart_dev, u8ClocksPerBit);
#endif /* ENABLE_ADVANCED_BAUD_SELECTION */

  /* Set the calculated divisor */
  vAHI_UartSetBaudDivisor(uart_dev, u16Divisor);
}

/****************************************************************************
 *
 * NAME:       uart_driver_isr
 *
 * DESCRIPTION:
 * Interrupt service callback for UART's
 *
 * PARAMETERS:        Name          RW  Usage
 *                    device_id     R   Device ID of whatever generated the
 *                                      interrupt
 *                    item_bitmap   R   Which part of the device generated
 *                                      the interrupt
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void
uart_driver_isr(uint32_t device_id, uint32_t item_bitmap)
{
  uint8_t uart_dev;
  switch(device_id) {
  case E_AHI_DEVICE_UART0:
    uart_dev = E_AHI_UART_0;
    break;
  case E_AHI_DEVICE_UART1:
    uart_dev = E_AHI_UART_1;
    break;
  default:
    return;
  }
  switch(item_bitmap) {
  /* byte available since a long time but RX-fifo not full: */
  case E_AHI_UART_INT_TIMEOUT:
  /* RX-fifo full: */
  case E_AHI_UART_INT_RXDATA:
    uart_driver_rx_handler(uart_dev);
    break;
  case E_AHI_UART_INT_TX:
    break;
  case E_AHI_UART_INT_RXLINE:
    /* rx-line interrupt is disabled. Should not get here */
    /* An error condition has occurred on the RxD line, such as
       a break indication, framing error, parity error or over-run. */
    break;
  }
}
/****************************************************************************
 *
 * NAME:       uart_driver_tx_in_progress
 *
 * DESCRIPTION:
 * Returns the state of data transmission
 *
 * PARAMETERS:      Name            RW  Usage
 *                  uart_dev        R   UART to use, eg, E_AHI_UART_0
 *
 * RETURNS:
 * uint8_t:   TRUE if data in buffer is being transmitted
 *            FALSE if all data in buffer has been transmitted by the UART
 *
 ****************************************************************************/
uint8_t
uart_driver_tx_in_progress(uint8_t uart_dev)
{

  if(u16AHI_UartReadTxFifoLevel(uart_dev) == 0) {
    if((u8AHI_UartReadLineStatus(uart_dev) & E_AHI_UART_LS_TEMT) != 0) {
      return FALSE;
    }
  }
  return TRUE;
}
#ifdef UART_EXTRAS

/****************************************************************************
 *
 * NAME:       uart_driver_flush
 *
 * DESCRIPTION:
 * Flushes the buffers of the specified UART
 *
 * PARAMETERS:      Name            RW  Usage
 *                  uart_dev        R   UART to disable, eg, E_AHI_UART_0
 *                  reset_tx        R   to reset the transmit FIFO
 *                  reset_rx        R   to reset the receive FIFO
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void
uart_driver_flush(uint8_t uart_dev, bool_t reset_tx, bool_t reset_rx)
{
  /* Disable TX Fifo empty and Rx data interrupts */
  uart_driver_disable_interrupts(uart_dev);

  /* flush hardware buffer */
  vAHI_UartReset(uart_dev, reset_tx, reset_rx);
  vAHI_UartReset(uart_dev, FALSE, FALSE);

  /* Re-enable TX Fifo empty and Rx data interrupts */
  uart_driver_enable_interrupts(uart_dev);
}
#endif /* UART_EXTRAS */

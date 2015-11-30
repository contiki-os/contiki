/** @file hal/micro/cortexm3/uart.h
 * @brief Header for STM32W  uart drivers, supporting IAR's standard library
 *        IO routines.
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */

#ifndef UART_MIN_H_
#define UART_MIN_H_
#ifdef __ICCARM__
#include <yfuns.h>
#endif

/**
 * @brief A list of the possible values for the parity parameter to uartInit()
 */
typedef enum
{
  PARITY_NONE = 0,
  PARITY_ODD = 1,
  PARITY_EVEN = 2
} SerialParity;

/**
 * @brief Initialize the UART
 *
 * @param baudrate  The baudrate which will be used for communication.
 *                  Ex: 115200
 *
 * @param databits  The number of data bits used for communication.
 *                  Valid values are 7 or 8
 *
 * @param parity    The type of parity used for communication.
 *                  See the SerialParity enum for possible values
 *
 * @param stopbits  The number of stop bits used for communication.
 *                  Valid values are 1 or 2
 */
void uartInit(uint32_t baudrate, uint8_t databits, SerialParity parity, uint8_t stopbits);

#ifdef __ICCARM__
/**
 * @brief Flush the output stream.  DLib_Config_Full.h defines fflush(), but
 * this library includes too much code so we compile with DLib_Config_Normal.h
 * instead which does not define fflush().  Therefore, we manually define
 * fflush() in the low level UART driver.  This function simply redirects
 * to the __write() function with a NULL buffer, triggering a flush.
 *
 * @param handle  The output stream.  Should be set to 'stdout' like normal.
 *
 * @return Zero, indicating success.
 */
size_t fflush(int handle);

/**
 * @brief Define the stdout stream.  Since we compile with DLib_Config_Normal.h
 * it does not define 'stdout'.  There is a low-level IO define '_LLIO_STDOUT'
 * which is equivalent to stdout.  Therefore, we define 'stdout' to be
 * '_LLIO_STDOUT'.
 */
#define stdout _LLIO_STDOUT
#endif
/**
 * @brief Read the input byte if any.
 */
boolean __io_getcharNonBlocking(uint8_t *data);
void __io_putchar( char c );
int __io_getchar(void);
void __io_flush( void );


#endif //UART_MIN_H_

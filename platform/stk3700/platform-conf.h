/*
 */

/**
 * \file
 *         A brief description of what this file is
 * \author
 *         Martin Chaplet <m.chaplet@kerlink.fr>
 */

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include <stdint.h>
#include "efm32gg990f1024.h"
#include "gpio.h"

#include <stdint.h>
#include "efm32gg990f1024.h"
#include "gpio.h"

#ifdef WITH_SPI1
#include "usart1.h"
#endif

// Default systick config : 1ms by tick
#define CLOCK_CONF_SECOND 1000

/*
 * High Frequencies clock configuration
 */

// Default RCO clock is 14 MHz
#define HFRCOBAND_VALUE _CMU_HFRCOCTRL_BAND_14MHZ
#define HFRCO_FREQ (14*MHZ)

//#define HFRCOBAND_VALUE _CMU_HFRCOCTRL_BAND_28MHZ
//#define HFRCO_FREQ (28*MHZ)

// Use HFRCO as HFCLK
#define HFCLK_FREQ HFRCO_FREQ

// No divider for peripheral clock
#define HFPERCLK_DIV  1
#define HFPERCLK_FREQ (HFCLK_FREQ / HFPERCLK_DIV)

#define RTC_PRESCALE 32768

// To set Also in Application linker script
#define USERAPP_START_ADDR 0x10000


#define SERIAL_LINE_CONF_LOCALECHO
#define SHELL_CONF_PROMPT
#define SERIAL_SHELL_CONF_PROMPT(str) \
  printf("%02X%02X: %s",rimeaddr_node_addr.u8[1],rimeaddr_node_addr.u8[0],str)

//#define DEBUGUART_USART0 1
//#define DEBUGUART_LEUART1 1
#define DEBUGUART_UART0 1

#if DEBUGUART_ITM
#include "core_cm3.h"
#define default_writeb     ITM_SendChar

#elif DEBUGUART_USART0
#define defaultuart_send_bytes   usart0_send_bytes
#define defaultuart_writeb       usart0_writeb
#define defaultuart_drain        usart0_drain
#define defaultuart_set_input    usart0_set_input
#define defaultuart_get_input    usart0_get_input

#elif DEBUGUART_UART0
#define defaultuart_send_bytes   uart0_send_bytes
#define defaultuart_writeb       uart0_writeb
#define defaultuart_drain        uart0_drain
#define defaultuart_set_input    uart0_set_input
#define defaultuart_get_input    uart0_get_input
#elif DEBUGUART_UART1
#define defaultuart_send_bytes   uart1_send_bytes
#define defaultuart_writeb       uart1_writeb
#define defaultuart_drain        uart1_drain
#define defaultuart_set_input    uart1_set_input
#define defaultuart_get_input    uart1_get_input

#elif DEBUGUART_LEUART1
#define defaultuart_send_bytes   leuart1_send_bytes
#define defaultuart_writeb       leuart1_writeb
#define defaultuart_drain        leuart1_drain
#define defaultuart_set_input    leuart1_set_input
#define defaultuart_get_input    leuart1_get_input

#endif

// Set USART0 as Debug UART
#define USART0_LOCATION  0
#if (USART0_LOCATION == 0)
#define GPIO_USART0_TX   GPIO_PE10
#define GPIO_USART0_RX   GPIO_PE11
#endif

#define USART1_LOCATION 1
#if (USART1_LOCATION == 1)
#define GPIO_USART1_TX  GPIO_PD0
#define GPIO_USART1_RX  GPIO_PD1
#define GPIO_USART1_CLK GPIO_PD2
#define GPIO_USART1_CS  GPIO_PD3
#else
#error "Unknown Location for USART1"
#endif

#define USART2_LOCATION 0
#if (USART2_LOCATION == 0)
#define GPIO_USART2_TX   GPIO_PC2
#define GPIO_USART2_RX   GPIO_PC3
#define GPIO_USART2_CLK  GPIO_PC4
#define GPIO_USART2_CS   GPIO_PC5
#else
#error "Unknown Location for USART2"
#endif

#define I2C0_LOCATION 3
#if (I2C0_LOCATION == 2)
#define GPIO_I2C0_SDA   GPIO_PC6
#define GPIO_I2C0_SCL   GPIO_PC7
#elif (I2C0_LOCATION == 3)
#define GPIO_I2C0_SDA   GPIO_PD14
#define GPIO_I2C0_SCL   GPIO_PD15
#endif

#define LEUART1_LOCATION 0
#if (LEUART1_LOCATION == 0)
#define GPIO_LEUART1_TX   GPIO_PC6
#define GPIO_LEUART1_RX   GPIO_PC7
#else
#error "Unknown Location for LEUART1"
#endif

#define UART0_LOCATION 1
#if (UART0_LOCATION == 1)
#define GPIO_UART0_TX   GPIO_PE0
#define GPIO_UART0_RX   GPIO_PE1
#else
#error "Unknown Location for UART0"
#endif

#define UART1_LOCATION 2
#if (UART1_LOCATION == 1)
#define GPIO_UART1_TX   GPIO_PF10
#define GPIO_UART1_RX   GPIO_PF11
#elif (UART1_LOCATION == 2)
#define GPIO_UART1_TX   GPIO_PB9
#define GPIO_UART1_RX   GPIO_PB10
#else
#error "Unknown Location for UART1"
#endif

#define GPIO_USER_LED1 GPIO_PE2
#define GPIO_USER_LED2 GPIO_PE3

#define GPIO_BOOTLOADER GPIO_PD7

#define SPI_ENABLE()     gpio_set_value(GPIO_USART1_CS, 0)
#define SPI_DISABLE()    gpio_set_value(GPIO_USART1_CS, 1)
#define SPI_IS_ENABLED() (!gpio_get_value(GPIO_USART1_CS))

#define SPI_INIT()              spi1_init()
#define SPI_READWRITE(val)      spi1_readwrite(val)
#define SPI_READBUF(ptab,len)   spi1_readbuf(ptab,len)
#define SPI_WRITEBUF(ptab,len)  spi1_writebuf(ptab,len)

#define LEDS_CONF_ALL 3
#define LEDS_USER1    1
#define LEDS_USER2    2
#define LEDS_USER     LEDS_USER1

#endif /* __PLATFORM_CONF_H__ */

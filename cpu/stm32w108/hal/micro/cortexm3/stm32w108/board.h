/** @file board.h
 * @brief Header file x STM32W108 Kits boards 
 *
 * <!--(C) COPYRIGHT 2010 STMicroelectronics. All rights reserved.        -->
 */
#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef BOARD_MB851

/* leds definitions */
#define LED_D1 PORTB_PIN(6)
#define LED_D3 PORTB_PIN(5)


/** Description buttons definition */
#define BUTTON_S1             PORTA_PIN(7)
#define BUTTON_S1_INPUT_GPIO  GPIO_PAIN
#define BUTTON_S1_OUTPUT_GPIO GPIO_PAOUT
#define BUTTON_S1_GPIO_PIN    PA7_BIT
#define BUTTON_S1_WAKE_SOURCE 0x00000080

/** Description uart definition */
#define UART_TX PORTB_PIN(1)
#define UART_RX PORTB_PIN(2)
#define UART_RX_WAKE_SOURCE   0x00000400

/** Description temperature sensor GPIO */
#define TEMPERATURE_SENSOR_GPIO  PORTB_PIN(7)
#endif /* BOARD_MB851 */

#endif /* _BOARD_H_ */


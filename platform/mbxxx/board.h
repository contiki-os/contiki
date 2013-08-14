/**
 * \addtogroup mbxxx-platform
 *
 * @{
 */
/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*          Header file x STM32W108 Kits boards abstraction.
* \brief
*          Header file x STM32W108 Kits boards abstraction.
* See \ref board for documentation.
*
*
* See hal/micro/cortexm3/stm32w108/board.h for source code.
*
*/
/*---------------------------------------------------------------------------*/
#ifndef _BOARD_H_
#define _BOARD_H_

char boardName[16];

/** @addtogroup board
 * \brief ST board abstraction layer
 *
 * This header defines API and data structures to handle ST boards with thei associated resources
 * on algorithm behavior.
 * See hal/micro/cortexm3/stm32w108/board.h for source code.
 *@{
 */

/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB851A 2
/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB851B 2
/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB851C 2
/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB851D 2



/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB954A 2
/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB954B 2
/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB954C 2



/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB950A 2
/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_MB951A 2



/**
 * \brief Define the number of LEDs in the specific board revision
 */
#define LEDS_IDZ401V1 1



/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB851A 1
/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB851B 1
/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB851C 1
/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB851D 1



/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB954A 1
/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB954B 1
/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB954C 1



/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB950A 5
/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_MB951A 1



/**
 * \brief Define the number of user buttons in the specific board revision
 */
#define BUTTONS_IDZ401V1 0



/**
 * \brief Data structure for led description
 */
typedef struct LedResourceStruct {
  /** Name of the LED as printed in the board */
  char *name;
  /** GPIO port associated with the LED */
  uint8_t    gpioPort;
  /** GPIO pin associated with the LED */
  uint8_t    gpioPin;
} LedResourceType;

typedef LedResourceType InfraRedLedResourceType;

/**
 * \brief Data structure for button description
 */
typedef struct ButtonResourceStruct {
  /** Name of the button as printed in the board */
  char *name;
  /** GPIO port associated with the button */
  uint8_t    gpioPort;
  /** GPIO pin associated with the button */
  uint8_t    gpioPin;
} ButtonResourceType;

/**
 * \brief Data structure for MEMS description
 */
typedef struct MemsResourceStruct {
  /** Name of the MEMS device */
  char *name;
  /** Serial communication port associated with the MEMS */
  uint8_t    scPort;
} MemsResourceType;

/**
 * \brief Data structure for temperature sensor description
 */
typedef struct TempSensorResourceStruct {
  /** Name of the temperature sensor device */
  char *name;
  /** GPIO port associated with the sensor */
  uint8_t    gpioPort;
  /** GPIO pin associated with the sensor */
  uint8_t    gpioPin;
  /** Voltage divider network from sensor vout and GPIO */
  uint8_t    div;
  /** Flag to indicate whether the ADC range extension bug fix is implemented */
  boolean  adcFix;
} TempSensorResourceType;

/**
 * \brief Data structure for board user I/O
 */
typedef struct BoardIOStruct {
  /** Pointer to LED resources */
  const LedResourceType *leds;
  /** Pointer to button resources */
  const ButtonResourceType *buttons;  
} BoardIOType;

/**
 * \brief Flag to indicate if MEMS is present
 */
#define BOARD_HAS_MEMS        (1 << 0)
/**
 * \brief Flag to indicate if temeprature sensor is present
 */
#define BOARD_HAS_TEMP_SENSOR (1 << 1)
/**
 * \brief Flag to indicate if external power amplifier is present
 */
#define BOARD_HAS_PA          (1 << 2)
/**
 * \brief Flag to indicate if EEPROM is present
 */
#define BOARD_HAS_EEPROM      (1 << 3)
/**
 * \brief Flag to indicate if FTDI is used as PC interface
 */
#define BOARD_HAS_FTDI        (1 << 4)
/**
 * \brief Flag to indicate if STM32F is used as PC interface
 */
#define BOARD_HAS_STM32F      (1 << 5)

/**
 * \brief Data structure describing board features
 */
typedef struct BoardResourcesStruct {
  const char *name;
  const uint32_t flags;
  /** Number of buttons */
  uint8_t    buttons;
  /** Number of leds */
  uint8_t    leds;
  /** Board I/O description */
  const BoardIOType *io;
  /** Board infrared led description */
  const InfraRedLedResourceType* infraredLed;
  /** Board infrared MEMS description */
  const MemsResourceType *mems;
  /** Board infrared temeprature sensor description */
  const TempSensorResourceType *temperatureSensor;
} BoardResourcesType;

extern BoardResourcesType const *boardDescription;

// Generic definitions
#define GPIO_PxCLR_BASE (GPIO_PACLR_ADDR)
#define GPIO_PxSET_BASE (GPIO_PASET_ADDR)
#define GPIO_PxOUT_BASE (GPIO_PAOUT_ADDR)
#define GPIO_PxIN_BASE  (GPIO_PAIN_ADDR)
// Each port is offset from the previous port by the same amount
#define GPIO_Px_OFFSET  (GPIO_PBCFGL_ADDR-GPIO_PACFGL_ADDR)


/* leds definitions */
#define LED_D1 PORTx_PIN(boardDescription->io->leds[0].gpioPort, boardDescription->io->leds[0].gpioPin)  //PORTB_PIN(6)
#define LED_D3 PORTx_PIN(boardDescription->io->leds[1].gpioPort, boardDescription->io->leds[1].gpioPin) // PORTB_PIN(5)
#define DUMMY_LED 0xff


/** Description buttons definition */
#define BUTTON_Sn(n) (PORTx_PIN(boardDescription->io->buttons[n].gpioPort, boardDescription->io->buttons[n].gpioPin))
#define BUTTON_Sn_WAKE_SOURCE(n) (1 << ((boardDescription->io->buttons[n].gpioPin) + (8 * (boardDescription->io->buttons[n].gpioPort >> 3))))
#define BUTTON_INPUT_GPIO(port) *((volatile uint32_t *) (GPIO_PxIN_BASE + GPIO_Px_OFFSET * port))
#define DUMMY_BUTTON 0xff

#define BUTTON_S1 (boardDescription->buttons>0 ? BUTTON_Sn(0): DUMMY_BUTTON)
#define BUTTON_S2 (boardDescription->buttons>1 ? BUTTON_Sn(1): DUMMY_BUTTON)
#define BUTTON_S3 (boardDescription->buttons>2 ? BUTTON_Sn(2): DUMMY_BUTTON)
#define BUTTON_S4 (boardDescription->buttons>3 ? BUTTON_Sn(3): DUMMY_BUTTON)
#define BUTTON_S5 (boardDescription->buttons>4 ? BUTTON_Sn(4): DUMMY_BUTTON)

#define BUTTON_S1_WAKE_SOURCE (boardDescription->buttons>0 ? BUTTON_Sn_WAKE_SOURCE(0): 0)
#define BUTTON_S2_WAKE_SOURCE (boardDescription->buttons>1 ? BUTTON_Sn_WAKE_SOURCE(1): 0)
#define BUTTON_S3_WAKE_SOURCE (boardDescription->buttons>2 ? BUTTON_Sn_WAKE_SOURCE(2): 0)
#define BUTTON_S4_WAKE_SOURCE (boardDescription->buttons>3 ? BUTTON_Sn_WAKE_SOURCE(3): 0)
#define BUTTON_S5_WAKE_SOURCE (boardDescription->buttons>4 ? BUTTON_Sn_WAKE_SOURCE(4): 0)

/** Description uart definition */
#define UART_TX PORTB_PIN(1)
#define UART_RX PORTB_PIN(2)
#define UART_RX_WAKE_SOURCE   0x00000400

/** Description temperature sensor GPIO */
#define TEMPERATURE_SENSOR_GPIO  PORTx_PIN(boardDescription->temperatureSensor->gpioPort, boardDescription->temperatureSensor->gpioPin) // PORTB_PIN(7)

/** \brief Return pointer to board description structure
 *
 * 
 * @return Pointer to board description structure
 */
BoardResourcesType const *halBoardGetDescription(void);
/**
 * \brief Initialize the board description data structure after
 *        autodetect of the boards based on the CIB Board name field
 *        content. In case of invalid CIB data it will default to MB851A.
 *        Customer normally needs to modify this file to adapt it to their specific board.
 */
void halBoardInit(void);
/**
 * \brief Perform board specific action to power up the system.
 *        This code depends on the actual board features and configure
 *        the stm32w and on board devices for proper operation.
 *        Customer normally needs to modify this file to adapt it to their specific board.
 */
void halBoardPowerUp(void);
/**
 * \brief Perform board specific action to power down the system, usually before going to deep sleep.
 *        This code depends on the actual board features and configure
 *        the stm32w and on board devices for minimal power consumption.
 *        Customer normally needs to modify this file to adapt it to their specific board.
 */
void halBoardPowerDown(void);


void boardPrintStringDescription(void);

#endif /* _BOARD_H_ */

/** @} // END addtogroup
 */
/** @} */

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
*          Software layer to support all the ST kits boards
*
* \brief
*          Board file x STM32W108 Kits boards
*
* This file implements a software layer to support all the ST kits boards
* and deal with the difference in leds, buttons and sensors connected
* to the board. This a bit too complex for customer boards adaptation
* and customer can simplify it to adapt it to their hardware environment
* as they normally don't need to have the software running on several boards.
* The boards supported are:
* - MB851 A
* - MB851 B
* - MB851 C
* - MB851 D
* - MB954 A
* - MB954 B
* - MB954 C
* - MB950 A
* - MB951 A
* - IDZ401V1
*/
/*---------------------------------------------------------------------------*/

#include PLATFORM_HEADER
#include BOARD_HEADER
#ifdef EMBERZNET_HAL
#include "stack/include/ember.h"
#include "stack/include/error.h"
#include "hal/hal.h"
#define PORTA (0 << 3)
#define PORTB (1 << 3)
#define PORTC (2 << 3)
#else
#include "error.h"
#include "hal/hal.h"
#include "hal/micro/cortexm3/mfg-token.h"
#endif
#include <string.h>

const LedResourceType LedsMB851A[] = {
  {
    "D1", /* Green LED */
    PORTB,
    6
  },
  {
    "D3", /* Yellow LED */
    PORTB,
    5
  }
};

const InfraRedLedResourceType infraRedLedMB851A = {
  "D5", /* Infrared LED */
  PORTB,
  4
};

const LedResourceType LedsMB954A[] = {
  {
    "D1", /* Green LED */
    PORTA,
    5
  },
  {
    "D3", /* Yellow LED */
    PORTA,
    0
  }
};

const LedResourceType LedsIDZ401V1[] = {
  {
    "D2", /* Yellow LED */
    PORTA,
    7
  }
};


const ButtonResourceType ButtonsMB851A[] = {
  {
    "S1",
    PORTA,
    7
  }
};

const ButtonResourceType ButtonsMB954A[] = {
  {
    "S1",
    PORTB,
    3
  }
};

const ButtonResourceType ButtonsMB950A[] = {
  {
    "S1",
    PORTB,
    3
  },
  {
    "S2",
    PORTA,
    7
  },
  {
    "S3",
    PORTB,
    7
  },
  {
    "S4",
    PORTB,
    0
  },
  {
    "S5",
    PORTB,
    6
  }
};

const ButtonResourceType ButtonsMB951A[] = {
  {
    "S1",
    PORTA,
    3
  },
};

const ButtonResourceType ButtonsIDZ401V1[] = {
  
};


const TempSensorResourceType stlm20PB7noDiv = {
  "STLM20",
  PORTB,
  7,
  1,
  FALSE
};

const TempSensorResourceType stlm20PA4noDiv = {
  "STLM20",
  PORTA,
  4,
  1,
  FALSE
};

const TempSensorResourceType stlm20PA4withDiv = {
  "STLM20",
  PORTA,
  4,
  2,
  TRUE  
};

const MemsResourceType memsSensor = {
  "LIS302",
  0
};

const BoardIOType ioMB851A = {
  LedsMB851A,
  ButtonsMB851A,  
};

const BoardIOType ioMB954A = {
  LedsMB954A,
  ButtonsMB954A,  
};

const BoardIOType ioMB950A = {
  LedsMB954A,
  ButtonsMB950A,  
};

const BoardIOType ioMB951A = {
  LedsMB954A,
  ButtonsMB951A,  
};

const BoardIOType ioIDZ401V1 = {
  LedsIDZ401V1,
  ButtonsIDZ401V1,  
};

const BoardResourcesType MB851A = {
  "MB851 A",
  (BOARD_HAS_MEMS | BOARD_HAS_TEMP_SENSOR | BOARD_HAS_FTDI),
  BUTTONS_MB851A,
  LEDS_MB851A,
  &ioMB851A,
  &infraRedLedMB851A,
  &memsSensor,
  &stlm20PB7noDiv,
};

const BoardResourcesType MB851B = {
  "MB851 B",
  (BOARD_HAS_MEMS | BOARD_HAS_TEMP_SENSOR | BOARD_HAS_FTDI),
  BUTTONS_MB851B,
  LEDS_MB851B,
  &ioMB851A,
  &infraRedLedMB851A,
  &memsSensor,
  &stlm20PB7noDiv,
};

const BoardResourcesType MB851C = {
  "MB851 C",
  (BOARD_HAS_MEMS | BOARD_HAS_TEMP_SENSOR | BOARD_HAS_STM32F | BOARD_HAS_EEPROM),
  BUTTONS_MB851C,
  LEDS_MB851C,
  &ioMB954A,
  &infraRedLedMB851A,
  &memsSensor,
  &stlm20PA4withDiv,
};

const BoardResourcesType MB851D = {
  "MB851 D",
  (BOARD_HAS_MEMS | BOARD_HAS_TEMP_SENSOR | BOARD_HAS_STM32F | BOARD_HAS_EEPROM),
    BUTTONS_MB851D,
    LEDS_MB851D,
    &ioMB954A,
    &infraRedLedMB851A,
    &memsSensor,
    &stlm20PA4withDiv,
};

const BoardResourcesType MB954A = {
  "MB954 A",
  (BOARD_HAS_MEMS | BOARD_HAS_TEMP_SENSOR | BOARD_HAS_FTDI | BOARD_HAS_PA),
  BUTTONS_MB954A,
  LEDS_MB954A,
  &ioMB954A,
  &infraRedLedMB851A,
  &memsSensor,
  &stlm20PA4noDiv,
};

const BoardResourcesType MB954B = {
  "MB954 B",
  (BOARD_HAS_MEMS | BOARD_HAS_TEMP_SENSOR | BOARD_HAS_STM32F | BOARD_HAS_EEPROM | BOARD_HAS_PA),
  BUTTONS_MB954B,
  LEDS_MB954B,
  &ioMB954A,
  &infraRedLedMB851A,
  &memsSensor,
  &stlm20PA4withDiv,
};

const BoardResourcesType MB954C = {
  "MB954 C",
  (BOARD_HAS_MEMS | BOARD_HAS_TEMP_SENSOR | BOARD_HAS_STM32F | BOARD_HAS_EEPROM | BOARD_HAS_PA),
    BUTTONS_MB954C,
    LEDS_MB954C,
    &ioMB954A,
    &infraRedLedMB851A,
    &memsSensor,
    &stlm20PA4noDiv,
};

const BoardResourcesType MB950A = {
  "MB950 A",
  (BOARD_HAS_MEMS | BOARD_HAS_TEMP_SENSOR | BOARD_HAS_STM32F),
  BUTTONS_MB950A,
  LEDS_MB950A,
  &ioMB950A,
  &infraRedLedMB851A,
  &memsSensor,
  &stlm20PA4noDiv,
};

const BoardResourcesType MB951A = {
  "MB951 A",
  (BOARD_HAS_STM32F),
  BUTTONS_MB951A,
  LEDS_MB951A,
  &ioMB951A,
  NULL,
  NULL,
  NULL,
};

const BoardResourcesType IDZ401V1 = {
  "IDZ401V1",
  (BOARD_HAS_STM32F),
  BUTTONS_IDZ401V1,
  LEDS_IDZ401V1,
  &ioIDZ401V1,
  NULL,
  NULL,
  NULL,
};

static const BoardResourcesType *boardList [] = {
  &MB851A,
  &MB851B,
  &MB851C,
  &MB851D,
  &MB954A,
  &MB954B,
  &MB954C,
  &MB950A,
  &MB951A,
  &IDZ401V1
};

BoardResourcesType const *boardDescription = NULL;

#define PORT_NAME(x) ((x == PORTA) ? "A": ((x == PORTB) ? "B" : ((x == PORTC) ? "C": "?")))

void halBoardInit(void)
{
  char boardName[16];
  int8_t i;
  boardDescription = NULL;
#ifdef EMBERZNET_HAL
  halCommonGetToken(boardName, TOKEN_MFG_BOARD_NAME);
#else
  halCommonGetMfgToken(boardName, TOKEN_MFG_BOARD_NAME);
#endif
  
  i = 15;
  while ((i >= 0) && (boardName[i] == 0xFF)) {
    boardName[i] = 0;
    i--;
  }

  for (i = 0; i < (sizeof(boardList)/4) ; i++) 
    if (strcmp(boardName, (boardList[i])->name) == 0) {
      boardDescription = (BoardResourcesType *) boardList[i];
      break;
    }

  if (boardDescription == NULL) {
    /* Board type not identified default to MB851A also to support legacy boards */
    boardDescription = (BoardResourcesType *) &IDZ401V1;
  }
  return;
}

BoardResourcesType const *halBoardGetDescription(void)
{
  return boardDescription;
}

void halBoardPowerDown(void)
{
  /* Set everything to input value */
  GPIO_PACFGL = (GPIOCFG_IN              <<PA0_CFG_BIT)|
                (GPIOCFG_IN              <<PA1_CFG_BIT)|
                (GPIOCFG_IN              <<PA2_CFG_BIT)|
                (GPIOCFG_IN              <<PA3_CFG_BIT);
  GPIO_PACFGH = (GPIOCFG_IN              <<PA4_CFG_BIT)|  /* PTI EN */
                (GPIOCFG_IN              <<PA5_CFG_BIT)|  /* PTI_DATA */
                (GPIOCFG_IN              <<PA6_CFG_BIT)|
                (GPIOCFG_IN              <<PA7_CFG_BIT);
  GPIO_PBCFGL = (GPIOCFG_IN              <<PB0_CFG_BIT)|
                (GPIOCFG_IN              <<PB1_CFG_BIT)|  /* Uart TX */
                (GPIOCFG_IN              <<PB2_CFG_BIT)|  /* Uart RX */
                (GPIOCFG_IN              <<PB3_CFG_BIT);
  GPIO_PBCFGH = (GPIOCFG_IN              <<PB4_CFG_BIT)|
                (GPIOCFG_IN              <<PB5_CFG_BIT)|
                (GPIOCFG_IN              <<PB6_CFG_BIT)|
                (GPIOCFG_IN              <<PB7_CFG_BIT);
  GPIO_PCCFGL = (GPIOCFG_IN              <<PC0_CFG_BIT)|
                (GPIOCFG_IN              <<PC1_CFG_BIT)|
                (GPIOCFG_IN              <<PC2_CFG_BIT)|
                (GPIOCFG_IN              <<PC3_CFG_BIT);
  GPIO_PCCFGH = (GPIOCFG_IN              <<PC4_CFG_BIT)|
                (GPIOCFG_IN              <<PC5_CFG_BIT)|
#ifdef EMBERZNET_HAL
                (CFG_C6                  <<PC6_CFG_BIT)|  /* OSC32K */
                (CFG_C7                  <<PC7_CFG_BIT);  /* OSC32K */
#else
                (GPIOCFG_IN              <<PC6_CFG_BIT)|  /* OSC32K */
                (GPIOCFG_IN              <<PC7_CFG_BIT);  /* OSC32K */
#endif


  /* Configure GPIO for BUTTONSs */
  {
    ButtonResourceType *buttons = (ButtonResourceType *) boardDescription->io->buttons;
    uint8_t i;
    for (i = 0; i < boardDescription->buttons; i++) {
        halGpioConfig(PORTx_PIN(buttons[i].gpioPort, buttons[i].gpioPin), GPIOCFG_IN_PUD);
        halGpioSet(PORTx_PIN(buttons[i].gpioPort, buttons[i].gpioPin), GPIOOUT_PULLUP);
    }
  }

  /* Configure GPIO for LEDs */
  {
    LedResourceType *leds = (LedResourceType *) boardDescription->io->leds;
    uint8_t i;
    for (i = 0; i < boardDescription->leds; i++) {
          /* LED default off */
      halGpioConfig(PORTx_PIN(leds[i].gpioPort, leds[i].gpioPin), GPIOCFG_OUT);
      halGpioSet(PORTx_PIN(leds[i].gpioPort, leds[i].gpioPin), 1);
    }
  }

  /* Configure GPIO for power amplifier */
  if (boardDescription->flags & BOARD_HAS_PA) {
	/* SiGe Ant Sel to output */
	halGpioConfig(PORTB_PIN(5), GPIOCFG_OUT);
	halGpioSet(PORTB_PIN(5), 1);
    /* SiGe Standby */
    halGpioConfig(PORTB_PIN(6), GPIOCFG_OUT);
    halGpioSet(PORTB_PIN(6), 0);
  }

}
void halBoardPowerUp(void)
{

  /* Set everything to input value */
  GPIO_PACFGL = (GPIOCFG_IN            <<PA0_CFG_BIT)|
                (GPIOCFG_IN            <<PA1_CFG_BIT)|
                (GPIOCFG_IN            <<PA2_CFG_BIT)|
                (GPIOCFG_IN            <<PA3_CFG_BIT);
  GPIO_PACFGH = (GPIOCFG_IN            <<PA4_CFG_BIT)|  /* PTI EN */
                (GPIOCFG_IN            <<PA5_CFG_BIT)|  /* PTI_DATA */
                (GPIOCFG_IN            <<PA6_CFG_BIT)|
                (GPIOCFG_IN            <<PA7_CFG_BIT);
  GPIO_PBCFGL = (GPIOCFG_IN            <<PB0_CFG_BIT)|
                (GPIOCFG_OUT_ALT       <<PB1_CFG_BIT)|  /* Uart TX */
                (GPIOCFG_IN            <<PB2_CFG_BIT)|  /* Uart RX */
                (GPIOCFG_IN            <<PB3_CFG_BIT);
  GPIO_PBCFGH = (GPIOCFG_IN            <<PB4_CFG_BIT)|
                (GPIOCFG_IN            <<PB5_CFG_BIT)|
                (GPIOCFG_IN            <<PB6_CFG_BIT)|
                (GPIOCFG_IN            <<PB7_CFG_BIT);
  GPIO_PCCFGL = (GPIOCFG_IN            <<PC0_CFG_BIT)|
                (GPIOCFG_IN            <<PC1_CFG_BIT)|
                (GPIOCFG_IN            <<PC2_CFG_BIT)|
                (GPIOCFG_IN            <<PC3_CFG_BIT);
  GPIO_PCCFGH = (GPIOCFG_IN            <<PC4_CFG_BIT)|
                (GPIOCFG_IN            <<PC5_CFG_BIT)|
#ifdef EMBERZNET_HAL
                (CFG_C6                  <<PC6_CFG_BIT)|  /* OSC32K */
                (CFG_C7                  <<PC7_CFG_BIT);  /* OSC32K */
#else
                (GPIOCFG_IN              <<PC6_CFG_BIT)|  /* OSC32K */
                (GPIOCFG_IN              <<PC7_CFG_BIT);  /* OSC32K */
#endif

  /* Configure GPIO for I2C access */
  if ((boardDescription->flags & BOARD_HAS_MEMS) || (boardDescription->flags & BOARD_HAS_EEPROM)) {
    halGpioConfig(PORTA_PIN(1), GPIOCFG_OUT_ALT_OD);
    halGpioConfig(PORTA_PIN(2), GPIOCFG_OUT_ALT_OD);
  }
  /* Configure GPIO for ADC access (temp sensor) */
  if (boardDescription->flags & BOARD_HAS_TEMP_SENSOR) {
    halGpioConfig(PORTx_PIN(boardDescription->temperatureSensor->gpioPort,
                            boardDescription->temperatureSensor->gpioPin),
                  GPIOCFG_ANALOG);
  }
  /* Configure GPIO for LEDs */
  {
    LedResourceType *leds = (LedResourceType *) boardDescription->io->leds;
    uint8_t i;
    for (i = 0; i < boardDescription->leds; i++) {
          /* LED default off */
      halGpioConfig(PORTx_PIN(leds[i].gpioPort, leds[i].gpioPin), GPIOCFG_OUT);
      halGpioSet(PORTx_PIN(leds[i].gpioPort, leds[i].gpioPin), 1);
    }
  }
  /* Configure GPIO for BUTTONSs */
  {
    ButtonResourceType *buttons = (ButtonResourceType *) boardDescription->io->buttons;
    uint8_t i;
    for (i = 0; i < boardDescription->buttons; i++) {
        halGpioConfig(PORTx_PIN(buttons[i].gpioPort, buttons[i].gpioPin), GPIOCFG_IN_PUD);
        halGpioSet(PORTx_PIN(buttons[i].gpioPort, buttons[i].gpioPin), GPIOOUT_PULLUP);
    }
  }

  /* Configure GPIO for power amplifier */
  if (boardDescription->flags & BOARD_HAS_PA) {
    /* SiGe Ant Sel (default ceramic antenna) */
    halGpioConfig(PORTB_PIN(5), GPIOCFG_OUT);
    halGpioSet(PORTB_PIN(5), 1);
    /* SiGe Standby (default out of standby) */
    halGpioConfig(PORTB_PIN(6), GPIOCFG_OUT);
    halGpioSet(PORTB_PIN(6), 1);
    /* SiGe LNA (default LNA off )*/
    halGpioConfig(PORTB_PIN(7), GPIOCFG_OUT);
    halGpioSet(PORTB_PIN(7), 0);
    /* SiGe nTX Active */
    halGpioConfig(PORTC_PIN(5), GPIOCFG_OUT_ALT);
  }
}


#include <stdio.h>
void printLeds (void)
{
  uint8_t i=0;
  LedResourceType *leds;
  leds = (LedResourceType *) boardDescription->io->leds;
  while (i < boardDescription->leds) {
    printf ("Led %s (port:%s, bit:%d)\r\n", leds->name, PORT_NAME(leds->gpioPort), leds->gpioPin);
    leds++; i++;
  }
}

void printButtons (void)
{
  uint8_t i=0;
  ButtonResourceType *buttons;
  buttons = (ButtonResourceType *) boardDescription->io->buttons;
  while (i < boardDescription->buttons) {
    printf ("Button %s (port:%s, bit:%d)\r\n", buttons->name, PORT_NAME(buttons->gpioPort), buttons->gpioPin);
    buttons++; i++;
  }
}

void boardPrintStringDescription(void)
{

  if (boardDescription != NULL) {
    printf ("*************************************\r\n");
    printf ("Board name = %s\r\n", boardDescription->name);
    printf ("*************************************\r\n");
    printf("Number of leds on the board: %d\n", boardDescription->leds);
    printf("Number of buttons on the board: %d\n", boardDescription->buttons);
    printLeds();
    printButtons();
    if (boardDescription->mems) {
      printf ("MEMS = %s\r\n", boardDescription->mems->name);
    }
    if (boardDescription->temperatureSensor) {
      printf ("Temp sensor = %s, port:%s, pin:%d, div:%d, adcFix:%s\r\n",
            boardDescription->temperatureSensor->name,
            PORT_NAME(boardDescription->temperatureSensor->gpioPort),
            boardDescription->temperatureSensor->gpioPin,
            boardDescription->temperatureSensor->div,
            boardDescription->temperatureSensor->adcFix ? "Yes" : "No"
            );
    }
    
    printf ("EEProm:%s\r\n", (boardDescription->flags & BOARD_HAS_EEPROM) ? "Yes" : "No");
    printf ("PC i/f:%s\r\n", (boardDescription->flags & BOARD_HAS_FTDI) ? "FTDI" : "STM32F");
    printf ("Power Amplifier:%s\r\n", (boardDescription->flags & BOARD_HAS_PA) ? "Yes" : "No");

  }
}
/** @} */

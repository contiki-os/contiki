/**
******************************************************************************
* @file    leds-arch.c
* @author  System LAB
* @version V1.0.0
* @date    17-June-2015
* @brief   Contiki LEDs API binding for the boards in use: Nucleo and SPIRIT1
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "dev/leds.h"
#include "stm32l1xx_nucleo.h"

#ifndef COMPILE_SENSORS
#include "radio_shield_config.h"
extern GPIO_TypeDef*  aLED_GPIO_PORT[];
extern const uint16_t aLED_GPIO_PIN[];
#endif /*COMPILE_SENSORS*/

extern GPIO_TypeDef* GPIO_PORT[];
extern const uint16_t GPIO_PIN[];


/*---------------------------------------------------------------------------*/
void leds_arch_init(void)
{
  /* We have two led, one on the Nucleo (GREEN) ....*/
  BSP_LED_Init(LED2);
  BSP_LED_Off(LED2);

#ifndef COMPILE_SENSORS
  /* ... and one the SPIRIT1 (RED) ....*/
  RadioShieldLedInit(RADIO_SHIELD_LED);
  RadioShieldLedOff(RADIO_SHIELD_LED);
#endif /*COMPILE_SENSORS*/
}

/*---------------------------------------------------------------------------*/
unsigned char leds_arch_get(void)
{
  unsigned char ret = 0 ;
  if (HAL_GPIO_ReadPin(GPIO_PORT[LED2],GPIO_PIN[LED2]))
  {
    ret |= LEDS_GREEN;
  }

#ifndef COMPILE_SENSORS
  if (HAL_GPIO_ReadPin(aLED_GPIO_PORT[RADIO_SHIELD_LED],
                       aLED_GPIO_PIN[RADIO_SHIELD_LED])
     )
  {
    ret |= LEDS_RED;
  }
#endif /*COMPILE_SENSORS*/

  return ret;
}

/*---------------------------------------------------------------------------*/
void leds_arch_set(unsigned char leds)
{
  if (leds & LEDS_GREEN)
  {
    BSP_LED_On(LED2);
  }
  else
  {
    BSP_LED_Off(LED2);
  }

#ifndef COMPILE_SENSORS
  if (leds & LEDS_RED)
  {
    RadioShieldLedOn(RADIO_SHIELD_LED);
  }
  else
  {
    RadioShieldLedOff(RADIO_SHIELD_LED);
  }
#endif /*COMPILE_SENSORS*/
}
/*---------------------------------------------------------------------------*/


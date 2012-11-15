/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32f10x_systick.h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : This file contains all the functions prototypes for the
*                      SysTick firmware library.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32W_SYSTICK_H
#define __STM32W_SYSTICK_H

#include "stm32w108_type.h"
#include "stm32w_conf.h"

#ifndef EXT
  #define EXT extern
#endif /* EXT */

typedef struct
{
  vu32 CTRL;
  vu32 LOAD;
  vu32 VAL;
  vuc32 CALIB;
} SysTick_TypeDef;

/* System Control Space memory map */
#define SCS_BASE              ((u32)0xE000E000)

#define SysTick_BASE          (SCS_BASE + 0x0010)
#define NVIC_BASE             (SCS_BASE + 0x0100)
#define SCB_BASE              (SCS_BASE + 0x0D00)

#ifdef _SysTick
  #define SysTick             ((SysTick_TypeDef *) SysTick_BASE)
#endif /*_SysTick */


/*****************  Bit definition for SysTick_CTRL register  *****************/
#define  SysTick_CTRL_ENABLE                 ((u32)0x00000001)        /* Counter enable */
#define  SysTick_CTRL_TICKINT                ((u32)0x00000002)        /* Counting down to 0 pends the SysTick handler */
#define  SysTick_CTRL_CLKSOURCE              ((u32)0x00000004)        /* Clock source */
#define  SysTick_CTRL_COUNTFLAG              ((u32)0x00010000)        /* Count Flag */


/*****************  Bit definition for SysTick_LOAD register  *****************/
#define  SysTick_LOAD_RELOAD                 ((u32)0x00FFFFFF)        /* Value to load into the SysTick Current Value Register when the counter reaches 0 */


/*****************  Bit definition for SysTick_VAL register  ******************/
#define  SysTick_VAL_CURRENT                 ((u32)0x00FFFFFF)        /* Current value at the time the register is accessed */


/*****************  Bit definition for SysTick_CALIB register  ****************/
#define  SysTick_CALIB_TENMS                 ((u32)0x00FFFFFF)        /* Reload value to use for 10ms timing */
#define  SysTick_CALIB_SKEW                  ((u32)0x40000000)        /* Calibration value is not exactly 10 ms */
#define  SysTick_CALIB_NOREF                 ((u32)0x80000000)        /* The reference clock is not provided */


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* SysTick clock source */
#define SysTick_CLKSource_HCLK_Div8    ((u32)0xFFFFFFFB)
#define SysTick_CLKSource_HCLK         ((u32)0x00000004)

#define IS_SYSTICK_CLK_SOURCE(SOURCE) (((SOURCE) == SysTick_CLKSource_HCLK) || \
                                       ((SOURCE) == SysTick_CLKSource_HCLK_Div8))

/* SysTick counter state */
#define SysTick_Counter_Disable        ((u32)0xFFFFFFFE)
#define SysTick_Counter_Enable         ((u32)0x00000001)
#define SysTick_Counter_Clear          ((u32)0x00000000)

#define IS_SYSTICK_COUNTER(COUNTER) (((COUNTER) == SysTick_Counter_Disable) || \
                                     ((COUNTER) == SysTick_Counter_Enable)  || \
                                     ((COUNTER) == SysTick_Counter_Clear))

/* SysTick Flag */
#define SysTick_FLAG_COUNT             ((u32)0x00000010)
#define SysTick_FLAG_SKEW              ((u32)0x0000001E)
#define SysTick_FLAG_NOREF             ((u32)0x0000001F)

#define IS_SYSTICK_FLAG(FLAG) (((FLAG) == SysTick_FLAG_COUNT) || \
                               ((FLAG) == SysTick_FLAG_SKEW)  || \
                               ((FLAG) == SysTick_FLAG_NOREF))

#define IS_SYSTICK_RELOAD(RELOAD) (((RELOAD) > 0) && ((RELOAD) <= 0xFFFFFF))

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SysTick_CLKSourceConfig(u32 SysTick_CLKSource);
void SysTick_SetReload(u32 Reload);
void SysTick_CounterCmd(u32 SysTick_Counter);
void SysTick_ITConfig(FunctionalState NewState);
u32 SysTick_GetCounter(void);
FlagStatus SysTick_GetFlagStatus(u8 SysTick_FLAG);

#endif /* __STM32F10x_SYSTICK_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

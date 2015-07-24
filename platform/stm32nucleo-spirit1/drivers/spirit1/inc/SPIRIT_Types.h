/**
  ******************************************************************************
 * @file    SPIRIT_Types.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Header file for SPIRIT types.
 * @details
 *
 * This module provide some types definitions which will be used in
 * all the modules of this library. Here is defined also the global
 * variable @ref g_xStatus which contains the status of Spirit and
 * is updated every time an SPI transaction occurs.
 *
  * @attention
 *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIRIT_GENERICTYPES_H
#define __SPIRIT_GENERICTYPES_H


/* Includes ------------------------------------------------------------------*/

/* Include all integer types definitions */
#include <stdint.h>
#include <stdio.h>
#include "SPIRIT_Regs.h"



#ifdef __cplusplus
 extern "C" {
#endif


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @defgroup SPIRIT_Types       Types
 * @brief Module for SPIRIT types definition.
 * * @details See the file <i>@ref SPIRIT_Types.h</i> for more details.
 * @{
 */

/**
 * @defgroup Types_Exported_Types       Types Exported Types
 * @{
 */

/**
 * @brief  Spirit Functional state. Used to enable or disable a specific option.
 */
typedef enum
{
  S_DISABLE = 0,
  S_ENABLE = !S_DISABLE

} SpiritFunctionalState;

#define IS_SPIRIT_FUNCTIONAL_STATE(STATE)   (STATE == S_DISABLE || STATE == S_ENABLE)

/**
 * @brief  Spirit Flag status. Used to control the state of a flag.
 */
typedef enum
{
  S_RESET = 0,
  S_SET = !S_RESET

} SpiritFlagStatus;

#define IS_SPIRIT_FLAG_STATUS(STATUS)   (STATUS == S_RESET || STATUS == S_SET)


/**
 * @brief  boolean type enumeration.
 */
typedef enum
{
  S_FALSE = 0,
  S_TRUE  = !S_FALSE

} SpiritBool;


/**
 * @brief  SPIRIT States enumeration.
 */
typedef enum
{
  MC_STATE_STANDBY           =0x40,	/*!< STANDBY */
  MC_STATE_SLEEP             =0x36,	/*!< SLEEP */
  MC_STATE_READY             =0x03,	/*!< READY */
  MC_STATE_PM_SETUP          =0x3D,	/*!< PM_SETUP */
  MC_STATE_XO_SETTLING       =0x23,	/*!< XO_SETTLING */
  MC_STATE_SYNTH_SETUP       =0x53,	/*!< SYNT_SETUP */
  MC_STATE_PROTOCOL          =0x1F,	/*!< PROTOCOL */
  MC_STATE_SYNTH_CALIBRATION =0x4F,	/*!< SYNTH */
  MC_STATE_LOCK              =0x0F,	/*!< LOCK */
  MC_STATE_RX                =0x33,	/*!< RX */
  MC_STATE_TX                =0x5F	/*!< TX */

} SpiritState;



/**
 * @brief SPIRIT Status. This definition represents the single field of the SPIRIT
 *        status returned on each SPI transaction, equal also to the MC_STATE registers.
 *        This field-oriented structure allows user to address in simple way the single
 *        field of the SPIRIT status.
 *        The user shall define a variable of SpiritStatus type to access on SPIRIT status fields.
 * @note  The fields order in the structure depends on used endianness (little or big
 *        endian). The actual definition is valid ONLY for LITTLE ENDIAN mode. Be sure to
 *        change opportunely the fields order when use a different endianness.
 */

typedef struct
{
  uint8_t XO_ON:1;		/*!< This one bit field notifies if XO is operating
  	  	  	  	     (XO_ON is 1) or not (XO_On is 0) */
  SpiritState MC_STATE: 7;	/*!< This 7 bits field indicates the state of the
   	   	   	   	     Main Controller of SPIRIT. The possible states
   	   	   	   	     and their corresponding values are defined in
   	   	   	   	     @ref SpiritState */
  uint8_t ERROR_LOCK: 1;       /*!< This one bit field notifies if there is an
   	   	   	   	     error on RCO calibration (ERROR_LOCK is 1) or
   	   	   	   	     not (ERROR_LOCK is 0) */
  uint8_t RX_FIFO_EMPTY: 1;    /*!< This one bit field notifies if RX FIFO is empty
   	   	   	   	     (RX_FIFO_EMPTY is 1) or not (RX_FIFO_EMPTY is 0) */
  uint8_t TX_FIFO_FULL: 1;	/*!< This one bit field notifies if TX FIFO is full
  	  	  	  	     (TX_FIFO_FULL is 1) or not (TX_FIFO_FULL is 0) */
  uint8_t ANT_SELECT: 1;       /*!< This one bit field notifies the currently selected
   	   	   	   	     antenna */
  uint8_t : 4;			/*!< This 4 bits field are reserved and equal to 5 */

}SpiritStatus;



/**
 * @}
 */


/**
 * @defgroup Types_Exported_Constants   Types Exported Constants
 * @{
 */


/**
 * @}
 */

/**
 * @defgroup Types_Exported_Variables   Types Exported Variables
 * @{
 */

extern volatile SpiritStatus g_xStatus;

/**
 * @}
 */

/**
 * @defgroup Types_Exported_Macros              Types Exported Macros
 * @{
 */

#ifdef  SPIRIT_USE_FULL_ASSERT
 /**
   * @brief  The s_assert_param macro is used for function's parameters check.
   * @param  expr If expr is false, it calls assert_failed function which reports
   *         the name of the source file and the source line number of the call
   *         that failed. If expr is true, it returns no value.
   * @retval None
   */
  #define s_assert_param(expr) ((expr) ? (void)0 : s_assert_failed((uint8_t *)__FILE__, __LINE__))
  void s_assert_failed(uint8_t* file, uint32_t line);
#elif  SPIRIT_USE_VCOM_ASSERT
  /**
   * @brief  The s_assert_param macro is used for function's parameters check.
   * @param  expr  If expr is false, it calls assert_failed function which reports
   *         the name of the source file and the source line number of the call
   *         that failed. If expr is true, it returns no value.
   * @retval None
   */
  #define s_assert_param(expr) ((expr) ? (void)0 : s_assert_failed((uint8_t *)__FILE__, __LINE__,#expr))
  void s_assert_failed(uint8_t* file, uint32_t line, char* expression);

#elif SPIRIT_USE_FRAME_ASSERT
   /**
   * @brief  The s_assert_param macro is used for function's parameters check.
   * @param  expr  If expr is false, it calls assert_failed function which reports
   *         the name of the source file and the source line number of the call
   *         that failed. If expr is true, it returns no value.
   * @retval None
   */
#define s_assert_param(expr) ((expr) ? (void)0 : s_assert_failed(#expr))
  void s_assert_failed(char* expression);
#else
#define s_assert_param(expr)        {}
#endif

/**
 * @brief  Returns the absolute value.
 */
#define S_ABS(a) ((a)>0?(a):-(a))


/**
 * @}
 */


/**
 * @defgroup Types_Exported_Functions   Types Exported Functions
 * @{
 */

void SpiritRefreshStatus(void);

/**
 *@}
 */

/**
 * @}
 */


/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
  * @file    SPIRIT_Types.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   File for SPIRIT types.
  * @details
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

/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_Types.h"
#include "MCU_Interface.h"


/** @addtogroup SPIRIT_Libraries
 * @{
 */


/** @addtogroup SPIRIT_Types
 * @{
 */


/** @defgroup Types_Private_TypesDefinitions    Types Private Types Definitions
 * @{
 */


/**
 * @}
 */



/** @defgroup Types_Private_Defines             Types Private Defines
 * @{
 */


/**
 * @}
 */



/** @defgroup Types_Private_Macros               Types Private Macros
 * @{
 */


/**
 * @}
 */



/** @defgroup Types_Private_Variables             Types Private Variables
 * @{
 */

/**
 * @brief  Spirit Status global variable.
 *         This global variable of @ref SpiritStatus type is updated on every SPI transaction
 *         to maintain memory of Spirit Status.
 */

volatile SpiritStatus g_xStatus;

/**
 * @}
 */



/** @defgroup Types_Private_FunctionPrototypes       Types Private FunctionPrototypes
 * @{
 */



/**
 * @}
 */



/** @defgroup Types_Private_Functions                 Types Private Functions
 * @{
 */

#ifdef  SPIRIT_USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param file  pointer to the source file name
 * @param line  assert_param error line source number
 * @retval : None
 */
void s_assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number */
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);

  /* Infinite loop */
  while (1)
  {
  }
}
#elif SPIRIT_USE_VCOM_ASSERT

#include "SDK_EVAL_VC_General.h"

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param file pointer to the source file name
 * @param line  assert_param error line source number
 * @param expression: string representing the assert failed expression
 * @retval : None
 */
void s_assert_failed(uint8_t* file, uint32_t line, char* expression)
{

  printf("\n\rVCOM DEBUG: Incorrect parameter. Please reboot.\n\r");
  printf("%s:%d \n\r",file,line);
  printf("The expression %s returned FALSE.\n\r", expression);

  /* Infinite loop */
  while (1)
  {
  }
}

#elif SPIRIT_USE_FRAME_ASSERT

#include "SdkUsbProtocol.h"

/**
 * @brief Sends a notify frame with a payload indicating the name 
 *        of the assert failed.
 * @param expression: string representing the assert failed expression
 * @retval : None
 */
void s_assert_failed(char* expression)
{
  char pcPayload[100];
  uint16_t i;
  
  for(i = 0 ; expression[i]!='(' ; i++);
  expression[i]='\0';
  
  strcpy(pcPayload, &expression[3]);
  
  //sprintf(pcPayload, "The expression %s returned FALSE.\n\r", expression);
  SpiritNotifyAssertFailed(pcPayload);

}

#endif


/**
 * @brief  Updates the gState (the global variable used to maintain memory of Spirit Status)
 *         reading the MC_STATE register of SPIRIT.
 * @param  None
 * @retval None
 */
void SpiritRefreshStatus(void)
{
  uint8_t tempRegValue;

  /* Reads the MC_STATUS register to update the g_xStatus */
  g_xStatus = SpiritSpiReadRegisters(MC_STATE1_BASE, 1, &tempRegValue);

}


/**
 * @}
 */



/**
 * @}
 */



/**
 * @}
 */



/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
 * @file    SPIRIT_Commands.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Management of SPIRIT Commands.
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
#include "SPIRIT_Commands.h"
#include "MCU_Interface.h"




/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_Commands
 * @{
 */


/**
 * @defgroup Commands_Private_TypesDefinitions  Commands Private TypesDefinitions
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Commands_Private_Defines           Commands Private Defines
 * @{
 */

/**
 *@}
 */

/**
 * @defgroup Commands_Private_Macros            Commands Private Macros
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Commands_Private_Variables         Commands Private Variables
 * @{
 */

/**
 *@}
 */



/**
 * @defgroup Commands_Private_FunctionPrototypes        Commands Private Function Prototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Commands_Private_Functions                 Commands Private Functions
 * @{
 */

/**
 * @brief  Sends a specific command to SPIRIT.
 * @param  xCommandCode code of the command to send.
           This parameter can be any value of @ref SpiritCmd.
 * @retval None.
 */
void SpiritCmdStrobeCommand(SpiritCmd xCommandCode)
{
  /* Check the parameters */
  s_assert_param(IS_SPIRIT_CMD(xCommandCode));

  g_xStatus = SpiritSpiCommandStrobes((uint8_t) xCommandCode);
}


/**
 *@}
 */


/**
 *@}
 */


/**
 *@}
 */




/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/

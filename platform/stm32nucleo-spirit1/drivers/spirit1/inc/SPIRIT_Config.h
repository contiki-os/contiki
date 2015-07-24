/**
  ******************************************************************************
 * @file    SPIRIT_Config.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Spirit Configuration and useful defines 
  * 
 * @details
 *
 * This file is used to include all or a part of the Spirit
 * libraries into the application program which will be used.
 * Moreover some important parameters are defined here and the
 * user is allowed to edit them.
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
#ifndef __SPIRIT_CONFIG_H
#define __SPIRIT_CONFIG_H


  /* Includes ------------------------------------------------------------------*/
#include "SPIRIT_Regs.h"
#include "SPIRIT_Aes.h"
#include "SPIRIT_Calibration.h"
#include "SPIRIT_Commands.h"
#include "SPIRIT_Csma.h"
#include "SPIRIT_DirectRF.h"
#include "SPIRIT_General.h"
#include "SPIRIT_Gpio.h"
#include "SPIRIT_Irq.h"
#include "SPIRIT_Timer.h"
#include "SPIRIT_LinearFifo.h"
#include "SPIRIT_PktBasic.h"
#include "SPIRIT_PktMbus.h"
#include "SPIRIT_PktStack.h"

#include "SPIRIT_Qi.h"
#include "SPIRIT_Radio.h"
#include "MCU_Interface.h"
#include "SPIRIT_Types.h"
#include "SPIRIT_Management.h"


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup SPIRIT_Libraries        SPIRIT Libraries
 * @brief This firmware implements libraries which allow the user
 * to manage the features of Spirit without knowing the hardware details.
 * @details The <i>SPIRIT_Libraries</i> modules are totally platform independent. The library provides one
 * module for each device feature. Each module refers to some functions whose prototypes are located in the
 * header file <i>@ref MCU_Interface.h</i>. The user who want to use these libraries on a particular
 * platform has to implement these functions respecting them signatures.
 * @{
 */

/** @defgroup SPIRIT_Configuration      Configuration
 * @brief Spirit Configuration and useful defines.
 * @details See the file <i>@ref SPIRIT_Config.h</i> for more details.
 * @{
 */


/** @defgroup Configuration_Exported_Types      Configuration Exported Types
 * @{
 */

/**
 * @}
 */


/** @defgroup Configuration_Exported_Constants  Configuration Exported Constants
 * @{
 */
#define DOUBLE_XTAL_THR                         30000000

/**
 * @}
 */


/** @defgroup Configuration_Exported_Macros     Configuration Exported Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup Configuration_Exported_Functions  Configuration Exported Functions
 * @{
 */

/**
 * @}
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

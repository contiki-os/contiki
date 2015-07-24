/**
  ******************************************************************************
 * @file    SPIRIT_Aes.h
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT AES Engine.
  *
 * @details
 *
 * In order to encrypt data, the user must manage the AES_END IRQ.
 * The data have to be splitted in blocks of 16 bytes and written
 * into the <i>AES DATA IN registers</i>. Then, after the key is written
 * into the <i>AES KEY registers</i>, a command of <i>Execute encryption</i>
 * has to be sent.
 *
 * <b>Example:</b>
 * @code
 *
 *  SpiritAesWriteDataIn(data_buff , N_BYTES);
 *  SpiritAesExecuteEncryption();
 *
 *  while(!aes_end_flag);       // the flag is set by the ISR routine which manages the AES_END irq
 *  aes_end_flag=RESET;
 *
 *  SpiritAesReadDataOut(enc_data_buff , N_BYTES);
 *
 * @endcode
 *
 * In order to decrypt data, the user must manage the AES_END IRQ and have a decryption key.
 * There are two operative modes to make the data decryption:
 * <ul>
 * <li> Derive the decryption key from the encryption key and decrypt data directly
 * using the <i>SpiritAesDeriveDecKeyExecuteDec()</i> function
 *
 * <b>Example:</b>
 * @code
 *
 *  SpiritAesWriteDataIn(enc_data_buff , N_BYTES);
 *  SpiritAesDeriveDecKeyExecuteDec();
 *
 *  while(!aes_end_flag);       // the flag is set by the ISR routine which manages the AES_END irq
 *  aes_end_flag=RESET;
 *
 *  SpiritAesReadDataOut(data_buff , N_BYTES);
 *
 * @endcode
 * </li>
 *
 * <li> Derive the decryption key from the encryption key using the <i>SpiritAesDeriveDecKeyFromEnc()</i>
 * function, store it into the <i>AES KEY registers</i> and then decrypt data using the
 * <i>SpiritAesExecuteDecryption()</i> function
 *
 * <b>Example:</b>
 * @code
 *
 *  SpiritAesWriteDataIn(key_enc , 16);
 *  SpiritAesDeriveDecKeyFromEnc();
 *
 *  while(!aes_end_flag);       // the flag is set by the ISR routine which manages the AES_END irq
 *  aes_end_flag=RESET;
 *
 *  SpiritAesReadDataOut(key_dec , 16);
 *
 *  SpiritAesWriteKey(key_dec);
 *  SpiritAesWriteDataIn(enc_data_buff , 16);
 *  SpiritAesExecuteDecryption();
 *
 *  while(!aes_end_flag);       // the flag is set by the ISR routine which manages the AES_END irq
 *  aes_end_flag=RESET;
 *
 *  SpiritAesReadDataOut(data_buff , N_BYTES);
 *
 * @endcode
 * </li>
 * </ul>
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
#ifndef __SPIRIT_AES_H
#define __SPIRIT_AES_H


/* Includes ------------------------------------------------------------------*/

#include "SPIRIT_Regs.h"
#include "SPIRIT_Types.h"


#ifdef __cplusplus
 extern "C" {
#endif


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @defgroup SPIRIT_Aes AES
 * @brief Configuration and management of SPIRIT AES Engine.
 * @details See the file <i>@ref SPIRIT_Aes.h</i> for more details.
 * @{
 */

/**
 * @defgroup Aes_Exported_Types AES Exported Types
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Aes_Exported_Constants     AES Exported Constants
 * @{
 */


/**
 * @}
 */


/**
 * @defgroup Aes_Exported_Macros        AES Exported Macros
 * @{
 */


/**
 * @}
 */


/**
 * @defgroup Aes_Exported_Functions     AES Exported Functions
 * @{
 */

void SpiritAesMode(SpiritFunctionalState xNewState);
void SpiritAesWriteDataIn(uint8_t* pcBufferDataIn, uint8_t cDataLength);
void SpiritAesReadDataOut(uint8_t* pcBufferDataOut, uint8_t cDataLength);
void SpiritAesWriteKey(uint8_t* pcKey);
void SpiritAesReadKey(uint8_t* pcKey);
void SpiritAesDeriveDecKeyFromEnc(void);
void SpiritAesExecuteEncryption(void);
void SpiritAesExecuteDecryption(void);
void SpiritAesDeriveDecKeyExecuteDec(void);

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

/**
  ******************************************************************************
 * @file    SPIRIT_Aes.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
 * @brief   Configuration and management of SPIRIT AES Engine.
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
#include "SPIRIT_Aes.h"
#include "MCU_Interface.h"


/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_Aes
 * @{
 */


/**
 * @defgroup Aes_Private_TypesDefinitions       AES Private Types Definitions
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Aes_Private_Defines                AES Private Defines
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Aes_Private_Macros                 AES Private Macros
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Aes_Private_Variables              AES Private Variables
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Aes_Private_FunctionPrototypes     AES Private Function Prototypes
 * @{
 */

/**
 * @}
 */


/**
 * @defgroup Aes_Private_Functions              AES Private Functions
 * @{
 */


/**
 * @brief  Enables or Disables the AES engine.
 * @param  xNewState new state for AES engine.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None
 */
void SpiritAesMode(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue = 0x00;

  /* Check the parameters */
  s_assert_param(IS_SPIRIT_FUNCTIONAL_STATE(xNewState));

  /* Modifies the register value */
  g_xStatus = SpiritSpiReadRegisters(ANA_FUNC_CONF0_BASE, 1, &tempRegValue);
  if(xNewState == S_ENABLE)
  {
    tempRegValue |= AES_MASK;
  }
  else
  {
    tempRegValue &= ~AES_MASK;
  }

  /* Writes the ANA_FUNC_CONF0 register to enable or disable the AES engine */
  g_xStatus = SpiritSpiWriteRegisters(ANA_FUNC_CONF0_BASE, 1, &tempRegValue);

}


/**
 * @brief  Writes the data to encrypt or decrypt, or the encryption key for the 
 *         derive decryption key operation into the AES_DATA_IN registers.
 * @param  pcBufferDataIn pointer to the user data buffer. The first byte of the array
 * 	   shall be the MSB byte and it will be put in the AES_DATA_IN[0] register, while
 * 	   the last one shall be the LSB and it will be put in the AES_DATA_IN[cDataLength-1]
 * 	   register. If data to write are less than 16 bytes the remaining AES_DATA_IN registers
 * 	   will be filled with bytes equal to 0. This parameter is an uint8_t*.
 * @param  cDataLength length of data in bytes.
 *         This parameter is an uint8_t.
 * @retval None
 */
void SpiritAesWriteDataIn(uint8_t* pcBufferDataIn, uint8_t cDataLength)
{
  uint8_t i, dataInArray[16];

  /* Verifies that there are no more than 16 bytes */
  (cDataLength>16) ? (cDataLength=16) : cDataLength;

  /* Fill the dataInArray with the data buffer, using padding */
  for(i=0;i<16;i++)
  {
    (i<(16 - cDataLength)) ? (dataInArray[i]=0):(dataInArray[i]=pcBufferDataIn[15-i]);

  }

  /* Writes the AES_DATA_IN registers */
  g_xStatus = SpiritSpiWriteRegisters(AES_DATA_IN_15_BASE, 16, dataInArray);

}


/**
 * @brief  Returns the encrypted or decrypted data or the decription key from the AES_DATA_OUT register.
 * @param  pcBufferDataOut pointer to the user data buffer. The AES_DATA_OUT[0]
 *         register value will be put as first element of the buffer (MSB), while the
 *         AES_DAT_OUT[cDataLength-1] register value will be put as last element of the buffer (LSB).
 * 	   This parameter is a uint8_t*.
 * @param  cDataLength length of data to read in bytes.
 *         This parameter is a uint8_t.
 * @retval None
 */
void SpiritAesReadDataOut(uint8_t* pcBufferDataOut, uint8_t cDataLength)
{
  uint8_t address, dataOutArray[16];

  /* Verifies that there are no more than 16 bytes */
  (cDataLength>16) ? (cDataLength=16) : cDataLength;

  /* Evaluates the address of AES_DATA_OUT from which start to read */
  address = AES_DATA_OUT_15_BASE+16-cDataLength;

  /* Reads the exact number of AES_DATA_OUT registers */
  g_xStatus = (SpiritSpiReadRegisters(address, cDataLength, dataOutArray));

  /* Copy in the user buffer the read values changing the order */
  for(int i = (cDataLength-1); i>=0; i--)
  {
    *pcBufferDataOut = dataOutArray[i];
    pcBufferDataOut++;
  }

}


/**
 * @brief  Writes the encryption key into the AES_KEY_IN register.
 * @param  pcKey pointer to the buffer of 4 words containing the AES key.
 *         The first byte of the buffer shall be the most significant byte AES_KEY_0 of the AES key.
 *         The last byte of the buffer shall be the less significant byte AES_KEY_15 of the AES key.
 * 	   This parameter is an uint8_t*.
 * @retval None
 */
void SpiritAesWriteKey(uint8_t* pcKey)
{
  uint8_t pcTempKey[16]; 
  for (uint8_t i = 0; i < 16; i++)
  {
    pcTempKey[15-i] = pcKey[i];
  }
  
  /* Writes the AES_DATA_IN registers */
  g_xStatus = SpiritSpiWriteRegisters(AES_KEY_IN_15_BASE, 16, pcTempKey);

}

/**
 * @brief  Returns the encryption/decryption key from the AES_KEY_IN register.
 * @param  pcKey  pointer to the buffer of 4 words (16 bytes) containing the AES key.
 *         The first byte of the buffer shall be the most significant byte AES_KEY_0 of the AES key.
 *         The last byte of the buffer shall be the less significant byte AES_KEY_15 of the AES key.
 *         This parameter is an uint8_t*.
 * @retval None
 */
void SpiritAesReadKey(uint8_t* pcKey)
{
  uint8_t pcTempKey[16];

  /* Reads the AES_DATA_IN registers */
  g_xStatus = SpiritSpiReadRegisters(AES_KEY_IN_15_BASE, 16, pcTempKey);


  for (uint8_t i = 0; i < 16; i++)
    pcKey[i] = pcTempKey[15-i];

}



/**
 * @brief  Derives the decryption key from a given encryption key.
 * @param  None.
 * @retval None.
 */
void SpiritAesDeriveDecKeyFromEnc(void)
{
  /* Sends the COMMAND_AES_KEY command */
  g_xStatus = SpiritSpiCommandStrobes(COMMAND_AES_KEY);

}


/**
 * @brief  Executes the encryption operation.
 * @param  None.
 * @retval None.
 */
void SpiritAesExecuteEncryption(void)
{
  /* Sends the COMMAND_AES_ENC command */
  g_xStatus = SpiritSpiCommandStrobes(COMMAND_AES_ENC);

}


/**
 * @brief  Executes the decryption operation.
 * @param  None.
 * @retval None.
 */
void SpiritAesExecuteDecryption(void)
{
  /* Sends the COMMAND_AES_DEC command */
  g_xStatus = SpiritSpiCommandStrobes(COMMAND_AES_DEC);

}


/**
 * @brief  Executes the key derivation and the decryption operation.
 * @param  None.
 * @retval None.
 */
void SpiritAesDeriveDecKeyExecuteDec(void)
{
  /* Sends the COMMAND_AES_KEY_DEC command */
  g_xStatus = SpiritSpiCommandStrobes(COMMAND_AES_KEY_DEC);

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

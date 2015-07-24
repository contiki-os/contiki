/**
  ******************************************************************************
  * @file    SPIRIT_Timer.c
  * @author  VMA division - AMS
  * @version 3.2.2
  * @date    08-July-2015
  * @brief   Configuration and management of SPIRIT timers.
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
#include "SPIRIT_Timer.h"
#include "SPIRIT_Radio.h"
#include "MCU_Interface.h"




/**
 * @addtogroup SPIRIT_Libraries
 * @{
 */


/**
 * @addtogroup SPIRIT_Timer
 * @{
 */


/**
 * @defgroup Timer_Private_TypesDefinitions             Timer Private Types Definitions
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Timer_Private_Defines                      Timer Private Defines
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Timer_Private_Macros                       Timer Private Macros
 * @{
 */


/**
 *@}
 */


/**
 * @defgroup Timer_Private_Variables                    Timer Private Variables
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Timer_Private_FunctionPrototypes            Timer Private Function Prototypes
 * @{
 */

/**
 *@}
 */


/**
 * @defgroup Timer_Private_Functions                    Timer Private Functions
 * @{
 */

/**
 * @brief  Enables or Disables the LDCR mode.
 * @param  xNewState new state for LDCR mode.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritTimerLdcrMode(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

  /* Mask the read value to enable or disable the LDC mode */
  if(xNewState==S_ENABLE)
  {
    tempRegValue |= PROTOCOL2_LDC_MODE_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL2_LDC_MODE_MASK;
  }

  /* Writes the register to Enable or Disable the LDCR mode */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL2_BASE, 1, &tempRegValue);

}


/**
 * @brief  Enables or Disables the LDCR timer reloading with the value stored in the LDCR_RELOAD registers.
 * @param  xNewState new state for LDCR reloading.
 *         This parameter can be: S_ENABLE or S_DISABLE.
 * @retval None.
 */
void SpiritTimerLdcrAutoReload(SpiritFunctionalState xNewState)
{
  uint8_t tempRegValue;

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  /* Mask te read value to enable or disable the reload on sync mode */
  if(xNewState==S_ENABLE)
  {
    tempRegValue |= PROTOCOL1_LDC_RELOAD_ON_SYNC_MASK;
  }
  else
  {
    tempRegValue &= ~PROTOCOL1_LDC_RELOAD_ON_SYNC_MASK;
  }

  /* Writes the register to Enable or Disable the Auto Reload */
  g_xStatus = SpiritSpiWriteRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

}


/**
 * @brief  Returns the LDCR timer reload bit.
 * @param  None.
 * @retval SpiritFunctionalState: value of the reload bit.
 */
SpiritFunctionalState SpiritTimerLdcrGetAutoReload(void)
{
  uint8_t tempRegValue;

  /* Reads the register value */
  g_xStatus = SpiritSpiReadRegisters(PROTOCOL1_BASE, 1, &tempRegValue);

  return (SpiritFunctionalState)(tempRegValue & 0x80);

}

/**
 * @brief  Sets the RX timeout timer initialization registers with the values of COUNTER and PRESCALER according to the formula: Trx=PRESCALER*COUNTER*Tck.
 *         Remember that it is possible to have infinite RX_Timeout writing 0 in the RX_Timeout_Counter and/or RX_Timeout_Prescaler registers.
 * @param  cCounter value for the timer counter.
 *         This parameter must be an uint8_t.
 * @param  cPrescaler value for the timer prescaler.
 *         This parameter must be an uint8_t.
 * @retval None.
 */
void SpiritTimerSetRxTimeout(uint8_t cCounter , uint8_t cPrescaler)
{
  uint8_t tempRegValue[2]={cPrescaler,cCounter};

  /* Writes the prescaler and counter value for RX timeout in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS5_RX_TIMEOUT_PRESCALER_BASE, 2, tempRegValue);

}


/**
 * @brief  Sets the RX timeout timer counter and prescaler from the desired value in ms. it is possible to fix the RX_Timeout to
 *         a minimum value of 50.417us to a maximum value of about 3.28 s.
 * @param  fDesiredMsec desired timer value.
 *         This parameter must be a float.
 * @retval None
 */

void SpiritTimerSetRxTimeoutMs(float fDesiredMsec)
{
  uint8_t tempRegValue[2];

  /* Computes the counter and prescaler value */
  SpiritTimerComputeRxTimeoutValues(fDesiredMsec , &tempRegValue[1] , &tempRegValue[0]);

  /* Writes the prescaler and counter value for RX timeout in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS5_RX_TIMEOUT_PRESCALER_BASE, 2, tempRegValue);

}


/**
 * @brief  Sets the RX timeout timer counter. If it is equal to 0 the timeout is infinite.
 * @param  cCounter value for the timer counter.
 *         This parameter must be an uint8_t.
 * @retval None.
 */
void SpiritTimerSetRxTimeoutCounter(uint8_t cCounter)
{
  /* Writes the counter value for RX timeout in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS4_RX_TIMEOUT_COUNTER_BASE, 1, &cCounter);

}


/**
 * @brief  Sets the RX timeout timer prescaler. If it is equal to 0 the timeout is infinite.
 * @param  cPrescaler value for the timer prescaler.
 *         This parameter must be an uint8_t.
 * @retval None
 */
void SpiritTimerSetRxTimeoutPrescaler(uint8_t cPrescaler)
{
  /* Writes the prescaler value for RX timeout in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS5_RX_TIMEOUT_PRESCALER_BASE, 1, &cPrescaler);

}


/**
 * @brief  Returns the RX timeout timer.
 * @param  pfTimeoutMsec pointer to the variable in which the timeout expressed in milliseconds has to be stored.
 *         If the returned value is 0, it means that the RX_Timeout is infinite.
 *         This parameter must be a float*.
 * @param  pcCounter pointer to the variable in which the timer counter has to be stored.
 *         This parameter must be an uint8_t*.
 * @param  pcPrescaler pointer to the variable in which the timer prescaler has to be stored.
 *         This parameter must be an uint8_t*.
 * @retval None.
 */
void SpiritTimerGetRxTimeout(float* pfTimeoutMsec, uint8_t* pcCounter , uint8_t* pcPrescaler)
{
  uint8_t tempRegValue[2];

  /* Reads the RX timeout registers value */
  g_xStatus = SpiritSpiReadRegisters(TIMERS5_RX_TIMEOUT_PRESCALER_BASE, 2, tempRegValue);

  /* Returns values */
  (*pcPrescaler) = tempRegValue[0];
  (*pcCounter) = tempRegValue[1];
    
  float nXtalFrequency = (float)SpiritRadioGetXtalFrequency();
  if(nXtalFrequency>DOUBLE_XTAL_THR) {
    nXtalFrequency /= 2.0;
  }
  nXtalFrequency /= 1000.0;
  *pfTimeoutMsec = (float)((tempRegValue[0]+1)*tempRegValue[1]*(1210.0/nXtalFrequency));
  

}


/**
 * @brief  Sets the LDCR wake up timer initialization registers with the values of
 *         COUNTER and PRESCALER according to the formula: Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where
 *         Tck = 28.818 us. The minimum vale of the wakeup timeout is 28.818us (PRESCALER and
 *         COUNTER equals to 0) and the maximum value is about 1.89 s (PRESCALER anc COUNTER equals
 *         to 255).
 * @param  cCounter value for the timer counter.
 *         This parameter must be an uint8_t.
 * @param  cPrescaler value for the timer prescaler.
 *         This parameter must be an uint8_t.
 * @retval None.
 */
void SpiritTimerSetWakeUpTimer(uint8_t cCounter , uint8_t cPrescaler)
{
  uint8_t tempRegValue[2]={cPrescaler,cCounter};

  /* Writes the counter and prescaler value of wake-up timer in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS3_LDC_PRESCALER_BASE, 2, tempRegValue);

}


/**
 * @brief  Sets the LDCR wake up timer counter and prescaler from the desired value in ms,
 *         according to the formula: Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where Tck = 28.818 us.
 *         The minimum vale of the wakeup timeout is 28.818us (PRESCALER and COUNTER equals to 0)
 *         and the maximum value is about 1.89 s (PRESCALER anc COUNTER equals to 255).
 * @param  fDesiredMsec desired timer value.
 *         This parameter must be a float.
 * @retval None.
 */
void SpiritTimerSetWakeUpTimerMs(float fDesiredMsec)
{
  uint8_t tempRegValue[2];

  /* Computes counter and prescaler */
  SpiritTimerComputeWakeUpValues(fDesiredMsec , &tempRegValue[1] , &tempRegValue[0]);

  /* Writes the counter and prescaler value of wake-up timer in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS3_LDC_PRESCALER_BASE, 2, tempRegValue);

}


/**
 * @brief  Sets the LDCR wake up timer counter. Remember that this value is incresead by one in the Twu calculation.
 *         Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where Tck = 28.818 us
 * @param  cCounter value for the timer counter.
 *         This parameter must be an uint8_t.
 * @retval None.
 */
void SpiritTimerSetWakeUpTimerCounter(uint8_t cCounter)
{
  /* Writes the counter value for Wake_Up timer in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS2_LDC_COUNTER_BASE, 1, &cCounter);

}


/**
 * @brief  Sets the LDCR wake up timer prescaler. Remember that this value is incresead by one in the Twu calculation.
 *         Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where Tck = 28.818 us
 * @param  cPrescaler value for the timer prescaler.
 *         This parameter must be an uint8_t.
 * @retval None.
 */
void SpiritTimerSetWakeUpTimerPrescaler(uint8_t cPrescaler)
{
  /* Writes the prescaler value for Wake_Up timer in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS3_LDC_PRESCALER_BASE, 1, &cPrescaler);

}


/**
 * @brief  Returns the LDCR wake up timer, according to the formula: Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where Tck = 28.818 us.
 * @param  pfWakeUpMsec pointer to the variable in which the wake-up time expressed in milliseconds has to be stored.
 *         This parameter must be a float*.
 * @param  pcCounter pointer to the variable in which the timer counter has to be stored.
 *         This parameter must be an uint8_t*.
 * @param  pcPrescaler pointer to the variable in which the timer prescaler has to be stored.
 *         This parameter must be an uint8_t*.
 * @retval None.
 */
void SpiritTimerGetWakeUpTimer(float* pfWakeUpMsec, uint8_t* pcCounter , uint8_t* pcPrescaler)
{
  uint8_t tempRegValue[2];
  uint32_t xtal=SpiritRadioGetXtalFrequency();
  float rco_freq;
  
  rco_freq=(float)SpiritTimerGetRcoFrequency();
  
  /* Reads the Wake_Up timer registers value */
  g_xStatus = SpiritSpiReadRegisters(TIMERS3_LDC_PRESCALER_BASE, 2, tempRegValue);

  /* Returns values */
  (*pcPrescaler)=tempRegValue[0];
  (*pcCounter)=tempRegValue[1];
  *pfWakeUpMsec = (float)((((*pcPrescaler)+1)*((*pcCounter)+1)*(1000.0/rco_freq)));

}


/**
 * @brief  Sets the LDCR wake up timer reloading registers with the values of
 *         COUNTER and PRESCALER according to the formula: Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where
 *         Tck = 28.818 us. The minimum vale of the wakeup timeout is 28.818us (PRESCALER and
 *         COUNTER equals to 0) and the maximum value is about 1.89 s (PRESCALER anc COUNTER equals
 *         to 255).
 * @param  cCounter reload value for the timer counter.
 *         This parameter must be an uint8_t.
 * @param  cPrescaler reload value for the timer prescaler.
 *         This parameter must be an uint8_t.
 * @retval None.
 */
void SpiritTimerSetWakeUpTimerReload(uint8_t cCounter , uint8_t cPrescaler)
{
  uint8_t tempRegValue[2]={cPrescaler,cCounter};

  /* Writes the counter and prescaler value of reload wake-up timer in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS1_LDC_RELOAD_PRESCALER_BASE, 2, tempRegValue);

}


/**
 * @brief  Sets the LDCR wake up reload timer counter and prescaler from the desired value in ms,
 *         according to the formula: Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where Tck = 28.818 us.
 *         The minimum vale of the wakeup timeout is 28.818us (PRESCALER and COUNTER equals to 0)
 *         and the maximum value is about 1.89 s (PRESCALER anc COUNTER equals to 255).
 * @param  fDesiredMsec desired timer value.
 *         This parameter must be a float.
 * @retval None.
 */
void SpiritTimerSetWakeUpTimerReloadMs(float fDesiredMsec)
{
  uint8_t tempRegValue[2];

  /* Computes counter and prescaler */
  SpiritTimerComputeWakeUpValues(fDesiredMsec , &tempRegValue[1] , &tempRegValue[0]);

  /* Writes the counter and prescaler value of reload wake-up timer in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS1_LDC_RELOAD_PRESCALER_BASE, 2, tempRegValue);

}


/**
 * @brief  Sets the LDCR wake up timer reload counter. Remember that this value is incresead by one in the Twu calculation.
 *         Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where Tck = 28.818 us
 * @param  cCounter value for the timer counter.
 *         This parameter must be an uint8_t.
 * @retval None
 */
void SpiritTimerSetWakeUpTimerReloadCounter(uint8_t cCounter)
{
  /* Writes the counter value for reload Wake_Up timer in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS0_LDC_RELOAD_COUNTER_BASE, 1, &cCounter);

}


/**
 * @brief  Sets the LDCR wake up timer reload prescaler. Remember that this value is incresead by one in the Twu calculation.
 *         Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where Tck = 28.818 us
 * @param  cPrescaler value for the timer prescaler.
 *         This parameter must be an uint8_t.
 * @retval None
 */
void SpiritTimerSetWakeUpTimerReloadPrescaler(uint8_t cPrescaler)
{
  /* Writes the prescaler value for reload Wake_Up timer in the corresponding register */
  g_xStatus = SpiritSpiWriteRegisters(TIMERS1_LDC_RELOAD_PRESCALER_BASE, 1, &cPrescaler);

}


/**
 * @brief  Returns the LDCR wake up reload timer, according to the formula: Twu=(PRESCALER +1)*(COUNTER+1)*Tck, where Tck = 28.818 us.
 * @param  pfWakeUpReloadMsec pointer to the variable in which the wake-up reload time expressed in milliseconds has to be stored.
 *         This parameter must be a float*.
 * @param  pcCounter pointer to the variable in which the timer counter has to be stored.
 *         This parameter must be an uint8_t*.
 * @param  pcPrescaler pointer to the variable in which the timer prescaler has to be stored.
 *         This parameter must be an uint8_t*.
 * @retval None.
 */
void SpiritTimerGetWakeUpTimerReload(float* pfWakeUpReloadMsec, uint8_t* pcCounter , uint8_t* pcPrescaler)
{
  uint8_t tempRegValue[2];
  uint32_t xtal=SpiritRadioGetXtalFrequency();
  float rco_freq;
  
  rco_freq=(float)SpiritTimerGetRcoFrequency();
  
  /* Reads the reload Wake_Up timer registers value */
  g_xStatus = SpiritSpiReadRegisters(TIMERS1_LDC_RELOAD_PRESCALER_BASE, 2, tempRegValue);

  /* Returns values */
  (*pcPrescaler)=tempRegValue[0];
  (*pcCounter)=tempRegValue[1];
  *pfWakeUpReloadMsec = (float)((((*pcPrescaler)+1)*((*pcCounter)+1)*(1000.0/rco_freq)));

}

/**
 * @brief  Computes and returns the RCO frequency. 
 *         This frequency depends on the xtal frequency and the XTAL bit in register 0x01.
 * @retval RCO frequency in Hz as an uint16_t.
 */
uint16_t SpiritTimerGetRcoFrequency(void)
{
  uint16_t rco_freq=34700;
  uint32_t xtal=SpiritRadioGetXtalFrequency();
  
  if(xtal>30000000) xtal/=2;
  
  if(xtal==25000000)
  {
    uint8_t xtal_flag;
    SpiritSpiReadRegisters(0x01, 1, &xtal_flag);
    xtal_flag=(xtal_flag&0x40);
    
    if(xtal_flag==0)
    {
      rco_freq=36100;
    }
    else
    {
      rco_freq=33300;
    }
  }
  
  return rco_freq;
}

/**
 * @brief  Computes the values of the wakeup timer counter and prescaler from the user time expressed in millisecond.
 *         The prescaler and the counter values are computed maintaining the prescaler value as
 *         small as possible in order to obtain the best resolution, and in the meantime minimizing the error.
 * @param  fDesiredMsec desired wakeup timeout in millisecs.
 *         This parameter must be a float. Since the counter and prescaler are 8 bit registers the maximum
 *         reachable value is maxTime = fTclk x 256 x 256.
 * @param  pcCounter pointer to the variable in which the value for the wakeup timer counter has to be stored.
 *         This parameter must be a uint8_t*.
 * @param  pcPrescaler pointer to the variable in which the value for the wakeup timer prescaler has to be stored.
 *         This parameter must be an uint8_t*.
 * @retval None
 */
void SpiritTimerComputeWakeUpValues(float fDesiredMsec , uint8_t* pcCounter , uint8_t* pcPrescaler)
{
  float rco_freq,err;
  uint32_t n;
  
  rco_freq=((float)SpiritTimerGetRcoFrequency())/1000;
  
  /* N cycles in the time base of the timer: 
     - clock of the timer is RCO frequency
     - divide times 1000 more because we have an input in ms (variable rco_freq is already this frequency divided by 1000)
  */
  n=(uint32_t)(fDesiredMsec*rco_freq);
    
  /* check if it is possible to reach that target with prescaler and counter of spirit1 */
  if(n/0xFF>0xFD)
  {
    /* if not return the maximum possible value */
    (*pcCounter) = 0xFF;
    (*pcPrescaler) = 0xFF;
    return;
  }
  
  /* prescaler is really 2 as min value */
  (*pcPrescaler)=(n/0xFF)+2;
  (*pcCounter) = n / (*pcPrescaler);
  
  /* check if the error is minimum */
  err=S_ABS((float)(*pcCounter)*(*pcPrescaler)/rco_freq-fDesiredMsec);
  
  if((*pcCounter)<=254)
  {
    if(S_ABS((float)((*pcCounter)+1)*(*pcPrescaler)/rco_freq-fDesiredMsec)<err)
      (*pcCounter)=(*pcCounter)+1;
  }
    
  /* decrement prescaler and counter according to the logic of this timer in spirit1 */
  (*pcPrescaler)--;
  if((*pcCounter)>1)
    (*pcCounter)--;
  else
    (*pcCounter)=1;
}


/**
 * @brief  Computes the values of the rx_timeout timer counter and prescaler from the user time expressed in millisecond.
 *         The prescaler and the counter values are computed maintaining the prescaler value as
 *         small as possible in order to obtain the best resolution, and in the meantime minimizing the error.
 * @param  fDesiredMsec desired rx_timeout in millisecs.
 *         This parameter must be a float. Since the counter and prescaler are 8 bit registers the maximum
 *         reachable value is maxTime = fTclk x 255 x 255.
 * @param  pcCounter pointer to the variable in which the value for the rx_timeout counter has to be stored.
 *         This parameter must be a uint8_t*.
 * @param  pcPrescaler pointer to the variable in which the value for the rx_timeout prescaler has to be stored.
 *         This parameter must be an uint8_t*.
 * @retval None
 */
void SpiritTimerComputeRxTimeoutValues(float fDesiredMsec , uint8_t* pcCounter , uint8_t* pcPrescaler)
{
  uint32_t nXtalFrequency = SpiritRadioGetXtalFrequency();
  uint32_t n;
  float err;
  
  /* if xtal is doubled divide it by 2 */
  if(nXtalFrequency>DOUBLE_XTAL_THR) {
    nXtalFrequency >>= 1;
  }
  
  /* N cycles in the time base of the timer: 
     - clock of the timer is xtal/1210
     - divide times 1000 more because we have an input in ms
  */
  n=(uint32_t)(fDesiredMsec*nXtalFrequency/1210000);
  
  /* check if it is possible to reach that target with prescaler and counter of spirit1 */
  if(n/0xFF>0xFD)
  {
    /* if not return the maximum possible value */
    (*pcCounter) = 0xFF;
    (*pcPrescaler) = 0xFF;
    return;
  }
  
  /* prescaler is really 2 as min value */
  (*pcPrescaler)=(n/0xFF)+2;
  (*pcCounter) = n / (*pcPrescaler);
  
  /* check if the error is minimum */
  err=S_ABS((float)(*pcCounter)*(*pcPrescaler)*1210000/nXtalFrequency-fDesiredMsec);
  
  if((*pcCounter)<=254)
  {
    if(S_ABS((float)((*pcCounter)+1)*(*pcPrescaler)*1210000/nXtalFrequency-fDesiredMsec)<err)
      (*pcCounter)=(*pcCounter)+1;
  }
    
  /* decrement prescaler and counter according to the logic of this timer in spirit1 */
  (*pcPrescaler)--;
  if((*pcCounter)>1)
    (*pcCounter)--;
  else
    (*pcCounter)=1;
}


/**
 * @brief  Sets the RX timeout stop conditions.
 * @param  xStopCondition new stop condition.
 *         This parameter can be any value of @ref RxTimeoutStopCondition.
 * @retval None
 */
void SpiritTimerSetRxTimeoutStopCondition(RxTimeoutStopCondition xStopCondition)
{
  uint8_t tempRegValue[2];

  /* Check the parameters */
  s_assert_param(IS_RX_TIMEOUT_STOP_CONDITION(xStopCondition));

  /* Reads value on the PKT_FLT_OPTIONS and PROTOCOL2 register */
  g_xStatus = SpiritSpiReadRegisters(PCKT_FLT_OPTIONS_BASE, 2, tempRegValue);

  tempRegValue[0] &= 0xBF;
  tempRegValue[0] |= ((xStopCondition & 0x08)  << 3);

  tempRegValue[1] &= 0x1F;
  tempRegValue[1] |= (xStopCondition << 5);

  /* Writes value on the PKT_FLT_OPTIONS and PROTOCOL2 register */
  g_xStatus = SpiritSpiWriteRegisters(PCKT_FLT_OPTIONS_BASE, 2, tempRegValue);

}

/**
 * @brief  Sends the LDC_RELOAD command to SPIRIT. Reload the LDC timer with the value stored in the LDC_PRESCALER / COUNTER registers.
 * @param  None.
 * @retval None
 */
void SpiritTimerReloadStrobe(void)
{
  /* Sends the CMD_LDC_RELOAD command */
  g_xStatus = SpiritSpiCommandStrobes(COMMAND_LDC_RELOAD);

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

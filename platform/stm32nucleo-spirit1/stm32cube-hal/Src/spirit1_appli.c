/**
******************************************************************************
* @file    spirit1_appli.c
* @author  System Lab - NOIDA
* @version V1.1.0
* @date    14-Aug-2014
* @brief   user file to configure Spirit1 transceiver.
*         
@verbatim
===============================================================================
##### How to use this driver #####
===============================================================================
[..]
This file is generated automatically by STM32CubeMX and eventually modified 
by the user

@endverbatim
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
#include "stm32l152xe.h"
#include "spirit1_appli.h"
#include "MCU_Interface.h"
#include "SPIRIT1_Util.h"

#include "lib/sensors.h"
extern const struct sensors_sensor button_sensor;

/** @addtogroup USER
* @{
*/

/** @defgroup SPIRIT1_APPLI
* @brief User file to configure spirit1 tranceiver for desired frequency and 
* @feature.
* @{
*/

/* Private typedef -----------------------------------------------------------*/

/**
* @brief RadioDriver_t structure fitting
*/
RadioDriver_t spirit_cb =
{
  .Init = Spirit1InterfaceInit, 
  .GpioIrq = Spirit1GpioIrqInit,
  .RadioInit = Spirit1RadioInit,
  .SetRadioPower = Spirit1SetPower,
  .PacketConfig = Spirit1PacketConfig,
  .SetPayloadLen = Spirit1SetPayloadlength,
  .SetDestinationAddress = Spirit1SetDestinationAddress,
  .EnableTxIrq = Spirit1EnableTxIrq,
  .EnableRxIrq = Spirit1EnableRxIrq,
  .DisableIrq = Spirit1DisableIrq,
  .SetRxTimeout = Spirit1SetRxTimeout,
  .EnableSQI = Spirit1EnableSQI,
  .SetRssiThreshold = Spirit1SetRssiTH,
  .ClearIrqStatus = Spirit1ClearIRQ,
  .StartRx = Spirit1StartRx,
  .StartTx = Spirit1StartTx,
  .GetRxPacket = Spirit1GetRxPacket
};

/**
* @brief MCULowPowerMode_t structure fitting
*/
MCULowPowerMode_t MCU_LPM_cb =
{
  .McuStopMode = MCU_Enter_StopMode,
  .McuStandbyMode = MCU_Enter_StandbyMode,
  .McuSleepMode = MCU_Enter_SleepMode
}; 

/**
* @brief RadioLowPowerMode_t structure fitting
*/
RadioLowPowerMode_t Radio_LPM_cb =
{
  .RadioShutDown = RadioPowerOFF,
  .RadioStandBy = RadioStandBy,
  .RadioSleep = RadioSleep,
  .RadioPowerON = RadioPowerON
};

/**
* @brief GPIO structure fitting
*/
SGpioInit xGpioIRQ={
  SPIRIT_GPIO_IRQ,
  SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP,
  SPIRIT_GPIO_DIG_OUT_IRQ
};

/**
* @brief Radio structure fitting
*/
SRadioInit xRadioInit = {
  XTAL_OFFSET_PPM,
  BASE_FREQUENCY,
  CHANNEL_SPACE,
  CHANNEL_NUMBER,
  MODULATION_SELECT,
  DATARATE,
  FREQ_DEVIATION,
  BANDWIDTH
};


#if defined(USE_STack_PROTOCOL)
/**
* @brief Packet Basic structure fitting
*/
PktStackInit xStackInit={
  PREAMBLE_LENGTH,
  SYNC_LENGTH,
  SYNC_WORD,
  LENGTH_TYPE,
  LENGTH_WIDTH,
  CRC_MODE,
  CONTROL_LENGTH,
  EN_FEC,
  EN_WHITENING
};

/* LLP structure fitting */
PktStackLlpInit xStackLLPInit ={
  EN_AUTOACK,
  EN_PIGGYBACKING,
  MAX_RETRANSMISSIONS
};

/**
* @brief Address structure fitting
*/
PktStackAddressesInit xAddressInit={
  EN_FILT_MY_ADDRESS,
  MY_ADDRESS,
  EN_FILT_MULTICAST_ADDRESS,
  MULTICAST_ADDRESS,
  EN_FILT_BROADCAST_ADDRESS,
  BROADCAST_ADDRESS
};

#elif defined(USE_BASIC_PROTOCOL)

/**
* @brief Packet Basic structure fitting
*/
PktBasicInit xBasicInit={
  PREAMBLE_LENGTH,
  SYNC_LENGTH,
  SYNC_WORD,
  LENGTH_TYPE,
  LENGTH_WIDTH,
  CRC_MODE,
  CONTROL_LENGTH,
  EN_ADDRESS,
  EN_FEC,
  EN_WHITENING
};


/**
* @brief Address structure fitting
*/
PktBasicAddressesInit xAddressInit={
  EN_FILT_MY_ADDRESS,
  MY_ADDRESS,
  EN_FILT_MULTICAST_ADDRESS,
  MULTICAST_ADDRESS,
  EN_FILT_BROADCAST_ADDRESS,
  BROADCAST_ADDRESS
};
#endif


/* Private define ------------------------------------------------------------*/
#define TIME_UP                                         0x01

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
RadioDriver_t *pRadioDriver;
MCULowPowerMode_t *pMCU_LPM_Comm;
RadioLowPowerMode_t  *pRadio_LPM_Comm;
/*Flags declarations*/
volatile FlagStatus xRxDoneFlag = RESET, xTxDoneFlag=RESET, cmdFlag=RESET;
volatile FlagStatus xStartRx=RESET, rx_timeout=RESET, exitTime=RESET;
volatile FlagStatus datasendFlag=RESET, wakeupFlag=RESET;
volatile FlagStatus PushButtonStatusWakeup=RESET;
volatile FlagStatus PushButtonStatusData=RESET;
/*IRQ status struct declaration*/
SpiritIrqs xIrqStatus;
static __IO uint32_t KEYStatusData = 0x00;
AppliFrame_t xTxFrame, xRxFrame;
uint8_t TxFrameBuff[MAX_BUFFER_LEN] = {0x00};
uint16_t exitCounter = 0;
uint16_t txCounter = 0;
uint16_t wakeupCounter = 0;
uint16_t dataSendCounter = 0x00;

/* Private function prototypes -----------------------------------------------*/

void HAL_Spirit1_Init(void);
void Data_Comm_On(uint8_t *pTxBuff, uint8_t cTxlen, uint8_t* pRxBuff, uint8_t cRxlen);
void Enter_LP_mode(void);
void Exit_LP_mode(void);
void MCU_Enter_StopMode(void);
void MCU_Enter_StandbyMode(void);
void MCU_Enter_SleepMode(void);
void RadioPowerON(void);
void RadioPowerOFF(void);
void RadioStandBy(void);
void RadioSleep(void);
void SPIRIT1_Init(void);
void STackProtocolInit(void);
void BasicProtocolInit(void);
void P2PInterruptHandler(void);
void Set_KeyStatus(FlagStatus val);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_SYSTICK_Callback(void);

/* Private functions ---------------------------------------------------------*/

/** @defgroup SPIRIT1_APPLI_Private_Functions
* @{
*/

/**
* @brief  Initializes RF Transceiver's HAL.
* @param  None
* @retval None.
*/
void HAL_Spirit1_Init(void)
{
  pRadioDriver = &spirit_cb;
  pRadioDriver->Init( ); 
}



/**
* @brief  This function initializes the protocol for point-to-point 
* communication
* @param  None
* @retval None
*/
void SPIRIT1_Init(void)
{
  pRadioDriver = &spirit_cb;
   
     /* Spirit IRQ config */
  pRadioDriver->GpioIrq(&xGpioIRQ);
  
  /* Spirit Radio config */    
  pRadioDriver->RadioInit(&xRadioInit);
  
  /* Spirit Radio set power */
  pRadioDriver->SetRadioPower(POWER_INDEX, POWER_DBM);  
  
  /* Spirit Packet config */  
  pRadioDriver->PacketConfig();
  
  pRadioDriver->EnableSQI();
  
  pRadioDriver->SetRssiThreshold(RSSI_THRESHOLD);
}


/**
* @brief  This function initializes the BASIC Packet handler of spirit1
* @param  None
* @retval None
*/
void BasicProtocolInit(void)
{ 
#if defined(USE_BASIC_PROTOCOL)
  /* Spirit Packet config */
  SpiritPktBasicInit(&xBasicInit);
  SpiritPktBasicAddressesInit(&xAddressInit);
#endif
}

/**
* @brief  This routine will put the radio and mcu in LPM
* @param  None
* @retval None
*/
void Enter_LP_mode(void)
{
  
  pMCU_LPM_Comm = &MCU_LPM_cb;
  pRadio_LPM_Comm = &Radio_LPM_cb;
  
#if defined(MCU_STOP_MODE)&&defined(RF_SHUTDOWN) 
  {
    pRadio_LPM_Comm->RadioShutDown();  
    pMCU_LPM_Comm->McuStopMode();
  }
#elif defined(MCU_STOP_MODE)&&defined(RF_STANDBY) 
  {
    pRadio_LPM_Comm->RadioStandBy();
    pMCU_LPM_Comm->McuStopMode();
  }  
#elif defined(MCU_STOP_MODE)&&defined(RF_SLEEP) 
  {
    pRadio_LPM_Comm->RadioSleep();
    pMCU_LPM_Comm->McuStopMode();
  }   
#elif defined(MCU_STANDBY_MODE)&&defined(RF_SHUTDOWN) 
  {
    pRadio_LPM_Comm->RadioShutDown(); 
    pMCU_LPM_Comm->McuStandbyMode();
  } 
#elif defined(MCU_STANDBY_MODE)&&defined(RF_STANDBY) 
  {
    pRadio_LPM_Comm->RadioStandBy();  
    pMCU_LPM_Comm->McuStandbyMode();
  }
#elif defined(MCU_STANDBY_MODE)&&defined(RF_SLEEP) 
  {
    pRadio_LPM_Comm->RadioSleep();
    pMCU_LPM_Comm->McuStandbyMode();
  }  
#elif defined(MCU_SLEEP_MODE)&&defined(RF_SHUTDOWN) 
  {
    pRadio_LPM_Comm->RadioShutDown(); 
    pMCU_LPM_Comm->McuSleepMode();
  }
#elif defined(MCU_SLEEP_MODE)&&defined(RF_STANDBY) 
  {
    pRadio_LPM_Comm->RadioStandBy(); 
    pMCU_LPM_Comm->McuSleepMode();
  }
#elif defined(MCU_SLEEP_MODE)&&defined(RF_SLEEP) 
  {
    pRadio_LPM_Comm->RadioSleep();
    pMCU_LPM_Comm->McuSleepMode();
  }
#elif defined(MCU_STOP_MODE)
  pMCU_LPM_Comm->McuStopMode();
  
#elif defined(MCU_STANDBY_MODE)
  pMCU_LPM_Comm->McuStandbyMode();
  
#else
  pMCU_LPM_Comm->McuSleepMode();
#endif
}

/**
* @brief  This routine wake-up the mcu and radio from LPM
* @param  None
* @retval None
*/
void Exit_LP_mode(void)
{
  pRadio_LPM_Comm = &Radio_LPM_cb;      
  pRadio_LPM_Comm->RadioPowerON();  
}

/**
* @brief  This routine puts the MCU in stop mode
* @param  None
* @retval None
*/
void MCU_Enter_StopMode(void)
{
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);  /* Infinite loop */
}

/**
* @brief  This routine puts the MCU in standby mode
* @param  None
* @retval None
*/
void MCU_Enter_StandbyMode(void)
{
  HAL_PWR_EnterSTANDBYMode();  /* Infinite loop */
}

/**
* @brief  This routine puts the MCU in sleep mode
* @param  None
* @retval None
*/
void MCU_Enter_SleepMode(void)
{
  /*Suspend Tick increment to prevent wakeup by Systick interrupt. 
  Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base)*/
  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);  /* Infinite loop */
}

/**
* @brief  This function will turn on the radio and waits till it enters the Ready state.
* @param  Param:None. 
* @retval None
*                       
*/
void RadioPowerON(void)
{
  SpiritCmdStrobeReady();   
  do{
    /* Delay for state transition */
    for(volatile uint8_t i=0; i!=0xFF; i++);
    
    /* Reads the MC_STATUS register */
    SpiritRefreshStatus();
  }
  while(g_xStatus.MC_STATE!=MC_STATE_READY);
}


/**
* @brief  This function will Shut Down the radio.
* @param  Param:None. 
* @retval None
*                       
*/
void RadioPowerOFF(void)
{
  SpiritEnterShutdown();
}


/**
* @brief  This function will put the radio in standby state.
* @param  None. 
* @retval None
*                       
*/
void RadioStandBy(void)
{
  SpiritCmdStrobeStandby();  
#if 0  
  do{
    /* Delay for state transition */
    for(volatile uint8_t i=0; i!=0xFF; i++);
    
    /* Reads the MC_STATUS register */
    SpiritRefreshStatus();
  }
  while(g_xStatus.MC_STATE!=MC_STATE_STANDBY);
#endif
}

/**
* @brief  This function will put the radio in sleep state.
* @param  None. 
* @retval None
*                       
*/
void RadioSleep(void)
{
  SpiritCmdStrobeSleep(); 
#if 0
  do{
    /* Delay for state transition */
    for(volatile uint8_t i=0; i!=0xFF; i++);
    
    /* Reads the MC_STATUS register */
    SpiritRefreshStatus();
  }
  while(g_xStatus.MC_STATE!=MC_STATE_SLEEP);
#endif
}

/**
* @brief  This routine updates the respective status for key press.
* @param  None
* @retval None
*/
void Set_KeyStatus(FlagStatus val)
{
  if(val==SET)
  {
    KEYStatusData = 1;
  }
  else
    KEYStatusData = 0;
}


/**
  * @brief  SYSTICK callback.
  * @param  None
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
  if(exitTime)
  {
    /*Decreament the counter to check when 3 seconds has been elapsed*/  
    exitCounter--;
    /*3 seconds has been elapsed*/
    if(exitCounter <= TIME_UP)
    {
        exitTime = RESET;
    }
  }
  
#if defined(RF_STANDBY)
  /*Check if Push Button pressed for wakeup or to send data*/
  if(PushButtonStatusWakeup)
  {
    /*Decreament the counter to check when 5 seconds has been elapsed*/  
    wakeupCounter--;
    
    /*5seconds has been elapsed*/
    if(wakeupCounter<=TIME_UP)
    {
      /*Perform wakeup opeartion*/
      wakeupFlag = SET;
      Exit_LP_mode();
      BSP_LED_Toggle(LED2);
      PushButtonStatusWakeup = RESET;
      PushButtonStatusData = SET;
    }
  }
  else if(PushButtonStatusData)
  {
    dataSendCounter--;
    if(dataSendCounter<=TIME_UP)
    {
      datasendFlag = SET;
      PushButtonStatusWakeup = RESET;
      PushButtonStatusData = RESET;
    }
  }
#endif
}
/**
* @}
*/
/**
  * @brief GPIO EXTI callback
  * @param uint16_t GPIO_Pin
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#if defined(MCU_STOP_MODE)/*if MCU is in stop mode*/        

  /* Clear Wake Up Flag */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
      
  /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
    PLL as system clock source (HSE and PLL are disabled in STOP mode) */
    SystemClockConfig_STOP(); 
#endif
#if defined(MCU_SLEEP_MODE) 
    /* Resume Tick interrupt if disabled prior to sleep mode entry*/
    HAL_ResumeTick();
#endif 
    
    /* Initialize LEDs*/
    RadioShieldLedInit(RADIO_SHIELD_LED);
    //BSP_LED_Init(LED2); 

  if (GPIO_Pin == USER_BUTTON_PIN)
  {
    sensors_changed(&button_sensor);
  }

}
/**
* @}
*/
/**
  * @brief  Configures system clock after wake-up from STOP: enable HSI, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
#if defined(MCU_STOP_MODE)/*if MCU is in stop mode*/        

static void SystemClockConfig_STOP(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable Power Control clock */
  __PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Get the Oscillators configuration according to the internal RCC registers */
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

  /* After wake-up from STOP reconfigure the system clock: Enable HSI and PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
}
#endif 
/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**
* @file    SPIRIT1_Util.c
* @author  High End Analog & RF BU - AMS / ART Team IMS-Systems Lab
* @version V3.0.1
* @date    November 19, 2012
* @brief   Identification functions for SPIRIT DK.
* @details
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*
* <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
*/  


/* Includes ------------------------------------------------------------------*/
#include "SPIRIT1_Util.h"
#include "SPIRIT_Config.h"
#include "spirit1-config.h"
#if defined(P2P_DEMO)
#include "spirit1_appli.h"
#endif
/**
* @addtogroup ST_SPIRIT1
* @{
*/


/**
* @addtogroup SPIRIT1_Util
* @{
*/


/**
* @defgroup SPIRIT1_Util_Private_TypesDefinitions       SPIRIT1_Util Private Types Definitions
* @{
*/

/**
* @}
*/


/**
* @defgroup SPIRIT1_Util_Private_Defines                SPIRIT1_Util Private Defines
* @{
*/

/**
* @}
*/


/**
* @defgroup SPIRIT1_Util_Private_Macros                 SPIRIT1_Util Private Macros
* @{
*/
#define SPIRIT_VERSION          SPIRIT_VERSION_3_0
#define RANGE_TYPE              RANGE_EXT_NONE       /*RANGE_EXT_SKYWORKS*/
/**
* @}
*/


/**
* @defgroup SPIRIT1_Util_Private_Variables              SPIRIT1_Util Private Variables
* @{
*/

/**
* @brief A map that contains the SPIRIT version
*/
const SpiritVersionMap xSpiritVersionMap[] =
{
  /* The Control Board frame handler functions */
  {CUT_2_1v4, SPIRIT_VERSION_2_1},
  {CUT_2_1v3, SPIRIT_VERSION_2_1},
  {CUT_3_0, SPIRIT_VERSION_3_0},
};
static RangeExtType xRangeExtType = RANGE_EXT_NONE;

static uint8_t s_RfModuleBand = 0;
static uint8_t s_eeprom = 0;
/**
* @}
*/


/**
* @defgroup SPIRIT1_Util_Private_FunctionPrototypes     SPIRIT1_Util Private Function Prototypes
* @{
*/

/**
* @}
*/


/**
* @defgroup SPIRIT1_Util_Private_Functions              SPIRIT1_Util Private Functions
* @{
*/

/**
* @brief  Read the status register.
* @param  None
* @retval Status
*/
void Spirit1InterfaceInit(void)
{ 
  /* Initialize the SDN pin micro side */
  RadioGpioInit(RADIO_GPIO_SDN,RADIO_MODE_GPIO_OUT);

  SpiritSpiInit();
  
#if defined(SPIRIT1_HAS_EEPROM)
  EepromSpiInitialization();
#endif
     
  /* Board management */   
  SpiritEnterShutdown(); 
  SpiritExitShutdown();   
    
  SpiritManagementIdentificationRFBoard();
    
  /* Initialize the signals to drive the range extender application board */
  SpiritManagementRangeExtInit(); 
  
  /* Micro EXTI config */   
  RadioGpioInit(RADIO_GPIO_IRQ,RADIO_MODE_EXTI_IN);
  RadioGpioInterruptCmd(RADIO_GPIO_IRQ,0x04,0x04,DISABLE); 
  RadioGpioInterruptCmd(RADIO_GPIO_IRQ,0x04,0x04,ENABLE);
}

#if defined(SPIRIT1_HAS_EEPROM)
/**
* @brief  Read the status register.
* @param  None
* @retval Status
*/
uint8_t EepromIdentification(void)
{
  uint8_t status;
  status = EepromSetSrwd();
  status = EepromStatus();
  if((status&0xF0) == EEPROM_STATUS_SRWD) 
  { 
    /*0xF0 mask [SRWD 0 0 0]*/
    status = 1;
    EepromResetSrwd();
  }
  else
    status = 0;
  
  return status;
}
#endif

#if defined(SPIRIT1_HAS_EEPROM)

/**
* @brief  Identifies the SPIRIT1 Xtal frequency and version.
* @param  None
* @retval Status
*/
void SpiritManagementIdentificationRFBoard(void)
{
    do{
      /* Delay for state transition */
      for(volatile uint8_t i=0; i!=0xFF; i++);
      
      /* Reads the MC_STATUS register */
     SpiritRefreshStatus();
    }while(g_xStatus.MC_STATE!=MC_STATE_READY);

  SdkEvalSetHasEeprom(EepromIdentification());
  
  if(!SdkEvalGetHasEeprom()) /* EEPROM is not present*/
  {    
    SpiritManagementComputeSpiritVersion();
    SpiritManagementComputeXtalFrequency(); 
  }
  else  /* EEPROM found*/
  {
    /*read the memory and set the variable*/
    EepromRead(0x0000, 32, tmpBuffer);
    uint32_t xtal;
    if(tmpBuffer[0]==0 || tmpBuffer[0]==0xFF) {
      SpiritManagementComputeSpiritVersion();
      SpiritManagementComputeXtalFrequency();
      return;
    }
    switch(tmpBuffer[1]) {
    case 0:
      xtal = 24000000;
      SpiritRadioSetXtalFrequency(xtal);
      break;
    case 1:
      xtal = 25000000;
      SpiritRadioSetXtalFrequency(xtal);
      break;
    case 2:
      xtal = 26000000;
      SpiritRadioSetXtalFrequency(xtal);
      break;
    case 3:
      xtal = 48000000;
      SpiritRadioSetXtalFrequency(xtal);
      break;
    case 4:
      xtal = 50000000;
      SpiritRadioSetXtalFrequency(xtal);
      break;
    case 5:
      xtal = 52000000;
      SpiritRadioSetXtalFrequency(xtal);
      break;
    default:
      SpiritManagementComputeXtalFrequency();
      break;
    }
    
    SpiritVersion spiritVersion;
    if(tmpBuffer[2]==0 || tmpBuffer[2]==1) {
      spiritVersion = SPIRIT_VERSION_2_1;
      //SpiritGeneralSetSpiritVersion(spiritVersion); 
    }
    else if(tmpBuffer[2]==2) {
      spiritVersion = SPIRIT_VERSION_3_0;
      //SpiritGeneralSetSpiritVersion(spiritVersion);
    }
    else {
      SpiritManagementComputeSpiritVersion();
    }
    if(tmpBuffer[14]==1) {
      spiritVersion = SPIRIT_VERSION_3_0_D1;
     // SpiritGeneralSetSpiritVersion(spiritVersion);
    }
    
    RangeExtType range;
    if(tmpBuffer[5]==0) {
      range = RANGE_EXT_NONE;
    }
    else if(tmpBuffer[5]==1) {
      range = RANGE_EXT_SKYWORKS_169;
    }
    else if(tmpBuffer[5]==2) {
      range = RANGE_EXT_SKYWORKS_868;
    }
    else {
      range = RANGE_EXT_NONE;
    }
    SpiritManagementSetRangeExtender(range);
    
    SpiritManagementSetBand(tmpBuffer[3]);
    
  }
}
#endif

#if defined(NO_EEPROM)
/**
* @brief  Identifies the SPIRIT1 Xtal frequency and version.
* @param  None
* @retval Status
*/
void SpiritManagementIdentificationRFBoard(void)
{
  do{
    /* Delay for state transition */
    for(volatile uint8_t i=0; i!=0xFF; i++);
    
    /* Reads the MC_STATUS register */
    SpiritRefreshStatus();
  }while(g_xStatus.MC_STATE!=MC_STATE_READY);

    SpiritRadioSetXtalFrequency(XTAL_FREQUENCY);        
    //SpiritGeneralSetSpiritVersion(SPIRIT_VERSION);
}
#endif


/**
* @brief  Sets the SPIRIT frequency band
* @param  uint8_t value: RF FREQUENCY
* @retval None
*/
void SpiritManagementSetBand(uint8_t value)
{
  s_RfModuleBand = value;
}


/**
* @brief  returns the SPIRIT frequency band
* @param  None
* @retval uint8_t value: RF FREQUENCY
*/
uint8_t SpiritManagementGetBand(void)
{
  return s_RfModuleBand;
}

/**
* @defgroup RANGE_EXT_MANAGEMENT_FUNCTIONS              SDK SPIRIT Management Range Extender Functions
* @{
*/
void SpiritManagementRangeExtInit(void)
{
  RangeExtType range_type = SpiritManagementGetRangeExtender();
  
  if(range_type==RANGE_EXT_SKYWORKS_169) {
    /* TCXO optimization power consumption */
    SpiritGeneralSetExtRef(MODE_EXT_XIN);
    uint8_t tmp = 0x01; SpiritSpiWriteRegisters(0xB6,1,&tmp);
    
    /* CSD control */
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_0, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_RX_MODE});
    
    /* CTX/BYP control */
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_1, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_STATE});
    
    /* Vcont control */
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_2, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_RX_STATE});
  }
  else if(range_type==RANGE_EXT_SKYWORKS_868) {   
    /* CSD control */
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_0, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_RX_MODE});
    
    /* CTX/BYP control */
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_1, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_RX_STATE});
    
    /* Vcont control */
    SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_2, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_HP, SPIRIT_GPIO_DIG_OUT_TX_STATE});
  }
}

/**
* @brief  returns the spirit1 range extender type
* @param  None
* @retval RangeExtType
*/
RangeExtType SpiritManagementGetRangeExtender(void)
{
  return xRangeExtType;
}

/**
* @brief  Sets the spirit1 range extender type
* @param  RangeExtType
* @retval None
*/
void SpiritManagementSetRangeExtender(RangeExtType xRangeType)
{
  xRangeExtType = xRangeType;
}

/**
* @brief  this function returns the value to indicate that EEPROM is present or not
* @param  None
* @retval uint8_t: 0 or 1
*/
uint8_t SdkEvalGetHasEeprom(void)
{
  return s_eeprom;
}

/**
* @brief  this function setc the value to indicate that EEPROM is present or not
* @param  None
* @retval uint8_t: 0 or 1
*/
void SdkEvalSetHasEeprom(uint8_t eeprom)
{
  s_eeprom = eeprom;
}

/**
* @brief  this function intializes the spirit1 gpio irq for TX and Rx
* @param  None
* @retval None
*/
void Spirit1GpioIrqInit(SGpioInit *pGpioIRQ)
{
  /* Spirit IRQ config */
  SpiritGpioInit(pGpioIRQ);
}

/**
* @brief  this function used to receive RX packet
* @param  None
* @retval None
*/
void Spirit1RadioInit(SRadioInit *pRadioInit)
{    
  /* Spirit Radio config */
  SpiritRadioInit(pRadioInit);

}

/**
* @brief  this function sets the radio power
* @param  uint8_t cIndex, float fPowerdBm
* @retval None
*/
void Spirit1SetPower(uint8_t cIndex, float fPowerdBm)
{
  /* Spirit Radio set power */
  SpiritRadioSetPALeveldBm(cIndex,fPowerdBm);
  SpiritRadioSetPALevelMaxIndex(cIndex);
}

/**
* @brief  this function sets the packet configuration according to the protocol used
* @param  None
* @retval None
*/
void Spirit1PacketConfig(void)
{
  BasicProtocolInit();
}

/**
* @brief  this function sets the payload length
* @param  uint8_t length
* @retval None
*/
void Spirit1SetPayloadlength(uint8_t length)
{
#if defined(USE_STack_PROTOCOL)
    /* Payload length config */
  SpiritPktStackSetPayloadLength(length);
  
#elif defined(USE_BASIC_PROTOCOL)
  /* payload length config */
  SpiritPktBasicSetPayloadLength(length);
#endif
}

/**
* @brief  this function sets the destination address
* @param  uint8_t adress
* @retval None
*/
void Spirit1SetDestinationAddress(uint8_t address)
{
#if defined(USE_STack_PROTOCOL)
  /* Destination address */
  SpiritPktStackSetDestinationAddress(address);
#elif defined(USE_BASIC_PROTOCOL)
  /* destination address */
  SpiritPktBasicSetDestinationAddress(address);
#endif
}

/**
* @brief  this function enables the Tx IRQ
* @param  None
* @retval None
*/
void Spirit1EnableTxIrq(void)
{
  /* Spirit IRQs enable */
  SpiritIrq(TX_DATA_SENT, S_ENABLE); 
#if defined(USE_STack_PROTOCOL)
  SpiritIrq(MAX_RE_TX_REACH, S_ENABLE);
#elif defined(USE_BASIC_PROTOCOL) //added inadr
  SpiritIrq(VALID_SYNC,S_ENABLE);
  SpiritIrq(TX_FIFO_ERROR, S_ENABLE);
  
#endif  
}

/**
* @brief  this function enables the Rx IRQ
* @param  None
* @retval None
*/
void Spirit1EnableRxIrq(void)
{
    /* Spirit IRQs enable */
  SpiritIrq(RX_DATA_READY, S_ENABLE);
  SpiritIrq(RX_DATA_DISC, S_ENABLE); 
  SpiritIrq(RX_TIMEOUT, S_ENABLE);
#ifdef USE_BASIC_PROTOCOL //added inadr
  SpiritIrq(RX_FIFO_ERROR, S_ENABLE);  
#endif  
  
}

/**
* @brief  this function disable IRQs
* @param  None
* @retval None
*/
void Spirit1DisableIrq(void)
{
  /* Spirit IRQs enable */
  SpiritIrqDeInit(NULL);
}
/**
* @brief  this function set the receive timeout period
* @param  None
* @retval None
*/
void Spirit1SetRxTimeout(float cRxTimeOut)
{
  if(cRxTimeOut == 0)
  {
    /* rx timeout config */
    SET_INFINITE_RX_TIMEOUT();
    SpiritTimerSetRxTimeoutStopCondition(ANY_ABOVE_THRESHOLD);
  }
  else
  {
    /* RX timeout config */
    SpiritTimerSetRxTimeoutMs(cRxTimeOut);
    Spirit1EnableSQI();
    SpiritTimerSetRxTimeoutStopCondition(RSSI_AND_SQI_ABOVE_THRESHOLD);  }
}

/**
* @brief  this function sets the RSSI threshold
* @param  int dbmValue
* @retval None
*/
void Spirit1SetRssiTH(int dbmValue)
{
  SpiritQiSetRssiThresholddBm(dbmValue);
}

/**
* @brief  this function sets the RSSI threshold
* @param  int dbmValue
* @retval None
*/
float Spirit1GetRssiTH(void)
{
  float dbmValue=0;
  dbmValue = SpiritQiGetRssidBm();
  return dbmValue;
}

/**
* @brief  this function enables SQI check
* @param  None
* @retval None
*/
void Spirit1EnableSQI(void)
{
  /* enable SQI check */
  SpiritQiSetSqiThreshold(SQI_TH_0);
  SpiritQiSqiCheck(S_ENABLE);
}

/**
* @brief  this function starts the RX process
* @param  None
* @retval None
*/
void Spirit1StartRx(void)
{
  /* RX command */
  SpiritCmdStrobeRx();
#if 0
    do{
    /* Delay for state transition */
    for(volatile uint8_t i=0; i!=0xFF; i++);
    
    /* Reads the MC_STATUS register */
    SpiritRefreshStatus();
  }
  while(g_xStatus.MC_STATE!=MC_STATE_RX);
#endif
}

/**
* @brief  this function receives the data
* @param  None
* @retval None
*/
void Spirit1GetRxPacket(uint8_t *buffer, uint8_t cRxData )
{
  /* when rx data ready read the number of received bytes */
  cRxData=SpiritLinearFifoReadNumElementsRxFifo();

  /* read the RX FIFO */
  SpiritSpiReadLinearFifo(cRxData, buffer);
  
  SpiritCmdStrobeFlushRxFifo();
}

/**
* @brief  this function starts the TX process
* @param  None
* @retval None
*/
void Spirit1StartTx(uint8_t *buffer, uint8_t size )
{
  /* fit the TX FIFO */
  SpiritCmdStrobeFlushTxFifo();
  
  SpiritSpiWriteLinearFifo(size, buffer);
  
  /* send the TX command */
  SpiritCmdStrobeTx();
#if 0
      do{
    /* Delay for state transition */
    for(volatile uint8_t i=0; i!=0xFF; i++);
    
    /* Reads the MC_STATUS register */
    SpiritRefreshStatus();
  }
  while(g_xStatus.MC_STATE!=MC_STATE_TX);
#endif
#if defined(P2P_DEMO)

  /* wait for TX done */
    while(!xTxDoneFlag);
    xTxDoneFlag = RESET;
#endif
}

/**
* @brief  this function clear the IRQ status
* @param  None
* @retval None
*/
void Spirit1ClearIRQ(void)
{
  SpiritIrqClearStatus();
}
/**
* @}
*/

/**
* @}
*/


/******************* (C) COPYRIGHT 2012 STMicroelectronics *****END OF FILE****/

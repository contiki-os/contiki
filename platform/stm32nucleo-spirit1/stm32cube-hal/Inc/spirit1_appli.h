/**
  ******************************************************************************
  * @file    spirit1_appli.h 
  * @author  System Lab - NOIDA
  * @version V1.1.0
  * @date    14-Aug-2014
  * @brief   Header for spirit1_appli.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIRIT1_APPLI_H
#define __SPIRIT1_APPLI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include "radio_shield_config.h"
#include "MCU_Interface.h" 
#include "SPIRIT_Config.h"
#include "stm32l1xx_nucleo.h"


/* Exported macro ------------------------------------------------------------*/

#if defined(X_NUCLEO_IDS01A3)
	 #define USE_SPIRIT1_433MHz
#elif defined(X_NUCLEO_IDS01A4)
         #define USE_SPIRIT1_868MHz
#elif defined(X_NUCLEO_IDS01A5)
         #define USE_SPIRIT1_915MHz
#else
#error SPIRIT1 Nucleo Shield undefined or unsupported
#endif

/*  Uncomment the Link Layer features to be used */
// #define USE_AUTO_ACK		
// #define USE_AUTO_ACK_PIGGYBACKING	
// #define USE_AUTO_RETRANSMISSION

#if defined(USE_AUTO_ACK)&& defined(USE_AUTO_ACK_PIGGYBACKING)&& defined(USE_AUTO_RETRANSMISSION) 
#define USE_STack_PROTOCOL
    
/* LLP configuration parameters */
#define EN_AUTOACK                      S_ENABLE
#define EN_PIGGYBACKING             	S_ENABLE
#define MAX_RETRANSMISSIONS         	PKT_N_RETX_2

#else
#define USE_BASIC_PROTOCOL
      
#endif

/*  Uncomment the system Operating mode */
//#define USE_LOW_POWER_MODE

#if defined (USE_LOW_POWER_MODE)
#define LPM_ENABLE
#define MCU_STOP_MODE
//#define MCU_SLEEP_MODE
//#define RF_STANDBY
#endif


/* Exported constants --------------------------------------------------------*/

/*  Radio configuration parameters  */
#define XTAL_OFFSET_PPM             0
#define INFINITE_TIMEOUT            0.0

#ifdef USE_SPIRIT1_433MHz
#define BASE_FREQUENCY              433.0e6
#endif

#ifdef USE_SPIRIT1_868MHz
#define BASE_FREQUENCY              868.0e6
#endif

#ifdef USE_SPIRIT1_915MHz
//#define BASE_FREQUENCY              915.0e6
#define BASE_FREQUENCY              902.0e6 
#endif


/*  Addresses configuration parameters  */
#define EN_FILT_MY_ADDRESS          S_DISABLE
#define MY_ADDRESS                  0x34
#define EN_FILT_MULTICAST_ADDRESS   S_DISABLE
#define MULTICAST_ADDRESS           0xEE
#define EN_FILT_BROADCAST_ADDRESS   S_DISABLE
#define BROADCAST_ADDRESS           0xFF
#define DESTINATION_ADDRESS         0x44
#define EN_FILT_SOURCE_ADDRESS      S_DISABLE
#define SOURCE_ADDR_MASK            0xf0
#define SOURCE_ADDR_REF             0x37

#define APPLI_CMD                       0x11
#define NWK_CMD                         0x22
#define LED_TOGGLE                      0xff
#define ACK_OK                          0x01
#define MAX_BUFFER_LEN                  96
#define TIME_TO_EXIT_RX                 3000
#define DELAY_RX_LED_TOGGLE             200   
#define DELAY_TX_LED_GLOW               1000 
#define LPM_WAKEUP_TIME                 100
#define DATA_SEND_TIME                  30

#define PREAMBLE_LENGTH             PKT_PREAMBLE_LENGTH_04BYTES
#define SYNC_LENGTH                 PKT_SYNC_LENGTH_4BYTES
#define CONTROL_LENGTH              PKT_CONTROL_LENGTH_0BYTES
#define EN_ADDRESS                  S_DISABLE
#define EN_FEC                      S_DISABLE
#define CHANNEL_NUMBER              0
#define LENGTH_TYPE                 PKT_LENGTH_VAR
#define POWER_INDEX                 7
#define RECEIVE_TIMEOUT             2000.0 /*change the value for required timeout period*/
#define RSSI_THRESHOLD              -120



#define POWER_DBM                   11.6
#define CHANNEL_SPACE               100e3
#define FREQ_DEVIATION              127e3
#define BANDWIDTH                   540.0e3
#define MODULATION_SELECT           GFSK_BT1
#define DATARATE                    250000
#define XTAL_OFFSET_PPM             0
#define SYNC_WORD                   0x88888888
#define LENGTH_WIDTH                8
#define CRC_MODE                    PKT_CRC_MODE_16BITS_2
#define EN_WHITENING                S_DISABLE
#define INFINITE_TIMEOUT            0.0


/* Exported types ------------------------------------------------------------*/
//extern LPTIM_HandleTypeDef             LptimHandle;
extern volatile FlagStatus xRxDoneFlag, xTxDoneFlag;
extern volatile FlagStatus PushButtonStatusWakeup; 
extern uint16_t wakeupCounter;
extern uint16_t dataSendCounter ;
extern volatile FlagStatus PushButtonStatusData, datasendFlag;

typedef struct sRadioDriver
{
    void ( *Init )( void );
    void ( *GpioIrq )( SGpioInit *pGpioIRQ );
    void ( *RadioInit )( SRadioInit *pRadioInit );
    void ( *SetRadioPower )( uint8_t cIndex, float fPowerdBm );
    void ( *PacketConfig )( void );
    void ( *SetPayloadLen )( uint8_t length);
    void ( *SetDestinationAddress )( uint8_t address);
    void ( *EnableTxIrq )( void );
    void ( *EnableRxIrq )( void );
    void ( *DisableIrq )(void);
    void ( *SetRxTimeout )( float cRxTimeout );
    void ( *EnableSQI )(void);
    void ( *SetRssiThreshold)(int dbmValue);
    void ( *ClearIrqStatus )(void);
    void ( *StartRx )( void );
    void ( *StartTx )( uint8_t *buffer, uint8_t size ); 
    void ( *GetRxPacket )( uint8_t *buffer, uint8_t size );
}RadioDriver_t;   

typedef struct sMCULowPowerMode
{
    void ( *McuStopMode )( void );
    void ( *McuStandbyMode )( void );
    void ( *McuSleepMode )( void );      
}MCULowPowerMode_t;

typedef struct sRadioLowPowerMode
{
    void ( *RadioShutDown )( void );
    void ( *RadioStandBy )( void );
    void ( *RadioSleep ) ( void );
    void ( *RadioPowerON )( void );
}RadioLowPowerMode_t; 

typedef struct
{
  uint8_t Cmdtag;
  uint8_t CmdType;
  uint8_t CmdLen;
  uint8_t Cmd;
  uint8_t DataLen;
  uint8_t* DataBuff;
}AppliFrame_t;


/* Exported functions ------------------------------------------------------- */
void HAL_Spirit1_Init(void);
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
void BasicProtocolInit(void);
void Set_KeyStatus(FlagStatus val);

#endif /* __SPIRIT1_APPLI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

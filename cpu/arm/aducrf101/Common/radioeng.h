/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
@file     radioeng.h
@brief    Radio Interface Engine Functions
@version  v1.0
@author   PAD CSE group, Analog Devices Inc
@date     May 08th 2013
**/

#define RIE_U32 unsigned long
#define RIE_U16 unsigned short int
#define RIE_U8  unsigned char
#define RIE_S8  signed char

/*! \enum RIE_BaseConfigs
 * Variables of this type are used to define the Base Configuration
 */
/*! \var RIE_BaseConfigs DR_1_0kbps_Dev10_0kHz
 * Base configuration of 1 kbps datarate, 10.0 kHz frequency deviation. 
   Use for achieving longer distances.
 */
/*! \var RIE_BaseConfigs DR_38_4kbps_Dev20kHz
 * Base configuration of 38.4 kbps datarate, 20 kHz frequency deviation.
   Use as a compromise of distance and power.
 */
/*! \var RIE_BaseConfigs DR_300_0kbps_Dev75_0kHz
 * Base configuration of 300 kbps datarate, 75 kHz frequency deviation.
   Use for achieving faster transmission times hence lower power.
 */
typedef enum
{
   DR_1_0kbps_Dev10_0kHz      =  0x0,
   DR_38_4kbps_Dev20kHz       =  0x1,
   DR_300_0kbps_Dev75_0kHz    =  0x2,
   UnsupportedDRDev
} RIE_BaseConfigs;

/*! \enum RIE_ModulationTypes
 * Variables of this type are used to define a tx modulation type
 */
/*! \var RIE_ModulationTypes FSK_Modulation
 * FSK Modulation
 */
/*! \var RIE_ModulationTypes GFSK_Modulation
 * GFSK Modulation
 */
typedef enum {FSK_Modulation = 0, GFSK_Modulation = 1} RIE_ModulationTypes;

/*! \enum RIE_PATypes
 * Variables of this type are used to define a PA type
 */
/*! \var RIE_PATypes DifferentialPA
 * Differential PA
 */
/*! \var RIE_PATypes SingleEndedPA
 * Single Ended PA
 */
typedef enum {DifferentialPA = 0, SingleEndedPA = 1} RIE_PATypes;



typedef enum {PowerLevel0 ,PowerLevel1 ,PowerLevel2 ,PowerLevel3,
              PowerLevel4 ,PowerLevel5 ,PowerLevel6 ,PowerLevel7,
              PowerLevel8 ,PowerLevel9 ,PowerLevel10,PowerLevel11,
              PowerLevel12,PowerLevel13,PowerLevel14,PowerLevel15
             } RIE_PAPowerLevel;


/*! \enum RIE_BOOL
 * Variables of this type are used to define a TRUE or FALSE condition
 */
/*! \var RIE_BOOL RIE_TRUE
 * TRUE condition
 */
/*! \var RIE_BOOL RIE_FALSE
 * FALSE condition
 */
typedef enum {RIE_FALSE = 0, RIE_TRUE = !RIE_FALSE} RIE_BOOL;


/*! \enum RIE_Responses
 * Variables of this type are used to define the return value from functions
 */
/*! \var RIE_Responses RIE_Success
 * Successful completion
 */
/*! \var RIE_Responses RIE_RadioSPICommsFail
 * SPI communications with the radio failure.
 */
/*! \var RIE_Responses RIE_UnsupportedRadioConfig
 *  This is an unsupported radio configuration
 */
/*! \var RIE_Responses RIE_Unimplemented
 * This feature has not been implemented
 */
/*! \var RIE_Responses RIE_InvalidParamter
 * An invaild parameter was passed
 */
typedef enum
{
   RIE_Success                 =  0x0,
   RIE_RadioSPICommsFail       =  0x1,
   RIE_UnsupportedRadioConfig  =  0x2,
   RIE_Unimplemented           =  0x3,
   RIE_InvalidParamter         =  0x4,
} RIE_Responses;

// Added in Radio Interface Engine v0.1 
RIE_Responses RadioGetAPIVersion        (RIE_U32 *pVersion);
RIE_Responses RadioInit                 (RIE_BaseConfigs BaseConfig);
RIE_Responses RadioPowerOff             (void);
RIE_Responses RadioTerminateRadioOp     (void);
RIE_Responses RadioSetFrequency         (RIE_U32 Frequency);
RIE_Responses RadioSetModulationType    (RIE_ModulationTypes ModulationType);
RIE_Responses RadioPayldManchesterEncode(RIE_BOOL bEnable);
RIE_Responses RadioPayldDataWhitening   (RIE_BOOL bEnable);
RIE_Responses RadioTxPacketFixedLen     (RIE_U8 Len, RIE_U8 *pData);
RIE_BOOL      RadioTxPacketComplete     (void);
RIE_Responses RadioTxSetPA              (RIE_PATypes PAType,RIE_PAPowerLevel Power);
RIE_Responses RadioTxCarrier            (void);
RIE_Responses RadioTxPreamble           (void);
RIE_Responses RadioRxPacketFixedLen     (RIE_U8 Len);
RIE_BOOL      RadioRxPacketAvailable    (void);
RIE_Responses RadioRxPacketRead         (RIE_U8 BufferLen,RIE_U8 *pPktLen,RIE_U8 *pData,RIE_S8 *pRSSIdBm);
RIE_Responses RadioRxBERTestMode        (void);

// Added in Radio Interface Engine v0.2 
RIE_Responses RadioSwitchConfig         (RIE_BaseConfigs BaseConfig);
RIE_Responses RadioRadioGetRSSI         (RIE_S8 *pRSSIdBm);
RIE_Responses RadioTxSetPower           (RIE_PAPowerLevel Power);

// Added in Radio Interface Engine v0.3 
RIE_Responses RadioTxPacketVariableLen  (RIE_U8 Len, RIE_U8 *pData);
RIE_Responses RadioRxPacketVariableLen  (void);

// Added in Radio Interface Engine v0.5
RIE_Responses RadioDeInit               (void);






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
   @file     radioeng.c
   @brief    Radio Interface Engine Functions
   @version  v1.0
   @author   PAD CSE group, Analog Devices Inc
   @date     May 08th 2013
**/

#include "aducrf101-include.h"

// 1.0 of the Engine
#define RIE_ENGINE_MAJOR_VERSION 1UL
#define RIE_ENGINE_MINOR_VERSION 0UL


#define RADIO_SPI_CLK_FREQ      4000000     // 4 MHz SPI CLK for radio interface
#define SYSTEM_UCLK             16000000    // 16 MHz UCLK
// Default Radio Parameters
#define DEFAULT_CHNL_FREQ       915000000
#define FREQ_CNVRT_VAL          0.00252061538
// Defines for radio memory mapped areas
#define PACKETRAM_START        0x10
#define PACKETRAM_LEN          240
#define BBRAM_START            0x100
#define PR_var_tx_mode_ADR                  0x00D

// PrF Table 35
#define PARAM_TX_NORMAL_PACKET              0
#define PARAM_TX_PREAMBLE_FOREVER           2
#define PARAM_TX_CARRIER_FOREVER            3

#define gpio_configure_sport_mode_0    0xA0
#define gpio_configure_default         0x00
#define MCR_pa_level_mcr_Adr           0x307
#define MCR_rssi_readback_Adr          0x312
#define MCR_gpio_configure_Adr         0x3fa
#define MCR_ext_uc_clk_divide_Adr      0x32e
#define MCR_interrupt_source_0_Adr     0x336
#define MCR_interrupt_source_1_Adr     0x337

// Macros for manual GPIO checking of Radio MISO pin P2.0 (SPI0)
#define RADIO_MISO_IN        GP2IN_IN0_BBA
// Macros for manual GPIO control of P2.3 (Radio SPI CS) (SPI0)
#define RADIO_CSN_DEASSERT   (pADI_GP2->GPSET = GP2SET_SET3)
#define RADIO_CSN_ASSERT     (pADI_GP2->GPCLR = GP2CLR_CLR3)
// Macros for Sending\Receiving single bytes via SPI
#define SEND_SPI(x)   pADI_SPI0->SPITX = x
#define WAIT_SPI_RX   while((pADI_SPI0->SPISTA & SPISTA_RXFSTA_MSK) == 0x0);
#define READ_SPI      pADI_SPI0->SPIRX

// Bit Manipulation Macros
#define MSKSET_VAL(byte,numbits,offset,value) ((byte & ~(((0x1 << numbits)-1) << offset)) | value)



/*************************************************************************/
/*                Local Types                                 */
/*************************************************************************/
/*************************************************************************/
/*                     Radio Command Codes                               */
/*************************************************************************/
typedef enum
{
   CMD_SYNC        = 0xA2, // Synchronizatio
   CMD_PHY_OFF     = 0xB0, // Transition to state PHY_OFF
   CMD_PHY_ON      = 0xB1, // transition to state PHY_ON
   CMD_PHY_RX      = 0xB2, // transition to state PHY_RX
   CMD_PHY_TX      = 0xB5, // transition to state PHY_TX
   CMD_PHY_SLEEP   = 0xBA, // transition to state PHY_SLEEP
   CMD_CONFIG_DEV  = 0xBB, // Apply Radio Configuration
   CMD_GET_RSSI    = 0xBC, // Performs an RSSI measurement
   CMD_HW_RESET    = 0xC8, // Power Down radio
   SPI_MEM_WR      = 0x18, // Sequential SPI Write
   SPI_MEM_RD      = 0x38, // Sequential SPI Read
   SPI_NOP         = 0xFF  // No operation
} Radio_CmdCodes;
/*************************************************************************/
/*                        Firmware States                                */
/*************************************************************************/
typedef enum
{
    FW_INIT        = 0x0F, // Radio Starting Up
    FW_BUSY        = 0x00, // Radio not completed current operation
    FW_RSSI        = 0x05, // Performing CMD_GET_RSSI
    FW_OFF         = 0x11, // Radio is OFF
    FW_ON          = 0x12, // Radio is ON
    FW_RX          = 0x13, // Radio is in receive mode
    FW_TX          = 0x14, // Radio is in transmit mode

} RadioState;
/*************************************************************************/
/*                        Status Byte Masks                              */
/*************************************************************************/
#define STATUS_BYTE_FW_STATE    (0x1F << 0)
#define STATUS_BYTE_CMD_READY   (0x1  << 5)
#define STATUS_BYTE_IRQ_STATUS  (0x1  << 6)
#define STATUS_BYTE_SPI_READY   (0x1  << 7)
/*************************************************************************/
/*                        SPI Memory Access Defs                         */
/*************************************************************************/
#define SPI_MEMCMD_BYTE0_ADR_MSK       (0x3  << 0)
#define SPI_MEMCMD_BYTE0_CMD_BITOFFSET 3
#define SPI_MEMCMD_BYTE0_CMD_MSK       (0x1F << SPI_MEMCMD_BYTE0_CMD_BITOFFSET)
/*************************************************************************/
/*                        Radio Configuration Structure                  */
/*************************************************************************/
/**
    \internal Hide from Doxegen
    \var TyRadioConfiguration
 **/
typedef struct
{
    RIE_U8 interrupt_mask_0_r;           // 0x100
    RIE_U8 cfg_101_r;                    // 0x101
    RIE_U8 cfg_102_r;                    // 0x102
    RIE_U8 cfg_103_r;                    // 0x103
    RIE_U8 cfg_104_r;                    // 0x104
    RIE_U8 cfg_105_r;                    // 0x105
    RIE_U8 cfg_106_r;                    // 0x106
    RIE_U8 cfg_107_r;                    // 0x107
    RIE_U8 cfg_108_r;                    // 0x108
    RIE_U8 channel_freq_0_r;             // 0x109
    RIE_U8 channel_freq_1_r;             // 0x10A
    RIE_U8 channel_freq_2_r;             // 0x10B
    RIE_U8 cfg_10C_r;                    // 0x10C
    RIE_U8 cfg_10D_r;                    // 0x10D
    RIE_U8 cfg_10E_r;                    // 0x10E
    RIE_U8 cfg_10F_r;                    // 0x10F
    RIE_U8 cfg_110_r;                    // 0x110
    RIE_U8 cfg_111_r;                    // 0x111
    RIE_U8 cfg_112_r;                    // 0x112
    RIE_U8 cfg_113_r;                    // 0x113
    RIE_U8 radio_cfg_8_r;                // 0x114
    RIE_U8 radio_cfg_9_r;                // 0x115
    RIE_U8 cfg_116_r;                    // 0x116
    RIE_U8 cfg_117_r;                    // 0x117
    RIE_U8 image_reject_cal_phase_r;     // 0x118
    RIE_U8 image_reject_cal_amplitude_r; // 0x119
    RIE_U8 cfg_11A_r;                    // 0x11A
    RIE_U8 cfg_11B_r;                    // 0x11B
    RIE_U8 symbol_mode_r;                // 0x11C
    RIE_U8 cfg_11D_r;                    // 0x11D
    RIE_U8 cfg_11E_r;                    // 0x11E
    RIE_U8 cfg_11F_r;                    // 0x11F
    RIE_U8 cfg_120_r;                    // 0x120
    RIE_U8 cfg_121_r;                    // 0x121
    RIE_U8 cfg_122_r;                    // 0x122
    RIE_U8 cfg_123_r;                    // 0x123
    RIE_U8 tx_base_adr_r;                // 0x124
    RIE_U8 rx_base_adr_r;                // 0x125
    RIE_U8 packet_length_control_r;      // 0x126
    RIE_U8 packet_length_max_r;          // 0x127
    RIE_U8 cfg_128_r;                    // 0x128
    RIE_U8 cfg_129_r;                    // 0x129
    RIE_U8 cfg_12A_r;                    // 0x12A
    RIE_U8 cfg_12B_r;                    // 0x12B
    RIE_U8 cfg_12C_r;                    // 0x12C
    RIE_U8 cfg_12D_r;                    // 0x12D
    RIE_U8 cfg_12E_r;                    // 0x12E
    RIE_U8 cfg_12F_r;                    // 0x12F
    RIE_U8 cfg_130_r;                    // 0x130
    RIE_U8 cfg_131_r;                    // 0x131
    RIE_U8 cfg_132_r;                    // 0x132
    RIE_U8 cfg_133_r;                    // 0x133
    RIE_U8 cfg_134_r;                    // 0x134
    RIE_U8 cfg_135_r;                    // 0x135
    RIE_U8 cfg_136_r;                    // 0x136
    RIE_U8 cfg_137_r;                    // 0x137
    RIE_U8 cfg_138_r;                    // 0x138
    RIE_U8 cfg_139_r;                    // 0x139
    RIE_U8 cfg_13A_r;                    // 0x13A
    RIE_U8 cfg_13B_r;                    // 0x13B
    RIE_U8 cfg_13C_r;                    // 0x13C
    RIE_U8 cfg_13D_r;                    // 0x13D
    RIE_U8 cfg_13E_r;                    // 0x13E
    RIE_U8 cfg_13F_r;                    // 0x13F
} TyRadioConfiguration;
/*************************************************************************/
/*                        Radio Configuration Constants                  */
/*************************************************************************/
#define interrupt_mask_0_interrupt_tx_eof                (0x1 << 4)
#define interrupt_mask_0_interrupt_crc_correct           (0x1 << 2)

#define packet_length_control_length_offset_offset       (0)
#define packet_length_control_length_offset_minus0       (0x4 << packet_length_control_length_offset_offset)
#define packet_length_control_data_mode_offset           (3)
#define packet_length_control_data_mode_packet           (0x0 << packet_length_control_data_mode_offset)
#define packet_length_control_crc_en_yes                 (0x1 << 5)
#define packet_length_control_packet_len_variable        (0x0 << 6)
#define packet_length_control_packet_len_fixed           (0x1 << 6)
#define packet_length_control_data_byte_lsb              (0x0 << 7)

#define symbol_mode_symbol_length_8_bit                  (0 << 0)
#define symbol_mode_data_whitening_disabled              (0 << 3)
#define symbol_mode_data_whitening_enabled               (1 << 3)
#define symbol_mode_eight_ten_enc_disabled               (0 << 4 )
#define symbol_mode_prog_crc_en_disabled                 (0 << 5)
#define symbol_mode_manchester_enc_enabled               (1 << 6)

#define radio_cfg_8_pa_single_diff_sel_single_ended      (0x0  << 7)
#define radio_cfg_8_pa_single_diff_sel_differential      (0x1  << 7)
#define radio_cfg_8_pa_power_numbits                     (4)
#define radio_cfg_8_pa_power_offset                      (3)
#define radio_cfg_8_pa_power_setting_63                  (0xF << radio_cfg_8_pa_power_offset)
#define radio_cfg_8_pa_ramp_numbits                      (3)
#define radio_cfg_8_pa_ramp_offset                       (0)
#define radio_cfg_8_pa_ramp_16                           (0x5 << radio_cfg_8_pa_ramp_offset)

#define radio_cfg_9_demod_scheme_offset                  (0)
#define radio_cfg_9_demod_scheme_FSK                     (0x0  << radio_cfg_9_demod_scheme_offset)
#define radio_cfg_9_mod_scheme_numbits                   (3)
#define radio_cfg_9_mod_scheme_offset                    (3)
#define radio_cfg_9_mod_scheme_2_level_FSK               (0x0  << radio_cfg_9_mod_scheme_offset)
#define radio_cfg_9_mod_scheme_2_level_GFSK              (0x1  << radio_cfg_9_mod_scheme_offset)
#define radio_cfg_9_ifbw_numbits                         (2)
#define radio_cfg_9_ifbw_offset                          (6)
#define radio_cfg_9_ifbw_100kHz                          (0x0  << radio_cfg_9_ifbw_offset)
#define radio_cfg_9_ifbw_150kHz                          (0x1  << radio_cfg_9_ifbw_offset)
#define radio_cfg_9_ifbw_200kHz                          (0x2  << radio_cfg_9_ifbw_offset)
#define radio_cfg_9_ifbw_300kHz                          (0x3  << radio_cfg_9_ifbw_offset)

/*************************************************************************/
/*                Local Variables                                        */
/*************************************************************************/
static TyRadioConfiguration RadioConfiguration;
static RIE_BOOL             bRadioConfigurationChanged    = RIE_FALSE;
static RIE_BOOL             bTestModeEnabled              = RIE_FALSE;
static RIE_U32              DataRate                      = 38400;
static volatile RIE_BOOL    bPacketTx                     = RIE_FALSE;
static volatile RIE_BOOL    bPacketRx                     = RIE_FALSE;

const RIE_U8 DR_38_4kbps_Dev20kHz_Configuration[] =
{
   0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x00,0x76,0x62,0x21,

// 0    1    2    3    4    5    6    7    8    9    A    B
   0x80,0x01,0xC8,0x20,0x0E,0x00,0x00,0x00,0xFD,0x00,0x0B,0x37,
   0x16,0x07,

   0x40,0x0C,0x00,0x0C,0x00,0x00,
   0x10,0x00,0xC3,0x36,0x10,0x10,0x24,0xF0,0x2A,0x00,0x2F,0x19,0x5E,0x46,0x5F,0x78,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


const RIE_U8 DR_300_0kbps_Dev75_0kHz_Configuration[] =
{
   0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x00,0x76,0x62,0x21,

// 0    1    2    3    4    5    6    7    8    9    A    B
   0xB8,0x2B,0xEE,0x0B,0x70,0x00,0x03,0x00,0xFD,0xC0,0x0B,0x37,
   0x16,0x07,

   0x40,0x0C,0x00,0x0C,0x00,0x00,
   0x10,0x00,0xC3,0x36,0x10,0x10,0x24,0xF0,0x2A,0x00,0x2F,0x19,0x5E,0x46,0x5F,0x78,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};



const RIE_U8 DR_1_0kbps_Dev10_0kHz_Configuration[] =
{
   0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x00,0x76,0x62,0x21,
//   0    1    2    3    4    5    6    7    8    9    A    B
   0x0A,0x00,0x64,0x41,0x01,0x00,0x02,0x00,0xFD,0x00,0x0B,0x37,
   0x16,0x07,

   0x40,0x0C,0x00,0x0C,0x00,0x00,
   0x10,0x00,0xC3,0x36,0x10,0x10,0x24,0xF0,0x2A,0x00,0x2F,0x19,0x5E,0x46,0x5F,0x78,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

/*************************************************************************/
/*                Local Functions                                        */
/*************************************************************************/
static RIE_Responses RadioSPIXferByte           (RIE_U8           ucByte,
                                                 RIE_U8 *         pData);
static RIE_Responses RadioSendCommandBytes      (RIE_U8 *         pCmdBytes,
                                                 RIE_U8           NumBytes);
static RIE_Responses RadioSendCommandNoWait     (Radio_CmdCodes   CmdCode);
static RIE_Responses RadioSendCommandWait       (Radio_CmdCodes   CmdCode);
static RIE_Responses RadioMMapRead              (RIE_U32          ulAdr,
                                                 RIE_U32          ulLen,
                                                 RIE_U8 *         pData);
static RIE_Responses RadioMMapWrite             (RIE_U32          ulAdr,
                                                 RIE_U32          ulLen,
                                                 RIE_U8 *         pData);
static RIE_Responses RadioReadState             (RadioState *     pState);
static RIE_Responses RadioWaitOnState           (RadioState       FinalState);
static RIE_Responses RadioWaitForPowerUp        (void);
static RIE_Responses RadioSyncComms             (void);
static RIE_Responses SetRadioConfiguration      (RIE_BaseConfigs BaseConfig);
static RIE_Responses RadioCommitRadioConfig     (void);
static RIE_Responses RadioConfigure             (void);
static RIE_Responses RadioToOnMode              (void);
static RIE_Responses RadioToOffMode             (void);
static RIE_Responses RadioWaitOnCmdLdr          (void);
/*************************************************************************/
/*                Functions Implementations - Start                      */
/*************************************************************************/

/**
   @fn RIE_Responses RadioGetAPIVersion(RIE_U32 *pVersion)
   @brief Return the Radio Interface Engine API Version
   @param pVersion :{}
      pVersion Storage for Radio Interface Engine API version.
   @code
      RIE_U32 Version;
      Response = RadioGetAPIVersion(&Version);
   @endcode
   @return RIE_Responses  Error code.
**/

RIE_Responses RadioGetAPIVersion(RIE_U32 *pVersion)
{
   RIE_Responses  Response = RIE_Success;

   if (pVersion)
      *pVersion = RIE_ENGINE_MINOR_VERSION | (RIE_ENGINE_MAJOR_VERSION << 8);
   return Response;
}


/**
   @fn RIE_U32 RadioSwitchConfig(RIE_BaseConfigs BaseConfig)
   @brief Change the Radio to using specified configuration.
   @param BaseConfig :{DR_1_0kbps_Dev10_0kHz, DR_38_4kbps_Dev20kHz, DR_300_0kbps_Dev75_0kHz}
      - DR_1_0kbps_Dev10_0kHz  Base configuration of 1 kbps datarate, 10.0 kHz frequency deviation.
      - DR_38_4kbps_Dev20kHz  Base configuration of 38.4 kbps datarate, 20 kHz frequency deviation.
      - DR_300_0kbps_Dev75_0kHz  Base configuration of 300 kbps datarate, 75 kHz frequency deviation.
   @pre
      RadioInit() must be called before this function is called.
   @return RIE_Responses  Error code.
**/

RIE_Responses RadioSwitchConfig(RIE_BaseConfigs BaseConfig)
{
   RIE_Responses  Response = RIE_Success;
   if(Response == RIE_Success)
      Response = RadioToOffMode();
   if(Response == RIE_Success)
      Response = SetRadioConfiguration(BaseConfig);
   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   if(Response == RIE_Success)
      Response = RadioToOnMode();
   return Response;
}


/**
   @fn RIE_U32 RadioInit(RIE_BaseConfigs BaseConfig)
   @brief Initialise the Radio, using specified configuration.
   @param BaseConfig :{DR_1_0kbps_Dev10_0kHz , DR_38_4kbps_Dev20kHz ,DR_300_0kbps_Dev75_0kHz }
      - DR_1_0kbps_Dev10_0kHz  Base configuration of 1 kbps datarate, 10.0 kHz frequency deviation.
      - DR_38_4kbps_Dev20kHz  Base configuration of 38.4 kbps datarate, 20 kHz frequency deviation.
      - DR_300_0kbps_Dev75_0kHz  Base configuration of 300 kbps datarate, 75 kHz frequency deviation.
   @note
      This must be called before any other function is called.
   @return RIE_Responses  Error code.
**/

RIE_Responses RadioInit(RIE_BaseConfigs BaseConfig)
{
   RIE_Responses  Response = RIE_Success;

   // Disable the radio interrupt until we have initialised the radio
   NVIC_DisableIRQ(UHFTRX_IRQn);

   // Initialise GPIO Port 2 for Radio Use
   pADI_GP2->GPCON   =  GP2CON_CON0_SPI0MISO  | GP2CON_CON1_SPI0SCLK |
                        GP2CON_CON2_SPI0MOSI  | GP2CON_CON3_GPIO     |
                        GP2CON_CON4_IRQ8      | GP2CON_CON5_GPIO     |
                        GP2CON_CON6_GPIO      | GP2CON_CON7_GPIOIRQ7;

   pADI_GP2->GPOEN   =  GP2OEN_OEN0_IN        |  GP2OEN_OEN1_IN      |
                        GP2OEN_OEN2_IN        |  GP2OEN_OEN3_OUT     |
                        GP2OEN_OEN4_IN        |  GP2OEN_OEN5_IN      |
                        GP2OEN_OEN6_IN        |  GP2OEN_OEN7_IN;

   // Disable the PULL-Up on P2.4 which is connected to the radio
   GP2PUL_PUL4_BBA = 0x0;

   // Configure the SPI Interface to the Radio and flush it
   pADI_SPI0->SPIDIV =  ((SYSTEM_UCLK/RADIO_SPI_CLK_FREQ)/2)-0x1;
   pADI_SPI0->SPICON =  SPICON_MASEN   |    // Master mode
                        SPICON_TIM     |    // Interrupt on transmit
                        SPICON_TFLUSH  |    // Flush FIFO
                        SPICON_RFLUSH  |    // Flush FIFO
                        SPICON_ENABLE;
   pADI_SPI0->SPICON =  SPICON_MASEN   |    // Master mode
                        SPICON_TIM     |    // Interrupt on transmit
                        SPICON_ENABLE;

   // Initialise the chip select line to starting position
   RADIO_CSN_DEASSERT;


   // Power it down and up again to return to a known state
   // which will be PHY_OFF.
   // This will clear any pre-existing radio interrupt before
   // we enable the Cortex interrupt handling of it
   if(Response == RIE_Success)
      Response = RadioPowerOff();
   // Configure a "high level" radio interrupt ...
   pADI_INTERRUPT->EI2CFG = EI2CFG_IRQ8MDE_HIGHLEVEL | EI2CFG_IRQ8EN;
   // ... and set it up in the NVIC so that our interrupt handler is called
   // when the radio wants our attention. Clear any pre-existing condition
   // before enabling the interrupt.
   pADI_INTERRUPT->EICLR  = EICLR_IRQ8;
   NVIC_ClearPendingIRQ(UHFTRX_IRQn);
   NVIC_SetPriority    (UHFTRX_IRQn,0x0);
   NVIC_EnableIRQ      (UHFTRX_IRQn);

   if(Response == RIE_Success)
      Response = RadioWaitForPowerUp();
   if(Response == RIE_Success)
      Response = RadioSyncComms();
   if(Response == RIE_Success)
      Response = RadioToOffMode();
   if(Response == RIE_Success)
      Response = SetRadioConfiguration(BaseConfig);
   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   if(Response == RIE_Success)
      Response = RadioToOnMode();
   return Response;
}

/**
   @fn RIE_U32 RadioDeInit(void)
   @brief Deinitialise the Radio, and power it down.
   @note
      This can be called independently of all other functions to power down
      the radio
   @return RIE_Responses  Error code.
**/

RIE_Responses RadioDeInit(void)
{
   RIE_Responses  Response = RIE_Success;

   // Disable the radio interrupt
   NVIC_DisableIRQ(UHFTRX_IRQn);

   // Initialise GPIO Port 2 for Radio Use
   pADI_GP2->GPCON   =  GP2CON_CON0_SPI0MISO  | GP2CON_CON1_SPI0SCLK |
                        GP2CON_CON2_SPI0MOSI  | GP2CON_CON3_GPIO     |
                        GP2CON_CON4_IRQ8      | GP2CON_CON5_GPIO     |
                        GP2CON_CON6_GPIO      | GP2CON_CON7_GPIOIRQ7;

   pADI_GP2->GPOEN   =  GP2OEN_OEN0_IN        |  GP2OEN_OEN1_IN      |
                        GP2OEN_OEN2_IN        |  GP2OEN_OEN3_OUT     |
                        GP2OEN_OEN4_IN        |  GP2OEN_OEN5_IN      |
                        GP2OEN_OEN6_IN        |  GP2OEN_OEN7_IN;

   // Enable the pull-up as we are powering down the radion
   GP2PUL_PUL4_BBA = 0x1;

   // Configure the SPI Interface to the Radio and flush it
   pADI_SPI0->SPIDIV =  ((SYSTEM_UCLK/RADIO_SPI_CLK_FREQ)/2)-0x1;
   pADI_SPI0->SPICON =  SPICON_MASEN   |    // Master mode
                        SPICON_TIM     |    // Interrupt on transmit
                        SPICON_TFLUSH  |    // Flush FIFO
                        SPICON_RFLUSH  |    // Flush FIFO
                        SPICON_ENABLE;
   pADI_SPI0->SPICON =  SPICON_MASEN   |    // Master mode
                        SPICON_TIM     |    // Interrupt on transmit
                        SPICON_ENABLE;

   // Initialise the chip select line to starting position
   RADIO_CSN_DEASSERT;

   // Power it down
   Response = RadioSendCommandNoWait(CMD_HW_RESET);

   return Response;
}

/**
   @fn RIE_Responses RadioPowerOff(void)
   @brief Shutdown the radio and place it in its lowest power sleep mode.
   @pre
      RadioInit() must be called before this function is called.
   @return RIE_Response  Error code.
**/

RIE_Responses RadioPowerOff(void)
{
   volatile RIE_U32  ulDelay;
   RIE_Responses     Response = RIE_Success;

   Response = RadioSendCommandNoWait(CMD_HW_RESET);

   // Delay for approximately 1 ms
   ulDelay  = 0x1000;
   while (ulDelay--);

   return Response;
}

/**
   @fn      RIE_Responses RadioTerminateRadioOp(void)
   @brief   Terminate a currently running radio RX or TX operation.
   @pre     RadioInit() must be called before this function is called.
   @code
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioRxPacketFixedLen(12);
      // Delay for a while waiting for a packet
      if (RIE_Response == RIE_Success)
         {
         // Abort the waiting
         RIE_Response = RadioTerminateRadioOp();
         }
   @endcode
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioTerminateRadioOp        (void)
{
   RIE_Responses Response = RIE_Success;

   Response = RadioToOnMode();

   return Response;
}

/**
   @fn RIE_Responses RadioSetFrequency(RIE_U32 Frequency)
   @brief Set frequency for radio communications
   @param Frequency :{431000000-928000000}
      - This must be within the available bands of the radio:
         - 431000000Hz to 464000000Hz and
         - 862000000Hz to 928000000Hz.
   @pre     RadioInit() must be called before this function is called.
   @code
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioSetFrequency(915000000);
   @endcode
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioSetFrequency(RIE_U32 Frequency)
{
   RIE_Responses  Response = RIE_Success;
   RIE_U32        EncodedFrequency;

   bRadioConfigurationChanged = RIE_TRUE;

   EncodedFrequency                     = (RIE_U32)(Frequency * FREQ_CNVRT_VAL);
   RadioConfiguration.channel_freq_0_r  = (EncodedFrequency >> 0) & 0xFF;
   RadioConfiguration.channel_freq_1_r  = (EncodedFrequency >> 8) & 0xFF;
   RadioConfiguration.channel_freq_2_r  = (EncodedFrequency >> 16)& 0xFF;
   if (Frequency >= 862000000)
      {
      RadioConfiguration.image_reject_cal_amplitude_r   = 0x07;
      RadioConfiguration.image_reject_cal_phase_r       = 0x16;
      }
   else
      {
      RadioConfiguration.image_reject_cal_amplitude_r   = 0x03;
      RadioConfiguration.image_reject_cal_phase_r       = 0x08;
      }
   return Response;
}

/**
   @fn RIE_Responses RadioSetModulationType(RIE_ModulationTypes ModulationType)
   @brief Set the Radio Transmitter Modulation Type. Can be FSK_Modulation or GFSK_Modulation.
   @param ModulationType :{DR_1_0kbps_Dev10_0kHz , DR_38_4kbps_Dev20kHz ,DR_300_0kbps_Dev75_0kHz }
      - DR_1_0kbps_Dev10_0kHz  Base configuration of 1 kbps datarate, 10.0 kHz frequency deviation.
      - DR_38_4kbps_Dev20kHz  Base configuration of 38.4 kbps datarate, 20 kHz frequency deviation.
      - DR_300_0kbps_Dev75_0kHz  Base configuration of 300 kbps datarate, 75 kHz frequency deviation.
   @pre     RadioInit() must be called before this function is called.
   @code
      Response = RadioSetModulationType(GFSK_Modulation);
   @endcode
   @note   FSK_Modulation is used by default.
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioSetModulationType(RIE_ModulationTypes ModulationType)
{
   RIE_Responses  Response    = RIE_Success;
   RIE_U8         ucNewCode;
   RIE_U8         ucNewRegVal = RadioConfiguration.radio_cfg_9_r;

   switch (ModulationType)
      {
      case FSK_Modulation:
         ucNewCode = radio_cfg_9_mod_scheme_2_level_FSK;
         break;
      case GFSK_Modulation:
         ucNewCode = radio_cfg_9_mod_scheme_2_level_GFSK;
         break;
      default:
         Response   = RIE_UnsupportedRadioConfig;
         break;
      }
   if(Response == RIE_Success)
      {
      ucNewRegVal  =  MSKSET_VAL(RadioConfiguration.radio_cfg_9_r,
                                 radio_cfg_9_mod_scheme_numbits,
                                 radio_cfg_9_mod_scheme_offset,
                                 ucNewCode);
      if (ucNewRegVal != RadioConfiguration.radio_cfg_9_r )
         {
         bRadioConfigurationChanged        = RIE_TRUE;
         RadioConfiguration.radio_cfg_9_r  = ucNewRegVal;
         }

      }
   return Response;
}

/**
    @fn    RIE_Responses RadioPayldManchesterEncode(RIE_BOOL bEnable)
    @brief Enable or Disable Manchester Encoding of payload data.

           Manchester encoding can be used to ensure a dc-free (zero mean)
           transmission.

           A Binary 0 is mapped to 10, and a Binary 1 is mapped to 01.

           Manchester encoding and decoding are applied to the payload data
           and the CRC.

    @param  bEnable :{RIE_FALSE,RIE_TRUE}
         - RIE_TRUE if Manchester Encoding is to be enabled.
         - RIE_FALSE if disabled.

    @pre     RadioInit() must be called before this function is called.
     @code
             Response = RadioPayldManchesterEncode(RIE_TRUE);

     @endcode
     @note   Manchester Encoding is disabled by default.
     @return RIE_Responses  Error code
**/
RIE_Responses RadioPayldManchesterEncode(RIE_BOOL bEnable)
{
   RIE_Responses  Response     = RIE_Success;
   RIE_U8         ucNewRegVal  = RadioConfiguration.symbol_mode_r;

   switch (bEnable)
      {
      case RIE_FALSE:
         ucNewRegVal &= ~symbol_mode_manchester_enc_enabled;
         break;
      case RIE_TRUE:
         ucNewRegVal |= symbol_mode_manchester_enc_enabled;
         break;
      default:
         Response   = RIE_UnsupportedRadioConfig;
         break;
      }
   if(Response == RIE_Success)
      {
      if (ucNewRegVal != RadioConfiguration.symbol_mode_r )
         {
         bRadioConfigurationChanged        = RIE_TRUE;
         RadioConfiguration.symbol_mode_r  = ucNewRegVal;
         }
      }
   return Response;
}
/**
    @fn      RIE_Responses RadioPayldDataWhitening(RIE_BOOL bEnable)
    @brief   Enable or Disable Data Whitening of payload data.

             Data whitening can be employed to avoid long runs of 1s or 0s
             in the transmitted data stream.

             This ensures sufficient bit transitions in the packet, which
             aids in receiver clock and data recovery because the encoding
             breaks up long runs of 1s or 0s in the transmit packet.

             The data, excluding the preamble and sync word, is automatically
             whitened before transmission by XORing the data with an 8-bit
             pseudorandom sequence.

             At the receiver, the data is XORed with the same pseudorandom
             sequence, thereby reversing the whitening.

             The linear feedback shift register polynomial used is x7 + x1 + 1.

    @param  bEnable :{RIE_FALSE, RIE_TRUE}
         - RIE_TRUE if Manchester Encoding is to be enabled.
         - RIE_FALSE if disabled.

    @pre     RadioInit() must be called before this function is called.
    @code
             Response = RadioPayldDataWhitening(RIE_TRUE);

     @endcode
     @note   Data Whitening is disabled by default.
     @return  RIE_Responses  Error code
**/
RIE_Responses RadioPayldDataWhitening(RIE_BOOL bEnable)
{
   RIE_Responses  Response     = RIE_Success;
   RIE_U8         ucNewRegVal  = RadioConfiguration.symbol_mode_r;

   switch (bEnable)
      {
      case RIE_FALSE:
         ucNewRegVal &= ~symbol_mode_data_whitening_enabled;
         break;
      case RIE_TRUE:
         ucNewRegVal |= symbol_mode_data_whitening_enabled;
         break;
      default:
         Response   = RIE_UnsupportedRadioConfig;
         break;
      }
   if(Response == RIE_Success)
      {
      if (ucNewRegVal != RadioConfiguration.symbol_mode_r )
         {
         bRadioConfigurationChanged        = RIE_TRUE;
         RadioConfiguration.symbol_mode_r  = ucNewRegVal;
         }
      }
   return Response;
}

/**
   @fn     RIE_Responses RadioTxPacketFixedLen(RIE_U8 Len, RIE_U8 *pData)
   @brief  Transmit a fixed length packet.
   @param  Len :{1-240}   Length of packet to be transmitted.
   @param  pData :{} Data bytes to be transmitted.
   @pre     RadioInit() must be called before this function is called.
   @code
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioTxSetPA(DifferentialPA,PowerLevel15);
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioTxPacketFixedLen(12, "HELLO WORLD");
      while (!RadioTxPacketComplete());
   @endcode
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioTxPacketFixedLen(RIE_U8 Len, RIE_U8 *pData)
{
   RIE_Responses  Response     = RIE_Success;

   bPacketTx = RIE_FALSE;
   if (Len > PACKETRAM_LEN)
      Response = RIE_InvalidParamter;
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   if (Response == RIE_Success)
      Response = RadioMMapWrite(PACKETRAM_START, Len, pData);
   if (Response == RIE_Success)
      {
      RadioConfiguration.packet_length_max_r     = Len;
      RadioConfiguration.packet_length_control_r |= packet_length_control_packet_len_fixed;
      }
   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_PHY_TX);

   return Response;
}

/**
   @fn     RIE_Responses RadioTxPacketVariableLen(RIE_U8 Len, RIE_U8 *pData)
   @brief  Transmit a Variable length packet.
   @param  Len :{1-240}   Length of packet to be transmitted.
   @param  pData :{} Data bytes to be transmitted.
   @pre     RadioInit() must be called before this function is called.
   @code
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioTxSetPA(DifferentialPA,PowerLevel15);
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioTxPacketVariableLen(12, "HELLO WORLD");
      while (!RadioTxPacketComplete());
   @endcode
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioTxPacketVariableLen(RIE_U8 Len, RIE_U8 *pData)
{
   RIE_Responses  Response     = RIE_Success;

   bPacketTx = RIE_FALSE;

   Len += 0x1;
   if (Len > PACKETRAM_LEN)
      Response = RIE_InvalidParamter;
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   if (Response == RIE_Success)
      Response = RadioMMapWrite(PACKETRAM_START, 0x1, &Len);
   if (Response == RIE_Success)
      Response = RadioMMapWrite(PACKETRAM_START+0x1, Len-1, pData);
   if (Response == RIE_Success)
      {
      RadioConfiguration.packet_length_max_r     = PACKETRAM_LEN;
      RadioConfiguration.packet_length_control_r &= ~packet_length_control_packet_len_fixed;
      }
   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_PHY_TX);

   return Response;
}


/**
   @fn    RIE_BOOL RadioTxPacketComplete(void)
   @brief Checks if a packet has finished transmitting
   @pre     RadioInit() must be called before this function is called.
   @pre     RadioRxPacketFixedLen() or equivalent should be called first.
   @code
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioTxSetPA(DifferentialPA,PowerLevel15);
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioTxPacketFixedLen(12, "HELLO WORLD");
      while (!RadioTxPacketComplete());
   @endcode
   @return  RIE_BOOL  RIE_TRUE if packet has finished transmitting, else RIE_FALSE
**/
RIE_BOOL      RadioTxPacketComplete     (void)
{
   return bPacketTx;
}

/**
   @fn     RIE_Responses RadioTxSetPA(RIE_PATypes PAType,RIE_PAPowerLevel Power)
   @brief  Set PA Type and the Transmit Power Level for Radio Transmission.
   @param  PAType :{DifferentialPA, SingleEndedPA} Select Single Ended or Differential PA Type
   @param  Power :{PowerLevel0 ,PowerLevel1 ,PowerLevel2 ,PowerLevel3,
           PowerLevel4 ,PowerLevel5 ,PowerLevel6 ,PowerLevel7,
           PowerLevel8 ,PowerLevel9 ,PowerLevel10,PowerLevel11,
           PowerLevel12,PowerLevel13,PowerLevel14,PowerLevel15}
   @pre    RadioInit() must be called before this function is called.
   @code
             Response = RadioTxSetPA(SingleEndedPA,PowerLevel8);
   @endcode
   @note   Differential PA is enabled by default.
   @note   Max TX Power is used by default.
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioTxSetPA(RIE_PATypes PAType,RIE_PAPowerLevel Power)
{
   RIE_Responses  Response     = RIE_Success;
   RIE_U8         ucNewRegVal  = 0x0;
   unsigned long  pa_level_mcr,pa_ramp, codes_per_bit,min_codes_per_bit;

   switch (PAType)
      {
      case DifferentialPA:
         ucNewRegVal |= radio_cfg_8_pa_single_diff_sel_differential;
         break;
      case SingleEndedPA:
         ucNewRegVal |= radio_cfg_8_pa_single_diff_sel_single_ended;
         break;
      default:
         Response   = RIE_UnsupportedRadioConfig;
         break;
      }
   if(Response == RIE_Success)
      {
      switch (Power)
         {
         case PowerLevel0 :
         case PowerLevel1 :
         case PowerLevel2 :
         case PowerLevel3 :
         case PowerLevel4 :
         case PowerLevel5 :
         case PowerLevel6 :
         case PowerLevel7 :
         case PowerLevel8 :
         case PowerLevel9 :
         case PowerLevel10:
         case PowerLevel11:
         case PowerLevel12:
         case PowerLevel13:
         case PowerLevel14:
         case PowerLevel15:
            ucNewRegVal |= ((RIE_U8)Power << radio_cfg_8_pa_power_offset);
            // Calculate the minimum allowable codes per bit
            pa_level_mcr =  (((RIE_U8)Power)* 4) + 0x3;
            min_codes_per_bit = (pa_level_mcr * 2500)/(DataRate/100);
            pa_ramp        = 0x1;
            codes_per_bit  = 256;
            while (codes_per_bit > min_codes_per_bit)
               {
               pa_ramp++;
               codes_per_bit = 512 >> pa_ramp;
               if (pa_ramp >= 7)
                  break; // This is the maximum
               }
            ucNewRegVal |= ((RIE_U8)pa_ramp << radio_cfg_8_pa_ramp_offset);
            break;
         default:
            Response   = RIE_UnsupportedRadioConfig;
            break;
         }
      }

   if(Response == RIE_Success)
      {
      if (ucNewRegVal != RadioConfiguration.radio_cfg_8_r )
         {
         bRadioConfigurationChanged        = RIE_TRUE;
         RadioConfiguration.radio_cfg_8_r  = ucNewRegVal;
         }
      }
   return Response;
}
/**
   @fn      RIE_Responses RadioTxCarrier(void)
   @brief   Transmit a carrier tone
            using the current radio configuration.
   @pre     RadioInit() must be called before this function is called.
   @code
      Response = RadioTxCarrier();
   @endcode
   @note   Terminate this mode by calling RadioTerminateRadioOp();
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioTxCarrier     (void)
{
   RIE_Responses Response = RIE_Success;
   RIE_U8        ParamTX  = PARAM_TX_CARRIER_FOREVER;

   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   bTestModeEnabled = RIE_TRUE;
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   // Mode needs to be set, before entry to PHY_TX
   if (Response == RIE_Success)
      Response = RadioMMapWrite(PR_var_tx_mode_ADR,sizeof(ParamTX),&ParamTX);
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_PHY_TX);
   return Response;
}
/**
   @fn      RIE_Responses RadioTxPreamble(void)
   @brief   Transmit a pre-amble (alternating ones and zeros)
            using the current radio configuration.
   @pre     RadioInit() must be called before this function is called.
   @code
      Response = RadioTxPreamble();
   @endcode
   @note   Terminate this mode by calling RadioTerminateRadioOp();
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioTxPreamble    (void)
{
   RIE_Responses Response = RIE_Success;
   RIE_U8        ParamTX  = PARAM_TX_PREAMBLE_FOREVER;

   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   bTestModeEnabled = RIE_TRUE;
   // Mode needs to be set, before entry to PHY_TX
   if (Response == RIE_Success)
      Response = RadioMMapWrite(PR_var_tx_mode_ADR,sizeof(ParamTX),&ParamTX);
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_PHY_TX);
   return Response;
}

/**
   @fn     RIE_Responses RadioRxPacketFixedLen(RIE_U8 Len)
   @brief  Enter receive mode and wait for a packet to be received.

      Radio will stay in Receive Mode until
         1) A packet is received.
         2) User manually exits Receive Mode with a call to RadioTerminateRadioOp()

   @param  Len :{1-240} Fixed Length of packet to be received.
   @pre     RadioInit() must be called before this function is called.
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioRxPacketFixedLen(RIE_U8 Len)
{
   RIE_Responses  Response = RIE_Success;

   bPacketRx = RIE_FALSE;
   if (Len > PACKETRAM_LEN)
      Response = RIE_InvalidParamter;

   if (Response == RIE_Success)
      {
      RadioConfiguration.packet_length_max_r     = Len;
      RadioConfiguration.packet_length_control_r |= packet_length_control_packet_len_fixed;
      }
   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_PHY_RX);
   return Response;
}

/**
    @fn     RIE_Responses RadioRxPacketVariableLen(void)
    @brief  Enter receive mode and wait for a packet to be received.

            Radio will stay in Receive Mode until
            1) A packet is received.
            2) User manually exits Receive Mode with a call to RadioTerminateRadioOp()

    @pre     RadioInit() must be called before this function is called.
    @return  RIE_Responses  Error code
**/
RIE_Responses RadioRxPacketVariableLen(void)
{
   RIE_Responses  Response = RIE_Success;

   bPacketRx = RIE_FALSE;

   if (Response == RIE_Success)
      {
      RadioConfiguration.packet_length_max_r     = PACKETRAM_LEN;
      RadioConfiguration.packet_length_control_r &= ~packet_length_control_packet_len_fixed;
      }
   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_PHY_RX);
   return Response;
}

/**
    @fn    RIE_BOOL RadioRxPacketAvailable(void)
    @brief Checks if a packet has been received.
    @pre   RadioInit() must be called before this function is called.
    @pre   RadioRxPacketFixedLen() or equivalent should be called first.
     @code
           if (RIE_Response == RIE_Success)
              RIE_Response = RadioRxPacketFixedLen(12);
           if (RIE_Response == RIE_Success)
              {
              while (!RadioRxPacketAvailable());
              }
           if (RIE_Response == RIE_Success)
              {
              unsigned char  Buffer[0x20];
              RIE_U8         PktLen;
              RIE_S8         RSSI;
              RIE_Response = RadioRxPacketRead(sizeof(Buffer),&PktLen,Buffer,&RSSI);
              }
     @endcode
    @return  RIE_BOOL  RIE_TRUE if packet received, else RIE_FALSE
**/
RIE_BOOL RadioRxPacketAvailable(void)
{
   return bPacketRx;

}

/**
   @fn     RIE_Responses RadioRxPacketRead(RIE_U8 BufferLen,RIE_U8 *pPktLen,RIE_U8 *pData,RIE_S8 *pRSSIdBm)
   @brief  Read the packet that was received by the radio.
   @param  BufferLen :{1-240} Size of passed in buffer
   @param  pPktLen :{1-240}   Storage for size of actual received packet
   @param  pData :{}     Received Packet will be stored here.
   @param  pRSSIdBm :{}  RSSI of received packet in dBm.
   @pre    RadioInit() must be called before this function is called.
   @pre    RadioRxPacketFixedLen() or equivalent should be called first.
   @code
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioRxPacketFixedLen(12);
      if (RIE_Response == RIE_Success)
         {
         while (!RadioRxPacketAvailable());
         }
      if (RIE_Response == RIE_Success)
         {
         unsigned char  Buffer[0x20];
         RIE_U8         PktLen;
         RIE_S8         RSSI;
         RIE_Response = RadioRxPacketRead(sizeof(Buffer),&PktLen,Buffer,&RSSI);
         }
   @endcode
   @note   Check for the presence of a packet by calling RadioRxPacketAvailable();
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioRxPacketRead(RIE_U8 BufferLen,RIE_U8 *pPktLen,RIE_U8 *pData,RIE_S8 *pRSSIdBm)
{
   RIE_Responses  Response     = RIE_Success;

   if (RadioRxPacketAvailable())
      {
      RIE_U8 RdLen;
      if(RadioConfiguration.packet_length_control_r & packet_length_control_packet_len_fixed)
         {
         if (pPktLen)
            *pPktLen = RadioConfiguration.packet_length_max_r;
         RdLen = RadioConfiguration.packet_length_max_r;
         if (RdLen > BufferLen)
            RdLen = BufferLen;
         if (Response == RIE_Success)
            Response = RadioMMapRead(PACKETRAM_START,RdLen, pData);
         }
      else
         {
         if (Response == RIE_Success)
            Response = RadioMMapRead(PACKETRAM_START,0x1, &RdLen);
         RdLen -= 0x1;
         if (pPktLen)
            *pPktLen = RdLen;
         if (RdLen > BufferLen)
            RdLen = BufferLen;
         if (Response == RIE_Success)
            Response = RadioMMapRead(PACKETRAM_START+0x1,RdLen, pData);
         }

      if (pRSSIdBm)
         {
         if (Response == RIE_Success)
            Response = RadioMMapRead(MCR_rssi_readback_Adr,0x1, (RIE_U8 *)pRSSIdBm);

         *pRSSIdBm -= 107; // Convert to dBm
         }

      }
   else
      {
      if (pPktLen)
         *pPktLen = 0x0;
      }

   return Response;
}

/**
   @fn      RIE_Responses RadioRxBERTestMode(void)
   @brief   Enter receiver Bit Error Rate (BER) test mode where the
             clock and data appear on GPIO pins.
             Clock on P0.6 and Data on P2.6
   @pre     RadioInit() must be called before this function is called.
   @code
      Response = RadioRxBERTestMode();
   @endcode
   @note   Terminate this mode by calling RadioTerminateRadioOp();
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioRxBERTestMode(void)
{
   RIE_Responses Response = RIE_Success;
   RIE_U8        Data;
   // Enables internal radio signals on external pins
   // but overrides some of the standard GPIO muxed
   // functionality (UART?)
   pADI_MISC->RFTST = 0x7E1;

   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();

   bTestModeEnabled = RIE_TRUE;
   // Enable the RX signals on GPIO pins
   Data = gpio_configure_sport_mode_0;
   if (Response == RIE_Success)
      Response = RadioMMapWrite(MCR_gpio_configure_Adr, 0x1, (RIE_U8 *)&Data);

   // disable ext_uc_clk on GP5
   Data = 0;
   if (Response == RIE_Success)
      Response = RadioMMapWrite(MCR_ext_uc_clk_divide_Adr,
                                0x1,
                                (RIE_U8 *)&Data);
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_PHY_RX);
   return Response;
}

/**
    @internal Hide from Doxegen
    @fn       RIE_Responses RadioCommitRadioConfig(void)
    @brief    Configures the radio if any changes were made
              since the last time.
    @return  RIE_Responses  Error code
**/
static RIE_Responses RadioCommitRadioConfig(void)
{
   RIE_Responses Response = RIE_Success;

   if(bTestModeEnabled)
      {
      RIE_U8        Data;
      Data = gpio_configure_default;
      if (Response == RIE_Success)
         Response = RadioMMapWrite(MCR_gpio_configure_Adr, 0x1, (RIE_U8 *)&Data);
      Data = 4;
      if (Response == RIE_Success)
         Response = RadioMMapWrite(MCR_ext_uc_clk_divide_Adr,
                                   0x1,
                                   (RIE_U8 *)&Data);

      Data = PARAM_TX_NORMAL_PACKET;
      if (Response == RIE_Success)
         Response = RadioMMapWrite(PR_var_tx_mode_ADR,sizeof(Data),&Data);
      bTestModeEnabled = RIE_FALSE;
      }
   if (bRadioConfigurationChanged)
      {
      Response = RadioConfigure();
      if(Response == RIE_Success)
         bRadioConfigurationChanged = RIE_FALSE;
      }
   return Response;
}
/**
    @fn      RIE_Responses RadioReadState(RadioState *pState)
    @brief   Read the current state
    @param   pState      Pointer to return storage of state
    @return  RIE_Responses  Error code
**/
static RIE_Responses RadioReadState(RadioState *pState)
{
   RIE_Responses  Response    = RIE_Success;
   RIE_U8         StatusByte;

   NVIC_DisableIRQ(UHFTRX_IRQn);
   RADIO_CSN_ASSERT;
   if (Response == RIE_Success)
      Response = RadioSPIXferByte(SPI_NOP,NULL);
   if (Response == RIE_Success)
      Response = RadioSPIXferByte(SPI_NOP,&StatusByte);
   RADIO_CSN_DEASSERT;
   NVIC_EnableIRQ      (UHFTRX_IRQn);
   if ((Response == RIE_Success) && pState)
      *pState = (RadioState)(StatusByte & STATUS_BYTE_FW_STATE);
   return Response;
}

/**
    @fn      RIE_Responses RadioWaitOnState(RadioState FinalState)
    @brief   Wait for Final State to be reached
    @param   FinalState    State to wait on
    @return  RIE_Responses  Error code
**/
static RIE_Responses  RadioWaitOnState(RadioState FinalState)
{
   RIE_Responses  Response = RIE_Success;
   RadioState     CurrState;
   do
      {
      Response   = RadioReadState(&CurrState);
      }
   while((Response == RIE_Success) && (CurrState != FinalState));
   return Response;
}

/**
    @fn      RIE_Responses  RadioWaitOnCmdLdr(void)
    @brief   Wait for Final State to be reached
    @return  RIE_Responses  Error code
**/
static RIE_Responses  RadioWaitOnCmdLdr(void)
{
   RIE_Responses  Response = RIE_Success;
   do
      {
      RIE_U8         StatusByte;
      NVIC_DisableIRQ(UHFTRX_IRQn);
      RADIO_CSN_ASSERT;
      if (Response == RIE_Success)
         Response = RadioSPIXferByte(SPI_NOP,NULL);
      if (Response == RIE_Success)
         Response = RadioSPIXferByte(SPI_NOP,&StatusByte);
      RADIO_CSN_DEASSERT;
      NVIC_EnableIRQ      (UHFTRX_IRQn);
      if ((Response == RIE_Success))
         if(StatusByte & STATUS_BYTE_CMD_READY)
            break;
      }
   while((Response == RIE_Success));
   return Response;
}

/**
    @internal Hide from Doxegen
    @fn      RIE_Responses RadioToOnMode(void)
    @brief   Transition to On Mode

             Handle all possible states that the radio could be in
             and brings it back to PHY_ON state
    @param   None
    @return  RIE_Responses  Error code
**/
static RIE_Responses RadioToOnMode(void)
{
   RIE_Responses  Response = RIE_Success;
   RadioState     FwState;

   if (Response == RIE_Success)
      Response = RadioReadState(&FwState);

   while ((FwState != FW_ON) && (Response == RIE_Success))
       {
       switch (FwState)
           {
           case FW_BUSY:
               break;
           case FW_TX:
              if(Response == RIE_Success)
                 Response = RadioSendCommandNoWait(CMD_PHY_ON);
               if (Response == RIE_Success)
                  Response = RadioWaitOnState   (FW_ON);
               break;
           case FW_RX:
              if(Response == RIE_Success)
                 Response = RadioSendCommandNoWait(CMD_PHY_ON);
               if (Response == RIE_Success)
                  Response = RadioWaitOnState   (FW_ON);
               break;
           default:
              if(Response == RIE_Success)
                 Response = RadioSendCommandNoWait(CMD_PHY_ON);
               if (Response == RIE_Success)
                  Response = RadioWaitOnState   (FW_ON);
               break;
           }
       if (Response == RIE_Success)
          Response = RadioReadState(&FwState);
       }
   return Response;
}
/**
    @internal Hide from Doxegen
    @fn      RIE_Responses RadioToOffMode(void)
    @brief   Transition to Off Mode

             Handle all possible states that the radio could be in
             and bring it back to PHY_OFF state.

    @param   None
    @return  RIE_Responses  Error code
**/
static RIE_Responses RadioToOffMode(void)
{
   RIE_Responses  Response = RIE_Success;
   RadioState     FwState;

   if (Response == RIE_Success)
      Response = RadioReadState(&FwState);

   while ((FwState != FW_OFF) && (Response == RIE_Success))
       {
       switch (FwState)
           {
           case FW_BUSY:
               break;
           case FW_TX:
              if(Response == RIE_Success)
                 Response = RadioSendCommandNoWait(CMD_PHY_ON);
               if (Response == RIE_Success)
                  Response = RadioWaitOnState   (FW_ON);
               break;
           case FW_RX:
              if(Response == RIE_Success)
                 Response = RadioSendCommandNoWait(CMD_PHY_ON);
               if (Response == RIE_Success)
                  Response = RadioWaitOnState   (FW_ON);
               break;
           default:
              if(Response == RIE_Success)
                 Response = RadioSendCommandNoWait(CMD_PHY_OFF);
               if (Response == RIE_Success)
                  Response = RadioWaitOnState   (FW_OFF);
               break;
           }
       if (Response == RIE_Success)
          Response = RadioReadState(&FwState);
       }
   return Response;
}
/**
    @internal Hide from Doxegen
    @fn      RIE_Responses  RadioSyncComms       (void)
    @brief   Sync comms with the radio
    @param   None
    @return  RIE_Responses  Error code
**/
static RIE_Responses  RadioSyncComms       (void)
{
   RIE_Responses  Response = RIE_Success;
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_SYNC);
   if (Response == RIE_Success)
      Response = RadioWaitOnCmdLdr();
   return Response;
}
/**
    @fn      RIE_Responses  RadioWaitForPowerUp(void)
    @brief   Wake Up the Part

             Assert SPI chip select which will wake up the radio if asleep
             Wait for MISO line to go high indicating SPI comms now possible

    @return  RIE_Responses  Error code
**/
static RIE_Responses  RadioWaitForPowerUp(void)
{
   RIE_Responses  Response = RIE_Success;
   int            i        = 0x0;
   RADIO_CSN_ASSERT;
   while (!RADIO_MISO_IN && (i < 1000))
      i++;
   if (1000 == i)// Timed out waiting for MISO high?
      Response = RIE_RadioSPICommsFail;
   RADIO_CSN_DEASSERT;
   return Response;
}

/**
    \internal Hide from Doxegen
    \fn      void Ext_Int8_Handler(void)
    \brief   Radio Interrupt Handler
**/
extern void aducrf101_rx_packet_hook(void);
void Ext_Int8_Handler (void)
{
   RIE_Responses  Response = RIE_Success;
   RIE_U8         ucInt0;
   RIE_U8         ucInt1;

   if (Response == RIE_Success)
      Response = RadioMMapRead(MCR_interrupt_source_0_Adr,0x1, &ucInt0);
   if (Response == RIE_Success)
      Response = RadioMMapRead(MCR_interrupt_source_1_Adr,0x1,&ucInt1);
   if (ucInt0 & interrupt_mask_0_interrupt_tx_eof)
      bPacketTx = RIE_TRUE;
   if (ucInt0 & interrupt_mask_0_interrupt_crc_correct) {
      bPacketRx = RIE_TRUE;
      aducrf101_rx_packet_hook();
   }
   // Clear all the interrupts that we have just handleed
   if (Response == RIE_Success)
      Response = RadioMMapWrite(MCR_interrupt_source_0_Adr,0x1, &ucInt0);
   if (Response == RIE_Success)
      Response = RadioMMapWrite(MCR_interrupt_source_1_Adr,0x1,&ucInt1);
    // Clear the interrupt
    pADI_INTERRUPT->EICLR = EICLR_IRQ8;
}
/**
    \internal Hide from Doxegen
    \fn      void RadioSPIXferByte(RIE_U8 ucByte,RIE_U8 *pData)
    \brief   Transfer a byte via SPI to the radio and optionally return
             received byte.
             Chip Select is manually controlled elsewhere.
    \param   ucByte  Command or data byte to be transferred.
    \param   pData   NULL, or storage for response
    \return  RIE_Responses  Error code
**/
static RIE_Responses RadioSPIXferByte(RIE_U8 ucByte,RIE_U8 *pData)
{
   RIE_Responses  Response = RIE_Success;

   SEND_SPI(ucByte);  // Send byte
   WAIT_SPI_RX;       // wait for data received status bit
   if(pData)
      *pData = READ_SPI;
   else
      (void)READ_SPI;
   return Response;
}
/**
    \internal Hide from Doxegen
    \fn      RIE_Responses RadioSendCommandBytes(RIE_U8 *pCmdBytes,RIE_U8 NumBytes)
    \brief   Send a complete command to the radio.

             It is neccessary to disable the radio interrupt when doing this
             as a command in progress must finish before a radio interrupt
             can be handled.

    \param   pCmdBytes Pointer to a number of bytes to be transferred.
    \param   NumBytes  Number of bytes to transfer

    \return  RIE_Responses  Error code
**/
static RIE_Responses RadioSendCommandBytes(RIE_U8 *pCmdBytes,RIE_U8 NumBytes)
{
   RIE_Responses  Response = RIE_Success;

   NVIC_DisableIRQ(UHFTRX_IRQn);
   RADIO_CSN_ASSERT;
   while ((NumBytes--) && (Response == RIE_Success))
      Response = RadioSPIXferByte(*(pCmdBytes++),NULL); // Send Command
   RADIO_CSN_DEASSERT;   // De-assert SPI chip select
   NVIC_EnableIRQ      (UHFTRX_IRQn);

   return Response;
}
/**
    \internal Hide from Doxegen
    \fn      RIE_Responses RadioSendCommandNoWait        (Radio_CmdCodes CmdCode )
    \brief   Send a single byte command to the radio.
    \param   CmdCode Command code to be sent
    \return  RIE_Responses  Error code
**/
static RIE_Responses RadioSendCommandNoWait (Radio_CmdCodes CmdCode )
{
   RIE_U8 Command = (RIE_U8)CmdCode;
   return RadioSendCommandBytes(&Command,0x1);
}
/**
    \internal Hide from Doxegen
    \fn      RIE_Responses RadioSendCommandWait        (Radio_CmdCodes CmdCode )
    \brief   Send a single byte command to the radio.
    \param   CmdCode Command code to be sent
    \return  RIE_Responses  Error code
**/
static RIE_Responses RadioSendCommandWait (Radio_CmdCodes CmdCode )
{
   RIE_Responses  Response = RIE_Success;
   RIE_U8         Command  = (RIE_U8)CmdCode;

   if (Response == RIE_Success)
      Response = RadioWaitOnCmdLdr();
   if (Response == RIE_Success)
      Response = RadioSendCommandBytes(&Command,0x1);
   return Response;
}
/**
    \fn      RIE_Responses RadioMMapRead(RIE_U32 ulAdr, RIE_U32 ulLen, RIE_U8 *pData)
    \brief   Read bytes from specified memory map address
    \param   ulAdr  Address to read at.
    \param   ulLen  Length of data to read.
    \param   pData  Pointer to location to stored read data.
    \return  RIE_Responses  Error code
**/
static RIE_Responses RadioMMapRead(RIE_U32 ulAdr, RIE_U32 ulLen, RIE_U8 *pData)
{
   RIE_Responses  Response = RIE_Success;

   NVIC_DisableIRQ(UHFTRX_IRQn);
   RADIO_CSN_ASSERT;

   if(Response == RIE_Success) // Send first byte (SPI_MEMR_RD + Bytes)
      Response = RadioSPIXferByte(SPI_MEM_RD | ((ulAdr & 0x700) >> 8),NULL);
   if(Response == RIE_Success)// Send Second byte remainder of address
      Response = RadioSPIXferByte((RIE_U8)(ulAdr & 0xFF),NULL);
   if(Response == RIE_Success)
      Response = RadioSPIXferByte((RIE_U8)SPI_NOP,NULL);
   while(ulLen-- && (Response == RIE_Success))
      Response = RadioSPIXferByte(SPI_NOP,pData++);
   RADIO_CSN_DEASSERT;
   NVIC_EnableIRQ      (UHFTRX_IRQn);

   return Response;
}
/**
    \fn      RIE_Responses RadioMMapWrite(RIE_U32 ulAdr, RIE_U32 ulLen, RIE_U8 *pData)
    \brief   Read bytes from specified memory map address
    \param   ulAdr  Address to read at.
    \param   ulLen  Length of data to read.
    \param   pData  Pointer to location of data to write.
    \return  RIE_Responses  Error code
**/
static RIE_Responses RadioMMapWrite(RIE_U32 ulAdr,RIE_U32   ulLen,RIE_U8 *  pData)
{
   RIE_Responses  Response = RIE_Success;

   NVIC_DisableIRQ(UHFTRX_IRQn);
   RADIO_CSN_ASSERT;
   if(Response == RIE_Success) // Send first byte (SPI_MEMR_WR + Bytes)
      Response = RadioSPIXferByte(SPI_MEM_WR | ((ulAdr & 0x700) >> 8),NULL);
   if(Response == RIE_Success) // Send Second byte remainder of addrress
      Response = RadioSPIXferByte((RIE_U8)(ulAdr & 0xFF),NULL);
   while(ulLen-- && (Response == RIE_Success))
      Response = RadioSPIXferByte(*(pData++),NULL);
   RADIO_CSN_DEASSERT;
   NVIC_EnableIRQ      (UHFTRX_IRQn);

   return Response;
}

/**
    \internal Hide from Doxegen
    \fn      void SetRadioConfiguration(void)
    \brief   Create a default radio configuration that all base configurations
             are derived from.

    \return  RIE_Responses  Error code
**/
static RIE_Responses SetRadioConfiguration(RIE_BaseConfigs BaseConfig)
{
   RIE_Responses  Response = RIE_Success;

   bRadioConfigurationChanged = RIE_TRUE;
   switch (BaseConfig)
      {
      case DR_1_0kbps_Dev10_0kHz:
         memcpy((void *)&RadioConfiguration,
                (void *)DR_1_0kbps_Dev10_0kHz_Configuration,
                sizeof(TyRadioConfiguration));
         DataRate = 1000;
         break;
      case DR_38_4kbps_Dev20kHz:
         memcpy((void *)&RadioConfiguration,
                (void *)DR_38_4kbps_Dev20kHz_Configuration,
                sizeof(TyRadioConfiguration));
         DataRate = 38400;
         break;
      case DR_300_0kbps_Dev75_0kHz:
         memcpy((void *)&RadioConfiguration,
                (void *)DR_300_0kbps_Dev75_0kHz_Configuration,
                sizeof(TyRadioConfiguration));
         DataRate = 300000;
         break;
      default:
         Response = RIE_UnsupportedRadioConfig;
         break;
      }
   return Response;
}
/**
    @internal Hide from Doxegen
    @fn      RIE_Responses RadioConfigure       (void)
    @brief   Configure the Radio as per the current configuration
    @return  RIE_Responses  Error code
**/
RIE_Responses RadioConfigure (void)
{
   RIE_Responses  Response = RIE_Success;
   if(Response == RIE_Success)
      Response = RadioToOffMode();
   if(Response == RIE_Success) // Write the configuration to the radio memory
      Response = RadioMMapWrite(BBRAM_START,
                                sizeof(TyRadioConfiguration),
                                (RIE_U8 *)&RadioConfiguration);
   if(Response == RIE_Success) // Apply that configuration to the radio
      Response = RadioSendCommandWait(CMD_CONFIG_DEV);
   if(Response == RIE_Success)
      Response = RadioToOnMode();
   return Response;
}


/**
   @fn     RIE_Responses RadioRadioGetRSSI (RIE_S8 *pRSSIdBm)
   @brief  Return a Received Signal Strength Indicator value
   @param  pRSSIdBm :{}  detected RSSI in dBm.
   @pre     RadioInit() must be called before this function is called.
   @code
      RIE_S8 RSSIdBm;
      if (RIE_Response == RIE_Success)
         RIE_Response = RadioRadioGetRSSI(&RSSIdBm);
   @endcode
   @return  RIE_Responses  Error code
**/
RIE_Responses RadioRadioGetRSSI (RIE_S8 *pRSSIdBm)
{
   RIE_Responses  Response     = RIE_Success;

   if(Response == RIE_Success)
      Response = RadioCommitRadioConfig();
   if (Response == RIE_Success)
      Response = RadioToOnMode();
   if (Response == RIE_Success)
      Response = RadioSendCommandWait(CMD_GET_RSSI);
   if (Response == RIE_Success)
      Response = RadioSyncComms(); //
   if (pRSSIdBm)
      {
      if (Response == RIE_Success)
         Response = RadioMMapRead(MCR_rssi_readback_Adr,0x1, (RIE_U8 *)pRSSIdBm);
      *pRSSIdBm -= 107; // Convert to dBm
      }
   return Response;
}

/**
   @fn     RIE_Responses RadioTxSetPower(RIE_PAPowerLevel Power)
   @brief  Set the Transmit Power Level for Radio Transmission.
   @param  Power :{PowerLevel0 ,PowerLevel1 ,PowerLevel2 ,PowerLevel3,
           PowerLevel4 ,PowerLevel5 ,PowerLevel6 ,PowerLevel7,
           PowerLevel8 ,PowerLevel9 ,PowerLevel10,PowerLevel11,
           PowerLevel12,PowerLevel13,PowerLevel14,PowerLevel15}
   @pre    RadioInit() must be called before this function is called.
   @code
      Response = RadioTxSetPower(PowerLevel8);
   @endcode
   @note   Max TX Power is used by default.
   @return  RIE_Responses  Error code
*/
RIE_Responses RadioTxSetPower (RIE_PAPowerLevel Power)
{
   RIE_Responses  Response     = RIE_Success;
   RIE_U8         ucNewRegVal  = RadioConfiguration.radio_cfg_8_r;
   unsigned long  pa_level_mcr,pa_ramp, codes_per_bit,min_codes_per_bit;

   if (RadioConfiguration.radio_cfg_8_r & radio_cfg_8_pa_single_diff_sel_differential)
      ucNewRegVal  = radio_cfg_8_pa_single_diff_sel_differential;
   else
      ucNewRegVal  = radio_cfg_8_pa_single_diff_sel_single_ended;

   if(Response == RIE_Success)
      {
      switch (Power)
         {
         case PowerLevel0 :
         case PowerLevel1 :
         case PowerLevel2 :
         case PowerLevel3 :
         case PowerLevel4 :
         case PowerLevel5 :
         case PowerLevel6 :
         case PowerLevel7 :
         case PowerLevel8 :
         case PowerLevel9 :
         case PowerLevel10:
         case PowerLevel11:
         case PowerLevel12:
         case PowerLevel13:
         case PowerLevel14:
         case PowerLevel15:
            ucNewRegVal |= ((RIE_U8)Power << radio_cfg_8_pa_power_offset);
            // Calculate the minimum allowable codes per bit
            pa_level_mcr =  (((RIE_U8)Power)* 4) + 0x3;
            min_codes_per_bit = (pa_level_mcr * 2500)/(DataRate/100);
            pa_ramp        = 0x1;
            codes_per_bit  = 256;
            while (codes_per_bit > min_codes_per_bit)
               {
               pa_ramp++;
               codes_per_bit = 512 >> pa_ramp;
               if (pa_ramp >= 7)
                  break; // This is the maximum
               }
            ucNewRegVal |= ((RIE_U8)pa_ramp << radio_cfg_8_pa_ramp_offset);
            break;
         default:
            Response   = RIE_UnsupportedRadioConfig;
            break;
         }
      }

   if(Response == RIE_Success)
      {
      if (ucNewRegVal != RadioConfiguration.radio_cfg_8_r )
         {
         // Write directly to the MCR in this case and avoid a reconfigure
         if (Response == RIE_Success)
            Response = RadioMMapWrite(MCR_pa_level_mcr_Adr, 0x1, (RIE_U8 *)&ucNewRegVal);
         RadioConfiguration.radio_cfg_8_r  = ucNewRegVal;
         }
      }
   return Response;
}


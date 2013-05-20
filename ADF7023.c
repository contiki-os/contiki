/***************************************************************************//**
 *   @file   ADF7023.c
 *   @brief  Implementation of ADF7023 Driver.
 *   @author DBogdan (Dragos.Bogdan@analog.com)
********************************************************************************
 * Copyright 2013(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: $WCREV$
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ADF7023.h"
#include "ADF7023_Config.h"
#include "Communication.h"

/******************************************************************************/
/*************************** Macros Definitions *******************************/
/******************************************************************************/
#define ADF7023_CS_ASSERT   CS_PIN_LOW
#define ADF7023_CS_DEASSERT CS_PIN_HIGH
#define ADF7023_MISO        MISO_PIN

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
struct ADF7023_BBRAM ADF7023_BBRAMCurrent;

/***************************************************************************//**
 * @brief Transfers one byte of data.
 *
 * @param writeByte - Write data.
 * @param readByte - Read data.
 *
 * @return None.
*******************************************************************************/
void ADF7023_WriteReadByte(unsigned char writeByte,
                           unsigned char* readByte)
{
    unsigned char data = 0;
    
    data = writeByte;
    SPI_Read(0, &data, 1);
    if(readByte)
    {
        *readByte = data;
    }
}

/***************************************************************************//**
 * @brief Initializes the ADF7023.
 *
 * @return retVal - Result of the initialization procedure.
 *                  Example: 0 - if initialization was successful;
 *                           -1 - if initialization was unsuccessful.
*******************************************************************************/
char ADF7023_Init(void)
{
    char           retVal  = 0;
    unsigned char  miso    = 0;
    unsigned short timeout = 0;
    unsigned char  status  = 0;
    
    ADF7023_BBRAMCurrent = ADF7023_BBRAMDefault;
    SPI_Init(0,         // MSB first.
             1000000,   // Clock frequency.
             0,         // Idle state for clock is a high level; active state is a low level.
             1);        // Serial output data changes on transition from idle clock state to active clock state.
    ADF7023_CS_ASSERT;
    while ((miso == 0) && (timeout < 1000))
    {
        miso = ADF7023_MISO;
        timeout++;
    }
    if(timeout == 1000)
    {
        retVal = -1;
    }
    while(!(status & STATUS_CMD_READY))
    {
        ADF7023_GetStatus(&status);
    }
    ADF7023_SetRAM(0x100, 64, (unsigned char*)&ADF7023_BBRAMCurrent);
    ADF7023_SetCommand(CMD_CONFIG_DEV);
    
    return retVal;
}

/***************************************************************************//**
 * @brief Reads the status word of the ADF7023.
 *
 * @param status - Status word.
 *
 * @return None.
*******************************************************************************/
void ADF7023_GetStatus(unsigned char* status)
{
    ADF7023_CS_ASSERT;
    ADF7023_WriteReadByte(SPI_NOP, 0);
    ADF7023_WriteReadByte(SPI_NOP, status);
    ADF7023_CS_DEASSERT;
}

/***************************************************************************//**
 * @brief Initiates a command.
 *
 * @param command - Command.
 *
 * @return None.
*******************************************************************************/
void ADF7023_SetCommand(unsigned char command)
{
    ADF7023_CS_ASSERT;
    ADF7023_WriteReadByte(command, 0);
    ADF7023_CS_DEASSERT;
}

/***************************************************************************//**
 * @brief Sets a FW state and waits until the device enters in that state.
 *
 * @param fwState - FW state.
 *
 * @return None.
*******************************************************************************/
void ADF7023_SetFwState(unsigned char fwState)
{
    unsigned char status = 0;
    
    switch(fwState)
    {
        case FW_STATE_PHY_OFF:
            ADF7023_SetCommand(CMD_PHY_OFF);
            break;
        case FW_STATE_PHY_ON:
            ADF7023_SetCommand(CMD_PHY_ON);
            break;
        case FW_STATE_PHY_RX:
            ADF7023_SetCommand(CMD_PHY_RX);
            break;
        case FW_STATE_PHY_TX:
            ADF7023_SetCommand(CMD_PHY_TX);
            break;
        default:
            ADF7023_SetCommand(CMD_PHY_SLEEP);
    }
    while((status & STATUS_FW_STATE) != fwState)
    {
        ADF7023_GetStatus(&status);
    }
}

/***************************************************************************//**
 * @brief Reads data from the RAM.
 *
 * @param address - Start address.
 * @param length - Number of bytes to write.
 * @param data - Read buffer.
 *
 * @return None.
*******************************************************************************/
void ADF7023_GetRAM(unsigned long address,
                    unsigned long length,
                    unsigned char* data)
{
    ADF7023_CS_ASSERT;
    ADF7023_WriteReadByte(SPI_MEM_RD | ((address & 0x700) >> 8), 0);
    ADF7023_WriteReadByte(address & 0xFF, 0);
    ADF7023_WriteReadByte(SPI_NOP, 0);
    while(length--)
    {
        ADF7023_WriteReadByte(SPI_NOP, data++);
    }
    ADF7023_CS_DEASSERT;
}

/***************************************************************************//**
 * @brief Writes data to RAM.
 *
 * @param address - Start address.
 * @param length - Number of bytes to write.
 * @param data - Write buffer.
 *
 * @return None.
*******************************************************************************/
void ADF7023_SetRAM(unsigned long address,
                    unsigned long length,
                    unsigned char* data)
{
    ADF7023_CS_ASSERT;
    ADF7023_WriteReadByte(SPI_MEM_WR | ((address & 0x700) >> 8), 0);
    ADF7023_WriteReadByte(address & 0xFF, 0);
    while(length--)
    {
        ADF7023_WriteReadByte(*(data++), 0);
    }
    ADF7023_CS_DEASSERT;
}

/***************************************************************************//**
 * @brief Receives one packet.
 *
 * @param packet - Data buffer.
 * @param length - Number of received bytes.
 *
 * @return None.
*******************************************************************************/
void ADF7023_ReceivePacket(unsigned char* packet, unsigned char* length)
{
    unsigned char interruptReg = 0;
    
    ADF7023_SetFwState(FW_STATE_PHY_ON);
    ADF7023_SetFwState(FW_STATE_PHY_RX);
    while(!(interruptReg & BBRAM_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT))
    {
        ADF7023_GetRAM(MCR_REG_INTERRUPT_SOURCE_0,
                       0x1,
                       &interruptReg);
    }
    ADF7023_SetRAM(MCR_REG_INTERRUPT_SOURCE_0,
                   0x1,
                   &interruptReg);
    ADF7023_GetRAM(0x10, 1, length);
    ADF7023_GetRAM(0x12, *length - 2, packet);
}

/***************************************************************************//**
 * @brief Transmits one packet.
 *
 * @param packet - Data buffer.
 * @param length - Number of bytes to transmit.
 *
 * @return None.
*******************************************************************************/
void ADF7023_TransmitPacket(unsigned char* packet, unsigned char length)
{
    unsigned char interruptReg = 0;
    unsigned char header[2]    = {0, 0};
    
    header[0] = 2 + length;
    header[1] = ADF7023_BBRAMCurrent.addressMatchOffset;
    ADF7023_SetRAM(0x10, 2, header);
    ADF7023_SetRAM(0x12, length, packet);
    ADF7023_SetFwState(FW_STATE_PHY_ON);
    ADF7023_SetFwState(FW_STATE_PHY_TX);
    while(!(interruptReg & BBRAM_INTERRUPT_MASK_0_INTERRUPT_TX_EOF))
    {
        ADF7023_GetRAM(MCR_REG_INTERRUPT_SOURCE_0,
                       0x1,
                       &interruptReg);
    }
}

/***************************************************************************//**
 * @brief Sets the channel frequency.
 *
 * @param chFreq - Channel frequency.
 *
 * @return None.
*******************************************************************************/
void ADF7023_SetChannelFrequency(unsigned long chFreq)
{
    chFreq = (unsigned long)(((float)chFreq / 26000000) * 65535);
    ADF7023_BBRAMCurrent.channelFreq0 = (chFreq & 0x0000FF) >> 0;
    ADF7023_BBRAMCurrent.channelFreq1 = (chFreq & 0x00FF00) >> 8;
    ADF7023_BBRAMCurrent.channelFreq2 = (chFreq & 0xFF0000) >> 16;
    ADF7023_SetRAM(0x100, 64, (unsigned char*)&ADF7023_BBRAMCurrent);
}

/***************************************************************************//**
 * @brief Sets the data rate.
 *
 * @param dataRate - Data rate.
 *
 * @return None.
*******************************************************************************/
void ADF7023_SetDataRate(unsigned long dataRate)
{
    unsigned char status = 0;
    
    dataRate = (unsigned long)(dataRate / 100);
    ADF7023_BBRAMCurrent.radioCfg0 = 
        BBRAM_RADIO_CFG_0_DATA_RATE_7_0((dataRate & 0x00FF) >> 0);
    ADF7023_BBRAMCurrent.radioCfg1 &= ~BBRAM_RADIO_CFG_1_DATA_RATE_11_8(0xF);
    ADF7023_BBRAMCurrent.radioCfg1 |= 
        BBRAM_RADIO_CFG_1_DATA_RATE_11_8((dataRate & 0x0F00) >> 8);
    ADF7023_SetRAM(0x100, 64, (unsigned char*)&ADF7023_BBRAMCurrent);
    ADF7023_SetFwState(FW_STATE_PHY_OFF);
    ADF7023_SetCommand(CMD_CONFIG_DEV);
}

/***************************************************************************//**
 * @brief Sets the frequency deviation.
 *
 * @param freqDev - Frequency deviation.
 *
 * @return None.
*******************************************************************************/
void ADF7023_SetFrequencyDeviation(unsigned long freqDev)
{
    unsigned char status = 0;
    
    freqDev = (unsigned long)(freqDev / 100);
    ADF7023_BBRAMCurrent.radioCfg1 &= 
        ~BBRAM_RADIO_CFG_1_FREQ_DEVIATION_11_8(0xF);
    ADF7023_BBRAMCurrent.radioCfg1 |=
        BBRAM_RADIO_CFG_1_FREQ_DEVIATION_11_8((freqDev & 0x0F00) >> 8);
    ADF7023_BBRAMCurrent.radioCfg2 =
        BBRAM_RADIO_CFG_2_FREQ_DEVIATION_7_0((freqDev & 0x00FF) >> 0);
    ADF7023_SetRAM(0x100, 64, (unsigned char*)&ADF7023_BBRAMCurrent);
    ADF7023_SetFwState(FW_STATE_PHY_OFF);
    ADF7023_SetCommand(CMD_CONFIG_DEV);
}

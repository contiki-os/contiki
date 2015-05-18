/*
 * Copyright (c) 2014, Analog Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \author Dragos Bogdan <Dragos.Bogdan@Analog.com>, Ian Martin <martini@redwirellc.com>
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>  /* for memcmp(). */
#include <stdbool.h>

#include "ADF7023.h"
#include "ADF7023_Config.h"
#include "Communication.h"

#include "sfrs.h"
#include "sfrs-ext.h"

#include "contiki.h"        /* for clock_wait() and CLOCK_SECOND. */
#include "sys/cc.h"

/******************************************************************************/
/*************************** Macros Definitions *******************************/
/******************************************************************************/
/*
   #define ADF7023_CS_ASSERT   CS_PIN_LOW
   #define ADF7023_CS_DEASSERT CS_PIN_HIGH
   #define ADF7023_MISO        MISO_PIN
 */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#undef BIT
#define BIT(n) (1 << (n))

#define ADF7023_CS_ASSERT   (P2 &= ~BIT(2))
#define ADF7023_CS_DEASSERT (P2 |= BIT(2))
#define ADF7023_MISO        (P0 & BIT(3))

#define ADF7023_SPI_BUS     (CSI10)

#define LOOP_LIMIT 100

#ifndef ADF7023_VERBOSE
/* ADF7023_VERBOSE Values: */
/*  2 = Inidicate when breaking stuck while loops. */
/*  5 = Dump all received and transmitted packets. */
/*  7 = Dump the ADF7023 commands, interrupt and status words. */
/* 10 = Dump all SPI transactions. */

#define ADF7023_VERBOSE 0
#endif

#if (ADF7023_VERBOSE >= 2)
#define break_loop() if(++counter >= LOOP_LIMIT) { printf("Breaking stuck while loop at %s line %u." NEWLINE, __FILE__, __LINE__); break; }
#else
#define break_loop() if(++counter >= LOOP_LIMIT) break
#endif

#define ADF7023_While(condition, body) do { \
    int counter = 0; \
    while(condition) { body; break_loop(); } \
} while(0)

/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
struct ADF7023_BBRAM ADF7023_BBRAMCurrent;

#if (ADF7023_VERBOSE >= 7)
static unsigned char status_old = 0xff;
static unsigned char int0_old = 0xff;
#endif

const char *ADF7023_state_lookup[] = {
  /* 0x00 */ "Busy, performing a state transition",
  /* 0x01 */ "(unknown)",
  /* 0x02 */ "(unknown)",
  /* 0x03 */ "(unknown)",
  /* 0x04 */ "(unknown)",
  /* 0x05 */ "Performing CMD_GET_RSSI",
  /* 0x06 */ "PHY_SLEEP",
  /* 0x07 */ "Performing CMD_IR_CAL",
  /* 0x08 */ "Performing CMD_AES_DECRYPT_INIT",
  /* 0x09 */ "Performing CMD_AES_DECRYPT",
  /* 0x0A */ "Performing CMD_AES_ENCRYPT",
  /* 0x0B */ "(unknown)",
  /* 0x0C */ "(unknown)",
  /* 0x0D */ "(unknown)",
  /* 0x0E */ "(unknown)",
  /* 0x0F */ "Initializing",
  /* 0x10 */ "(unknown)",
  /* 0x11 */ "PHY_OFF",
  /* 0x12 */ "PHY_ON",
  /* 0x13 */ "PHY_RX",
  /* 0x14 */ "PHY_TX",
};

const char *ADF7023_cmd_lookup[] = {
  [CMD_SYNC] = "CMD_SYNC",
  [CMD_PHY_OFF] = "CMD_PHY_OFF",
  [CMD_PHY_ON] = "CMD_PHY_ON",
  [CMD_PHY_RX] = "CMD_PHY_RX",
  [CMD_PHY_TX] = "CMD_PHY_TX",
  [CMD_PHY_SLEEP] = "CMD_PHY_SLEEP",
  [CMD_CONFIG_DEV] = "CMD_CONFIG_DEV",
  [CMD_GET_RSSI] = "CMD_GET_RSSI",
  [CMD_BB_CAL] = "CMD_BB_CAL",
  [CMD_HW_RESET] = "CMD_HW_RESET",
  [CMD_RAM_LOAD_INIT] = "CMD_RAM_LOAD_INIT",
  [CMD_RAM_LOAD_DONE] = "CMD_RAM_LOAD_DONE",
  [CMD_IR_CAL] = "CMD_IR_CAL",
  [CMD_AES_ENCRYPT] = "CMD_AES_ENCRYPT",
  [CMD_AES_DECRYPT] = "CMD_AES_DECRYPT",
  [CMD_AES_DECRYPT_INIT] = "CMD_AES_DECRYPT_INIT",
  [CMD_RS_ENCODE_INIT] = "CMD_RS_ENCODE_INIT",
  [CMD_RS_ENCODE] = "CMD_RS_ENCODE",
  [CMD_RS_DECODE] = "CMD_RS_DECODE",
};

static int spi_busy = 0;
static uint8_t tx_rec[255];
static uint8_t rx_rec[255];
static uint8_t tx_pos;
static uint8_t rx_pos;

static void ADF7023_SetCommand_Assume_CMD_READY(unsigned char command);

void
hexdump(const void *data, size_t len)
{
  size_t n;
  if(len <= 0) {
    return;
  }
  printf("%02x", ((const unsigned char *)data)[0]);
  for(n = 1; n < len; n++) {
    printf(" %02x", ((const unsigned char *)data)[n]);
  }
}
void
ADF7023_SPI_Begin(void)
{
  assert(spi_busy == 0);
  spi_busy++;

  tx_pos = 0;
  rx_pos = 0;

  ADF7023_CS_ASSERT;
}
void
ADF7023_SPI_End(void)
{
  assert(spi_busy > 0);
  spi_busy--;
  ADF7023_CS_DEASSERT;

#if (ADF7023_VERBOSE >= 10)
  printf("ADF7023_SPI_End(): wrote \"");
  hexdump(tx_rec, tx_pos);
  printf("\", read \"");
  hexdump(rx_rec, rx_pos);
  printf("\"." NEWLINE);
#endif
}
/***************************************************************************//**
 * @brief Transfers one byte of data.
 *
 * @param writeByte - Write data.
 * @param readByte - Read data.
 *
 * @return None.
 *******************************************************************************/
void
ADF7023_WriteReadByte(unsigned char writeByte,
                      unsigned char *readByte)
{
  unsigned char data = 0;

  data = writeByte;
  SPI_Read(ADF7023_SPI_BUS, 0, &data, 1);
  if(readByte) {
    *readByte = data;
  }

  assert(tx_pos < ARRAY_SIZE(tx_rec));
  tx_rec[tx_pos] = writeByte;
  tx_pos++;

  assert(rx_pos < ARRAY_SIZE(rx_rec));
  rx_rec[rx_pos] = data;
  rx_pos++;
}
void
ADF7023_Wait_for_CMD_READY(void)
{
  unsigned char status;
  int counter = 0;

  for(;;) {
    break_loop();

    ADF7023_GetStatus(&status);

    if((status & STATUS_SPI_READY) == 0) {
      /* The SPI bus is not ready. Continue polling the status word. */
      continue;
    }

    if(status & STATUS_CMD_READY) {
      /* The SPI bus is ready and CMD_READY == 1. This is the state we want. */
      break;
    }

    if((status & STATUS_FW_STATE) == FW_STATE_PHY_OFF) {
      /* SPI is ready, but CMD_READY == 0 and the radio is in state PHY_OFF. */
      /* It seems that the ADF7023 gets stuck in this state sometimes (errata?), so transition to PHY_ON: */
      ADF7023_SetCommand_Assume_CMD_READY(CMD_PHY_ON);
    }
  }
}
static void
ADF7023_Init_Procedure(void)
{
  ADF7023_SPI_Begin();
  ADF7023_While(!ADF7023_MISO, (void)0);
  ADF7023_SPI_End();
  ADF7023_Wait_for_CMD_READY();
}
/***************************************************************************//**
 * @brief Initializes the ADF7023.
 *
 * @return retVal - Result of the initialization procedure.
 *                  Example: 0 - if initialization was successful;
 *                           -1 - if initialization was unsuccessful.
 *******************************************************************************/
char
ADF7023_Init(void)
{
  char retVal = 0;

  ADF7023_CS_DEASSERT;
  PM2 &= ~BIT(2);      /* Configure ADF7023_CS as an output. */

  ADF7023_BBRAMCurrent = ADF7023_BBRAMDefault;
  SPI_Init(ADF7023_SPI_BUS,
           0,       /* MSB first. */
           1000000,     /* Clock frequency. */
           0,           /* Idle state for clock is a high level; active state is a low level. */
           1);          /* Serial output data changes on transition from idle clock state to active clock state. */

  ADF7023_Init_Procedure();

  ADF7023_SetCommand(CMD_HW_RESET);
  clock_wait(MIN(CLOCK_SECOND / 1000, 1));
  ADF7023_Init_Procedure();

  ADF7023_SetRAM_And_Verify(0x100, 64, (unsigned char *)&ADF7023_BBRAMCurrent);
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
void
ADF7023_GetStatus(unsigned char *status)
{
  ADF7023_SPI_Begin();
  ADF7023_WriteReadByte(SPI_NOP, 0);
  ADF7023_WriteReadByte(SPI_NOP, status);
  ADF7023_SPI_End();

#if (ADF7023_VERBOSE >= 7)
  if(*status != status_old) {
    printf("ADF7023_GetStatus: SPI_READY=%u, IRQ_STATUS=%u, CMD_READY=%u, FW_STATE=0x%02x",
           (*status >> 7) & 1,
           (*status >> 6) & 1,
           (*status >> 5) & 1,
           *status & STATUS_FW_STATE
           );
    if((*status & STATUS_FW_STATE) < ARRAY_SIZE(ADF7023_state_lookup)) {
      printf("=\"%s\"", ADF7023_state_lookup[*status & STATUS_FW_STATE]);
    }
    printf("." NEWLINE);
    status_old = *status;
  }
#endif
}
static void
ADF7023_SetCommand_Assume_CMD_READY(unsigned char command)
{
#if (ADF7023_VERBOSE >= 7)
  assert(ADF7023_cmd_lookup[command] != NULL);
  printf("Sending command 0x%02x = \"%s\"." NEWLINE, command, ADF7023_cmd_lookup[command]);
#endif
  ADF7023_SPI_Begin();
  ADF7023_WriteReadByte(command, 0);
  ADF7023_SPI_End();
}
/***************************************************************************//**
 * @brief Initiates a command.
 *
 * @param command - Command.
 *
 * @return None.
 *******************************************************************************/
void
ADF7023_SetCommand(unsigned char command)
{
  ADF7023_Wait_for_CMD_READY();
  ADF7023_SetCommand_Assume_CMD_READY(command);
}
void
ADF7023_SetFwState_NoWait(unsigned char fwState)
{
  switch(fwState) {
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
}
/***************************************************************************//**
 * @brief Sets a FW state and waits until the device enters in that state.
 *
 * @param fwState - FW state.
 *
 * @return None.
 *******************************************************************************/
void
ADF7023_SetFwState(unsigned char fwState)
{
  unsigned char status = 0;
  ADF7023_SetFwState_NoWait(fwState);
  ADF7023_While((status & STATUS_FW_STATE) != fwState, ADF7023_GetStatus(&status));
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
void
ADF7023_GetRAM(unsigned long address,
               unsigned long length,
               unsigned char *data)
{
  ADF7023_SPI_Begin();
  ADF7023_WriteReadByte(SPI_MEM_RD | ((address & 0x700) >> 8), 0);
  ADF7023_WriteReadByte(address & 0xFF, 0);
  ADF7023_WriteReadByte(SPI_NOP, 0);
  while(length--) {
    ADF7023_WriteReadByte(SPI_NOP, data++);
  }
  ADF7023_SPI_End();
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
void
ADF7023_SetRAM(unsigned long address,
               unsigned long length,
               unsigned char *data)
{
  ADF7023_Wait_for_CMD_READY();

  ADF7023_SPI_Begin();
  ADF7023_WriteReadByte(SPI_MEM_WR | ((address & 0x700) >> 8), 0);
  ADF7023_WriteReadByte(address & 0xFF, 0);
  while(length--) {
    ADF7023_WriteReadByte(*(data++), 0);
  }
  ADF7023_SPI_End();
}
void
ADF7023_SetRAM_And_Verify(unsigned long address, unsigned long length, unsigned char *data)
{
  unsigned char readback[256];

  ADF7023_SetRAM(address, length, data);

  assert(length <= sizeof(readback));
  if(length > sizeof(readback)) {
    return;
  }
  ADF7023_GetRAM(address, length, readback);

  if(memcmp(data, readback, length)) {
    printf("ADF7023_SetRAM_And_Verify failed. Wrote:" NEWLINE);
    hexdump(data, length);
    printf(NEWLINE "Read:" NEWLINE);
    hexdump(readback, length);
    printf(NEWLINE);
  }
}
unsigned char
ADF7023_Wait_for_SPI_READY(void)
{
  unsigned char status = 0;
  ADF7023_While((status & STATUS_SPI_READY) == 0, ADF7023_GetStatus(&status));
  return status; /* Return the status -- why not? */
}
void
ADF7023_PHY_ON(void)
{
  unsigned char status;
  unsigned int counter = 0;

  for(;;) {
    status = ADF7023_Wait_for_SPI_READY();

    switch(status & STATUS_FW_STATE) {
    default:
      ADF7023_SetCommand(CMD_PHY_ON);
      break;

    case FW_STATE_BUSY:
      /* Wait! */
      break;

    case FW_STATE_PHY_ON:
      /* This is the desired state. */
      return;
    }

    break_loop();
  }
}
void
ADF7023_PHY_RX(void)
{
  unsigned char status;
  unsigned int counter = 0;

  for(;;) {
    status = ADF7023_Wait_for_SPI_READY();

    switch(status & STATUS_FW_STATE) {
    default:
      /* Need to turn the PHY_ON. */
      ADF7023_PHY_ON();
      break;

    case FW_STATE_BUSY:
      /* Wait! */
      break;

    case FW_STATE_PHY_ON:
    case FW_STATE_PHY_TX:
      ADF7023_While((status & STATUS_CMD_READY) == 0, ADF7023_GetStatus(&status));
      ADF7023_SetCommand(CMD_PHY_RX);
      return;

    case FW_STATE_PHY_RX:
      /* This is the desired state. */
      return;
    }

    break_loop();
  }
}
void
ADF7023_PHY_TX(void)
{
  unsigned char status;
  unsigned int counter = 0;

  for(;;) {
    status = ADF7023_Wait_for_SPI_READY();

    switch(status & STATUS_FW_STATE) {
    default:
      /* Need to turn the PHY_ON. */
      ADF7023_PHY_ON();
      break;

    case FW_STATE_BUSY:
      /* Wait! */
      break;

    case FW_STATE_PHY_ON:
    case FW_STATE_PHY_RX:
      ADF7023_While((status & STATUS_CMD_READY) == 0, ADF7023_GetStatus(&status));
      ADF7023_SetCommand(CMD_PHY_TX);
      return;
    }

    break_loop();
  }
}
static unsigned char
ADF7023_ReadInterruptSource(void)
{
  unsigned char interruptReg;

  ADF7023_GetRAM(MCR_REG_INTERRUPT_SOURCE_0, 0x1, &interruptReg);

#if (ADF7023_VERBOSE >= 7)
  if(interruptReg != int0_old) {
    printf("ADF7023_ReadInterruptSource: %u%u%u%u%u%u%u%u." NEWLINE,
           (interruptReg >> 7) & 1,
           (interruptReg >> 6) & 1,
           (interruptReg >> 5) & 1,
           (interruptReg >> 4) & 1,
           (interruptReg >> 3) & 1,
           (interruptReg >> 2) & 1,
           (interruptReg >> 1) & 1,
           (interruptReg >> 0) & 1
           );
    int0_old = interruptReg;
  }
#endif
  return interruptReg;
}
unsigned char
ADF7023_ReceivePacketAvailable(void)
{
  unsigned char status;

  ADF7023_GetStatus(&status);
  if((status & STATUS_SPI_READY) == 0) {
    return false;
  }

  if((status & STATUS_FW_STATE) != FW_STATE_PHY_RX) {
    ADF7023_PHY_RX();
    return false;
  }

  if((status & STATUS_IRQ_STATUS) == 0) {
    return false;
  }

  return ADF7023_ReadInterruptSource() & BBRAM_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT;
}
/***************************************************************************//**
 * @brief Receives one packet.
 *
 * @param packet - Data buffer.
 * @param length - Number of received bytes.
 *
 * @return None.
 *******************************************************************************/
void
ADF7023_ReceivePacket(unsigned char *packet, unsigned char *payload_length)
{
  unsigned char length;
  unsigned char interruptReg = 0;

  ADF7023_While(!(interruptReg & BBRAM_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT),
                interruptReg = ADF7023_ReadInterruptSource());

  interruptReg = BBRAM_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT;

  ADF7023_SetRAM(MCR_REG_INTERRUPT_SOURCE_0,
                 0x1,
                 &interruptReg);

  ADF7023_GetRAM(ADF7023_RX_BASE_ADR, 1, &length);

  *payload_length = length - 1 + LENGTH_OFFSET - 4;

  ADF7023_GetRAM(ADF7023_RX_BASE_ADR + 1, *payload_length, packet);

#if (ADF7023_VERBOSE >= 5)
  do {
    unsigned char n;
    printf("ADF7023_ReceivePacket, length=%u: ", *payload_length);
    hexdump(packet, *payload_length);
    printf(NEWLINE);
  } while(false);
#endif
}
/***************************************************************************//**
 * @brief Transmits one packet.
 *
 * @param packet - Data buffer.
 * @param length - Number of bytes to transmit.
 *
 * @return None.
 *******************************************************************************/
void
ADF7023_TransmitPacket(unsigned char *packet, unsigned char length)
{
  unsigned char interruptReg = 0;
  unsigned char status;
  unsigned char length_plus_one;

  for(;;) {
    ADF7023_GetStatus(&status);
    if((status & STATUS_SPI_READY) == 0) {
      continue;
    }
    if((status & STATUS_CMD_READY) == 0) {
      continue;
    }
    break;
  }

  length_plus_one = length + 1;
  ADF7023_SetRAM_And_Verify(ADF7023_TX_BASE_ADR, 1, &length_plus_one);
  ADF7023_SetRAM_And_Verify(ADF7023_TX_BASE_ADR + 1, length, packet);

#if (ADF7023_VERBOSE >= 5)
  do {
    unsigned char n;
    printf("ADF7023_TransmitPacket, length=%u: ", length);
    hexdump(packet, length);
    printf(NEWLINE);
  } while(false);
#endif

  ADF7023_PHY_TX();

  ADF7023_While(!(interruptReg & BBRAM_INTERRUPT_MASK_0_INTERRUPT_TX_EOF),
                ADF7023_GetRAM(MCR_REG_INTERRUPT_SOURCE_0, 0x1, &interruptReg));

  ADF7023_PHY_RX();
}
/***************************************************************************//**
 * @brief Sets the channel frequency.
 *
 * @param chFreq - Channel frequency.
 *
 * @return None.
 *******************************************************************************/
void
ADF7023_SetChannelFrequency(unsigned long chFreq)
{
  chFreq = (unsigned long)(((float)chFreq / 26000000) * 65535);
  ADF7023_BBRAMCurrent.channelFreq0 = (chFreq & 0x0000FF) >> 0;
  ADF7023_BBRAMCurrent.channelFreq1 = (chFreq & 0x00FF00) >> 8;
  ADF7023_BBRAMCurrent.channelFreq2 = (chFreq & 0xFF0000) >> 16;
  ADF7023_SetRAM_And_Verify(0x100, 64, (unsigned char *)&ADF7023_BBRAMCurrent);
}
/***************************************************************************//**
 * @brief Sets the data rate.
 *
 * @param dataRate - Data rate.
 *
 * @return None.
 *******************************************************************************/
void
ADF7023_SetDataRate(unsigned long dataRate)
{
  dataRate = (unsigned long)(dataRate / 100);
  ADF7023_BBRAMCurrent.radioCfg0 =
    BBRAM_RADIO_CFG_0_DATA_RATE_7_0((dataRate & 0x00FF) >> 0);
  ADF7023_BBRAMCurrent.radioCfg1 &= ~BBRAM_RADIO_CFG_1_DATA_RATE_11_8(0xF);
  ADF7023_BBRAMCurrent.radioCfg1 |=
    BBRAM_RADIO_CFG_1_DATA_RATE_11_8((dataRate & 0x0F00) >> 8);
  ADF7023_SetRAM_And_Verify(0x100, 64, (unsigned char *)&ADF7023_BBRAMCurrent);
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
void
ADF7023_SetFrequencyDeviation(unsigned long freqDev)
{
  freqDev = (unsigned long)(freqDev / 100);
  ADF7023_BBRAMCurrent.radioCfg1 &=
    ~BBRAM_RADIO_CFG_1_FREQ_DEVIATION_11_8(0xF);
  ADF7023_BBRAMCurrent.radioCfg1 |=
    BBRAM_RADIO_CFG_1_FREQ_DEVIATION_11_8((freqDev & 0x0F00) >> 8);
  ADF7023_BBRAMCurrent.radioCfg2 =
    BBRAM_RADIO_CFG_2_FREQ_DEVIATION_7_0((freqDev & 0x00FF) >> 0);
  ADF7023_SetRAM_And_Verify(0x100, 64, (unsigned char *)&ADF7023_BBRAMCurrent);
  ADF7023_SetFwState(FW_STATE_PHY_OFF);
  ADF7023_SetCommand(CMD_CONFIG_DEV);
}

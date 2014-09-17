/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         1-wire driver using hardware UART as a bus master.
 *
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 *
 * \note
 *         This is heavily based on Maxim document TUTORIAL 214
 *         "Using a UART to Implement a 1-Wire Bus Master"
 *
 * \note Nothing in this driver is thread safe, make sure to call all ow_*
 *       functions from a single thread or things will break.
 *
 * \todo Use FIFO if UART supports it (UART0, UART1).
 * \todo Low power wait/stop during sends in 1-wire driver.
 */

#include "onewire.h"
#include "onewire-crc-table.h"
#include "interrupt.h"
#include "udelay.h"
#include "K60.h"
#include "config-clocks.h"
#include <stdio.h>
#include <stdint.h>

#define ONEWIRE_UART UART4
#define ONEWIRE_ISR_FUNC _isr_uart4_status
#define ONEWIRE_IRQn UART4_RX_TX_IRQn
#define ONEWIRE_UART_MODULE_FREQUENCY F_BUS
#define ONEWIRE_RXPIN_PCR PORTE->PCR[25]
#define ONEWIRE_TXPIN_PCR PORTE->PCR[24]

/* TX pin port clock gate mask */
#define ONEWIRE_TXPIN_PORT_CG SIM_SCGC5_PORTE_MASK
/* RX pin port clock gate mask */
#define ONEWIRE_RXPIN_PORT_CG SIM_SCGC5_PORTE_MASK

/* Data bytes used in the UART to generate the correct 1-wire waveforms. */
#define ONEWIRE_D_RESET (0xF0)
#define ONEWIRE_D_READ (0xFF)
#define ONEWIRE_D_WRITE_0 (0x00)
#define ONEWIRE_D_WRITE_1 (0xFF)

/* Length in bytes of the 1-wire ROM code */
#define OW_ROM_ID_LENGTH (8)

static volatile const uint8_t *ow_write_bytes_buf;
static volatile uint8_t ow_write_bytes_count;
static volatile uint8_t *ow_read_bytes_buf;
static volatile uint8_t ow_read_bytes_count;
static volatile uint8_t ow_write_bits;
static volatile uint8_t ow_read_bits;
static volatile uint8_t ow_isr_scratch;

#define OW_TIMING_FAST() \
  ONEWIRE_UART->BDH = UART_BDH_SBR(UART_SBR(ONEWIRE_UART_MODULE_FREQUENCY, ONEWIRE_UART_BAUD_FAST) / 256); \
  ONEWIRE_UART->BDL = UART_BDL_SBR(UART_SBR(ONEWIRE_UART_MODULE_FREQUENCY, ONEWIRE_UART_BAUD_FAST) % 256); \
  ONEWIRE_UART->C4 = UART_C4_BRFA(UART_BRFA(ONEWIRE_UART_MODULE_FREQUENCY, ONEWIRE_UART_BAUD_FAST))

#define OW_TIMING_SLOW() \
  ONEWIRE_UART->BDH = UART_BDH_SBR(UART_SBR(ONEWIRE_UART_MODULE_FREQUENCY, ONEWIRE_UART_BAUD_SLOW) / 256); \
  ONEWIRE_UART->BDL = UART_BDL_SBR(UART_SBR(ONEWIRE_UART_MODULE_FREQUENCY, ONEWIRE_UART_BAUD_SLOW) % 256); \
  ONEWIRE_UART->C4 = UART_C4_BRFA(UART_BRFA(ONEWIRE_UART_MODULE_FREQUENCY, ONEWIRE_UART_BAUD_SLOW))

#define OW_WAIT_WRITE() \
  /* Wait until the 1-wire bus is idle */ \
  while(ow_write_bytes_count > 0 || ow_read_bytes_count > 0 || ow_write_bits > 0 || ow_read_bits > 0)

#define OW_WAIT_READ() \
  /* Wait until there are no reads queued */ \
  while(ow_read_bytes_count > 0 || ow_read_bits > 0 /*|| ow_write_bytes_count > 0 || ow_write_bits > 0*/)

#define OW_BUSY_WAIT() \
  /* Wait until the last bit of the previous transmission has been transferred */ \
  while(ow_write_bytes_count > 0 || ow_read_bytes_count > 0 || ow_write_bits > 0 || ow_read_bits > 0 || !(ONEWIRE_UART->S1 & UART_S1_TC_MASK) || (ONEWIRE_UART->S2 & UART_S2_RAF_MASK))

/**
 * Used by the ISR handler to queue the next byte for transmission on the bus.
 */
void
ow_begin_next_byte(void)
{
  static volatile uint8_t dummy;
  /* Shut up GCC warnings about set but not used variable [-Wunused-but-set-variable] */
  (void)dummy;
  if(ow_write_bytes_count > 0) {
    /* Queue next byte */
    OW_TIMING_FAST();
    /* Enable transmitter */
    ONEWIRE_UART->C2 |= UART_C2_TE_MASK;
    /* Disable receiver */
    ONEWIRE_UART->C2 &= ~UART_C2_RE_MASK;
    ow_isr_scratch = (*ow_write_bytes_buf);
    ow_write_bits = 8;
    /* Move source buffer location forward */
    --ow_write_bytes_count;
    ++ow_write_bytes_buf;
    /* Disable RX interrupts */
    ONEWIRE_UART->C2 &= ~(UART_C2_RIE_MASK);
    /* Enable TX interrupts */
    ONEWIRE_UART->C2 |= UART_C2_TCIE_MASK;
  } else if(ow_read_bytes_count > 0) {
    /* Set up reading */
    OW_TIMING_FAST();
    /* Enable transmitter and receiver */
    ONEWIRE_UART->C2 |= UART_C2_TE_MASK | UART_C2_RE_MASK;
    ow_read_bits = 8;
    --ow_read_bytes_count;
    ow_isr_scratch = 0x00;

    /* Dummy read to clear out any crap left in the data buffer */
    dummy = ONEWIRE_UART->D;
    /* Flush the RX buffer before we begin */
    ONEWIRE_UART->CFIFO |= UART_CFIFO_RXFLUSH_MASK;

    /* Disable TX interrupts */
    ONEWIRE_UART->C2 &= ~(UART_C2_TIE_MASK | UART_C2_TCIE_MASK);
    /* Trigger a read slot */
    ONEWIRE_UART->D = ONEWIRE_D_READ;
    /* Enable RX interrupts */
    ONEWIRE_UART->C2 |= UART_C2_RIE_MASK;
  } else {
    /* All done! */
    /* disable interrupts */
    ONEWIRE_UART->C2 &= ~(UART_C2_TIE_MASK | UART_C2_TCIE_MASK | UART_C2_RIE_MASK);
    /* Disable transmitter and receiver */
    ONEWIRE_UART->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);

    /** \todo Trigger an event after reading 1-wire stuff */
  }
}
/**
 * This ISR handles most of the business interacting with the 1-wire bus.
 *
 * \note The implementation depends on the fact that no writes are queued before
 *       any preceding reads have completed.
 */

void __attribute__((interrupt))
ONEWIRE_ISR_FUNC(void)
{
  static volatile uint8_t data;
  static volatile uint8_t status;
  status = ONEWIRE_UART->S1;

  if(ow_write_bits > 0) {
    if(status & (UART_S1_TDRE_MASK | UART_S1_TC_MASK)) {
      ONEWIRE_UART->D = ((ow_isr_scratch & 0x01) ? ONEWIRE_D_WRITE_1 : ONEWIRE_D_WRITE_0);
      ow_isr_scratch >>= 1;
      --ow_write_bits;
    }
  } else if(ow_read_bits > 0) {
    /* Check if we got something yet */
    /* We can end up here without anything received when being called from a TX
     * interrupt in the transition from a 1-wire write to a 1-wire read */
    if(ONEWIRE_UART->RCFIFO > 0 || (status & UART_S1_RDRF_MASK)) {
      --ow_read_bits;
      ow_isr_scratch >>= 1;
      data = ONEWIRE_UART->D;
      if(data == ONEWIRE_D_READ) {
        /* We read a 1 */
        ow_isr_scratch |= (1 << 7);
      }

      if(ow_read_bits == 0) {
        /* Place output in destination buffer */
        (*ow_read_bytes_buf) = ow_isr_scratch;
        ++ow_read_bytes_buf;
        ow_begin_next_byte();
      } else {
        /* Trigger next read slot */
        ONEWIRE_UART->D = ONEWIRE_D_READ;
      }
    }
  } else {
    ow_begin_next_byte();
  }
}
/**
 * Initialize the 1-wire driver.
 *
 * This will set up the clocks, interrupts and I/O pins.
 */
void
ow_init(void)
{

  /* initialize counters */
  ow_read_bytes_buf = 0;
  ow_read_bytes_count = 0;
  ow_write_bytes_buf = 0;
  ow_write_bytes_count = 0;
  ow_isr_scratch = 0x00;
  ow_write_bits = 0;
  ow_read_bits = 0;

  /* Enable clock gate on I/O pins */
  SIM->SCGC5 |= ONEWIRE_TXPIN_PORT_CG | ONEWIRE_RXPIN_PORT_CG;
  /* Choose UART in pin mux */
  ONEWIRE_RXPIN_PCR = PORT_PCR_MUX(3);

  /* Use open drain on UART TX pin */
  /* The built in pull-up in the I/O pin is too weak (20k-50k Ohm) on TX pin,
   * you should add an external pull up of circa 5k Ohm. */
  ONEWIRE_TXPIN_PCR = PORT_PCR_MUX(3) | PORT_PCR_ODE_MASK;

  /* Enable clock gate on UART module */
  SIM->SCGC1 |= SIM_SCGC1_UART4_MASK;

  /* Make sure MSBF bit is not set */
  ONEWIRE_UART->S2 = 0;

  /* Disable transmitter and receiver */
  ONEWIRE_UART->C2 = 0;

  /* Set up UART for two wire operation. */
  /* The reason we are not using single wire mode (LOOPS=1, RSRC=1) is that
   * because of limitations in the MCU, we are reduced to half duplex and can
   * not see when the slave is pulling the line low after we release it */
  ONEWIRE_UART->C1 = 0;

  /* Disable FIFO (set buffer depth to 1) */
  ONEWIRE_UART->PFIFO = 0;

  /* Set up interrupt controller */
  NVIC_EnableIRQ(ONEWIRE_IRQn);
}
/**
 * Reset the 1-wire bus.
 *
 * A reset is always the first step in communicating with the bus slaves.
 */
void
ow_reset(void)
{
  uint8_t dummy;
  /* Shut up GCC warnings about set but not used variable [-Wunused-but-set-variable] */
  (void)dummy;

  OW_BUSY_WAIT();

  MK60_ENTER_CRITICAL_REGION();

  /*
   * The reset pulse must be a long zero, switch to slow timing to allow the reset
   * byte to stretch on for long enough.
   * ISR will switch back to fast timing when sending other stuff.
   */
  OW_TIMING_SLOW();
  /* Enable transmitter */
  ONEWIRE_UART->C2 |= UART_C2_TE_MASK;
  /* Disable receiver */
  ONEWIRE_UART->C2 &= ~UART_C2_RE_MASK;

  /* Read whatever crap was left in the data buffer */
  dummy = ONEWIRE_UART->D;
  /* Flush the RX buffer before we begin */
  ONEWIRE_UART->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
  /* Send a reset pulse */
  ONEWIRE_UART->D = ONEWIRE_D_RESET;
  MK60_LEAVE_CRITICAL_REGION();

  /* Wait until the byte has been transferred to the shift register */
  while(!(ONEWIRE_UART->S1 & UART_S1_TC_MASK));

  /*
   * Queue up an IDLE character (all 1's, no start bit or stop bit) by disabling
   * the transmitter and then reenabling it in order to let the reset pulse
   * finish before we switch baud rates.
   */

  ONEWIRE_UART->C2 &= ~(UART_C2_TE_MASK);

  ONEWIRE_UART->C2 |= UART_C2_TE_MASK;

  /* Wait until the byte has been transferred to the shift register */
  while(!(ONEWIRE_UART->S1 & UART_S1_TC_MASK));

  /* We don't check for presence pulses */
}
/**
 * Write a sequence of bytes to the 1-wire bus.
 *
 * The transmission to the 1-wire bus is asynchronous, which means this function
 * will return before all bits have gone out the wire.
 *
 * \note As src is not copied to a local buffer and the transmission is
 *       asynchronous, be careful when using stack-allocated buffers for src.
 *
 * \param src the source buffer.
 * \param count number of bytes to write to the bus.
 */
void
ow_write_bytes(const uint8_t *src, const uint8_t count)
{
  OW_WAIT_WRITE();

  /* Place the byte in the tx buffer */
  MK60_ENTER_CRITICAL_REGION();
  ow_write_bytes_count = count;
  ow_write_bytes_buf = src;
  MK60_LEAVE_CRITICAL_REGION();

  /* Enable TX interrupts, all bits will be pushed by the ISR */
  ONEWIRE_UART->C2 |= (UART_C2_TCIE_MASK);
}
/**
 * Shorthand function to write a single byte to the 1-wire bus.
 *
 * \param data the data byte to write.
 */
void
ow_write_byte(const uint8_t data)
{
  static uint8_t buf;
  buf = data;
  ow_write_bytes(&buf, 1);
}
/**
 * Read a sequence of bytes from the 1-wire bus.
 *
 * Contrary to the ow_write_bytes function, this function is synchronous and
 * will do a busy wait until all bits have been read. This is not really
 * desirable but is so only because of laziness and lack of time.
 *
 * \param dest the destination buffer.
 * \param count number of bytes to read from the bus.
 */
void
ow_read_bytes(uint8_t *dest, const uint8_t count)
{
  OW_WAIT_READ();

  /* Compute where the received byte will be at */
  MK60_ENTER_CRITICAL_REGION();
  ow_read_bytes_count = count;
  ow_read_bytes_buf = dest;
  MK60_LEAVE_CRITICAL_REGION();

  /* Enable TX interrupts, all bits will be pushed by the ISR */
  ONEWIRE_UART->C2 |= (UART_C2_TCIE_MASK);

  /** \todo sleep while waiting for rx bits on 1-wire bus. */
  while(ow_read_bits > 0 || ow_read_bytes_count > 0);
}
/**
 * Compute a 1-wire 8-bit CRC.
 *
 * This function uses a lookup table method of computing the CRC one byte at a
 * time, instead of bit per bit. The table is found in ow_crc_table in
 * onewire-crc-table.h.
 *
 * The polynomial used by the 1-wire bus is x^8 + x^5 + x^4 + x^0, or 0x31.
 * But take care when computing the CRC (or producing a lookup table) that the
 * CRC is computed LSB first and using the reversed polynomial 0x8C instead.
 *
 * \note Because of the mathematics of the CRC, if passed a message where the
 *       last byte is the CRC, the return value will be 0 if the CRC is correct
 *       for the message.
 *
 * \param data The message.
 * \param count Number of bytes in the message.
 * \return The CRC
 */
uint8_t
ow_compute_crc(const uint8_t *data, const uint8_t count)
{
  uint8_t i;
  uint8_t crc = 0;
  uint8_t index;
  for(i = 0; i < count; ++i) {
    index = data[i] ^ crc;
    crc = ow_crc_table[index];
  }

  return crc;
}
/**
 * Issue a 1-wire READ ROM command
 *
 * The value will be returned in the order it arrives on the bus, i.e. 8 bit
 * family code first, then 48 bit serial number, last 8 bit CRC.
 *
 * On a little endian machine this will be the exact same representation as in
 * the 1-wire data sheets, LSB first.
 *
 * \note Use ow_compute_crc(rom_code, 8) to verify the ROM code, return value
 *       should be 0x00.
 *
 * \return 64 bit ROM CODE from the bus, including CRC.
 */
ow_rom_code_t
ow_read_rom(void)
{
  uint16_t buf;
  uint8_t rom[ONEWIRE_ROM_CODE_LENGTH];
  uint8_t i;
  ow_rom_code_t rom_code;
  static const ow_rom_cmd_t cmd = ONEWIRE_CMD_READ_ROM;
  printf("READ ROM: ");
  ow_reset();
  ow_write_bytes((const uint8_t *)(&cmd), 1);
  ow_read_bytes(rom, ONEWIRE_ROM_CODE_LENGTH);
  /* Little endian */
  rom_code = *((uint64_t *)rom);
  /* For non-Little endian machines */
  /*rom_code = (((ow_rom_code_t)rom[0]) |
     (((ow_rom_code_t)rom[1]) <<  8) |
     (((ow_rom_code_t)rom[2]) << 16) |
     (((ow_rom_code_t)rom[3]) << 24) |
     (((ow_rom_code_t)rom[4]) << 32) |
     (((ow_rom_code_t)rom[5]) << 40) |
     (((ow_rom_code_t)rom[6]) << 48) |
     (((ow_rom_code_t)rom[7]) << 56));
   */

  for(i = 0; i < sizeof(rom) / 2; ++i) {
    buf = (rom[2 * i] << 8) | (rom[2 * i + 1]);
    printf("%x", buf);
  }
  printf("\n");
  printf("CRC: 0x%x\n", ow_compute_crc(rom, ONEWIRE_ROM_CODE_LENGTH));
  return rom_code;
}
/**
 * Issue a 1-wire SKIP ROM command.
 *
 * The SKIP ROM command is used to issue commands to all devices on the bus.
 */
void
ow_skip_rom(void)
{
  static const ow_rom_cmd_t cmd = ONEWIRE_CMD_SKIP_ROM;
  printf("SKIP ROM\n");
  ow_reset();
  ow_write_bytes((const uint8_t *)(&cmd), 1);
}
/**
 * Issue a 1-wire MATCH ROM command.
 *
 * The MATCH ROM command is used to single out a specific device using its ROM
 * code as identifier.
 *
 * \param id The ROM code of the selected device.
 */
void
ow_match_rom(const ow_rom_code_t id)
{
  static ow_rom_code_t rom_code;
  static const ow_rom_cmd_t cmd = ONEWIRE_CMD_MATCH_ROM;
  printf("MATCH ROM\n");
  ow_reset();
  ow_write_bytes((const uint8_t *)(&cmd), 1);
  /* Copy from stack to a static variable to avoid messing up the buffer when
   * returning from this function before all bytes have gone out on the wire. */
  rom_code = id;
  ow_write_bytes((const uint8_t *)(&rom_code), ONEWIRE_ROM_CODE_LENGTH);
}
/**
 * Shorthand function for MATCH ROM or SKIP ROM if id is zero.
 *
 * \param id The ROM code of the selected device, or 0 for all devices.
 */
void
ow_skip_or_match_rom(const ow_rom_code_t id)
{
#if ONEWIRE_ALWAYS_SKIP_ROM
  ow_skip_rom();
#else
  if(id == 0) {
    /* SKIP ROM */
    ow_skip_rom();
  } else {
    /* MATCH ROM */
    ow_match_rom(id);
  }
#endif /* ONEWIRE_ALWAYS_SKIP_ROM */
}

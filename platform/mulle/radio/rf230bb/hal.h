/*   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *
 *	Colin O'Flynn coflynn@newae.com
 *	Eric Gnoske egnoske@gmail.com
 *	Blake Leverett bleverett@gmail.com
 *	Mike Vidales mavida404@gmail.com
 *	Kevin Brown kbrown3@uccs.edu
 *	Nate Bohlmann nate@elfwerks.com
 *  David Kopf dak664@embarqmail.com
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of the copyright holders nor the names of
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *    \addtogroup hal
 *    @{
 */

/**
 *  \file
 *  \brief This file contains low-level radio driver code.
 *
 *   $Id: hal.h,v 1.5 2010/12/03 20:42:01 dak664 Exp $
 */

#ifndef HAL_AVR_H
#define HAL_AVR_H

/*============================ INCLUDE =======================================*/

#include <stdint.h>
#include <stdbool.h>
#include "K60.h"
/* #include <util/crc16.h> */
#include "contiki-conf.h"
#include "interrupt.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/

/**
 * \name Pin macros
 * \brief These macros convert the platform-specific pin defines into names and functions
 *       that the source code can directly use.
 * \{
 */
#define SLPTR_GPIO PTE
#define SLPTR_PIN 6

#define hal_set_slptr_high() (BITBAND_REG(SLPTR_GPIO->PSOR, SLPTR_PIN) = 1) /**< This macro pulls the SLP_TR pin high. */
#define hal_set_slptr_low()  (BITBAND_REG(SLPTR_GPIO->PCOR, SLPTR_PIN) = 1) /**< This macro pulls the SLP_TR pin low. */
#define hal_get_slptr()      (BITBAND_REG(SLPTR_GPIO->PDOR, SLPTR_PIN))    /**< Read current state of the SLP_TR pin (High/Low). */
/* rst and pwr is the same */
#define hal_set_rst_high()   PTD->PSOR = (1 << 7); udelay(0xFFFF)          /**< This macro pulls the RST pin high. */
#define hal_set_rst_low()    (PTD->PCOR = (1 << 7))           /**< This macro pulls the RST pin low. */
#define hal_get_rst()        ((PTD->PDOR & (1 << 7)) >> 7)    /**< Read current state of the RST pin (High/Low). */
#define hal_set_pwr_high()   (PTD->PSOR = (1 << 7))           /**< This macro pulls the RST pin high. */
#define hal_set_pwr_low()    (PTD->PCOR = (1 << 7))           /**< This macro pulls the RST pin low. */
#define hal_get_pwr()        ((PTD->PDOR & (1 << 7)) >> 7)    /**< Read current state of the RST pin (High/Low). */
#define HAL_SS_PIN            1                                /**< The slave select pin. */

/** \} */

#define HAL_SS_HIGH()  /* Done in HW on K60 */
#define HAL_SS_LOW()   /* Done in HW on K60 */

#define HAL_ENABLE_RADIO_INTERRUPT() { PORTB->PCR[9] |= (1 << 24); \
  NVIC_ClearPendingIRQ(PORTB_IRQn); \
  NVIC_EnableIRQ(PORTB_IRQn); }
#define HAL_DISABLE_RADIO_INTERRUPT() (NVIC_DisableIRQ(PORTB_IRQn))

#define HAL_ENABLE_OVERFLOW_INTERRUPT() ()
#define HAL_DISABLE_OVERFLOW_INTERRUPT() ()

#define HAL_ENTER_CRITICAL_REGION() MK60_ENTER_CRITICAL_REGION()
#define HAL_LEAVE_CRITICAL_REGION() MK60_LEAVE_CRITICAL_REGION()

/** \brief  Enable the interrupt from the radio transceiver.
 */
#define hal_enable_trx_interrupt() HAL_ENABLE_RADIO_INTERRUPT()

/** \brief  Disable the interrupt from the radio transceiver.
 *
 *  \retval 0 if the pin is low, 1 if the pin is high.
 */
#define hal_disable_trx_interrupt() HAL_DISABLE_RADIO_INTERRUPT()

/*============================ TYPDEFS =======================================*/

/*============================ PROTOTYPES ====================================*/

/*============================ MACROS ========================================*/

/** \name Macros for radio operation.
 * \{
 */
#define HAL_BAT_LOW_MASK       (0x80)   /**< Mask for the BAT_LOW interrupt. */
#define HAL_TRX_UR_MASK        (0x40)   /**< Mask for the TRX_UR interrupt. */
#define HAL_TRX_END_MASK       (0x08)   /**< Mask for the TRX_END interrupt. */
#define HAL_RX_START_MASK      (0x04)   /**< Mask for the RX_START interrupt. */
#define HAL_PLL_UNLOCK_MASK    (0x02)   /**< Mask for the PLL_UNLOCK interrupt. */
#define HAL_PLL_LOCK_MASK      (0x01)   /**< Mask for the PLL_LOCK interrupt. */

#define HAL_MIN_FRAME_LENGTH   (0x03)   /**< A frame should be at least 3 bytes. */
#define HAL_MAX_FRAME_LENGTH   (0x7F)   /**< A frame should no more than 127 bytes. */
/** \} */

/*============================ TYPDEFS =======================================*/
/** \struct hal_rx_frame_t
 *  \brief  This struct defines the rx data container.
 *
 *  \see hal_frame_read
 */
typedef struct {
  uint8_t length;                         /**< Length of frame. */
  uint8_t data[HAL_MAX_FRAME_LENGTH];     /**< Actual frame data. */
  uint8_t lqi;                            /**< LQI value for received frame. */
  uint8_t rssi;
  bool crc;                               /**< Flag - did CRC pass for received frame? */
} hal_rx_frame_t;

/** RX_START event handler callback type. Is called with timestamp in IEEE 802.15.4 symbols and frame length. See hal_set_rx_start_event_handler(). */
typedef void (*hal_rx_start_isr_event_handler_t)(uint32_t const isr_timestamp, uint8_t const frame_length);

/** RRX_END event handler callback type. Is called with timestamp in IEEE 802.15.4 symbols and frame length. See hal_set_trx_end_event_handler(). */
typedef void (*hal_trx_end_isr_event_handler_t)(uint32_t const isr_timestamp);

typedef void (*rx_callback_t) (uint16_t data);

/*============================ PROTOTYPES ====================================*/
void hal_init(void);

void hal_reset_flags(void);
uint8_t hal_get_bat_low_flag(void);
void hal_clear_bat_low_flag(void);

hal_trx_end_isr_event_handler_t hal_get_trx_end_event_handler(void);
void hal_set_trx_end_event_handler(hal_trx_end_isr_event_handler_t trx_end_callback_handle);
void hal_clear_trx_end_event_handler(void);

hal_rx_start_isr_event_handler_t hal_get_rx_start_event_handler(void);
void hal_set_rx_start_event_handler(hal_rx_start_isr_event_handler_t rx_start_callback_handle);
void hal_clear_rx_start_event_handler(void);

uint8_t hal_get_pll_lock_flag(void);
void hal_clear_pll_lock_flag(void);

uint8_t hal_register_read(uint8_t address);
void hal_register_write(uint8_t address, uint8_t value);
uint8_t hal_subregister_read(uint8_t address, uint8_t mask, uint8_t position);
void hal_subregister_write(uint8_t address, uint8_t mask, uint8_t position, uint8_t value);

/* For speed RF230BB does not use a callback */
void hal_frame_read(hal_rx_frame_t *rx_frame);
void hal_frame_write(uint8_t *write_buffer, uint8_t length);
void hal_sram_read(uint8_t address, uint8_t length, uint8_t *data);
void hal_sram_write(uint8_t address, uint8_t length, uint8_t *data);

/* Number of receive buffers in RAM. */
#ifndef RF230_CONF_RX_BUFFERS
#define RF230_CONF_RX_BUFFERS 1
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

/** @} */
/*EOF*/

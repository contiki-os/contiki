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
 *    \addtogroup radiorf230
 *   @{
 */
/**
 *  \file
 *  \brief This file contains radio driver code.
 *
 */

#ifndef RADIO_H
#define RADIO_H
/*============================ INCLUDE =======================================*/
#include <stdint.h>
#include <stdbool.h>
#include "hal.h"
#if defined(__AVR_ATmega128RFA1__)
#include "atmega128rfa1_registermap.h"
#else
#include "at86rf230_registermap.h"
#endif


/*============================ MACROS ========================================*/
#define SUPPORTED_PART_NUMBER                   ( 2 )
#define RF230_REVA                              ( 1 )
#define RF230_REVB                              ( 2 )
#define SUPPORTED_MANUFACTURER_ID               ( 31 )

#if defined(__AVR_ATmega128RFA1__)
#define RF230_SUPPORTED_INTERRUPT_MASK          ( 0xFF )
#else
/* RF230 does not support RX_START interrupts in extended mode, but it seems harmless to always enable it. */
/* In non-extended mode this allows RX_START to sample the RF rssi at the end of the preamble */
//#define RF230_SUPPORTED_INTERRUPT_MASK        ( 0x08 )  //enable trx end only
//#define RF230_SUPPORTED_INTERRUPT_MASK          ( 0x0F ) //disable bat low, trx underrun
#define RF230_SUPPORTED_INTERRUPT_MASK          ( 0x0C )  //disable bat low, trx underrun, pll lock/unlock
#endif

#define RF230_MIN_CHANNEL                       ( 11 )
#define RF230_MAX_CHANNEL                       ( 26 )
#define RF230_MIN_ED_THRESHOLD                  ( 0 )
#define RF230_MAX_ED_THRESHOLD                  ( 15 )
#define RF230_MAX_TX_FRAME_LENGTH               ( 127 ) /**< 127 Byte PSDU. */
//#define RF230_MAX_PACKET_LEN                    127

#define TX_PWR_3DBM                             ( 0 )
#define TX_PWR_17_2DBM                          ( 15 )

#define TX_PWR_MAX                             TX_PWR_3DBM
#define TX_PWR_MIN                             TX_PWR_17_2DBM
#define TX_PWR_UNDEFINED                       (TX_PWR_MIN+1)


#define BATTERY_MONITOR_HIGHEST_VOLTAGE         ( 15 )
#define BATTERY_MONITOR_VOLTAGE_UNDER_THRESHOLD ( 0 )
#define BATTERY_MONITOR_HIGH_VOLTAGE            ( 1 )
#define BATTERY_MONITOR_LOW_VOLTAGE             ( 0 )

#define FTN_CALIBRATION_DONE                    ( 0 )
#define PLL_DCU_CALIBRATION_DONE                ( 0 )
#define PLL_CF_CALIBRATION_DONE                 ( 0 )

#define RC_OSC_REFERENCE_COUNT_MAX  (1.005*F_CPU*31250UL/8000000UL)
#define RC_OSC_REFERENCE_COUNT_MIN  (0.995*F_CPU*31250UL/8000000UL)

#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif
/*============================ TYPEDEFS ======================================*/

/** \brief  This macro defines the start value for the RADIO_* status constants.
 *
 *          It was chosen to have this macro so that the user can define where
 *          the status returned from the TAT starts. This can be useful in a
 *          system where numerous drivers are used, and some range of status codes
 *          are occupied.
 *
 *  \see radio_status_t
 */
#define RADIO_STATUS_START_VALUE                  ( 0x40 )

/** \brief  This enumeration defines the possible return values for the TAT API
 *          functions.
 *
 *          These values are defined so that they should not collide with the
 *          return/status codes defined in the IEEE 802.15.4 standard.
 *
 */
typedef enum{
    RADIO_SUCCESS = RADIO_STATUS_START_VALUE,  /**< The requested service was performed successfully. */
    RADIO_UNSUPPORTED_DEVICE,         /**< The connected device is not an Atmel AT86RF230. */
    RADIO_INVALID_ARGUMENT,           /**< One or more of the supplied function arguments are invalid. */
    RADIO_TIMED_OUT,                  /**< The requested service timed out. */
    RADIO_WRONG_STATE,                /**< The end-user tried to do an invalid state transition. */
    RADIO_BUSY_STATE,                 /**< The radio transceiver is busy receiving or transmitting. */
    RADIO_STATE_TRANSITION_FAILED,    /**< The requested state transition could not be completed. */
    RADIO_CCA_IDLE,                   /**< Channel is clear, available to transmit a new frame. */
    RADIO_CCA_BUSY,                   /**< Channel busy. */
    RADIO_TRX_BUSY,                   /**< Transceiver is busy receiving or transmitting data. */
    RADIO_BAT_LOW,                    /**< Measured battery voltage is lower than voltage threshold. */
    RADIO_BAT_OK,                     /**< Measured battery voltage is above the voltage threshold. */
    RADIO_CRC_FAILED,                 /**< The CRC failed for the actual frame. */
    RADIO_CHANNEL_ACCESS_FAILURE,     /**< The channel access failed during the auto mode. */
    RADIO_NO_ACK,                     /**< No acknowledge frame was received. */
}radio_status_t;


/**
 * \name Transaction status codes
 * \{
 */
#define TRAC_SUCCESS                0
#define TRAC_SUCCESS_DATA_PENDING   1
#define TRAC_SUCCESS_WAIT_FOR_ACK   2
#define TRAC_CHANNEL_ACCESS_FAILURE 3
#define TRAC_NO_ACK                 5
#define TRAC_INVALID                7
/** \} */


/** \brief  This enumeration defines the possible modes available for the
 *          Clear Channel Assessment algorithm.
 *
 *          These constants are extracted from the datasheet.
 *
 */
typedef enum{
//    CCA_ED                   = 0,    /**< Use energy detection above threshold mode. */ conflicts with atmega128rfa1 mcu definition
    CCA_ENERGY_DETECT         = 0,    /**< Use energy detection above threshold mode. */
    CCA_CARRIER_SENSE         = 1,    /**< Use carrier sense mode. */
    CCA_CARRIER_SENSE_WITH_ED = 2     /**< Use a combination of both energy detection and carrier sense. */
}radio_cca_mode_t;


/** \brief  This enumeration defines the possible CLKM speeds.
 *
 *          These constants are extracted from the RF230 datasheet.
 *
 */
typedef enum{
    CLKM_DISABLED      = 0,
    CLKM_1MHZ          = 1,
    CLKM_2MHZ          = 2,
    CLKM_4MHZ          = 3,
    CLKM_8MHZ          = 4,
    CLKM_16MHZ         = 5
}radio_clkm_speed_t;

typedef void (*radio_rx_callback) (uint16_t data);


/*	Hook Documentation 
**	
**	Sniffing Hooks:
**		RF230BB_HOOK_TX_PACKET(buffer,total_len)
**		RF230BB_HOOK_RX_PACKET(buf,len)
**
**	RF230BB_HOOK_IS_SEND_ENABLED()
**	RF230BB_HOOK_RADIO_ON()
**	RF230BB_HOOK_RADIO_OFF()
**	
*/


/*============================ PROTOTYPES ====================================*/

const struct radio_driver rf230_driver;

int rf230_init(void);
void rf230_warm_reset(void);
void rf230_start_sneeze(void);
//int rf230_on(void);
//int rf230_off(void);
void rf230_set_channel(uint8_t channel);
void rf230_listen_channel(uint8_t channel);
uint8_t rf230_get_channel(void);
void rf230_set_pan_addr(unsigned pan,unsigned addr,const uint8_t ieee_addr[8]);
void rf230_set_txpower(uint8_t power);
uint8_t rf230_get_txpower(void);

void rf230_set_promiscuous_mode(bool isPromiscuous);
bool rf230_is_ready_to_send();

extern uint8_t rf230_last_correlation,rf230_last_rssi,rf230_smallest_rssi;

uint8_t rf230_get_raw_rssi(void);

#define rf230_rssi	rf230_get_raw_rssi

#endif
/** @} */
/*EOF*/

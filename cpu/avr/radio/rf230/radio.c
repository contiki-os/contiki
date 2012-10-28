/*   Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *      Colin O'Flynn coflynn@newae.com
 *      Eric Gnoske egnoske@gmail.com
 *      Blake Leverett bleverett@gmail.com
 *      Mike Vidales mavida404@gmail.com
 *      Kevin Brown kbrown3@uccs.edu
 *      Nate Bohlmann nate@elfwerks.com
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
 *
*/

/**
 *  \brief This module contains the radio driver code for the Atmel
 *  AT86RF230, '231, and '212 chips.
 *
 *  \author Blake Leverett <bleverett@gmail.com>
 *          Mike Vidales <mavida404@gmail.com>
 *          Eric Gnoske <egnoske@gmail.com>
 *
*/

/**  \addtogroup wireless
 * @{
 */

/**
 *  \defgroup radiorf230 RF230 interface
 * @{
 */
/**
 *  \file
 *  This file contains radio driver code.
 *
 */



/*============================ INCLUDE =======================================*/
#include <stdlib.h>
#include <util/delay.h>
#include "radio.h"
#include "hal.h"
#include "process.h"
#include "sicslowmac.h"
#include "frame.h"

/*============================ MACROS ========================================*/
#define RADIO_CCA_DONE_MASK     (1 << 7) /**<  Mask used to check the CCA_DONE bit. */
#define RADIO_CCA_IDLE_MASK     (1 << 6) /**<  Mask used to check the CCA_STATUS bit. */

#define RADIO_START_CCA (1) /**<  Value in the CCA_REQUEST subregister that initiate a cca. */

#define RADIO_TRANSMISSION_SUCCESS  (0)
#define RADIO_BUSY_CHANNEL          (3)
#define RADIO_MIN_IEEE_FRAME_LENGTH (5)
/*============================ TYPEDEFS ======================================*/

/** \brief  This enumeration defines the necessary timing information for the
 *          AT86RF230 radio transceiver. All times are in microseconds.
 *
 *          These constants are extracted from the datasheet.
 */
typedef enum{
    TIME_TO_ENTER_P_ON               = 510, /**<  Transition time from VCC is applied to P_ON. */
    TIME_P_ON_TO_TRX_OFF             = 510, /**<  Transition time from P_ON to TRX_OFF. */
    TIME_SLEEP_TO_TRX_OFF            = 880, /**<  Transition time from SLEEP to TRX_OFF. */
    TIME_RESET                       = 6,   /**<  Time to hold the RST pin low during reset */
    TIME_ED_MEASUREMENT              = 140, /**<  Time it takes to do a ED measurement. */
    TIME_CCA                         = 140, /**<  Time it takes to do a CCA. */
    TIME_PLL_LOCK                    = 150, /**<  Maximum time it should take for the PLL to lock. */
    TIME_FTN_TUNING                  = 25,  /**<  Maximum time it should take to do the filter tuning. */
    TIME_NOCLK_TO_WAKE               = 6,   /**<  Transition time from *_NOCLK to being awake. */
    TIME_CMD_FORCE_TRX_OFF           = 1,    /**<  Time it takes to execute the FORCE_TRX_OFF command. */
    TIME_TRX_OFF_TO_PLL_ACTIVE       = 180, /**<  Transition time from TRX_OFF to: RX_ON, PLL_ON, TX_ARET_ON and RX_AACK_ON. */
    TIME_STATE_TRANSITION_PLL_ACTIVE = 1, /**<  Transition time from PLL active state to another. */
}radio_trx_timing_t;

/*============================ VARIABLES =====================================*/
static hal_rx_start_isr_event_handler_t user_rx_event;
static hal_trx_end_isr_event_handler_t user_trx_end_event;
static radio_rx_callback rx_frame_callback;
static uint8_t rssi_val;
static uint8_t rx_mode;
uint8_t rxMode = RX_AACK_ON;

/* See clock.c and httpd-cgi.c for RADIOSTATS code */
#define RADIOSTATS 0
#if RADIOSTATS
uint8_t RF230_radio_on, RF230_rsigsi;
uint16_t RF230_sendpackets,RF230_receivepackets,RF230_sendfail,RF230_receivefail;
#endif

static hal_rx_frame_t rx_frame;
static parsed_frame_t parsed_frame;

/*============================ PROTOTYPES ====================================*/
bool radio_is_sleeping(void);
static void radio_rx_start_event(uint32_t const isr_timestamp, uint8_t const frame_length);
static void radio_trx_end_event(uint32_t const isr_timestamp);

/** \brief  Initialize the Transceiver Access Toolbox and lower layers.
 *
 *          If the initialization is successful the radio transceiver will be in
 *          TRX_OFF state.
 *
 *  \note  This function must be called prior to any of the other functions in
 *         this file! Can be called from any transceiver state.
 *
 *  \param cal_rc_osc If true, the radio's accurate clock is used to calibrate the
 *                    CPU's internal RC oscillator.
 *
 *  \param rx_event Optional pointer to a user-defined function to be called on an
 *                  RX_START interrupt.  Use NULL for no handler.
 *
 *  \param trx_end_event Optional pointer to a user-defined function to be called on an
 *                  TRX_END interrupt.  Use NULL for no handler.
 *
 *  \param rx_callback Optional pointer to a user-defined function that receives
 *         a frame from the radio one byte at a time.  If the index parameter to
 *         this callback is 0xff, then the function should reset its state and prepare
 *         for a frame from the radio, with one call per byte.
 *
 *  \retval RADIO_SUCCESS     The radio transceiver was successfully initialized
 *                          and put into the TRX_OFF state.
 *  \retval RADIO_UNSUPPORTED_DEVICE  The connected device is not an Atmel
 *                                  AT86RF230 radio transceiver.
 *  \retval RADIO_TIMED_OUT   The radio transceiver was not able to initialize and
 *                          enter TRX_OFF state within the specified time.
 */
radio_status_t
radio_init(bool cal_rc_osc,
           hal_rx_start_isr_event_handler_t rx_event,
           hal_trx_end_isr_event_handler_t trx_end_event,
           radio_rx_callback rx_callback)
{
    radio_status_t init_status = RADIO_SUCCESS;

    delay_us(TIME_TO_ENTER_P_ON);

    /*  calibrate oscillator */
    if (cal_rc_osc){
        calibrate_rc_osc_32k();
    }

    /* Initialize Hardware Abstraction Layer. */
    hal_init();

    radio_reset_trx(); /* Do HW reset of radio transeiver. */

    /* Force transition to TRX_OFF. */
    hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_P_ON_TO_TRX_OFF); /* Wait for the transition to be complete. */

    if (radio_get_trx_state() != TRX_OFF){
        init_status = RADIO_TIMED_OUT;
    } else {
        /* Read Version Number */
        uint8_t version_number = hal_register_read(RG_VERSION_NUM);

        if ((version_number != RF230_REVA) && (version_number != RF230_REVB))
            init_status = RADIO_UNSUPPORTED_DEVICE;
        else {
            if (hal_register_read(RG_MAN_ID_0) != SUPPORTED_MANUFACTURER_ID)
                init_status = RADIO_UNSUPPORTED_DEVICE;
            else
                hal_register_write(RG_IRQ_MASK, RF230_SUPPORTED_INTERRUPT_MASK);
        }
#if RADIOSTATS
        RF230_radio_on = 1;
#endif
    }

    /*  set callbacks for events.  Save user's rx_event, which we will */
    /*  call from radio_rx_start_event().  Same with trx_end */
    user_rx_event = rx_event;
    user_trx_end_event = trx_end_event;
    hal_set_rx_start_event_handler(radio_rx_start_event);
    hal_set_trx_end_event_handler(radio_trx_end_event);

    rx_frame_callback = rx_callback;

    return init_status;
}

/*---------------------------------------------------------------------------*/
uint8_t *
radio_frame_data(void)
{
        return rx_frame.data;
}

uint8_t
radio_frame_length(void)
{
        return rx_frame.length;
}

/*---------------------------------------------------------------------------*/
static void
radio_rx_start_event(uint32_t const isr_timestamp, uint8_t const frame_length)
{
    /*  save away RSSI */
    rssi_val =  hal_subregister_read( SR_RSSI );

    /*  call user's rx_start event handler */
    if (user_rx_event)
        user_rx_event(isr_timestamp, frame_length);
}

/*---------------------------------------------------------------------------*/
uint8_t
radio_get_saved_rssi_value(void)
{
    return rssi_val;
}

/*---------------------------------------------------------------------------*/
static void
radio_trx_end_event(uint32_t const isr_timestamp)
{
    volatile uint8_t status;

    /*  call user's trx_end event handler */
    if (user_trx_end_event){
        user_trx_end_event(isr_timestamp);
        return;
    }
    if (rx_mode){
        /* radio has received frame, store it away */
#if RADIOSTATS
        RF230_rsigsi=rssi_val;
        RF230_receivepackets++;
#endif
        parsed_frame.time = isr_timestamp;
        parsed_frame.rssi = rssi_val;
        
        hal_frame_read(&rx_frame, NULL);
        rx_frame_parse(&rx_frame, &parsed_frame);
        }

    if (!rx_mode){
        /*  Put radio back into receive mode. */
        radio_set_trx_state(TRX_OFF);
        radio_set_trx_state(rxMode);

        /*  transmit mode, put end-of-transmit event in queue */
        event_object_t event;
        event.event = 0;
        event.data = 0;
        status = hal_subregister_read(SR_TRAC_STATUS);
        switch(status){
        case TRAC_SUCCESS:
        case TRAC_SUCCESS_DATA_PENDING:
            event.event = MAC_EVENT_ACK;
            break;
        case TRAC_NO_ACK:
        case TRAC_CHANNEL_ACCESS_FAILURE:
            event.event = MAC_EVENT_NACK;
#if RADIOSTATS
        RF230_sendfail++;
#endif
            break;
        case TRAC_SUCCESS_WAIT_FOR_ACK:
            /*  should only happen in RX mode */
        case TRAC_INVALID:
            /*  should never happen here */
        default:
#if RADIOSTATS
            RF230_sendfail++;
#endif
            break;
        }
        if (event.event)
            mac_put_event(&event);
        process_post(&mac_process, event.event, event.data);
    }
}
/*----------------------------------------------------------------------------*/
/** \brief  This function will return the channel used by the radio transceiver.
 *
 *  \return Current channel, 11 to 26.
 */
uint8_t
radio_get_operating_channel(void)
{
    return hal_subregister_read(SR_CHANNEL);
}
/*----------------------------------------------------------------------------*/
/** \brief This function will change the operating channel.
 *
 *  \param  channel New channel to operate on. Must be between 11 and 26.
 *
 *  \retval RADIO_SUCCESS New channel set.
 *  \retval RADIO_WRONG_STATE Transceiver is in a state where the channel cannot
 *                          be changed (SLEEP).
 *  \retval RADIO_INVALID_ARGUMENT Channel argument is out of bounds.
 *  \retval RADIO_TIMED_OUT The PLL did not lock within the specified time.
 */
radio_status_t
radio_set_operating_channel(uint8_t channel)
{
    /*Do function parameter and state check.*/
    if ((channel < RF230_MIN_CHANNEL) ||
        (channel > RF230_MAX_CHANNEL)){
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    if (radio_get_operating_channel() == channel){
        return RADIO_SUCCESS;
    }

    /*Set new operating channel.*/
    hal_subregister_write(SR_CHANNEL, channel);

    /* Read current state and wait for the PLL_LOCK interrupt if the */
    /* radio transceiver is in either RX_ON or PLL_ON. */
    uint8_t trx_state = radio_get_trx_state();

    if ((trx_state == RX_ON) ||
        (trx_state == PLL_ON)){
        delay_us(TIME_PLL_LOCK);
    }

    radio_status_t channel_set_status = RADIO_TIMED_OUT;

    /* Check that the channel was set properly. */
    if (radio_get_operating_channel() == channel){
        channel_set_status = RADIO_SUCCESS;
    }

    return channel_set_status;
}

/*----------------------------------------------------------------------------*/
/** \brief This function will read and return the output power level.
 *
 *  \returns 0 to 15 Current output power in "TX power settings" as defined in
 *          the radio transceiver's datasheet
 */
uint8_t
radio_get_tx_power_level(void)
{
    return hal_subregister_read(SR_TX_PWR);
}

/*----------------------------------------------------------------------------*/
/** \brief This function will change the output power level.
 *
 *  \param  power_level New output power level in the "TX power settings"
 *                      as defined in the radio transceiver's datasheet.
 *
 *  \retval RADIO_SUCCESS New output power set successfully.
 *  \retval RADIO_INVALID_ARGUMENT The supplied function argument is out of bounds.
 *  \retval RADIO_WRONG_STATE It is not possible to change the TX power when the
 *                          device is sleeping.
 */
radio_status_t
radio_set_tx_power_level(uint8_t power_level)
{

    /*Check function parameter and state.*/
    if (power_level > TX_PWR_17_2DBM){
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    /*Set new power level*/
    hal_subregister_write(SR_TX_PWR, power_level);

    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the current CCA mode used.
 *
 *  \return CCA mode currently used, 0 to 3.
 */
uint8_t
radio_get_cca_mode(void)
{
    return hal_subregister_read(SR_CCA_MODE);
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the current ED threshold used by the CCA algorithm.
 *
 *  \return Current ED threshold, 0 to 15.
 */
uint8_t
radio_get_ed_threshold(void)
{
    return hal_subregister_read(SR_CCA_ED_THRES);
}

/*----------------------------------------------------------------------------*/
/** \brief This function will configure the Clear Channel Assessment algorithm.
 *
 *  \param  mode Three modes are available: Energy above threshold, carrier
 *               sense only and carrier sense with energy above threshold.
 *  \param  ed_threshold Above this energy threshold the channel is assumed to be
 *                      busy. The threshold is given in positive dBm values.
 *                      Ex. -91 dBm gives a csThreshold of 91. Value range for
 *                      the variable is [61 to 91]. Only valid for the CCA_ED
 *                      and CCA_CARRIER_SENSE_ED modes.
 *
 *  \retval RADIO_SUCCESS Mode and its parameters successfully changed.
 *  \retval RADIO_WRONG_STATE This function cannot be called in the SLEEP state.
 *  \retval RADIO_INVALID_ARGUMENT If one of the three function arguments are out
 *                               of bounds.
 */
radio_status_t
radio_set_cca_mode(uint8_t mode, uint8_t ed_threshold)
{
    /*Check function parameters and state.*/
    if ((mode != CCA_ED) &&
        (mode != CCA_CARRIER_SENSE) &&
        (mode != CCA_CARRIER_SENSE_WITH_ED)){
        return RADIO_INVALID_ARGUMENT;
    }

    /* Ensure that the ED threshold is within bounds. */
    if (ed_threshold > RF230_MAX_ED_THRESHOLD){
        return RADIO_INVALID_ARGUMENT;
    }

    /* Ensure that the radio transceiver is not sleeping. */
    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    /*Change cca mode and ed threshold.*/
    hal_subregister_write(SR_CCA_MODE, mode);
    hal_subregister_write(SR_CCA_ED_THRES, ed_threshold);

    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the Received Signal Strength Indication.
 *
 *  \note This function should only be called from the: RX_ON and BUSY_RX. This
 *        can be ensured by reading the current state of the radio transceiver
 *        before executing this function!
 *  \param rssi Pointer to memory location where RSSI value should be written.
 *  \retval RADIO_SUCCESS The RSSI measurement was successful.
 *  \retval RADIO_WRONG_STATE The radio transceiver is not in RX_ON or BUSY_RX.
 */
radio_status_t
radio_get_rssi_value(uint8_t *rssi)
{

    uint8_t current_state = radio_get_trx_state();
    radio_status_t retval = RADIO_WRONG_STATE;

    /*The RSSI measurement should only be done in RX_ON or BUSY_RX.*/
    if ((current_state == RX_ON) ||
        (current_state == BUSY_RX)){
        *rssi = hal_subregister_read(SR_RSSI);
        retval = RADIO_SUCCESS;
    }

    return retval;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the current threshold volatge used by the
 *         battery monitor (BATMON_VTH).
 *
 *  \note This function can not be called from P_ON or SLEEP. This is ensured
 *        by reading the device state before calling this function.
 *
 *  \return Current threshold voltage, 0 to 15.
 */
uint8_t
radio_batmon_get_voltage_threshold(void)
{
    return hal_subregister_read(SR_BATMON_VTH);
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns if high or low voltage range is used.
 *
 *  \note This function can not be called from P_ON or SLEEP. This is ensured
 *        by reading the device state before calling this function.
 *
 *  \retval 0 Low voltage range selected.
 *  \retval 1 High voltage range selected.
 */
uint8_t
radio_batmon_get_voltage_range(void)
{
    return hal_subregister_read(SR_BATMON_HR);
}

/*----------------------------------------------------------------------------*/
/** \brief This function is used to configure the battery monitor module
 *
 *  \param range True means high voltage range and false low voltage range.
 *  \param voltage_threshold The datasheet defines 16 voltage levels for both
 *                          low and high range.
 *  \retval RADIO_SUCCESS Battery monitor configured
 *  \retval RADIO_WRONG_STATE The device is sleeping.
 *  \retval RADIO_INVALID_ARGUMENT The voltage_threshold parameter is out of
 *                               bounds (Not within [0 - 15]).
 */
radio_status_t
radio_batmon_configure(bool range, uint8_t voltage_threshold)
{

    /*Check function parameters and state.*/
    if (voltage_threshold > BATTERY_MONITOR_HIGHEST_VOLTAGE){
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    /*Write new voltage range and voltage level.*/
    if (range == true){
        hal_subregister_write(SR_BATMON_HR, BATTERY_MONITOR_HIGH_VOLTAGE);
    } else {
        hal_subregister_write(SR_BATMON_HR, BATTERY_MONITOR_LOW_VOLTAGE);
    }

    hal_subregister_write(SR_BATMON_VTH, voltage_threshold);

    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the status of the Battery Monitor module.
 *
 *  \note This function can not be called from P_ON or SLEEP. This is ensured
 *        by reading the device state before calling this function.
 *
 *  \retval RADIO_BAT_LOW Battery voltage is below the programmed threshold.
 *  \retval RADIO_BAT_OK Battery voltage is above the programmed threshold.
 */
radio_status_t
radio_batmon_get_status(void)
{

    radio_status_t batmon_status = RADIO_BAT_LOW;

    if (hal_subregister_read(SR_BATMON_OK) !=
        BATTERY_MONITOR_VOLTAGE_UNDER_THRESHOLD){
        batmon_status = RADIO_BAT_OK;
    }

    return batmon_status;
}

/*----------------------------------------------------------------------------*/
/** \brief This function returns the current clock setting for the CLKM pin.
 *
 *  \retval CLKM_DISABLED CLKM pin is disabled.
 *  \retval CLKM_1MHZ CLKM pin is prescaled to 1 MHz.
 *  \retval CLKM_2MHZ CLKM pin is prescaled to 2 MHz.
 *  \retval CLKM_4MHZ CLKM pin is prescaled to 4 MHz.
 *  \retval CLKM_8MHZ CLKM pin is prescaled to 8 MHz.
 *  \retval CLKM_16MHZ CLKM pin is not prescaled. Output is 16 MHz.
 */
uint8_t
radio_get_clock_speed(void)
{
    return hal_subregister_read(SR_CLKM_CTRL);
}

/*----------------------------------------------------------------------------*/
/** \brief This function changes the prescaler on the CLKM pin.
 *
 *  \param direct   This boolean variable is used to determine if the frequency
 *                  of the CLKM pin shall be changed directly or not. If direct
 *                  equals true, the frequency will be changed directly. This is
 *                  fine if the CLKM signal is used to drive a timer etc. on the
 *                  connected microcontroller. However, the CLKM signal can also
 *                  be used to clock the microcontroller itself. In this situation
 *                  it is possible to change the CLKM frequency indirectly
 *                  (direct == false). When the direct argument equlas false, the
 *                  CLKM frequency will be changed first after the radio transceiver
 *                  has been taken to SLEEP and awaken again.
 *  \param clock_speed This parameter can be one of the following constants:
 *                     CLKM_DISABLED, CLKM_1MHZ, CLKM_2MHZ, CLKM_4MHZ, CLKM_8MHZ
 *                     or CLKM_16MHZ.
 *
 *  \retval RADIO_SUCCESS Clock speed updated. New state is TRX_OFF.
 *  \retval RADIO_INVALID_ARGUMENT Requested clock speed is out of bounds.
 */
radio_status_t
radio_set_clock_speed(bool direct, uint8_t clock_speed)
{
        /*Check function parameter and current clock speed.*/
    if (clock_speed > CLKM_16MHZ){
            return RADIO_INVALID_ARGUMENT;
    }

    if (radio_get_clock_speed() == clock_speed){
        return RADIO_SUCCESS;
    }

        /*Select to change the CLKM frequency directly or after returning from SLEEP.*/
    if (direct == false){
            hal_subregister_write(SR_CLKM_SHA_SEL, 1);
    } else {
            hal_subregister_write(SR_CLKM_SHA_SEL, 0);
    }
        
        hal_subregister_write(SR_CLKM_CTRL, clock_speed);

    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function calibrates the Single Side Band Filter.
 *
 *  \retval     RADIO_SUCCESS    Filter is calibrated.
 *  \retval     RADIO_TIMED_OUT  The calibration could not be completed within time.
 *  \retval     RADIO_WRONG_STATE This function can only be called from TRX_OFF or
 *              PLL_ON.
 */
radio_status_t
radio_calibrate_filter(void)
{
    /*Check current state. Only possible to do filter calibration from TRX_OFF or PLL_ON.*/
    uint8_t trx_state = radio_get_trx_state();

    if ((trx_state != TRX_OFF) &&
        (trx_state != PLL_ON)){
        return RADIO_WRONG_STATE;
    }

    /* Start the tuning algorithm by writing one to the FTN_START subregister. */
    hal_subregister_write(SR_FTN_START, 1);
    delay_us(TIME_FTN_TUNING); /* Wait for the calibration to finish. */

    radio_status_t filter_calibration_status = RADIO_TIMED_OUT;

    /* Verify the calibration result. */
    if (hal_subregister_read(SR_FTN_START) == FTN_CALIBRATION_DONE){
        filter_calibration_status = RADIO_SUCCESS;
    }

    return filter_calibration_status;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function calibrates the PLL.
 *
 *  \retval     RADIO_SUCCESS    PLL Center Frequency and Delay Cell is calibrated.
 *  \retval     RADIO_TIMED_OUT  The calibration could not be completed within time.
 *  \retval     RADIO_WRONG_STATE This function can only be called from PLL_ON.
 */
radio_status_t
radio_calibrate_pll(void)
{

    /*Check current state. Only possible to calibrate PLL from PLL_ON state*/
    if (radio_get_trx_state() != PLL_ON){
        return RADIO_WRONG_STATE;
    }

    /* Initiate the DCU and CF calibration loops. */
    hal_subregister_write(SR_PLL_DCU_START, 1);
    hal_subregister_write(SR_PLL_CF_START, 1);

    /* Wait maximum 150 us for the PLL to lock. */
    hal_clear_pll_lock_flag();
    delay_us(TIME_PLL_LOCK);

    radio_status_t pll_calibration_status = RADIO_TIMED_OUT;

    if (hal_get_pll_lock_flag() > 0){
        if (hal_subregister_read(SR_PLL_DCU_START) == PLL_DCU_CALIBRATION_DONE){
            if (hal_subregister_read(SR_PLL_CF_START) == PLL_CF_CALIBRATION_DONE){
                pll_calibration_status = RADIO_SUCCESS;
            }
        }
    }

    return pll_calibration_status;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function return the Radio Transceivers current state.
 *
 *  \retval     P_ON               When the external supply voltage (VDD) is
 *                                 first supplied to the transceiver IC, the
 *                                 system is in the P_ON (Poweron) mode.
 *  \retval     BUSY_RX            The radio transceiver is busy receiving a
 *                                 frame.
 *  \retval     BUSY_TX            The radio transceiver is busy transmitting a
 *                                 frame.
 *  \retval     RX_ON              The RX_ON mode enables the analog and digital
 *                                 receiver blocks and the PLL frequency
 *                                 synthesizer.
 *  \retval     TRX_OFF            In this mode, the SPI module and crystal
 *                                 oscillator are active.
 *  \retval     PLL_ON             Entering the PLL_ON mode from TRX_OFF will
 *                                 first enable the analog voltage regulator. The
 *                                 transceiver is ready to transmit a frame.
 *  \retval     BUSY_RX_AACK       The radio was in RX_AACK_ON mode and received
 *                                 the Start of Frame Delimiter (SFD). State
 *                                 transition to BUSY_RX_AACK is done if the SFD
 *                                 is valid.
 *  \retval     BUSY_TX_ARET       The radio transceiver is busy handling the
 *                                 auto retry mechanism.
 *  \retval     RX_AACK_ON         The auto acknowledge mode of the radio is
 *                                 enabled and it is waiting for an incomming
 *                                 frame.
 *  \retval     TX_ARET_ON         The auto retry mechanism is enabled and the
 *                                 radio transceiver is waiting for the user to
 *                                 send the TX_START command.
 *  \retval     RX_ON_NOCLK        The radio transceiver is listening for
 *                                 incomming frames, but the CLKM is disabled so
 *                                 that the controller could be sleeping.
 *                                 However, this is only true if the controller
 *                                 is run from the clock output of the radio.
 *  \retval     RX_AACK_ON_NOCLK   Same as the RX_ON_NOCLK state, but with the
 *                                 auto acknowledge module turned on.
 *  \retval     BUSY_RX_AACK_NOCLK Same as BUSY_RX_AACK, but the controller
 *                                 could be sleeping since the CLKM pin is
 *                                 disabled.
 *  \retval     STATE_TRANSITION   The radio transceiver's state machine is in
 *                                 transition between two states.
 */
uint8_t
radio_get_trx_state(void)
{
    return hal_subregister_read(SR_TRX_STATUS);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function checks if the radio transceiver is sleeping.
 *
 *  \retval     true    The radio transceiver is in SLEEP or one of the *_NOCLK
 *                      states.
 *  \retval     false   The radio transceiver is not sleeping.
 */
bool radio_is_sleeping(void)
{
    bool sleeping = false;

    /* The radio transceiver will be at SLEEP or one of the *_NOCLK states only if */
    /* the SLP_TR pin is high. */
    if (hal_get_slptr() != 0){
        sleeping = true;
    }

    return sleeping;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will change the current state of the radio
 *          transceiver's internal state machine.
 *
 *  \param     new_state        Here is a list of possible states:
 *             - RX_ON        Requested transition to RX_ON state.
 *             - TRX_OFF      Requested transition to TRX_OFF state.
 *             - PLL_ON       Requested transition to PLL_ON state.
 *             - RX_AACK_ON   Requested transition to RX_AACK_ON state.
 *             - TX_ARET_ON   Requested transition to TX_ARET_ON state.
 *
 *  \retval    RADIO_SUCCESS          Requested state transition completed
 *                                  successfully.
 *  \retval    RADIO_INVALID_ARGUMENT Supplied function parameter out of bounds.
 *  \retval    RADIO_WRONG_STATE      Illegal state to do transition from.
 *  \retval    RADIO_BUSY_STATE       The radio transceiver is busy.
 *  \retval    RADIO_TIMED_OUT        The state transition could not be completed
 *                                  within resonable time.
 */
radio_status_t
radio_set_trx_state(uint8_t new_state)
{
    uint8_t original_state;

    /*Check function paramter and current state of the radio transceiver.*/
    if (!((new_state == TRX_OFF)    ||
          (new_state == RX_ON)      ||
          (new_state == PLL_ON)     ||
          (new_state == RX_AACK_ON) ||
          (new_state == TX_ARET_ON))){
        return RADIO_INVALID_ARGUMENT;
    }

    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    // Wait for radio to finish previous operation
    for(;;)
    {
        original_state = radio_get_trx_state();
        if (original_state != BUSY_TX_ARET &&
            original_state != BUSY_RX_AACK &&
            original_state != BUSY_RX && 
            original_state != BUSY_TX)
            break;
    }

    if (new_state == original_state){
        return RADIO_SUCCESS;
    }


    /* At this point it is clear that the requested new_state is: */
    /* TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON or TX_ARET_ON. */

    /* The radio transceiver can be in one of the following states: */
    /* TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON, TX_ARET_ON. */
    if(new_state == TRX_OFF){
        radio_reset_state_machine(); /* Go to TRX_OFF from any state. */
    } else {
        /* It is not allowed to go from RX_AACK_ON or TX_AACK_ON and directly to */
        /* TX_AACK_ON or RX_AACK_ON respectively. Need to go via RX_ON or PLL_ON. */
        if ((new_state == TX_ARET_ON) &&
            (original_state == RX_AACK_ON)){
            /* First do intermediate state transition to PLL_ON, then to TX_ARET_ON. */
            /* The final state transition to TX_ARET_ON is handled after the if-else if. */
            hal_subregister_write(SR_TRX_CMD, PLL_ON);
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        } else if ((new_state == RX_AACK_ON) &&
                 (original_state == TX_ARET_ON)){
            /* First do intermediate state transition to RX_ON, then to RX_AACK_ON. */
            /* The final state transition to RX_AACK_ON is handled after the if-else if. */
            hal_subregister_write(SR_TRX_CMD, RX_ON);
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }

        /* Any other state transition can be done directly. */
        hal_subregister_write(SR_TRX_CMD, new_state);

        /* When the PLL is active most states can be reached in 1us. However, from */
        /* TRX_OFF the PLL needs time to activate. */
        if (original_state == TRX_OFF){
            delay_us(TIME_TRX_OFF_TO_PLL_ACTIVE);
        } else {
            delay_us(TIME_STATE_TRANSITION_PLL_ACTIVE);
        }
    } /*  end: if(new_state == TRX_OFF) ... */

    /*Verify state transition.*/
    radio_status_t set_state_status = RADIO_TIMED_OUT;

    if (radio_get_trx_state() == new_state){
        set_state_status = RADIO_SUCCESS;
        /*  set rx_mode flag based on mode we're changing to */
        if (new_state == RX_ON ||
            new_state == RX_AACK_ON){
            rx_mode = true;
        } else {
            rx_mode = false;
    }
    }

    return set_state_status;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will put the radio transceiver to sleep.
 *
 *  \retval    RADIO_SUCCESS          Sleep mode entered successfully.
 *  \retval    RADIO_TIMED_OUT        The transition to TRX_OFF took too long.
 */
radio_status_t
radio_enter_sleep_mode(void)
{
    if (radio_is_sleeping() == true){
        return RADIO_SUCCESS;
    }

    radio_reset_state_machine(); /* Force the device into TRX_OFF. */

    radio_status_t enter_sleep_status = RADIO_TIMED_OUT;

    if (radio_get_trx_state() == TRX_OFF){
        /* Enter Sleep. */
        hal_set_slptr_high();
        enter_sleep_status = RADIO_SUCCESS;
#if RADIOSTATS
        RF230_radio_on = 0;
#endif
    }

    return enter_sleep_status;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will take the radio transceiver from sleep mode and
 *          put it into the TRX_OFF state.
 *
 *  \retval    RADIO_SUCCESS          Left sleep mode and entered TRX_OFF state.
 *  \retval    RADIO_TIMED_OUT        Transition to TRX_OFF state timed out.
 */
radio_status_t
radio_leave_sleep_mode(void)
{
    /* Check if the radio transceiver is actually sleeping. */
    if (radio_is_sleeping() == false){
        return RADIO_SUCCESS;
    }

    hal_set_slptr_low();
    delay_us(TIME_SLEEP_TO_TRX_OFF);

    radio_status_t leave_sleep_status = RADIO_TIMED_OUT;

    /* Ensure that the radio transceiver is in the TRX_OFF state. */
    if (radio_get_trx_state() == TRX_OFF){
        leave_sleep_status = RADIO_SUCCESS;
#if RADIOSTATS
        RF230_radio_on = 1;
#endif
    }

    return leave_sleep_status;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will reset the state machine (to TRX_OFF) from any of
 *          its states, except for the SLEEP state.
 */
void
radio_reset_state_machine(void)
{
    hal_set_slptr_low();
    delay_us(TIME_NOCLK_TO_WAKE);
    hal_subregister_write(SR_TRX_CMD, CMD_FORCE_TRX_OFF);
    delay_us(TIME_CMD_FORCE_TRX_OFF);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will reset all the registers and the state machine of
 *          the radio transceiver.
 */
void
radio_reset_trx(void)
{
    hal_set_rst_low();
    hal_set_slptr_low();
    delay_us(TIME_RESET);
    hal_set_rst_high();
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will enable or disable automatic CRC during frame
 *          transmission.
 *
 *  \param  auto_crc_on If this parameter equals true auto CRC will be used for
 *                      all frames to be transmitted. The framelength must be
 *                      increased by two bytes (16 bit CRC). If the parameter equals
 *                      false, the automatic CRC will be disabled.
 */
void
radio_use_auto_tx_crc(bool auto_crc_on)
{
    if (auto_crc_on == true){
        hal_subregister_write(SR_TX_AUTO_CRC_ON, 1);
    } else {
        hal_subregister_write(SR_TX_AUTO_CRC_ON, 0);
    }
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's
 *          transmit buffer and send it.
 *
 *  \param  data_length Length of the frame to be transmitted. 1 to 128 bytes are the valid lengths.
 *  \param  *data   Pointer to the data to transmit
 *
 *  \retval RADIO_SUCCESS Frame downloaded and sent successfully.
 *  \retval RADIO_INVALID_ARGUMENT If the dataLength is 0 byte or more than 127
 *                               bytes the frame will not be sent.
 *  \retval RADIO_WRONG_STATE It is only possible to use this function in the
 *                          PLL_ON and TX_ARET_ON state. If any other state is
 *                          detected this error message will be returned.
 */
radio_status_t
radio_send_data(uint8_t data_length, uint8_t *data)
{
    /*Check function parameters and current state.*/
    if (data_length > RF230_MAX_TX_FRAME_LENGTH){
#if RADIOSTATS
        RF230_sendfail++;
#endif
        return RADIO_INVALID_ARGUMENT;
    }

     /* If we are busy, return */
        if ((radio_get_trx_state() == BUSY_TX) || (radio_get_trx_state() == BUSY_TX_ARET) )
        {
#if RADIOSTATS
        RF230_sendfail++;
#endif
        return RADIO_WRONG_STATE;
        }

    radio_set_trx_state(TRX_OFF);
    radio_set_trx_state(TX_ARET_ON);

    /*Do frame transmission.*/
    /* Toggle the SLP_TR pin to initiate the frame transmission. */
    hal_set_slptr_high();
    hal_set_slptr_low();

    hal_frame_write(data, data_length); /* Then write data to the frame buffer. */
#if RADIOSTATS
    RF230_sendpackets++;
#endif
    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will read the I_AM_COORD sub register.
 *
 *  \retval 0 Not coordinator.
 *  \retval 1 Coordinator role enabled.
 */
uint8_t
radio_get_device_role(void)
{
    return hal_subregister_read(SR_I_AM_COORD);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will set the I_AM_COORD sub register.
 *
 *  \param[in] i_am_coordinator If this parameter is true, the associated
 *                              coordinator role will be enabled in the radio
 *                              transceiver's address filter.
 *                              False disables the same feature.
 */
void
radio_set_device_role(bool i_am_coordinator)
{
    hal_subregister_write(SR_I_AM_COORD, i_am_coordinator);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will return the PANID used by the address filter.
 *
 *  \retval Any value from 0 to 0xFFFF.
 */
uint16_t
radio_get_pan_id(void)
{

    uint8_t pan_id_15_8 = hal_register_read(RG_PAN_ID_1); /*  Read pan_id_15_8. */
    uint8_t pan_id_7_0 = hal_register_read(RG_PAN_ID_0); /*  Read pan_id_7_0. */

    uint16_t pan_id = ((uint16_t)(pan_id_15_8 << 8)) | pan_id_7_0;

    return pan_id;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will set the PANID used by the address filter.
 *
 *  \param  new_pan_id Desired PANID. Can be any value from 0x0000 to 0xFFFF
 */
void
radio_set_pan_id(uint16_t new_pan_id)
{

    uint8_t pan_byte = new_pan_id & 0xFF; /*  Extract new_pan_id_7_0. */
    hal_register_write(RG_PAN_ID_0, pan_byte);

    pan_byte = (new_pan_id >> 8*1) & 0xFF;  /*  Extract new_pan_id_15_8. */
    hal_register_write(RG_PAN_ID_1, pan_byte);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will return the current short address used by the
 *          address filter.
 *
 *  \retval Any value from 0x0000 to 0xFFFF
 */
uint16_t
radio_get_short_address(void)
{

    uint8_t short_address_15_8 = hal_register_read(RG_SHORT_ADDR_1); /*  Read short_address_15_8. */
    uint8_t short_address_7_0  = hal_register_read(RG_SHORT_ADDR_1); /*  Read short_address_7_0. */

    uint16_t short_address = ((uint16_t)(short_address_15_8 << 8)) | short_address_7_0;

    return short_address;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will set the short address used by the address filter.
 *
 *  \param  new_short_address Short address to be used by the address filter.
 */
void
radio_set_short_address(uint16_t new_short_address)
{

    uint8_t short_address_byte = new_short_address & 0xFF; /*  Extract short_address_7_0. */
    hal_register_write(RG_SHORT_ADDR_0, short_address_byte);

    short_address_byte = (new_short_address >> 8*1) & 0xFF; /*  Extract short_address_15_8. */
    hal_register_write(RG_SHORT_ADDR_1, short_address_byte);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will read the extended address used by the address
 *          filter.
 *
 *  \note In this function a pointer is used to convey the 64-bit result, since
 *        it is very inefficient to use the stack for this.
 *
 *  \return Extended Address, any 64-bit value.
 */
void
radio_get_extended_address(uint8_t *extended_address)
{
    *extended_address++ = hal_register_read(RG_IEEE_ADDR_0);
    *extended_address++ = hal_register_read(RG_IEEE_ADDR_1);
    *extended_address++ = hal_register_read(RG_IEEE_ADDR_2);
    *extended_address++ = hal_register_read(RG_IEEE_ADDR_3);
    *extended_address++ = hal_register_read(RG_IEEE_ADDR_4);
    *extended_address++ = hal_register_read(RG_IEEE_ADDR_5);
    *extended_address++ = hal_register_read(RG_IEEE_ADDR_6);
    *extended_address   = hal_register_read(RG_IEEE_ADDR_7);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will set a new extended address to be used by the
 *          address filter.
 *
 *  \param  extended_address Extended address to be used by the address filter.
 */
void
radio_set_extended_address(uint8_t *extended_address)
{
    hal_register_write(RG_IEEE_ADDR_0, *extended_address++);
    hal_register_write(RG_IEEE_ADDR_1, *extended_address++);
    hal_register_write(RG_IEEE_ADDR_2, *extended_address++);
    hal_register_write(RG_IEEE_ADDR_3, *extended_address++);
    hal_register_write(RG_IEEE_ADDR_4, *extended_address++);
    hal_register_write(RG_IEEE_ADDR_5, *extended_address++);
    hal_register_write(RG_IEEE_ADDR_6, *extended_address++);
    hal_register_write(RG_IEEE_ADDR_7, *extended_address++);
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will configure the CSMA algorithm used by the radio
 *          transceiver when transmitting data from TX_ARET_ON state.
 *
 *  \param  seed0 Lower 8 bits of the seed used for the random number generator
 *                in the CSMA algorithm. Value range: 0 to 255.
 *  \param  be_csma_seed1 Is a combined argument of the MIN_BE, MAX_CSMA_RETRIES
 *                        and SEED1 variables:
 *                        -# MIN_BE: Bit[7:6] Minimum back-off exponent in the
 *                           CSMA/CA algorithm.
 *                        -# MAX_CSMA_RETRIES: Bit[5:3] Number of retries in
 *                          TX_ARET_ON mode to repeat the CSMA/CA procedures
 *                          before the ARET procedure gives up.
 *                        -# SEED1: Bits[2:0] Higher 3 bits of CSMA_SEED, bits[10:8]
 *                           Seed for the random number generator in the
 *                           CSMA/CA algorithm.
 *  \retval RADIO_SUCCESS The CSMA algorithm was configured successfully.
 *  \retval RADIO_WRONG_STATE This function should not be called in the
 *                          SLEEP state.
 */
radio_status_t
radio_configure_csma(uint8_t seed0, uint8_t be_csma_seed1)
{

    /*Check state.*/
    if (radio_is_sleeping() == true){
        return RADIO_WRONG_STATE;
    }

    /*Extract parameters, and configure the CSMA-CA algorithm.*/
    uint8_t back_off_exponent = (be_csma_seed1 & 0xC0) >> 6;
    uint8_t csma_retries      = (be_csma_seed1 & 0x38) >> 3;
    uint8_t seed1             = (be_csma_seed1 & 0x07);

    hal_subregister_write(SR_MAX_FRAME_RETRIES, 0); /* AT86RF230 rev A errata. */
    hal_subregister_write(SR_MAX_CSMA_RETRIES, csma_retries);
    hal_subregister_write(SR_MIN_BE, back_off_exponent);
    hal_register_write(RG_CSMA_SEED_0, seed0);
    hal_subregister_write(SR_CSMA_SEED_1, seed1);

    return RADIO_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/**
    \brief Calibrate the internal RC oscillator

    This function calibrates the internal RC oscillator, based
    on the 1 MHz clock supplied by the AT86RF2xx. In order to
    verify the calibration result you can program the CKOUT fuse
    and monitor the CPU clock on an I/O pin.

    \return TRUE if calibrate passed; FALSE if calibrate failed.
*/
bool
calibrate_rc_osc_clkm(void)
{
        bool success = false;

    /*  Use the 1 MHz CLK_M from the AT86RF230. */
    uint16_t temp, counter;
    uint8_t osccal_saved;
    uint8_t tccr2b, tccr1b, tccr1a;

    /*  in the following line, 1000000ULL represents the 1MHz input signal */
    /*  from the radio.  265 is the number of counts to overflow 8-bit */
    /*  timer 2.  32 is the divide by 32 prescaler for timer 1.  F_CPU is */
    /*  the main clock frequency. */
#define TARGETVAL ((1000000ULL * 256 * 32) / F_CPU)


        osccal_saved = OSCCAL;
        cli();

        radio_set_clock_speed(true, CLKM_1MHz);

    /*  Save current values of timer status. */
        tccr2b = TCCR2B;
        tccr1b = TCCR1B;
        tccr1a = TCCR1A;

    /*  Stop timers 1 and 2. */
    /*  Set timer 1 to normal mode (no CTC, no PWM, just count). */
        TCCR2B = 0;
        TCCR1B = 0;
        TCCR1A = 0;

    for (counter = 0; counter < 1000;  counter++){
        /*  Delete pending timer 1 and 2 interrupts, and clear the */
        /*  counters. */
            TIFR1 = 0xFF;
            TIFR2 = 0xFF;
            TCNT2 = 0;
            TCNT1 = 0;
        /*  Timer 2 driven from clock divided by 32 */
            TCCR2B = (1 << CS21) | (1 << CS20);
        /*  Timer 1 driven with external clock */
            TCCR1B = (1 << CS12) | (1 << CS11);

        /*  Wait for timer 2 to overflow. */
        while (!(TIFR2 & (1 << TOV2))){
                ;
        }

        /*  Stop timer 1.  Now, TCNT1 contains the number of CPU cycles */
        /*  counted while timer 2 was counting */
            TCCR1B = 0;
            TCCR2B = 0;

            temp = TCNT1;

        if (temp < (uint16_t)(0.995 * TARGETVAL)){
            /*  Too fast, slow down */
                OSCCAL--;
        } else if (temp > (uint16_t)(1.005 * TARGETVAL)){
            /*  Too slow, speed up */
                OSCCAL++;
        } else {
            /*  We are within +/- 0.5 % of our target frequency, so we're */
            /*  done. */
                success = true;
                break;
            }
        }

        radio_set_clock_speed(true, CLKM_DISABLED);

    /*  restore timer status regs */
        TCCR2B = tccr2b;
        TCCR1B = tccr1b;
        TCCR1A = tccr1a;
    if (!success){
        /*  We failed, therefore restore previous OSCCAL value. */
            OSCCAL = osccal_saved;
        }

        return success;
}

/*----------------------------------------------------------------------------*/
/**
    \brief Calibrate the internal RC oscillator

    This function calibrates the internal RC oscillator, based
    on an external 32KHz crystal connected to TIMER2. In order to
    verify the calibration result you can program the CKOUT fuse
    and monitor the CPU clock on an I/O pin.
*/
void
calibrate_rc_osc_32k(void)
{
    /* Calibrate RC Oscillator: The calibration routine is done by clocking TIMER2
     * from the external 32kHz crystal while running an internal timer simultaneously.
     * The internal timer will be clocked at the same speed as the internal RC
     * oscillator, while TIMER2 is running at 32768 Hz. This way it is not necessary
     * to use a timed loop, and keep track cycles in timed loop vs. optimization
     * and compiler.
     */
    uint8_t osccal_original = OSCCAL;
    volatile uint16_t temp;
        
    /* This is bad practice, but seems to work. */
    OSCCAL = 0x80;


  //    PRR0 &= ~((1 << PRTIM2)|(1 << PRTIM1)); /*  Enable Timer 1 and 2 */

    TIMSK2 = 0x00; /*  Disable Timer/Counter 2 interrupts. */
    TIMSK1 = 0x00; /*  Disable Timer/Counter 1 interrupts. */

    /* Enable TIMER/COUNTER 2 to be clocked from the external 32kHz clock crystal.
     * Then wait for the timer to become stable before doing any calibration.
     */
    ASSR |= (1 << AS2);
    while (ASSR & ((1 << TCN2UB)|(1 << OCR2AUB)|(1 << TCR2AUB)|(1 << TCR2BUB))) { ; }
    TCCR2B = 1 << CS20;   /* run timer 2 at divide by 1 (32KHz) */

    AVR_ENTER_CRITICAL_REGION();

    uint8_t counter = 128;
    bool cal_ok = false;
    do{
        /* wait for timer to be ready for updated config */
        TCCR1B = 1 << CS10;

        while (ASSR & ((1 << TCN2UB)|(1 << OCR2AUB)|(1 << TCR2AUB)|(1 << TCR2BUB))) { ; }

        TCNT2 = 0x80;
        TCNT1 = 0;

        TIFR2 = 0xFF;

        /* Wait for TIMER/COUNTER 2 to overflow. Stop TIMER/COUNTER 1 and 2, and
         * read the counter value of TIMER/COUNTER 1. It will now contain the
         * number of cpu cycles elapsed within the period.
         */
        while (!(TIFR2 & (1 << TOV2))){
            ;
            }
        temp = TCNT1;

        TCCR1B = 0;

#define cal_upper (31250*1.05) // 32812 = 0x802c
#define cal_lower (31250*0.95) // 29687 = 0x73f7
        /* Iteratively reduce the error to be within limits */
        if (temp < cal_lower) {
            /* Too slow. Put the hammer down. */
            OSCCAL++;
        } else if (temp > cal_upper) {
            /* Too fast, retard. */
            OSCCAL--;
        } else {
            /* The CPU clock frequency is now within +/- 0.5% of the target value. */
            cal_ok = true;
        }

        counter--;
    } while ((counter != 0) && (false == cal_ok));

    if (true != cal_ok) {
        /* We failed, therefore restore previous OSCCAL value. */
        OSCCAL = osccal_original;
    }

    TCCR2B = 0;

    ASSR &= ~(1 << AS2);

    /* Disable both timers again to save power. */
    //    PRR0 |= (1 << PRTIM2);/* |(1 << PRTIM1); */

    AVR_LEAVE_CRITICAL_REGION();
}

/** @} */
/** @} */
/*EOF*/

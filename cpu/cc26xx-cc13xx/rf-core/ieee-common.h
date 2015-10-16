/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup rf-core-ieee
 * @{
 *
 *
 * \file
 * Header file for CC13xx/CC26xx common ieee functions
 *
 * These functions are shared between the asynchronous and regular ieee
 * mode drivers.
 */
/*---------------------------------------------------------------------------*/
#ifndef IEEE_COMMON_H_
#define IEEE_COMMON_H_
/*---------------------------------------------------------------------------*/
#include "dev/radio.h"

#include "rf-core/api/ieee_cmd.h"
#include "rtimer.h"
#include "rf-core/api/data_entry.h"

#include <stdint.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
/* Configuration to enable/disable auto ACKs in IEEE mode */
#ifdef IEEE_MODE_CONF_AUTOACK
#define IEEE_MODE_AUTOACK IEEE_MODE_CONF_AUTOACK
#else
#define IEEE_MODE_AUTOACK 1
#endif /* IEEE_MODE_CONF_AUTOACK */

/* Configuration to enable/disable frame filtering in IEEE mode */
#ifdef IEEE_MODE_CONF_PROMISCOUS
#define IEEE_MODE_PROMISCOUS IEEE_MODE_CONF_PROMISCOUS
#else
#define IEEE_MODE_PROMISCOUS 0
#endif /* IEEE_MODE_CONF_PROMISCOUS */

#ifdef IEEE_MODE_CONF_RSSI_THRESHOLD
#define IEEE_MODE_RSSI_THRESHOLD IEEE_MODE_CONF_RSSI_THRESHOLD
#else
#define IEEE_MODE_RSSI_THRESHOLD 0xA6
#endif /* IEEE_MODE_CONF_RSSI_THRESHOLD */
/*---------------------------------------------------------------------------*/
/* Data entry status field constants */
#define DATA_ENTRY_STATUS_PENDING    0x00 /* Not in use by the Radio CPU */
#define DATA_ENTRY_STATUS_ACTIVE     0x01 /* Open for r/w by the radio CPU */
#define DATA_ENTRY_STATUS_BUSY       0x02 /* Ongoing r/w */
#define DATA_ENTRY_STATUS_FINISHED   0x03 /* Free to use and to free */
#define DATA_ENTRY_STATUS_UNFINISHED 0x04 /* Partial RX entry */
/*---------------------------------------------------------------------------*/
/* The size of the RF commands buffer */
#define RF_CMD_BUFFER_SIZE             128
/*---------------------------------------------------------------------------*/
/* Special value returned by CMD_IEEE_CCA_REQ when an RSSI is not available */
#define RF_CMD_CCA_REQ_RSSI_UNKNOWN     -128

/* Used for the return value of channel_clear */
#define RF_CCA_CLEAR                       1
#define RF_CCA_BUSY                        0

/* Used as an error return value for get_cca_info */
#define RF_GET_CCA_INFO_ERROR           0xFF

/*
 * Values of the individual bits of the ccaInfo field in CMD_IEEE_CCA_REQ's
 * status struct
 */
#define RF_CMD_CCA_REQ_CCA_STATE_IDLE      0 /* 00 */
#define RF_CMD_CCA_REQ_CCA_STATE_BUSY      1 /* 01 */
#define RF_CMD_CCA_REQ_CCA_STATE_INVALID   2 /* 10 */
/*---------------------------------------------------------------------------*/
#define IEEE_MODE_CHANNEL_MIN            11
#define IEEE_MODE_CHANNEL_MAX            26
/*---------------------------------------------------------------------------*/
/* How long to wait for an ongoing ACK TX to finish before starting frame TX */
#define TX_WAIT_TIMEOUT       (RTIMER_SECOND >> 11)

/* How long to wait for the RF to enter RX in rf_cmd_ieee_rx */
#define ENTER_RX_WAIT_TIMEOUT (RTIMER_SECOND >> 10)
/*---------------------------------------------------------------------------*/
/* The outgoing frame buffer */
#define IEEE_TX_BUF_PAYLOAD_LEN 180
#define IEEE_TX_BUF_HDR_LEN       2

#define IEEE_TX_BUF_MAX_LEN (IEEE_TX_BUF_PAYLOAD_LEN + IEEE_TX_BUF_HDR_LEN)
/*---------------------------------------------------------------------------*/
#define IEEE_RX_BUF_SIZE 144
/*---------------------------------------------------------------------------*/
/* TX Power dBm lookup table - values from SmartRF Studio */
typedef struct output_config {
  radio_value_t dbm;
  uint8_t register_ib;
  uint8_t register_gc;
  uint8_t temp_coeff;
} output_config_t;
/*---------------------------------------------------------------------------*/
/**
 * \brief return true ifff channel is within valid range for this radio / mode
 */
static inline bool
ieee_mode_channel_valid(int chan)
{
  return (chan >= IEEE_MODE_CHANNEL_MIN) && (chan <= IEEE_MODE_CHANNEL_MAX);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief get a power configuration for the given radio RX power (in dbm)
 *
 * If the exact desired power is not avaiable, this function will always
 * round down to the next lowest avaiable configuration.
 *
 * \param power the desired transmitter power in dbm
 * \return a configuration for the desired transmitter power
 */
const output_config_t *ieee_common_get_power_config(radio_value_t power);
/*---------------------------------------------------------------------------*/
/**
 * \brief return the maximum dbm config struct
 */
const output_config_t *ieee_common_get_power_config_max(void);
/**
 * \brief return the minimum dbm config struct
 */
const output_config_t *ieee_common_get_power_config_min(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief return a list of default register overrides for ieee operation
 */
const uint32_t *ieee_common_get_overrides(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief sets up the given rx command to use default settings
 *
 * This module allocates an RX buffer queue. This queue is given to the cmd
 * struct for the radio to use in this function.
 */
void ieee_common_init_rf_params(rfc_CMD_IEEE_RX_t *cmd);
/*---------------------------------------------------------------------------*/
/**
 * \brief Checks whether the RFC domain is accessible and the RFC is in IEEE RX
 * \param cmd the RX radio command struct last given to the radio
 * \return 1: RFC in RX mode (and therefore accessible too). 0 otherwise
 */
uint8_t ieee_common_rf_is_on(rfc_CMD_IEEE_RX_t *cmd);
/*---------------------------------------------------------------------------*/
/**
 * \brief Initilizes the rx buffers
 */
void ieee_common_init_rx_buffers(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief sets all rx buffers to pending
 *
 * Necessarry to run prior to the next RX command, if the reciever was
 * stopped abruptly
 */
void ieee_common_set_rx_buffers_pending(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief set each rx buffer which is in the BUSY state to PENDING state
 */
void ieee_common_set_rx_pending_if_busy(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief wait up to max_wait seconds for an incoming packet to arrive
 *
 * The function returns immediatley if a packet has already been recieved
 * or if no packet is currently incoming.
 *
 * \param max_wait the max number of rtimer ticks to wait for an incoming
 * packet to arrive.
 * \return True if a packet is waiting in an rx buffer.
 */
int ieee_common_frame_wait(rtimer_clock_t max_wait);
/*---------------------------------------------------------------------------*/
/**
 * \brief reads data into the buffer (if any available)
 * \param buf buffer to read data into
 * \param buf_len the size of buf
 * \param rssi space in which to store the rssi value of the frame
 * \param lqi space in which to store the lqi value of the frame
 * \param timestamp space in which to store the radio timestamp (if configured
 * to use a radio timestamp) of the frame.
 * \return the number of bytes written to buf
 */
int ieee_common_read_frame(void *buf, uint16_t buf_len, int8_t *rssi,
                           uint8_t *lqi, uint32_t *timestamp);
/*---------------------------------------------------------------------------*/
/**
 * \brief returns true if any packets are pending being read
 * \return true ifff a packet is available to be read
 */
int ieee_common_pending_packet(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief returns true if any packets are recieving or recieved
 * \return true ifff a packet is incoming or already arrived
 */
int ieee_common_incoming_packet(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief initilize the data queue
 *
 * Must be called prior to using any of the RX functionality.
 */
void ieee_common_init_data_queue(void);
/*---------------------------------------------------------------------------*/
/**
 * \brief get the data entry which was most recently finished
 *
 * Gives the data entry in the queue which comes before the active entry. If
 * a rx just finished this will point to the packet which has most recently
 * finished.
 */
volatile rfc_dataEntry_t *ieee_common_last_data_entry(void);
/*---------------------------------------------------------------------------*/
#endif /* IEEE_COMMON_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
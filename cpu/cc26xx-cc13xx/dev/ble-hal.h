/*
 * Copyright (c) 2017, Graz University of Technology
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

/**
 * \file
 * 		hardware abstraction for a BLE controller
 *
 * \author
 * 		Michael Spoerk <michael.spoerk@tugraz.at>
 */
/*---------------------------------------------------------------------------*/

#ifndef BLE_HAL_H_
#define BLE_HAL_H_

#include <stddef.h>
#include "ble-addr.h"

/*---------------------------------------------------------------------------*/
/* Advertisement channel definitions                                         */
#define BLE_ADV_DATA_LEN         31
#define BLE_SCAN_RESP_DATA_LEN   31
#define BLE_ADV_CHANNEL_1        37
#define BLE_ADV_CHANNEL_1_MASK   0b001
#define BLE_ADV_CHANNEL_2        38
#define BLE_ADV_CHANNEL_2_MASK   0b010
#define BLE_ADV_CHANNEL_3        39
#define BLE_ADV_CHANNEL_3_MASK   0b100
#define BLE_ADV_INTERVAL_MIN     20
#define BLE_ADV_INTERVAL_MAX     0x4000
#define BLE_SCAN_INTERVAL_MIN	 0x0004
#define BLE_SCAN_INTERVAL_MAX	 0x4000
/*---------------------------------------------------------------------------*/
/* Data channel definitions                                                  */
#define BLE_DATA_CHANNEL_MIN     0
#define BLE_DATA_CHANNEL_MAX     36
/* Types of data PDU frames                                                  */
#define BLE_DATA_PDU_LLID_DATA_FRAGMENT     0b01
#define BLE_DATA_PDU_LLID_DATA_MESSAGE      0b10
#define BLE_DATA_PDU_LLID_CONTROL           0b11
/*---------------------------------------------------------------------------*/
/* Types of LL control PDUs                                                  */
#define BLE_LL_CONN_UPDATE_REQ              0x00
#define BLE_LL_CHANNEL_MAP_REQ              0x01
#define BLE_LL_TERMINATE_IND                0x02
#define BLE_LL_ENC_REQ                      0x03
#define BLE_LL_ENC_RSP                      0x04
#define BLE_LL_START_ENC_REQ                0x05
#define BLE_LL_START_ENC_RSP                0x06
#define BLE_LL_UNKNOWN_RSP                  0x07
#define BLE_LL_FEATURE_REQ                  0x08
#define BLE_LL_FEATURE_RSP                  0x09
#define BLE_LL_PAUSE_ENC_REQ                0x0A
#define BLE_LL_PAUSE_ENC_RSP                0x0B
#define BLE_LL_VERSION_IND                  0x0C
#define BLE_LL_REJECT_IND                   0x0D
#define BLE_LL_SLAVE_FEATURE_REQ            0x0E
#define BLE_LL_CONN_PARAM_REQ               0x0F
#define BLE_LL_CONN_PARAM_RSP               0x10
#define BLE_LL_REJECT_IND_EXT               0x11
#define BLE_LL_PING_REQ                     0x12
#define BLE_LL_PING_RSP                     0x13
/*---------------------------------------------------------------------------*/
#define FRAME_BLE_RX_EVENT					0x00	/* signaling that data was received (standard) */
#define FRAME_BLE_TX_EVENT					0x10	/* signaling that data was successfully sent */
#define FRAME_BLE_CONNECTION_EVENT			0x20	/* signaling that a new BLE connection was established */
#define FRAME_BLE_CONNECTION_UPDATED		0x30	/* signaling that the BLE connection parameter were successfully updated */
/*---------------------------------------------------------------------------*/
/* Return values for functions of ble_controller_driver implementations      */
typedef enum {
  BLE_RESULT_OK,
  BLE_RESULT_NOT_SUPPORTED,
  BLE_RESULT_INVALID_PARAM,
  BLE_RESULT_ERROR
} ble_result_t;

/*---------------------------------------------------------------------------*/
/* Advertising modes of BLE */
typedef enum {
  /* connectable undirected advertising */
  BLE_ADV_IND,

  /* connectable directed advertising (high duty cycle) */
  BLE_ADV_DIR_IND_HDC,

  /* scannable undirected advertising */
  BLE_ADV_SCAN_IND,

  /* non connectable undirected advertising */
  BLE_ADV_NONCONN_IND,

  /* connectable directed advertising (low duty cycle) */
  BLE_ADV_DIR_IND_LDC
} ble_adv_type_t;

/*---------------------------------------------------------------------------*/
/* Scanning modes of BLE */
typedef enum {
	/* no SCAN REQUESTS are sent */
  BLE_SCAN_PASSIVE,
  /* SCAN REQUESTS may be sent */
  BLE_SCAN_ACTIVE
} ble_scan_type_t;

/*---------------------------------------------------------------------------*/
/* Scanning filter policy */
typedef enum {
	/* accept all advertisement packets */
	BLE_SCAN_FILTER_POLICY_ACCEPT,
	/* ignore all advertisement packets from devices not on the white list */
	BLE_SCAN_FILTER_POLICY_IGNORE
} ble_scan_filter_policy_t;
/*---------------------------------------------------------------------------*/
/* List of packets to be sent by RDC layer */
struct ble_buf_list {
  struct ble_buf_list *next;
  struct queuebuf *buf;
  void *ptr;
};

/*---------------------------------------------------------------------------*/
/* Extension of the RADIO_PARAM fields for the BLE radios                    */
enum {
  /* start with 100 to be sure to not interfere with the standard values*/
  /*-----------------------------------------------------------------------*/
  /* BLE controller general                                                */
  /* The bluetooth device address */
  RADIO_CONST_BLE_BD_ADDR = 100,

  /* the size of a single BLE command buffer */
  RADIO_CONST_BLE_BUFFER_SIZE,

  /* the amount of single BLE command buffers */
  RADIO_CONST_BLE_BUFFER_AMOUNT,

  /*-----------------------------------------------------------------------*/
  /* BLE advertisement                                                     */

  /* advertisement interval */
  RADIO_PARAM_BLE_ADV_INTERVAL,

  /* BLE advertisement type (directed/undirected, ...) */
  RADIO_PARAM_BLE_ADV_TYPE,

  /* type of own address during advertisement */
  RADIO_PARAM_BLE_ADV_OWN_ADDR_TYPE,

  /* advertisement channel map */
  RADIO_PARAM_BLE_ADV_CHANNEL_MAP,

  /* advertisement payload */
  RADIO_PARAM_BLE_ADV_PAYLOAD,

  /* scan response payload */
  RADIO_PARAM_BLE_ADV_SCAN_RESPONSE,

  /* 1: enable advertisement / 0: disable advertisement */
  RADIO_PARAM_BLE_ADV_ENABLE,

  /*-----------------------------------------------------------------------*/
  /* BLE scanning                                                          */

  /* scanning interval */
  RADIO_PARAM_BLE_SCAN_INTERVAL,

  /* scanning window */
  RADIO_PARAM_BLE_SCAN_WINDOW,

  /* BLE scanning type (active/passive) */
  RADIO_PARAM_BLE_SCAN_TYPE,

  /* type of own address during scanning */
  RADIO_PARAM_BLE_SCAN_OWN_ADDR_TYPE,

  /* scanning channel */
  RADIO_PARAM_BLE_SCAN_CHANNEL,

  /* 1: enable scanning / 0: disable scanning */
  RADIO_PARAM_BLE_SCAN_ENABLE,

  /*-----------------------------------------------------------------------*/
  /* BLE initiating                                                        */
  /* The initiating command uses some parameters from scanning 			   */
  /* (scan interval, window, address type)								   */

  /* address type of the advertising device */
  RADIO_PARAM_BLE_PEER_ADDR_TYPE,

  /* address of the advertising device */
  RADIO_PARAM_BLE_PEER_ADDR,

  /* connection interval */
  RADIO_PARAM_BLE_CONN_INTERVAL,

  /* slave latency */
  RADIO_PARAM_BLE_CONN_LATENCY,

  /* supervision timeout */
  RADIO_PARAM_BLE_CONN_SUPERVISION_TIMEOUT,

  /* 1: start connection / 0: cancel connection creation */
  RADIO_PARAM_BLE_INITIATOR_ENABLE,

  RADIO_PARAM_BLE_CONN_UPDATE
};

/*---------------------------------------------------------------------------*/
/**
 * The structure of a ble radio controller driver in Contiki.
 */
struct ble_hal_driver {

  /*------------------------------------------------------------------------*/
  /* GENERAL COMMANDS                                                       */
  /**
   *  Resets the BLE controller
   */
  ble_result_t (*reset)(void);

  /**
   * Reads the static BLE device address.
   *
   * \param addr the static device address
   */
  ble_result_t (*read_bd_addr)(uint8_t *addr);

  /**
   * Reads the size of the data buffers.
   *
   * \param buf_len the length of a single data buffer
   * \param num_buf the number of data buffers
   */
  ble_result_t (*read_buffer_size) (unsigned int *buf_len,
                                    unsigned int *num_buf);

  /*------------------------------------------------------------------------*/
  /* ADVERTISING COMMANDS                                                   */
  /**
   * Sets the parameter for advertising.
   *
   * \param adv_interval advertising interval
   *                     (interval = adv_interval * 0.625 ms)
   * \param type type of advertising
   * \param own_addr_type indicator if own address is public/random
   * \param adv_channel_map map of advertising channels to use
   */
  ble_result_t (*set_adv_param) (unsigned int adv_interval,
                                 ble_adv_type_t type,
                                 ble_addr_type_t own_addr_type,
                                 unsigned short adv_channel_map);

  /**
   * Reads the used power on the advertisement channels.
   *
   * \param power the used power in dBm
   */
  ble_result_t (*read_adv_channel_tx_power) (short *power);

  /**
   * Sets the advertising data.
   *
   * \param data_len the length of the advertising data
   * \param data the data to advertise
   */
  ble_result_t (*set_adv_data) (unsigned short data_len,
                                char *data);

  /**
   * Sets the scan response data.
   *
   * \param data_len the length of the scan response data
   * \param data the data of a scan response
   */
  ble_result_t (*set_scan_resp_data) (unsigned short data_len,
                                      char *data);

  /**
   * Enables/disables advertising.
   *
   * \param enable if 1 then enable advertising, otherwise disable
   */
  ble_result_t (*set_adv_enable) (unsigned short enable);

  /*------------------------------------------------------------------------*/
  /* SCANNING COMMANDS                                                      */
  /**
   * Sets the parameter for scanning.
   *
   * \param type scan mode
   * \param scan_interval scan interval (interval = scan_interval * 0.625 ms)
   * \param scan_window scan window (window = scan_window * 0.625 ms)
   * \param own_addr_type indicator if own address is public/random
   */
  ble_result_t (*set_scan_param) (ble_scan_type_t type,
                                  unsigned int scan_interval,
                                  unsigned int scan_window,
                                  ble_addr_type_t own_addr_type);

  /**
   * Enables/disables scanning.
   *
   * \param enable 1: enable scanning, otherwise disable
   * \param filter_duplicates: 1: filter duplicates, otherwise no filtering
   */
  ble_result_t (*set_scan_enable) (unsigned short enable,
                                   unsigned short filter_duplicates);

  /*------------------------------------------------------------------------*/
  /* INITIATING COMMANDS                                                    */
  /**
   * Initiates the creation of a BLE connection.
   *
   * \param scan_interval scan interval (interval = scan_interval * 0.625 ms)
   * \param scan_window scan window (window = scan_window * 0.625 ms)
   * \param peer_addr_type indicator if peer address is public/random
   * \param peer_addr ble address of the device to connect to
   * \param own_addr_type indicator if own address is public/random
   * \param conn_interval connection interval
   *                      (interval = conn_interval * 1.25 ms)
   * \param conn_latency slave latency
   * \param supervision_timeout (timeout = supervision_timeout * 10 ms)
   */
  ble_result_t (*create_connection) (unsigned int scan_interval,
                                     unsigned int scan_window,
                                     ble_addr_type_t peer_addr_type,
                                     uint8_t *peer_addr,
                                     ble_addr_type_t own_addr_type,
                                     unsigned int conn_interval,
                                     unsigned int conn_latency,
                                     unsigned int supervision_timeout);

  /**
   * Cancels the initiation of a BLE connection.
   */
  ble_result_t (*create_connection_cancel) (void);

  /*------------------------------------------------------------------------*/
  /* CONNECTION COMMANDS                                                    */
  /**
   * Updates the connection parameters.
   * \param conn_interval connection interval
   *                      (interval = conn_interval * 1.25 ms)
   * \param conn_latency slave latency
   * \param supervision_timeout (timeout = supervision_timeout * 10 ms)
   */
  ble_result_t (*connection_update) (unsigned int connection_handle,
                                     unsigned int conn_interval,
                                     unsigned int conn_latency,
                                     unsigned int supervision_timeout);

  /**
   * Disconnects the connection.
   *
   * \param connection_handle
   * \param reason see error codes of Bluetooth specification
   */
  ble_result_t (*disconnect) (unsigned int connection_handle,
                              unsigned short reason);

  ble_result_t (*send) (void *buf, unsigned short buf_len);

  ble_result_t (*send_list) (struct ble_buf_list *list);

  ble_result_t (*read_connection_interval) (unsigned int conn_handle,
		  	  	  	  	  	  	  	  	    unsigned int *conn_interval_ms);
};

#endif /* BLE_HAL_H_ */

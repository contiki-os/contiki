/*
 * Copyright (c) 2016, Michael Spoerk
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
 *
 * Author: Michael Spoerk <mi.spoerk@gmail.com>
 *
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
#define BLE_ADV_INTERVAL_MIN     0x0020
#define BLE_ADV_INTERVAL_MAX     0x4000
/*---------------------------------------------------------------------------*/
/* Data channel definitions                                                  */
#define BLE_DATA_CHANNEL_MIN     0
#define BLE_DATA_CHANNEL_MAX     36
/* Types of data PDU frames                                                  */
#define BLE_DATA_PDU_LLID_DATA_FRAGMENT     0b01
#define BLE_DATA_PDU_LLID_DATA_MESSAGE      0b10
#define BLE_DATA_PDU_LLID_CONTROL           0b11
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
  BLE_SCAN_PASSIVE,
  BLE_SCAN_ACTIVE
} ble_scan_type_t;

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

  /* minimum advertisement interval
   * value can range from RADIO_CONST_BLE_ADV_INTERVAL_MIN to
   * RADIO_CONST_BLE_ADV_INTERVAL_MAX and must not be greater than
   * RADIO_RARAM_BLE_ADV_INTERVAL_MAX */
  RADIO_PARAM_BLE_ADV_INTERVAL,

  /* minimum advertisement interval according to standard */
  RADIO_CONST_BLE_ADV_INTERVAL_MIN,

  /* maximum advertisement interval according to standard */
  RADIO_CONST_BLE_ADV_INTERVAL_MAX,

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
  RADIO_PARAM_BLE_ADV_ENABLE
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
};

#endif /* BLE_HAL_H_ */

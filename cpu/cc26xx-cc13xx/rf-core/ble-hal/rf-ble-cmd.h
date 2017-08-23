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
 * 		BLE commands for the TI CC26xx BLE radio
 *
 * \author
 * 		Michael Spoerk <michael.spoerk@tugraz.at>
 */
/*---------------------------------------------------------------------------*/

#ifndef RF_BLE_CMD_H_
#define RF_BLE_CMD_H_

#include "ble-hal.h"
#include "rf-core/api/common_cmd.h"
#include "ble-addr.h"

#define RF_BLE_CMD_OK    1
#define RF_BLE_CMD_ERROR 0

unsigned short rf_ble_cmd_send(uint8_t *cmd);

unsigned short rf_ble_cmd_wait(uint8_t *cmd);

unsigned short rf_ble_cmd_setup_ble_mode(void);

void rf_ble_cmd_create_adv_cmd(uint8_t *command, uint8_t channel,
                               uint8_t *param, uint8_t *output);

void rf_ble_cmd_create_adv_params(uint8_t *param, dataQueue_t *rx_queue,
                                  uint8_t adv_data_len, uint8_t *adv_data,
                                  uint8_t scan_resp_data_len, uint8_t *scan_resp_data,
                                  ble_addr_type_t own_addr_type, uint8_t *own_addr);

void rf_ble_cmd_create_scan_cmd(uint8_t *command, uint8_t channel,
                           uint8_t *param, uint8_t *output);

void rf_ble_cmd_create_scan_params(uint8_t *param, dataQueue_t *rx_queue,
							 ble_scan_type_t scan_type, uint32_t scan_window,
                             ble_addr_type_t own_addr_type, uint8_t *own_addr,
							 ble_scan_filter_policy_t filter_policy,
							 uint8_t first_packet);

void rf_ble_cmd_create_initiator_cmd(uint8_t *cmd, uint8_t channel, uint8_t *params,
								uint8_t *output, uint32_t start_time);

void rf_ble_cmd_create_initiator_params(uint8_t *param, dataQueue_t *rx_queue,
							 uint32_t initiator_window,
                             ble_addr_type_t own_addr_type, uint8_t *own_addr,
							 ble_addr_type_t peer_addr_type, uint8_t *peer_addr,
							 uint32_t connect_time,
							 uint8_t *conn_req_data);


void rf_ble_cmd_create_slave_cmd(uint8_t *cmd, uint8_t channel, uint8_t *params,
                                 uint8_t *output, uint32_t start_time);

void rf_ble_cmd_create_slave_params(uint8_t *params, dataQueue_t *rx_queue,
                                    dataQueue_t *tx_queue, uint32_t access_address,
                                    uint8_t crc_init_0, uint8_t crc_init_1,
                                    uint8_t crc_init_2, uint32_t win_size,
                                    uint32_t window_widening, uint8_t first_packet);

void rf_ble_cmd_create_master_cmd(uint8_t *cmd, uint8_t channel, uint8_t *params,
                            uint8_t *output, uint32_t start_time);

void rf_ble_cmd_create_master_params(uint8_t *params, dataQueue_t *rx_queue,
                               dataQueue_t *tx_queue, uint32_t access_address,
                               uint8_t crc_init_0, uint8_t crc_init_1,
                               uint8_t crc_init_2, uint8_t first_packet);

unsigned short rf_ble_cmd_add_data_queue_entry(dataQueue_t *q, uint8_t *e);

#endif /* RF_BLE_CMD_H_ */

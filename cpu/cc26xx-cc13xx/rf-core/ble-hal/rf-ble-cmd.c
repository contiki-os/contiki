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
#include "contiki.h"

#include "rf-core/api/ble_cmd.h"
#include "rf-core/rf-core.h"
#include "rf-core/ble-hal/rf-ble-cmd.h"

/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
typedef struct tx_power_config {
  uint8_t register_ib;
  uint8_t register_gc;
  uint8_t temp_coeff;
} tx_power_config_t;

/* values for a selection of available TX powers (values from SmartRF Studio) */
/*static tx_power_config_t tx_power  = {0x30, 0x00, 0x93};    // +5 dBm */
static tx_power_config_t tx_power = { 0x21, 0x01, 0x31 };    /*  0 dBm */
/*static tx_power_config_t tx_power  = {0x0E, 0x01, 0x19};    // -9 dBm */
/*static tx_power_config_t tx_power  = {0x07, 0x03, 0x0C};    //-21 dBm */

/*---------------------------------------------------------------------------*/
/* BLE overrides */
static uint32_t ble_overrides[] = {
  0x00364038, /* Synth: Set RTRIM (POTAILRESTRIM) to 6 */
  0x000784A3, /* Synth: Set FREF = 3.43 MHz (24 MHz / 7) */
  0xA47E0583, /* Synth: Set loop bandwidth after lock to 80 kHz (K2) */
  0xEAE00603, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB) */
  0x00010623, /* Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB) */
  0x00456088, /* Adjust AGC reference level */
  0x008F88B3, /* GPIO mode: https://e2e.ti.com/support/wireless_connectivity/proprietary_sub_1_ghz_simpliciti/f/156/t/488244?*/
  0xFFFFFFFF, /* End of override list */
};
/*---------------------------------------------------------------------------*/
/* GENERAL functions                                                         */
static void
print_cmdsta(uint32_t cmdsta)
{
  uint8_t val = (uint8_t)(cmdsta & RF_CORE_CMDSTA_RESULT_MASK);

  if(val == RF_CORE_CMDSTA_PENDING) {
    PRINTF("CMDSTA: PENDING\n");
  } else if(val == RF_CORE_CMDSTA_DONE) {
    PRINTF("CMDSTA: DONE\n");
  } else if(val == RF_CORE_CMDSTA_ILLEGAL_PTR) {
    PRINTF("CMDSTA: ILLEGAL_PTR\n");
  } else if(val == RF_CORE_CMDSTA_UNKNOWN_CMD) {
    PRINTF("CMDSTA: UNKNOWN_CMD\n");
  } else if(val == RF_CORE_CMDSTA_UNKNOWN_DIR_CMD) {
    PRINTF("CMDSTA: UNKNOWN_DIR_CMD\n");
  } else if(val == RF_CORE_CMDSTA_CONTEXT_ERR) {
    PRINTF("CMDSTA: CONTEXT_ERR\n");
  } else if(val == RF_CORE_CMDSTA_SCHEDULING_ERR) {
    PRINTF("CMDSTA: SCHEDULING_ERR\n");
  } else if(val == RF_CORE_CMDSTA_PAR_ERR) {
    PRINTF("CMDSTA: PAR_ERR\n");
  } else if(val == RF_CORE_CMDSTA_QUEUE_ERR) {
    PRINTF("CMDSTA: QUEUE_ERR\n");
  } else if(val == RF_CORE_CMDSTA_QUEUE_BUSY) {
    PRINTF("CMDSTA: QUEUE_BUSY\n");
  } else {
    PRINTF("CMDSTA: 0x%08lx\n", cmdsta);
  }
}
/*---------------------------------------------------------------------------*/
static void
print_command_status(uint16_t status_field)
{
  if(status_field == RF_CORE_RADIO_OP_STATUS_IDLE) {
    PRINTF("Status: IDLE\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_PENDING) {
    PRINTF("Status: PENDING\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ACTIVE) {
    PRINTF("Status: ACTIVE\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_PAST_START) {
    PRINTF("Status: ERROR_PAST_START\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_START_TRIG) {
    PRINTF("Status: ERROR_START_TRIG\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_CONDITION) {
    PRINTF("Status: ERROR_CONDITION\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_PAR) {
    PRINTF("Status: ERROR_PAR\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_POINTER) {
    PRINTF("Status: ERROR_POINTER\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_CMDID) {
    PRINTF("Status: ERROR_CMDID\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_NO_SETUP) {
    PRINTF("Status: ERROR_NO_SETUP\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_NO_FS) {
    PRINTF("Status: ERROR_NO_FS\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_ERROR_SYNTH_PROG) {
    PRINTF("Status: ERROR_SYNTH_PROG\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_OK) {
    PRINTF("Status: BLE_DONE_OK\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_RXTIMEOUT) {
    PRINTF("Status: BLE_DONE_RXTIMEOUT\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_NOSYNC) {
    PRINTF("Status: BLE_DONE_NOSYNC\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_RXERR) {
    PRINTF("Status: BLE_DONE_RXERR\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_CONNECT) {
    PRINTF("Status: BLE_DONE_CONNECT\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_MAXNACK) {
    PRINTF("Status: BLE_DONE_MAXNACK\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_ENDED) {
    PRINTF("Status: BLE_DONE_ENDED\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_ABORT) {
    PRINTF("Status: BLE_DONE_ABORT\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_DONE_STOPPED) {
    PRINTF("Status: BLE_DONE_STOPPED\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_ERROR_PAR) {
    PRINTF("Status: BLE_ERROR_PAR\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_ERROR_RXBUF) {
    PRINTF("Status: BLE_ERROR_RXBUF\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_ERROR_NO_SETUP) {
    PRINTF("Status: BLE_ERROR_NO_SETUP\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_ERROR_NO_FS) {
    PRINTF("Status: BLE_ERROR_NO_FS\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_ERROR_SYNTH_PROG) {
    PRINTF("Status: BLE_ERROR_SYNTH_PROT\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_ERROR_RXOVF) {
    PRINTF("Status: BLE_ERROR_RXOVF\n");
  } else if(status_field == RF_CORE_RADIO_OP_STATUS_BLE_ERROR_TXUNF) {
    PRINTF("Status: BLE_ERROR_TXUNF\n");
  } else {
    PRINTF("cmd->status: 0x%04X\n", status_field);
  }
}
/*---------------------------------------------------------------------------*/
unsigned short
rf_ble_cmd_send(uint8_t *command)
{
  uint32_t cmdsta;
  rfc_radioOp_t *cmd = (rfc_radioOp_t *)command;

  if(rf_core_send_cmd((uint32_t)cmd, &cmdsta) != RF_CORE_CMD_OK) {
    PRINTF("rf_ble_cmd_send() could not send cmd\n");
    print_cmdsta(cmdsta);
    print_command_status(cmd->status);
    return RF_BLE_CMD_ERROR;
  }
  return RF_BLE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
unsigned short
rf_ble_cmd_wait(uint8_t *command)
{
  rfc_radioOp_t *cmd = (rfc_radioOp_t *)command;
  if(rf_core_wait_cmd_done((void *)cmd) != RF_CORE_CMD_OK) {
    PRINTF("rf_ble_cmd_wait() could not wait\n");
    print_command_status(cmd->status);
    return RF_BLE_CMD_ERROR;
  }
  return RF_BLE_CMD_OK;
}
/*---------------------------------------------------------------------------*/
unsigned short
rf_ble_cmd_setup_ble_mode(void)
{
  rfc_CMD_RADIO_SETUP_t cmd;

  /* Create radio setup command */
  rf_core_init_radio_op((rfc_radioOp_t *)&cmd, sizeof(cmd), CMD_RADIO_SETUP);

  cmd.txPower.IB = tx_power.register_ib;
  cmd.txPower.GC = tx_power.register_gc;
  cmd.txPower.tempCoeff = tx_power.temp_coeff;
  cmd.pRegOverride = ble_overrides;
  cmd.mode = 0;

  /* Send Radio setup to RF Core */
  if(rf_ble_cmd_send((uint8_t *)&cmd) != RF_BLE_CMD_OK) {
    return RF_BLE_CMD_ERROR;
  }

  /* Wait until radio setup is done */
  return rf_ble_cmd_wait((uint8_t *)&cmd);
}
/*---------------------------------------------------------------------------*/
/* ADVERTISING functions                                                     */
void
rf_ble_cmd_create_adv_cmd(uint8_t *command, uint8_t channel,
                          uint8_t *param, uint8_t *output)
{
  rfc_CMD_BLE_ADV_t *c = (rfc_CMD_BLE_ADV_t *)command;

  memset(c, 0x00, sizeof(rfc_CMD_BLE_ADV_t));
  c->commandNo = CMD_BLE_ADV;
  c->condition.rule = COND_NEVER;
  c->whitening.bOverride = 0;
  c->channel = channel;
  c->pParams = (rfc_bleAdvPar_t *)param;
  c->startTrigger.triggerType = TRIG_NOW;
  c->pOutput = (rfc_bleAdvOutput_t *)output;
}
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_adv_params(uint8_t *param, dataQueue_t *rx_queue,
                             uint8_t adv_data_len, uint8_t *adv_data,
                             uint8_t scan_resp_data_len, uint8_t *scan_resp_data,
                             ble_addr_type_t own_addr_type, uint8_t *own_addr)
{
  rfc_bleAdvPar_t *p = (rfc_bleAdvPar_t *)param;

  memset(p, 0x00, sizeof(rfc_bleAdvPar_t));

  p->pRxQ = rx_queue;
  p->rxConfig.bAutoFlushIgnored = 1;
  p->rxConfig.bAutoFlushCrcErr = 0;
  p->rxConfig.bAutoFlushEmpty = 1;
  p->rxConfig.bIncludeLenByte = 1;
  p->rxConfig.bIncludeCrc = 0;
  p->rxConfig.bAppendRssi = 1;
  p->rxConfig.bAppendStatus = 1;
  p->rxConfig.bAppendTimestamp = 1;
  p->advConfig.advFilterPolicy = 0;
  p->advConfig.bStrictLenFilter = 0;
  p->advConfig.deviceAddrType = own_addr_type;
  p->pDeviceAddress = (uint16_t *)own_addr;
  p->advLen = adv_data_len;
  p->scanRspLen = scan_resp_data_len;
  p->pAdvData = adv_data;
  p->pScanRspData = scan_resp_data;
  p->endTrigger.triggerType = TRIG_NEVER;
}
/*---------------------------------------------------------------------------*/
/* SCANNING functions                                                        */
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_scan_cmd(uint8_t *command, uint8_t channel,
                           uint8_t *param, uint8_t *output)
{
	rfc_CMD_BLE_SCANNER_t *c = (rfc_CMD_BLE_SCANNER_t *) command;

	memset(c, 0x00, sizeof(rfc_CMD_BLE_SCANNER_t));
	c->commandNo = CMD_BLE_SCANNER;
	c->condition.rule = COND_NEVER;
	c->whitening.bOverride = 0;
	c->channel = channel;
	c->pParams = (rfc_bleScannerPar_t *) param;
	c->startTrigger.triggerType = TRIG_NOW;
	c->pOutput = (rfc_bleScannerOutput_t *) output;
}
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_scan_params(uint8_t *param, dataQueue_t *rx_queue,
							 ble_scan_type_t scan_type, uint32_t scan_window,
                             ble_addr_type_t own_addr_type, uint8_t *own_addr,
							 ble_scan_filter_policy_t filter_policy,
							 uint8_t first_packet)
{
	rfc_bleScannerPar_t *p = (rfc_bleScannerPar_t *) param;

	if(first_packet) {
		/*
		 * only reset the memory for the first packet when in scanning mode.
		 * values for backoff procedure are incremented by the RFcore.
		 */
		memset(p, 0x00, sizeof(rfc_bleScannerPar_t));
		p->randomState = 0;
		p->backoffCount = 1;
		p->backoffPar.logUpperLimit = 0;
		p->backoffPar.bLastSucceeded = 0;
		p->backoffPar.bLastFailed = 0;
	}
	p->pRxQ = rx_queue;
	p->rxConfig.bAutoFlushIgnored = 1;
	p->rxConfig.bAutoFlushCrcErr = 0;
	p->rxConfig.bAutoFlushEmpty = 1;
	p->rxConfig.bIncludeLenByte = 1;
	p->rxConfig.bIncludeCrc = 0;
	p->rxConfig.bAppendRssi = 1;
	p->rxConfig.bAppendStatus = 1;
	p->rxConfig.bAppendTimestamp = 1;
	p->scanConfig.scanFilterPolicy = 0;
	p->scanConfig.bActiveScan = scan_type;
	p->scanConfig.deviceAddrType = own_addr_type;
	p->scanConfig.bStrictLenFilter = 1;

	if(filter_policy == BLE_SCAN_FILTER_POLICY_ACCEPT) {
		p->scanConfig.bAutoWlIgnore = 1;
	} else {
		p->scanConfig.bAutoWlIgnore = 0;
	}
	p->scanConfig.bEndOnRpt = 0;
	p->scanReqLen = 0;
	p->pScanReqData = NULL;
	p->pDeviceAddress = (uint16_t *) own_addr;
	p->pWhiteList = NULL;
	p->timeoutTrigger.triggerType = TRIG_REL_START;
	p->timeoutTime = scan_window;
	p->endTrigger.triggerType = TRIG_NEVER;
}
/*---------------------------------------------------------------------------*/
/* INITIATOR functions                                                       */
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_initiator_cmd(uint8_t *cmd, uint8_t channel, uint8_t *params,
								uint8_t *output, uint32_t start_time)
{
	rfc_CMD_BLE_INITIATOR_t *c = (rfc_CMD_BLE_INITIATOR_t *) cmd;

	memset(c, 0x00, sizeof(rfc_CMD_BLE_INITIATOR_t));

	c->commandNo = CMD_BLE_INITIATOR;
	c->condition.rule = COND_NEVER;
	c->whitening.bOverride = 0;
	c->channel = channel;
	c->pParams = (rfc_bleInitiatorPar_t *)params;
	c->startTrigger.triggerType = TRIG_ABSTIME;
	c->startTrigger.pastTrig = 1;
	c->startTime = start_time;
	c->pOutput = (rfc_bleInitiatorOutput_t *)output;
}
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_initiator_params(uint8_t *param, dataQueue_t *rx_queue,
							 uint32_t initiator_time,
                             ble_addr_type_t own_addr_type, uint8_t *own_addr,
							 ble_addr_type_t peer_addr_type, uint8_t *peer_addr,
							 uint32_t connect_time, uint8_t *conn_req_data)
{
	rfc_bleInitiatorPar_t *p = (rfc_bleInitiatorPar_t *) param;

	p->pRxQ = rx_queue;
	p->rxConfig.bAutoFlushIgnored = 1;
	p->rxConfig.bAutoFlushCrcErr = 0;
	p->rxConfig.bAutoFlushEmpty = 1;
	p->rxConfig.bIncludeLenByte = 1;
	p->rxConfig.bIncludeCrc = 0;
	p->rxConfig.bAppendRssi = 1;
	p->rxConfig.bAppendStatus = 1;
	p->rxConfig.bAppendTimestamp = 1;

//	p->initConfig.bUseWhiteList = 0;
	p->initConfig.bUseWhiteList = 1;
	p->initConfig.bDynamicWinOffset = 0;
	p->initConfig.deviceAddrType = own_addr_type;
	p->initConfig.peerAddrType = peer_addr_type;
	p->initConfig.bStrictLenFilter = 1;

	p->connectReqLen = 22;
	p->pConnectReqData = conn_req_data;
	p->pDeviceAddress = (uint16_t *) own_addr;
	p->pWhiteList = (rfc_bleWhiteListEntry_t *) peer_addr;
	p->connectTime = connect_time;
	p->timeoutTrigger.triggerType = TRIG_REL_START;
	p->timeoutTime = initiator_time;
	p->endTrigger.triggerType = TRIG_NEVER;
}
/*---------------------------------------------------------------------------*/
/* CONNECTION slave functions                                                */
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_slave_cmd(uint8_t *cmd, uint8_t channel, uint8_t *params,
                            uint8_t *output, uint32_t start_time)
{
  rfc_CMD_BLE_SLAVE_t *c = (rfc_CMD_BLE_SLAVE_t *)cmd;

  memset(c, 0x00, sizeof(rfc_CMD_BLE_SLAVE_t));

  c->commandNo = CMD_BLE_SLAVE;
  c->condition.rule = COND_NEVER;
  c->whitening.bOverride = 0;
  c->channel = channel;
  c->pParams = (rfc_bleSlavePar_t *)params;
  c->startTrigger.triggerType = TRIG_ABSTIME;
  c->startTrigger.pastTrig = 0;
  c->startTime = start_time;
  c->pOutput = (rfc_bleMasterSlaveOutput_t *)output;
}
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_slave_params(uint8_t *params, dataQueue_t *rx_queue,
                               dataQueue_t *tx_queue, uint32_t access_address,
                               uint8_t crc_init_0, uint8_t crc_init_1,
                               uint8_t crc_init_2, uint32_t win_size,
                               uint32_t window_widening, uint8_t first_packet)
{
  rfc_bleSlavePar_t *p = (rfc_bleSlavePar_t *)params;

  p->pRxQ = rx_queue;
  p->pTxQ = tx_queue;
  p->rxConfig.bAutoFlushIgnored = 1;
  p->rxConfig.bAutoFlushCrcErr = 1;
  p->rxConfig.bAutoFlushEmpty = 1;
  p->rxConfig.bIncludeLenByte = 1;
  p->rxConfig.bIncludeCrc = 0;
  p->rxConfig.bAppendRssi = 1;
  p->rxConfig.bAppendStatus = 1;
  p->rxConfig.bAppendTimestamp = 1;

  if(first_packet) {
    /* set parameters for first packet according to TI Technical Reference Manual */
    p->seqStat.lastRxSn = 1;
    p->seqStat.lastTxSn = 1;
    p->seqStat.nextTxSn = 0;
    p->seqStat.bFirstPkt = 1;
    p->seqStat.bAutoEmpty = 0;
    p->seqStat.bLlCtrlTx = 0;
    p->seqStat.bLlCtrlAckRx = 0;
    p->seqStat.bLlCtrlAckPending = 0;
  }

  p->maxNack = 0;
  p->maxPkt = 0;
  p->accessAddress = access_address;
  p->crcInit0 = crc_init_0;
  p->crcInit1 = crc_init_1;
  p->crcInit2 = crc_init_2;
  p->timeoutTrigger.triggerType = TRIG_REL_START;
  if(first_packet) {
	  p->timeoutTime = (uint32_t)(10 * win_size);
  }
  else {
	  p->timeoutTime = (uint32_t)(win_size + 2 * window_widening);
  }
  p->endTrigger.triggerType = TRIG_NEVER;
}
/*---------------------------------------------------------------------------*/
/* CONNECTION master functions                                               */
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_master_cmd(uint8_t *cmd, uint8_t channel, uint8_t *params,
                            uint8_t *output, uint32_t start_time)
{
  rfc_CMD_BLE_MASTER_t *c = (rfc_CMD_BLE_MASTER_t *)cmd;

  memset(c, 0x00, sizeof(rfc_CMD_BLE_MASTER_t));

  c->commandNo = CMD_BLE_MASTER;
  c->condition.rule = COND_NEVER;
  c->whitening.bOverride = 0;
  c->channel = channel;
  c->pParams = (rfc_bleMasterPar_t *)params;
  c->startTrigger.triggerType = TRIG_ABSTIME;
  c->startTrigger.pastTrig = 0;
  c->startTime = start_time;
  c->pOutput = (rfc_bleMasterSlaveOutput_t *)output;
}
/*---------------------------------------------------------------------------*/
void
rf_ble_cmd_create_master_params(uint8_t *params, dataQueue_t *rx_queue,
                               dataQueue_t *tx_queue, uint32_t access_address,
                               uint8_t crc_init_0, uint8_t crc_init_1,
                               uint8_t crc_init_2, uint8_t first_packet)
{
	rfc_bleMasterPar_t *p = (rfc_bleMasterPar_t *) params;

	p->pRxQ = rx_queue;
	p->pTxQ = tx_queue;
	p->rxConfig.bAutoFlushIgnored = 1;
	p->rxConfig.bAutoFlushCrcErr = 1;
	p->rxConfig.bAutoFlushEmpty = 1;
	p->rxConfig.bIncludeLenByte = 1;
	p->rxConfig.bIncludeCrc = 0;
	p->rxConfig.bAppendRssi = 1;
	p->rxConfig.bAppendStatus = 1;
	p->rxConfig.bAppendTimestamp = 1;

  if(first_packet) {
    /* set parameters for first packet according to TI Technical Reference Manual */
    p->seqStat.lastRxSn = 1;
    p->seqStat.lastTxSn = 1;
    p->seqStat.nextTxSn = 0;
    p->seqStat.bFirstPkt = 1;
    p->seqStat.bAutoEmpty = 0;
    p->seqStat.bLlCtrlTx = 0;
    p->seqStat.bLlCtrlAckRx = 0;
    p->seqStat.bLlCtrlAckPending = 0;
  }

  p->maxPkt = 12;
  p->accessAddress = access_address;
  p->crcInit0 = crc_init_0;
  p->crcInit1 = crc_init_1;
  p->crcInit2 = crc_init_2;
  p->endTrigger.triggerType = TRIG_REL_START;
  p->endTime = (uint32_t) 15 * 4000;	// a connection event must end after 10 ms
}
/*---------------------------------------------------------------------------*/
/* DATA queue functions                                                      */
/*---------------------------------------------------------------------------*/
unsigned short
rf_ble_cmd_add_data_queue_entry(dataQueue_t *q, uint8_t *e)
{
  uint32_t cmdsta;

  rfc_CMD_ADD_DATA_ENTRY_t cmd;
  cmd.commandNo = CMD_ADD_DATA_ENTRY;
  cmd.pQueue = q;
  cmd.pEntry = e;

  if(rf_core_send_cmd((uint32_t)&cmd, &cmdsta) != RF_CORE_CMD_OK) {
    PRINTF("rf_ble_cmd_add_data_queue_entry: ");
    print_cmdsta(cmdsta);
    return RF_BLE_CMD_ERROR;
  }
  return RF_BLE_CMD_OK;
}

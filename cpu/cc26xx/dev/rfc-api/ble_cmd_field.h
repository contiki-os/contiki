/******************************************************************************
*  Filename:       ble_cmd_field.h
*  Revised:        $ $
*  Revision:       $ $
*
*  Description:    CC26xx/CC13xx API for Bluetooth Low Energy commands
*
*  Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __BLE_CMD_FIELD_H
#define __BLE_CMD_FIELD_H

#include <stdint.h>
#include "mailbox.h"
#include "common_cmd.h"

#define _POSITION_bleRadioOp_channel                            14
#define _TYPE_bleRadioOp_channel                                uint8_t
#define _POSITION_bleRadioOp_whitening                          15
#define _TYPE_bleRadioOp_whitening                              uint8_t
#define _BITPOS_bleRadioOp_whitening_init                       0
#define _NBITS_bleRadioOp_whitening_init                        7
#define _BITPOS_bleRadioOp_whitening_bOverride                  7
#define _NBITS_bleRadioOp_whitening_bOverride                   1
#define _POSITION_bleRadioOp_pParams                            16
#define _TYPE_bleRadioOp_pParams                                uint8_t*
#define _POSITION_bleRadioOp_pOutput                            20
#define _TYPE_bleRadioOp_pOutput                                uint8_t*
#define _SIZEOF_bleRadioOp                                      24

#define _SIZEOF_CMD_BLE_SLAVE                                   24

#define _SIZEOF_CMD_BLE_MASTER                                  24

#define _SIZEOF_CMD_BLE_ADV                                     24

#define _SIZEOF_CMD_BLE_ADV_DIR                                 24

#define _SIZEOF_CMD_BLE_ADV_NC                                  24

#define _SIZEOF_CMD_BLE_ADV_SCAN                                24

#define _SIZEOF_CMD_BLE_SCANNER                                 24

#define _SIZEOF_CMD_BLE_INITIATOR                               24

#define _SIZEOF_CMD_BLE_GENERIC_RX                              24

#define _SIZEOF_CMD_BLE_TX_TEST                                 24

#define _POSITION_CMD_BLE_ADV_PAYLOAD_payloadType               2
#define _TYPE_CMD_BLE_ADV_PAYLOAD_payloadType                   uint8_t
#define _POSITION_CMD_BLE_ADV_PAYLOAD_newLen                    3
#define _TYPE_CMD_BLE_ADV_PAYLOAD_newLen                        uint8_t
#define _POSITION_CMD_BLE_ADV_PAYLOAD_pNewData                  4
#define _TYPE_CMD_BLE_ADV_PAYLOAD_pNewData                      uint8_t*
#define _POSITION_CMD_BLE_ADV_PAYLOAD_pParams                   8
#define _TYPE_CMD_BLE_ADV_PAYLOAD_pParams                       uint8_t*
#define _SIZEOF_CMD_BLE_ADV_PAYLOAD                             12

#define _POSITION_bleMasterSlavePar_pRxQ                        0
#define _TYPE_bleMasterSlavePar_pRxQ                            dataQueue_t*
#define _POSITION_bleMasterSlavePar_pTxQ                        4
#define _TYPE_bleMasterSlavePar_pTxQ                            dataQueue_t*
#define _POSITION_bleMasterSlavePar_rxConfig                    8
#define _TYPE_bleMasterSlavePar_rxConfig                        uint8_t
#define _BITPOS_bleMasterSlavePar_rxConfig_bAutoFlushIgnored    0
#define _NBITS_bleMasterSlavePar_rxConfig_bAutoFlushIgnored     1
#define _BITPOS_bleMasterSlavePar_rxConfig_bAutoFlushCrcErr     1
#define _NBITS_bleMasterSlavePar_rxConfig_bAutoFlushCrcErr      1
#define _BITPOS_bleMasterSlavePar_rxConfig_bAutoFlushEmpty      2
#define _NBITS_bleMasterSlavePar_rxConfig_bAutoFlushEmpty       1
#define _BITPOS_bleMasterSlavePar_rxConfig_bIncludeLenByte      3
#define _NBITS_bleMasterSlavePar_rxConfig_bIncludeLenByte       1
#define _BITPOS_bleMasterSlavePar_rxConfig_bIncludeCrc          4
#define _NBITS_bleMasterSlavePar_rxConfig_bIncludeCrc           1
#define _BITPOS_bleMasterSlavePar_rxConfig_bAppendRssi          5
#define _NBITS_bleMasterSlavePar_rxConfig_bAppendRssi           1
#define _BITPOS_bleMasterSlavePar_rxConfig_bAppendStatus        6
#define _NBITS_bleMasterSlavePar_rxConfig_bAppendStatus         1
#define _BITPOS_bleMasterSlavePar_rxConfig_bAppendTimestamp     7
#define _NBITS_bleMasterSlavePar_rxConfig_bAppendTimestamp      1
#define _POSITION_bleMasterSlavePar_seqStat                     9
#define _TYPE_bleMasterSlavePar_seqStat                         uint8_t
#define _BITPOS_bleMasterSlavePar_seqStat_lastRxSn              0
#define _NBITS_bleMasterSlavePar_seqStat_lastRxSn               1
#define _BITPOS_bleMasterSlavePar_seqStat_lastTxSn              1
#define _NBITS_bleMasterSlavePar_seqStat_lastTxSn               1
#define _BITPOS_bleMasterSlavePar_seqStat_nextTxSn              2
#define _NBITS_bleMasterSlavePar_seqStat_nextTxSn               1
#define _BITPOS_bleMasterSlavePar_seqStat_bFirstPkt             3
#define _NBITS_bleMasterSlavePar_seqStat_bFirstPkt              1
#define _BITPOS_bleMasterSlavePar_seqStat_bAutoEmpty            4
#define _NBITS_bleMasterSlavePar_seqStat_bAutoEmpty             1
#define _BITPOS_bleMasterSlavePar_seqStat_bLlCtrlTx             5
#define _NBITS_bleMasterSlavePar_seqStat_bLlCtrlTx              1
#define _BITPOS_bleMasterSlavePar_seqStat_bLlCtrlAckRx          6
#define _NBITS_bleMasterSlavePar_seqStat_bLlCtrlAckRx           1
#define _BITPOS_bleMasterSlavePar_seqStat_bLlCtrlAckPending     7
#define _NBITS_bleMasterSlavePar_seqStat_bLlCtrlAckPending      1
#define _POSITION_bleMasterSlavePar_maxNack                     10
#define _TYPE_bleMasterSlavePar_maxNack                         uint8_t
#define _POSITION_bleMasterSlavePar_maxPkt                      11
#define _TYPE_bleMasterSlavePar_maxPkt                          uint8_t
#define _POSITION_bleMasterSlavePar_accessAddress               12
#define _TYPE_bleMasterSlavePar_accessAddress                   uint32_t
#define _POSITION_bleMasterSlavePar_crcInit0                    16
#define _TYPE_bleMasterSlavePar_crcInit0                        uint8_t
#define _POSITION_bleMasterSlavePar_crcInit1                    17
#define _TYPE_bleMasterSlavePar_crcInit1                        uint8_t
#define _POSITION_bleMasterSlavePar_crcInit2                    18
#define _TYPE_bleMasterSlavePar_crcInit2                        uint8_t
#define _POSITION_bleMasterSlavePar_crcInit                     16
#define _TYPE_bleMasterSlavePar_crcInit                         uint32_t
#define _SIZEOF_bleMasterSlavePar                               20

#define _POSITION_bleMasterPar_endTrigger                       19
#define _TYPE_bleMasterPar_endTrigger                           uint8_t
#define _BITPOS_bleMasterPar_endTrigger_triggerType             0
#define _NBITS_bleMasterPar_endTrigger_triggerType              4
#define _BITPOS_bleMasterPar_endTrigger_bEnaCmd                 4
#define _NBITS_bleMasterPar_endTrigger_bEnaCmd                  1
#define _BITPOS_bleMasterPar_endTrigger_triggerNo               5
#define _NBITS_bleMasterPar_endTrigger_triggerNo                2
#define _BITPOS_bleMasterPar_endTrigger_pastTrig                7
#define _NBITS_bleMasterPar_endTrigger_pastTrig                 1
#define _POSITION_bleMasterPar_endTime                          20
#define _TYPE_bleMasterPar_endTime                              ratmr_t
#define _SIZEOF_bleMasterPar                                    24

#define _POSITION_bleSlavePar_timeoutTrigger                    19
#define _TYPE_bleSlavePar_timeoutTrigger                        uint8_t
#define _BITPOS_bleSlavePar_timeoutTrigger_triggerType          0
#define _NBITS_bleSlavePar_timeoutTrigger_triggerType           4
#define _BITPOS_bleSlavePar_timeoutTrigger_bEnaCmd              4
#define _NBITS_bleSlavePar_timeoutTrigger_bEnaCmd               1
#define _BITPOS_bleSlavePar_timeoutTrigger_triggerNo            5
#define _NBITS_bleSlavePar_timeoutTrigger_triggerNo             2
#define _BITPOS_bleSlavePar_timeoutTrigger_pastTrig             7
#define _NBITS_bleSlavePar_timeoutTrigger_pastTrig              1
#define _POSITION_bleSlavePar_timeoutTime                       20
#define _TYPE_bleSlavePar_timeoutTime                           ratmr_t
#define _POSITION_bleSlavePar_endTrigger                        27
#define _TYPE_bleSlavePar_endTrigger                            uint8_t
#define _BITPOS_bleSlavePar_endTrigger_triggerType              0
#define _NBITS_bleSlavePar_endTrigger_triggerType               4
#define _BITPOS_bleSlavePar_endTrigger_bEnaCmd                  4
#define _NBITS_bleSlavePar_endTrigger_bEnaCmd                   1
#define _BITPOS_bleSlavePar_endTrigger_triggerNo                5
#define _NBITS_bleSlavePar_endTrigger_triggerNo                 2
#define _BITPOS_bleSlavePar_endTrigger_pastTrig                 7
#define _NBITS_bleSlavePar_endTrigger_pastTrig                  1
#define _POSITION_bleSlavePar_endTime                           28
#define _TYPE_bleSlavePar_endTime                               ratmr_t
#define _SIZEOF_bleSlavePar                                     32

#define _POSITION_bleAdvPar_pRxQ                                0
#define _TYPE_bleAdvPar_pRxQ                                    dataQueue_t*
#define _POSITION_bleAdvPar_rxConfig                            4
#define _TYPE_bleAdvPar_rxConfig                                uint8_t
#define _BITPOS_bleAdvPar_rxConfig_bAutoFlushIgnored            0
#define _NBITS_bleAdvPar_rxConfig_bAutoFlushIgnored             1
#define _BITPOS_bleAdvPar_rxConfig_bAutoFlushCrcErr             1
#define _NBITS_bleAdvPar_rxConfig_bAutoFlushCrcErr              1
#define _BITPOS_bleAdvPar_rxConfig_bAutoFlushEmpty              2
#define _NBITS_bleAdvPar_rxConfig_bAutoFlushEmpty               1
#define _BITPOS_bleAdvPar_rxConfig_bIncludeLenByte              3
#define _NBITS_bleAdvPar_rxConfig_bIncludeLenByte               1
#define _BITPOS_bleAdvPar_rxConfig_bIncludeCrc                  4
#define _NBITS_bleAdvPar_rxConfig_bIncludeCrc                   1
#define _BITPOS_bleAdvPar_rxConfig_bAppendRssi                  5
#define _NBITS_bleAdvPar_rxConfig_bAppendRssi                   1
#define _BITPOS_bleAdvPar_rxConfig_bAppendStatus                6
#define _NBITS_bleAdvPar_rxConfig_bAppendStatus                 1
#define _BITPOS_bleAdvPar_rxConfig_bAppendTimestamp             7
#define _NBITS_bleAdvPar_rxConfig_bAppendTimestamp              1
#define _POSITION_bleAdvPar_advConfig                           5
#define _TYPE_bleAdvPar_advConfig                               uint8_t
#define _BITPOS_bleAdvPar_advConfig_advFilterPolicy             0
#define _NBITS_bleAdvPar_advConfig_advFilterPolicy              2
#define _BITPOS_bleAdvPar_advConfig_deviceAddrType              2
#define _NBITS_bleAdvPar_advConfig_deviceAddrType               1
#define _BITPOS_bleAdvPar_advConfig_peerAddrType                3
#define _NBITS_bleAdvPar_advConfig_peerAddrType                 1
#define _BITPOS_bleAdvPar_advConfig_bStrictLenFilter            4
#define _NBITS_bleAdvPar_advConfig_bStrictLenFilter             1
#define _POSITION_bleAdvPar_advLen                              6
#define _TYPE_bleAdvPar_advLen                                  uint8_t
#define _POSITION_bleAdvPar_scanRspLen                          7
#define _TYPE_bleAdvPar_scanRspLen                              uint8_t
#define _POSITION_bleAdvPar_pAdvData                            8
#define _TYPE_bleAdvPar_pAdvData                                uint8_t*
#define _POSITION_bleAdvPar_pScanRspData                        12
#define _TYPE_bleAdvPar_pScanRspData                            uint8_t*
#define _POSITION_bleAdvPar_pDeviceAddress                      16
#define _TYPE_bleAdvPar_pDeviceAddress                          uint16_t*
#define _POSITION_bleAdvPar_pWhiteList                          20
#define _TYPE_bleAdvPar_pWhiteList                              uint32_t*
#define _POSITION_bleAdvPar_endTrigger                          27
#define _TYPE_bleAdvPar_endTrigger                              uint8_t
#define _BITPOS_bleAdvPar_endTrigger_triggerType                0
#define _NBITS_bleAdvPar_endTrigger_triggerType                 4
#define _BITPOS_bleAdvPar_endTrigger_bEnaCmd                    4
#define _NBITS_bleAdvPar_endTrigger_bEnaCmd                     1
#define _BITPOS_bleAdvPar_endTrigger_triggerNo                  5
#define _NBITS_bleAdvPar_endTrigger_triggerNo                   2
#define _BITPOS_bleAdvPar_endTrigger_pastTrig                   7
#define _NBITS_bleAdvPar_endTrigger_pastTrig                    1
#define _POSITION_bleAdvPar_endTime                             28
#define _TYPE_bleAdvPar_endTime                                 ratmr_t
#define _SIZEOF_bleAdvPar                                       32

#define _POSITION_bleScannerPar_pRxQ                            0
#define _TYPE_bleScannerPar_pRxQ                                dataQueue_t*
#define _POSITION_bleScannerPar_rxConfig                        4
#define _TYPE_bleScannerPar_rxConfig                            uint8_t
#define _BITPOS_bleScannerPar_rxConfig_bAutoFlushIgnored        0
#define _NBITS_bleScannerPar_rxConfig_bAutoFlushIgnored         1
#define _BITPOS_bleScannerPar_rxConfig_bAutoFlushCrcErr         1
#define _NBITS_bleScannerPar_rxConfig_bAutoFlushCrcErr          1
#define _BITPOS_bleScannerPar_rxConfig_bAutoFlushEmpty          2
#define _NBITS_bleScannerPar_rxConfig_bAutoFlushEmpty           1
#define _BITPOS_bleScannerPar_rxConfig_bIncludeLenByte          3
#define _NBITS_bleScannerPar_rxConfig_bIncludeLenByte           1
#define _BITPOS_bleScannerPar_rxConfig_bIncludeCrc              4
#define _NBITS_bleScannerPar_rxConfig_bIncludeCrc               1
#define _BITPOS_bleScannerPar_rxConfig_bAppendRssi              5
#define _NBITS_bleScannerPar_rxConfig_bAppendRssi               1
#define _BITPOS_bleScannerPar_rxConfig_bAppendStatus            6
#define _NBITS_bleScannerPar_rxConfig_bAppendStatus             1
#define _BITPOS_bleScannerPar_rxConfig_bAppendTimestamp         7
#define _NBITS_bleScannerPar_rxConfig_bAppendTimestamp          1
#define _POSITION_bleScannerPar_scanConfig                      5
#define _TYPE_bleScannerPar_scanConfig                          uint8_t
#define _BITPOS_bleScannerPar_scanConfig_scanFilterPolicy       0
#define _NBITS_bleScannerPar_scanConfig_scanFilterPolicy        1
#define _BITPOS_bleScannerPar_scanConfig_bActiveScan            1
#define _NBITS_bleScannerPar_scanConfig_bActiveScan             1
#define _BITPOS_bleScannerPar_scanConfig_deviceAddrType         2
#define _NBITS_bleScannerPar_scanConfig_deviceAddrType          1
#define _BITPOS_bleScannerPar_scanConfig_bStrictLenFilter       4
#define _NBITS_bleScannerPar_scanConfig_bStrictLenFilter        1
#define _BITPOS_bleScannerPar_scanConfig_bAutoWlIgnore          5
#define _NBITS_bleScannerPar_scanConfig_bAutoWlIgnore           1
#define _BITPOS_bleScannerPar_scanConfig_bEndOnRpt              6
#define _NBITS_bleScannerPar_scanConfig_bEndOnRpt               1
#define _POSITION_bleScannerPar_randomState                     6
#define _TYPE_bleScannerPar_randomState                         uint16_t
#define _POSITION_bleScannerPar_backoffCount                    8
#define _TYPE_bleScannerPar_backoffCount                        uint16_t
#define _POSITION_bleScannerPar_backoffPar                      10
#define _TYPE_bleScannerPar_backoffPar                          uint8_t
#define _BITPOS_bleScannerPar_backoffPar_logUpperLimit          0
#define _NBITS_bleScannerPar_backoffPar_logUpperLimit           4
#define _BITPOS_bleScannerPar_backoffPar_bLastSucceeded         4
#define _NBITS_bleScannerPar_backoffPar_bLastSucceeded          1
#define _BITPOS_bleScannerPar_backoffPar_bLastFailed            5
#define _NBITS_bleScannerPar_backoffPar_bLastFailed             1
#define _POSITION_bleScannerPar_scanReqLen                      11
#define _TYPE_bleScannerPar_scanReqLen                          uint8_t
#define _POSITION_bleScannerPar_pScanReqData                    12
#define _TYPE_bleScannerPar_pScanReqData                        uint8_t*
#define _POSITION_bleScannerPar_pDeviceAddress                  16
#define _TYPE_bleScannerPar_pDeviceAddress                      uint16_t*
#define _POSITION_bleScannerPar_pWhiteList                      20
#define _TYPE_bleScannerPar_pWhiteList                          uint32_t*
#define _POSITION_bleScannerPar_timeoutTrigger                  26
#define _TYPE_bleScannerPar_timeoutTrigger                      uint8_t
#define _BITPOS_bleScannerPar_timeoutTrigger_triggerType        0
#define _NBITS_bleScannerPar_timeoutTrigger_triggerType         4
#define _BITPOS_bleScannerPar_timeoutTrigger_bEnaCmd            4
#define _NBITS_bleScannerPar_timeoutTrigger_bEnaCmd             1
#define _BITPOS_bleScannerPar_timeoutTrigger_triggerNo          5
#define _NBITS_bleScannerPar_timeoutTrigger_triggerNo           2
#define _BITPOS_bleScannerPar_timeoutTrigger_pastTrig           7
#define _NBITS_bleScannerPar_timeoutTrigger_pastTrig            1
#define _POSITION_bleScannerPar_endTrigger                      27
#define _TYPE_bleScannerPar_endTrigger                          uint8_t
#define _BITPOS_bleScannerPar_endTrigger_triggerType            0
#define _NBITS_bleScannerPar_endTrigger_triggerType             4
#define _BITPOS_bleScannerPar_endTrigger_bEnaCmd                4
#define _NBITS_bleScannerPar_endTrigger_bEnaCmd                 1
#define _BITPOS_bleScannerPar_endTrigger_triggerNo              5
#define _NBITS_bleScannerPar_endTrigger_triggerNo               2
#define _BITPOS_bleScannerPar_endTrigger_pastTrig               7
#define _NBITS_bleScannerPar_endTrigger_pastTrig                1
#define _POSITION_bleScannerPar_timeoutTime                     28
#define _TYPE_bleScannerPar_timeoutTime                         ratmr_t
#define _POSITION_bleScannerPar_endTime                         32
#define _TYPE_bleScannerPar_endTime                             ratmr_t
#define _SIZEOF_bleScannerPar                                   36

#define _POSITION_bleInitiatorPar_pRxQ                          0
#define _TYPE_bleInitiatorPar_pRxQ                              dataQueue_t*
#define _POSITION_bleInitiatorPar_rxConfig                      4
#define _TYPE_bleInitiatorPar_rxConfig                          uint8_t
#define _BITPOS_bleInitiatorPar_rxConfig_bAutoFlushIgnored      0
#define _NBITS_bleInitiatorPar_rxConfig_bAutoFlushIgnored       1
#define _BITPOS_bleInitiatorPar_rxConfig_bAutoFlushCrcErr       1
#define _NBITS_bleInitiatorPar_rxConfig_bAutoFlushCrcErr        1
#define _BITPOS_bleInitiatorPar_rxConfig_bAutoFlushEmpty        2
#define _NBITS_bleInitiatorPar_rxConfig_bAutoFlushEmpty         1
#define _BITPOS_bleInitiatorPar_rxConfig_bIncludeLenByte        3
#define _NBITS_bleInitiatorPar_rxConfig_bIncludeLenByte         1
#define _BITPOS_bleInitiatorPar_rxConfig_bIncludeCrc            4
#define _NBITS_bleInitiatorPar_rxConfig_bIncludeCrc             1
#define _BITPOS_bleInitiatorPar_rxConfig_bAppendRssi            5
#define _NBITS_bleInitiatorPar_rxConfig_bAppendRssi             1
#define _BITPOS_bleInitiatorPar_rxConfig_bAppendStatus          6
#define _NBITS_bleInitiatorPar_rxConfig_bAppendStatus           1
#define _BITPOS_bleInitiatorPar_rxConfig_bAppendTimestamp       7
#define _NBITS_bleInitiatorPar_rxConfig_bAppendTimestamp        1
#define _POSITION_bleInitiatorPar_initConfig                    5
#define _TYPE_bleInitiatorPar_initConfig                        uint8_t
#define _BITPOS_bleInitiatorPar_initConfig_bUseWhiteList        0
#define _NBITS_bleInitiatorPar_initConfig_bUseWhiteList         1
#define _BITPOS_bleInitiatorPar_initConfig_bDynamicWinOffset    1
#define _NBITS_bleInitiatorPar_initConfig_bDynamicWinOffset     1
#define _BITPOS_bleInitiatorPar_initConfig_deviceAddrType       2
#define _NBITS_bleInitiatorPar_initConfig_deviceAddrType        1
#define _BITPOS_bleInitiatorPar_initConfig_peerAddrType         3
#define _NBITS_bleInitiatorPar_initConfig_peerAddrType          1
#define _BITPOS_bleInitiatorPar_initConfig_bStrictLenFilter     4
#define _NBITS_bleInitiatorPar_initConfig_bStrictLenFilter      1
#define _POSITION_bleInitiatorPar_connectReqLen                 7
#define _TYPE_bleInitiatorPar_connectReqLen                     uint8_t
#define _POSITION_bleInitiatorPar_pConnectReqData               8
#define _TYPE_bleInitiatorPar_pConnectReqData                   uint8_t*
#define _POSITION_bleInitiatorPar_pDeviceAddress                12
#define _TYPE_bleInitiatorPar_pDeviceAddress                    uint16_t*
#define _POSITION_bleInitiatorPar_pWhiteList                    16
#define _TYPE_bleInitiatorPar_pWhiteList                        uint32_t*
#define _POSITION_bleInitiatorPar_connectTime                   20
#define _TYPE_bleInitiatorPar_connectTime                       ratmr_t
#define _POSITION_bleInitiatorPar_timeoutTrigger                26
#define _TYPE_bleInitiatorPar_timeoutTrigger                    uint8_t
#define _BITPOS_bleInitiatorPar_timeoutTrigger_triggerType      0
#define _NBITS_bleInitiatorPar_timeoutTrigger_triggerType       4
#define _BITPOS_bleInitiatorPar_timeoutTrigger_bEnaCmd          4
#define _NBITS_bleInitiatorPar_timeoutTrigger_bEnaCmd           1
#define _BITPOS_bleInitiatorPar_timeoutTrigger_triggerNo        5
#define _NBITS_bleInitiatorPar_timeoutTrigger_triggerNo         2
#define _BITPOS_bleInitiatorPar_timeoutTrigger_pastTrig         7
#define _NBITS_bleInitiatorPar_timeoutTrigger_pastTrig          1
#define _POSITION_bleInitiatorPar_endTrigger                    27
#define _TYPE_bleInitiatorPar_endTrigger                        uint8_t
#define _BITPOS_bleInitiatorPar_endTrigger_triggerType          0
#define _NBITS_bleInitiatorPar_endTrigger_triggerType           4
#define _BITPOS_bleInitiatorPar_endTrigger_bEnaCmd              4
#define _NBITS_bleInitiatorPar_endTrigger_bEnaCmd               1
#define _BITPOS_bleInitiatorPar_endTrigger_triggerNo            5
#define _NBITS_bleInitiatorPar_endTrigger_triggerNo             2
#define _BITPOS_bleInitiatorPar_endTrigger_pastTrig             7
#define _NBITS_bleInitiatorPar_endTrigger_pastTrig              1
#define _POSITION_bleInitiatorPar_timeoutTime                   28
#define _TYPE_bleInitiatorPar_timeoutTime                       ratmr_t
#define _POSITION_bleInitiatorPar_endTime                       32
#define _TYPE_bleInitiatorPar_endTime                           ratmr_t
#define _SIZEOF_bleInitiatorPar                                 36

#define _POSITION_bleGenericRxPar_pRxQ                          0
#define _TYPE_bleGenericRxPar_pRxQ                              dataQueue_t*
#define _POSITION_bleGenericRxPar_rxConfig                      4
#define _TYPE_bleGenericRxPar_rxConfig                          uint8_t
#define _BITPOS_bleGenericRxPar_rxConfig_bAutoFlushIgnored      0
#define _NBITS_bleGenericRxPar_rxConfig_bAutoFlushIgnored       1
#define _BITPOS_bleGenericRxPar_rxConfig_bAutoFlushCrcErr       1
#define _NBITS_bleGenericRxPar_rxConfig_bAutoFlushCrcErr        1
#define _BITPOS_bleGenericRxPar_rxConfig_bAutoFlushEmpty        2
#define _NBITS_bleGenericRxPar_rxConfig_bAutoFlushEmpty         1
#define _BITPOS_bleGenericRxPar_rxConfig_bIncludeLenByte        3
#define _NBITS_bleGenericRxPar_rxConfig_bIncludeLenByte         1
#define _BITPOS_bleGenericRxPar_rxConfig_bIncludeCrc            4
#define _NBITS_bleGenericRxPar_rxConfig_bIncludeCrc             1
#define _BITPOS_bleGenericRxPar_rxConfig_bAppendRssi            5
#define _NBITS_bleGenericRxPar_rxConfig_bAppendRssi             1
#define _BITPOS_bleGenericRxPar_rxConfig_bAppendStatus          6
#define _NBITS_bleGenericRxPar_rxConfig_bAppendStatus           1
#define _BITPOS_bleGenericRxPar_rxConfig_bAppendTimestamp       7
#define _NBITS_bleGenericRxPar_rxConfig_bAppendTimestamp        1
#define _POSITION_bleGenericRxPar_bRepeat                       5
#define _TYPE_bleGenericRxPar_bRepeat                           uint8_t
#define _POSITION_bleGenericRxPar_accessAddress                 8
#define _TYPE_bleGenericRxPar_accessAddress                     uint32_t
#define _POSITION_bleGenericRxPar_crcInit0                      12
#define _TYPE_bleGenericRxPar_crcInit0                          uint8_t
#define _POSITION_bleGenericRxPar_crcInit1                      13
#define _TYPE_bleGenericRxPar_crcInit1                          uint8_t
#define _POSITION_bleGenericRxPar_crcInit2                      14
#define _TYPE_bleGenericRxPar_crcInit2                          uint8_t
#define _POSITION_bleGenericRxPar_crcInit                       12
#define _TYPE_bleGenericRxPar_crcInit                           uint32_t
#define _POSITION_bleGenericRxPar_endTrigger                    15
#define _TYPE_bleGenericRxPar_endTrigger                        uint8_t
#define _BITPOS_bleGenericRxPar_endTrigger_triggerType          0
#define _NBITS_bleGenericRxPar_endTrigger_triggerType           4
#define _BITPOS_bleGenericRxPar_endTrigger_bEnaCmd              4
#define _NBITS_bleGenericRxPar_endTrigger_bEnaCmd               1
#define _BITPOS_bleGenericRxPar_endTrigger_triggerNo            5
#define _NBITS_bleGenericRxPar_endTrigger_triggerNo             2
#define _BITPOS_bleGenericRxPar_endTrigger_pastTrig             7
#define _NBITS_bleGenericRxPar_endTrigger_pastTrig              1
#define _POSITION_bleGenericRxPar_endTime                       16
#define _TYPE_bleGenericRxPar_endTime                           ratmr_t
#define _SIZEOF_bleGenericRxPar                                 20

#define _POSITION_bleTxTestPar_numPackets                       0
#define _TYPE_bleTxTestPar_numPackets                           uint16_t
#define _POSITION_bleTxTestPar_payloadLength                    2
#define _TYPE_bleTxTestPar_payloadLength                        uint8_t
#define _POSITION_bleTxTestPar_packetType                       3
#define _TYPE_bleTxTestPar_packetType                           uint8_t
#define _POSITION_bleTxTestPar_period                           4
#define _TYPE_bleTxTestPar_period                               ratmr_t
#define _POSITION_bleTxTestPar_config                           8
#define _TYPE_bleTxTestPar_config                               uint8_t
#define _BITPOS_bleTxTestPar_config_bOverrideDefault            0
#define _NBITS_bleTxTestPar_config_bOverrideDefault             1
#define _BITPOS_bleTxTestPar_config_bUsePrbs9                   1
#define _NBITS_bleTxTestPar_config_bUsePrbs9                    1
#define _BITPOS_bleTxTestPar_config_bUsePrbs15                  2
#define _NBITS_bleTxTestPar_config_bUsePrbs15                   1
#define _POSITION_bleTxTestPar_byteVal                          9
#define _TYPE_bleTxTestPar_byteVal                              uint8_t
#define _POSITION_bleTxTestPar_endTrigger                       11
#define _TYPE_bleTxTestPar_endTrigger                           uint8_t
#define _BITPOS_bleTxTestPar_endTrigger_triggerType             0
#define _NBITS_bleTxTestPar_endTrigger_triggerType              4
#define _BITPOS_bleTxTestPar_endTrigger_bEnaCmd                 4
#define _NBITS_bleTxTestPar_endTrigger_bEnaCmd                  1
#define _BITPOS_bleTxTestPar_endTrigger_triggerNo               5
#define _NBITS_bleTxTestPar_endTrigger_triggerNo                2
#define _BITPOS_bleTxTestPar_endTrigger_pastTrig                7
#define _NBITS_bleTxTestPar_endTrigger_pastTrig                 1
#define _POSITION_bleTxTestPar_endTime                          12
#define _TYPE_bleTxTestPar_endTime                              ratmr_t
#define _SIZEOF_bleTxTestPar                                    16

#define _POSITION_bleMasterSlaveOutput_nTx                      0
#define _TYPE_bleMasterSlaveOutput_nTx                          uint8_t
#define _POSITION_bleMasterSlaveOutput_nTxAck                   1
#define _TYPE_bleMasterSlaveOutput_nTxAck                       uint8_t
#define _POSITION_bleMasterSlaveOutput_nTxCtrl                  2
#define _TYPE_bleMasterSlaveOutput_nTxCtrl                      uint8_t
#define _POSITION_bleMasterSlaveOutput_nTxCtrlAck               3
#define _TYPE_bleMasterSlaveOutput_nTxCtrlAck                   uint8_t
#define _POSITION_bleMasterSlaveOutput_nTxCtrlAckAck            4
#define _TYPE_bleMasterSlaveOutput_nTxCtrlAckAck                uint8_t
#define _POSITION_bleMasterSlaveOutput_nTxRetrans               5
#define _TYPE_bleMasterSlaveOutput_nTxRetrans                   uint8_t
#define _POSITION_bleMasterSlaveOutput_nTxEntryDone             6
#define _TYPE_bleMasterSlaveOutput_nTxEntryDone                 uint8_t
#define _POSITION_bleMasterSlaveOutput_nRxOk                    7
#define _TYPE_bleMasterSlaveOutput_nRxOk                        uint8_t
#define _POSITION_bleMasterSlaveOutput_nRxCtrl                  8
#define _TYPE_bleMasterSlaveOutput_nRxCtrl                      uint8_t
#define _POSITION_bleMasterSlaveOutput_nRxCtrlAck               9
#define _TYPE_bleMasterSlaveOutput_nRxCtrlAck                   uint8_t
#define _POSITION_bleMasterSlaveOutput_nRxNok                   10
#define _TYPE_bleMasterSlaveOutput_nRxNok                       uint8_t
#define _POSITION_bleMasterSlaveOutput_nRxIgnored               11
#define _TYPE_bleMasterSlaveOutput_nRxIgnored                   uint8_t
#define _POSITION_bleMasterSlaveOutput_nRxEmpty                 12
#define _TYPE_bleMasterSlaveOutput_nRxEmpty                     uint8_t
#define _POSITION_bleMasterSlaveOutput_nRxBufFull               13
#define _TYPE_bleMasterSlaveOutput_nRxBufFull                   uint8_t
#define _POSITION_bleMasterSlaveOutput_lastRssi                 14
#define _TYPE_bleMasterSlaveOutput_lastRssi                     int8_t
#define _POSITION_bleMasterSlaveOutput_pktStatus                15
#define _TYPE_bleMasterSlaveOutput_pktStatus                    uint8_t
#define _BITPOS_bleMasterSlaveOutput_pktStatus_bTimeStampValid  0
#define _NBITS_bleMasterSlaveOutput_pktStatus_bTimeStampValid   1
#define _BITPOS_bleMasterSlaveOutput_pktStatus_bLastCrcErr      1
#define _NBITS_bleMasterSlaveOutput_pktStatus_bLastCrcErr       1
#define _BITPOS_bleMasterSlaveOutput_pktStatus_bLastIgnored     2
#define _NBITS_bleMasterSlaveOutput_pktStatus_bLastIgnored      1
#define _BITPOS_bleMasterSlaveOutput_pktStatus_bLastEmpty       3
#define _NBITS_bleMasterSlaveOutput_pktStatus_bLastEmpty        1
#define _BITPOS_bleMasterSlaveOutput_pktStatus_bLastCtrl        4
#define _NBITS_bleMasterSlaveOutput_pktStatus_bLastCtrl         1
#define _BITPOS_bleMasterSlaveOutput_pktStatus_bLastMd          5
#define _NBITS_bleMasterSlaveOutput_pktStatus_bLastMd           1
#define _BITPOS_bleMasterSlaveOutput_pktStatus_bLastAck         6
#define _NBITS_bleMasterSlaveOutput_pktStatus_bLastAck          1
#define _POSITION_bleMasterSlaveOutput_timeStamp                16
#define _TYPE_bleMasterSlaveOutput_timeStamp                    ratmr_t
#define _SIZEOF_bleMasterSlaveOutput                            20

#define _POSITION_bleAdvOutput_nTxAdvInd                        0
#define _TYPE_bleAdvOutput_nTxAdvInd                            uint16_t
#define _POSITION_bleAdvOutput_nTxScanRsp                       2
#define _TYPE_bleAdvOutput_nTxScanRsp                           uint8_t
#define _POSITION_bleAdvOutput_nRxScanReq                       3
#define _TYPE_bleAdvOutput_nRxScanReq                           uint8_t
#define _POSITION_bleAdvOutput_nRxConnectReq                    4
#define _TYPE_bleAdvOutput_nRxConnectReq                        uint8_t
#define _POSITION_bleAdvOutput_nRxNok                           6
#define _TYPE_bleAdvOutput_nRxNok                               uint16_t
#define _POSITION_bleAdvOutput_nRxIgnored                       8
#define _TYPE_bleAdvOutput_nRxIgnored                           uint16_t
#define _POSITION_bleAdvOutput_nRxBufFull                       10
#define _TYPE_bleAdvOutput_nRxBufFull                           uint8_t
#define _POSITION_bleAdvOutput_lastRssi                         11
#define _TYPE_bleAdvOutput_lastRssi                             int8_t
#define _POSITION_bleAdvOutput_timeStamp                        12
#define _TYPE_bleAdvOutput_timeStamp                            ratmr_t
#define _SIZEOF_bleAdvOutput                                    16

#define _POSITION_bleScannerOutput_nTxScanReq                   0
#define _TYPE_bleScannerOutput_nTxScanReq                       uint16_t
#define _POSITION_bleScannerOutput_nBackedOffScanReq            2
#define _TYPE_bleScannerOutput_nBackedOffScanReq                uint16_t
#define _POSITION_bleScannerOutput_nRxAdvOk                     4
#define _TYPE_bleScannerOutput_nRxAdvOk                         uint16_t
#define _POSITION_bleScannerOutput_nRxAdvIgnored                6
#define _TYPE_bleScannerOutput_nRxAdvIgnored                    uint16_t
#define _POSITION_bleScannerOutput_nRxAdvNok                    8
#define _TYPE_bleScannerOutput_nRxAdvNok                        uint16_t
#define _POSITION_bleScannerOutput_nRxScanRspOk                 10
#define _TYPE_bleScannerOutput_nRxScanRspOk                     uint16_t
#define _POSITION_bleScannerOutput_nRxScanRspIgnored            12
#define _TYPE_bleScannerOutput_nRxScanRspIgnored                uint16_t
#define _POSITION_bleScannerOutput_nRxScanRspNok                14
#define _TYPE_bleScannerOutput_nRxScanRspNok                    uint16_t
#define _POSITION_bleScannerOutput_nRxAdvBufFull                16
#define _TYPE_bleScannerOutput_nRxAdvBufFull                    uint8_t
#define _POSITION_bleScannerOutput_nRxScanRspBufFull            17
#define _TYPE_bleScannerOutput_nRxScanRspBufFull                uint8_t
#define _POSITION_bleScannerOutput_lastRssi                     18
#define _TYPE_bleScannerOutput_lastRssi                         int8_t
#define _POSITION_bleScannerOutput_timeStamp                    20
#define _TYPE_bleScannerOutput_timeStamp                        ratmr_t
#define _SIZEOF_bleScannerOutput                                24

#define _POSITION_bleInitiatorOutput_nTxConnectReq              0
#define _TYPE_bleInitiatorOutput_nTxConnectReq                  uint8_t
#define _POSITION_bleInitiatorOutput_nRxAdvOk                   1
#define _TYPE_bleInitiatorOutput_nRxAdvOk                       uint8_t
#define _POSITION_bleInitiatorOutput_nRxAdvIgnored              2
#define _TYPE_bleInitiatorOutput_nRxAdvIgnored                  uint16_t
#define _POSITION_bleInitiatorOutput_nRxAdvNok                  4
#define _TYPE_bleInitiatorOutput_nRxAdvNok                      uint16_t
#define _POSITION_bleInitiatorOutput_nRxAdvBufFull              6
#define _TYPE_bleInitiatorOutput_nRxAdvBufFull                  uint8_t
#define _POSITION_bleInitiatorOutput_lastRssi                   7
#define _TYPE_bleInitiatorOutput_lastRssi                       int8_t
#define _POSITION_bleInitiatorOutput_timeStamp                  8
#define _TYPE_bleInitiatorOutput_timeStamp                      ratmr_t
#define _SIZEOF_bleInitiatorOutput                              12

#define _POSITION_bleGenericRxOutput_nRxOk                      0
#define _TYPE_bleGenericRxOutput_nRxOk                          uint16_t
#define _POSITION_bleGenericRxOutput_nRxNok                     2
#define _TYPE_bleGenericRxOutput_nRxNok                         uint16_t
#define _POSITION_bleGenericRxOutput_nRxBufFull                 4
#define _TYPE_bleGenericRxOutput_nRxBufFull                     uint16_t
#define _POSITION_bleGenericRxOutput_lastRssi                   6
#define _TYPE_bleGenericRxOutput_lastRssi                       int8_t
#define _POSITION_bleGenericRxOutput_timeStamp                  8
#define _TYPE_bleGenericRxOutput_timeStamp                      ratmr_t
#define _SIZEOF_bleGenericRxOutput                              12

#define _POSITION_bleTxTestOutput_nTx                           0
#define _TYPE_bleTxTestOutput_nTx                               uint16_t
#define _SIZEOF_bleTxTestOutput                                 2

#define _POSITION_bleWhiteListEntry_size                        0
#define _TYPE_bleWhiteListEntry_size                            uint8_t
#define _POSITION_bleWhiteListEntry_conf                        1
#define _TYPE_bleWhiteListEntry_conf                            uint8_t
#define _BITPOS_bleWhiteListEntry_conf_bEnable                  0
#define _NBITS_bleWhiteListEntry_conf_bEnable                   1
#define _BITPOS_bleWhiteListEntry_conf_addrType                 1
#define _NBITS_bleWhiteListEntry_conf_addrType                  1
#define _BITPOS_bleWhiteListEntry_conf_bWlIgn                   2
#define _NBITS_bleWhiteListEntry_conf_bWlIgn                    1
#define _POSITION_bleWhiteListEntry_address                     2
#define _TYPE_bleWhiteListEntry_address                         uint16_t
#define _POSITION_bleWhiteListEntry_addressHi                   4
#define _TYPE_bleWhiteListEntry_addressHi                       uint32_t
#define _SIZEOF_bleWhiteListEntry                               8

#define _POSITION_bleRxStatus_status                            0
#define _TYPE_bleRxStatus_status                                uint8_t
#define _BITPOS_bleRxStatus_status_channel                      0
#define _NBITS_bleRxStatus_status_channel                       6
#define _BITPOS_bleRxStatus_status_bIgnore                      6
#define _NBITS_bleRxStatus_status_bIgnore                       1
#define _BITPOS_bleRxStatus_status_bCrcErr                      7
#define _NBITS_bleRxStatus_status_bCrcErr                       1
#define _SIZEOF_bleRxStatus                                     1

#endif

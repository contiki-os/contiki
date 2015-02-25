/******************************************************************************
*  Filename:       common_cmd_field.h
*  Revised:        $ $
*  Revision:       $ $
*
*  Description:    CC26xx API for common/generic commands
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

#ifndef __COMMON_CMD_FIELD_H
#define __COMMON_CMD_FIELD_H

#include <stdint.h>
#include "mailbox.h"

#define _POSITION_command_commandNo                             0
#define _TYPE_command_commandNo                                 uint16_t
#define _SIZEOF_command                                         2

#define _POSITION_radioOp_commandNo                             0
#define _TYPE_radioOp_commandNo                                 uint16_t
#define _POSITION_radioOp_status                                2
#define _TYPE_radioOp_status                                    uint16_t
#define _POSITION_radioOp_pNextOp                               4
#define _TYPE_radioOp_pNextOp                                   uint8_t*
#define _POSITION_radioOp_startTime                             8
#define _TYPE_radioOp_startTime                                 ratmr_t
#define _POSITION_radioOp_startTrigger                          12
#define _TYPE_radioOp_startTrigger                              uint8_t
#define _BITPOS_radioOp_startTrigger_triggerType                0
#define _NBITS_radioOp_startTrigger_triggerType                 4
#define _BITPOS_radioOp_startTrigger_bEnaCmd                    4
#define _NBITS_radioOp_startTrigger_bEnaCmd                     1
#define _BITPOS_radioOp_startTrigger_triggerNo                  5
#define _NBITS_radioOp_startTrigger_triggerNo                   2
#define _BITPOS_radioOp_startTrigger_pastTrig                   7
#define _NBITS_radioOp_startTrigger_pastTrig                    1
#define _POSITION_radioOp_condition                             13
#define _TYPE_radioOp_condition                                 uint8_t
#define _BITPOS_radioOp_condition_rule                          0
#define _NBITS_radioOp_condition_rule                           4
#define _BITPOS_radioOp_condition_nSkip                         4
#define _NBITS_radioOp_condition_nSkip                          4
#define _SIZEOF_radioOp                                         14

#define _SIZEOF_CMD_NOP                                         14

#define _POSITION_CMD_RADIO_SETUP_mode                          14
#define _TYPE_CMD_RADIO_SETUP_mode                              uint8_t
#define _POSITION_CMD_RADIO_SETUP_config                        16
#define _TYPE_CMD_RADIO_SETUP_config                            uint16_t
#define _BITPOS_CMD_RADIO_SETUP_config_frontEndMode             0
#define _NBITS_CMD_RADIO_SETUP_config_frontEndMode              3
#define _BITPOS_CMD_RADIO_SETUP_config_biasMode                 3
#define _NBITS_CMD_RADIO_SETUP_config_biasMode                  1
#define _BITPOS_CMD_RADIO_SETUP_config_bNoAdi0Setup             4
#define _NBITS_CMD_RADIO_SETUP_config_bNoAdi0Setup              1
#define _BITPOS_CMD_RADIO_SETUP_config_bNoAdi0Trim              5
#define _NBITS_CMD_RADIO_SETUP_config_bNoAdi0Trim               1
#define _BITPOS_CMD_RADIO_SETUP_config_bNoAdi0Ovr               6
#define _NBITS_CMD_RADIO_SETUP_config_bNoAdi0Ovr                1
#define _BITPOS_CMD_RADIO_SETUP_config_bNoAdi1Setup             7
#define _NBITS_CMD_RADIO_SETUP_config_bNoAdi1Setup              1
#define _BITPOS_CMD_RADIO_SETUP_config_bNoAdi1Trim              8
#define _NBITS_CMD_RADIO_SETUP_config_bNoAdi1Trim               1
#define _BITPOS_CMD_RADIO_SETUP_config_bNoAdi1Ovr               9
#define _NBITS_CMD_RADIO_SETUP_config_bNoAdi1Ovr                1
#define _BITPOS_CMD_RADIO_SETUP_config_bNoFsPowerUp             10
#define _NBITS_CMD_RADIO_SETUP_config_bNoFsPowerUp              1
#define _POSITION_CMD_RADIO_SETUP_txPower                       18
#define _TYPE_CMD_RADIO_SETUP_txPower                           uint16_t
#define _BITPOS_CMD_RADIO_SETUP_txPower_IB                      0
#define _NBITS_CMD_RADIO_SETUP_txPower_IB                       6
#define _BITPOS_CMD_RADIO_SETUP_txPower_GC                      6
#define _NBITS_CMD_RADIO_SETUP_txPower_GC                       2
#define _BITPOS_CMD_RADIO_SETUP_txPower_tempCoeff               8
#define _NBITS_CMD_RADIO_SETUP_txPower_tempCoeff                8
#define _POSITION_CMD_RADIO_SETUP_pRegOverride                  20
#define _TYPE_CMD_RADIO_SETUP_pRegOverride                      uint32_t*
#define _SIZEOF_CMD_RADIO_SETUP                                 24

#define _POSITION_CMD_FS_frequency                              14
#define _TYPE_CMD_FS_frequency                                  uint16_t
#define _POSITION_CMD_FS_fractFreq                              16
#define _TYPE_CMD_FS_fractFreq                                  uint16_t
#define _POSITION_CMD_FS_synthConf                              18
#define _TYPE_CMD_FS_synthConf                                  uint8_t
#define _BITPOS_CMD_FS_synthConf_bTxMode                        0
#define _NBITS_CMD_FS_synthConf_bTxMode                         1
#define _BITPOS_CMD_FS_synthConf_refFreq                        1
#define _NBITS_CMD_FS_synthConf_refFreq                         6
#define _POSITION_CMD_FS_calibConf                              19
#define _TYPE_CMD_FS_calibConf                                  uint8_t
#define _BITPOS_CMD_FS_calibConf_bOverrideCalib                 0
#define _NBITS_CMD_FS_calibConf_bOverrideCalib                  1
#define _BITPOS_CMD_FS_calibConf_bSkipTdcCalib                  1
#define _NBITS_CMD_FS_calibConf_bSkipTdcCalib                   1
#define _BITPOS_CMD_FS_calibConf_bSkipCoarseCalib               2
#define _NBITS_CMD_FS_calibConf_bSkipCoarseCalib                1
#define _BITPOS_CMD_FS_calibConf_bSkipMidCalib                  3
#define _NBITS_CMD_FS_calibConf_bSkipMidCalib                   1
#define _BITPOS_CMD_FS_calibConf_coarsePrecal                   4
#define _NBITS_CMD_FS_calibConf_coarsePrecal                    4
#define _POSITION_CMD_FS_midPrecal                              20
#define _TYPE_CMD_FS_midPrecal                                  uint8_t
#define _POSITION_CMD_FS_ktPrecal                               21
#define _TYPE_CMD_FS_ktPrecal                                   uint8_t
#define _POSITION_CMD_FS_tdcPrecal                              22
#define _TYPE_CMD_FS_tdcPrecal                                  uint16_t
#define _SIZEOF_CMD_FS                                          24

#define _SIZEOF_CMD_FS_OFF                                      14

#define _POSITION_CMD_RX_pktConfig                              14
#define _TYPE_CMD_RX_pktConfig                                  uint16_t
#define _BITPOS_CMD_RX_pktConfig_endianness                     0
#define _NBITS_CMD_RX_pktConfig_endianness                      1
#define _BITPOS_CMD_RX_pktConfig_numHdrBits                     1
#define _NBITS_CMD_RX_pktConfig_numHdrBits                      6
#define _BITPOS_CMD_RX_pktConfig_bFsOff                         7
#define _NBITS_CMD_RX_pktConfig_bFsOff                          1
#define _BITPOS_CMD_RX_pktConfig_bUseCrc                        8
#define _NBITS_CMD_RX_pktConfig_bUseCrc                         1
#define _BITPOS_CMD_RX_pktConfig_bCrcIncSw                      9
#define _NBITS_CMD_RX_pktConfig_bCrcIncSw                       1
#define _BITPOS_CMD_RX_pktConfig_bCrcIncHdr                     10
#define _NBITS_CMD_RX_pktConfig_bCrcIncHdr                      1
#define _BITPOS_CMD_RX_pktConfig_bReportCrc                     11
#define _NBITS_CMD_RX_pktConfig_bReportCrc                      1
#define _BITPOS_CMD_RX_pktConfig_endType                        12
#define _NBITS_CMD_RX_pktConfig_endType                         1
#define _BITPOS_CMD_RX_pktConfig_bDualSw                        13
#define _NBITS_CMD_RX_pktConfig_bDualSw                         1
#define _POSITION_CMD_RX_syncWord                               16
#define _TYPE_CMD_RX_syncWord                                   uint32_t
#define _POSITION_CMD_RX_syncWord2                              20
#define _TYPE_CMD_RX_syncWord2                                  uint32_t
#define _POSITION_CMD_RX_lenConfig                              24
#define _TYPE_CMD_RX_lenConfig                                  uint16_t
#define _BITPOS_CMD_RX_lenConfig_numLenBits                     0
#define _NBITS_CMD_RX_lenConfig_numLenBits                      4
#define _BITPOS_CMD_RX_lenConfig_lenFieldPos                    4
#define _NBITS_CMD_RX_lenConfig_lenFieldPos                     5
#define _BITPOS_CMD_RX_lenConfig_lenOffset                      9
#define _NBITS_CMD_RX_lenConfig_lenOffset                       7
#define _POSITION_CMD_RX_maxLen                                 26
#define _TYPE_CMD_RX_maxLen                                     uint16_t
#define _POSITION_CMD_RX_pRecPkt                                28
#define _TYPE_CMD_RX_pRecPkt                                    uint8_t*
#define _POSITION_CMD_RX_endTime                                32
#define _TYPE_CMD_RX_endTime                                    ratmr_t
#define _POSITION_CMD_RX_endTrigger                             36
#define _TYPE_CMD_RX_endTrigger                                 uint8_t
#define _BITPOS_CMD_RX_endTrigger_triggerType                   0
#define _NBITS_CMD_RX_endTrigger_triggerType                    4
#define _BITPOS_CMD_RX_endTrigger_bEnaCmd                       4
#define _NBITS_CMD_RX_endTrigger_bEnaCmd                        1
#define _BITPOS_CMD_RX_endTrigger_triggerNo                     5
#define _NBITS_CMD_RX_endTrigger_triggerNo                      2
#define _BITPOS_CMD_RX_endTrigger_pastTrig                      7
#define _NBITS_CMD_RX_endTrigger_pastTrig                       1
#define _POSITION_CMD_RX_rssi                                   37
#define _TYPE_CMD_RX_rssi                                       int8_t
#define _POSITION_CMD_RX_recLen                                 38
#define _TYPE_CMD_RX_recLen                                     uint16_t
#define _POSITION_CMD_RX_timeStamp                              40
#define _TYPE_CMD_RX_timeStamp                                  ratmr_t
#define _POSITION_CMD_RX_nRxOk                                  44
#define _TYPE_CMD_RX_nRxOk                                      uint16_t
#define _POSITION_CMD_RX_nRxNok                                 46
#define _TYPE_CMD_RX_nRxNok                                     uint16_t
#define _POSITION_CMD_RX_nRx2Ok                                 48
#define _TYPE_CMD_RX_nRx2Ok                                     uint16_t
#define _POSITION_CMD_RX_nRx2Nok                                50
#define _TYPE_CMD_RX_nRx2Nok                                    uint16_t
#define _SIZEOF_CMD_RX                                          52

#define _POSITION_CMD_TX_pktConfig                              14
#define _TYPE_CMD_TX_pktConfig                                  uint16_t
#define _BITPOS_CMD_TX_pktConfig_endianness                     0
#define _NBITS_CMD_TX_pktConfig_endianness                      1
#define _BITPOS_CMD_TX_pktConfig_numHdrBits                     1
#define _NBITS_CMD_TX_pktConfig_numHdrBits                      6
#define _BITPOS_CMD_TX_pktConfig_bFsOff                         7
#define _NBITS_CMD_TX_pktConfig_bFsOff                          1
#define _BITPOS_CMD_TX_pktConfig_bUseCrc                        8
#define _NBITS_CMD_TX_pktConfig_bUseCrc                         1
#define _BITPOS_CMD_TX_pktConfig_bCrcIncSw                      9
#define _NBITS_CMD_TX_pktConfig_bCrcIncSw                       1
#define _BITPOS_CMD_TX_pktConfig_bCrcIncHdr                     10
#define _NBITS_CMD_TX_pktConfig_bCrcIncHdr                      1
#define _POSITION_CMD_TX_syncWord                               16
#define _TYPE_CMD_TX_syncWord                                   uint32_t
#define _POSITION_CMD_TX_pTxPkt                                 20
#define _TYPE_CMD_TX_pTxPkt                                     uint8_t*
#define _POSITION_CMD_TX_pktLen                                 24
#define _TYPE_CMD_TX_pktLen                                     uint16_t
#define _SIZEOF_CMD_TX                                          26

#define _POSITION_CMD_RX_TEST_config                            14
#define _TYPE_CMD_RX_TEST_config                                uint8_t
#define _BITPOS_CMD_RX_TEST_config_bEnaFifo                     0
#define _NBITS_CMD_RX_TEST_config_bEnaFifo                      1
#define _BITPOS_CMD_RX_TEST_config_bFsOff                       1
#define _NBITS_CMD_RX_TEST_config_bFsOff                        1
#define _BITPOS_CMD_RX_TEST_config_bNoSync                      2
#define _NBITS_CMD_RX_TEST_config_bNoSync                       1
#define _POSITION_CMD_RX_TEST_endTrigger                        15
#define _TYPE_CMD_RX_TEST_endTrigger                            uint8_t
#define _BITPOS_CMD_RX_TEST_endTrigger_triggerType              0
#define _NBITS_CMD_RX_TEST_endTrigger_triggerType               4
#define _BITPOS_CMD_RX_TEST_endTrigger_bEnaCmd                  4
#define _NBITS_CMD_RX_TEST_endTrigger_bEnaCmd                   1
#define _BITPOS_CMD_RX_TEST_endTrigger_triggerNo                5
#define _NBITS_CMD_RX_TEST_endTrigger_triggerNo                 2
#define _BITPOS_CMD_RX_TEST_endTrigger_pastTrig                 7
#define _NBITS_CMD_RX_TEST_endTrigger_pastTrig                  1
#define _POSITION_CMD_RX_TEST_syncWord                          16
#define _TYPE_CMD_RX_TEST_syncWord                              uint32_t
#define _POSITION_CMD_RX_TEST_endTime                           20
#define _TYPE_CMD_RX_TEST_endTime                               ratmr_t
#define _SIZEOF_CMD_RX_TEST                                     24

#define _POSITION_CMD_TX_TEST_config                            14
#define _TYPE_CMD_TX_TEST_config                                uint8_t
#define _BITPOS_CMD_TX_TEST_config_bUseCw                       0
#define _NBITS_CMD_TX_TEST_config_bUseCw                        1
#define _BITPOS_CMD_TX_TEST_config_bFsOff                       1
#define _NBITS_CMD_TX_TEST_config_bFsOff                        1
#define _BITPOS_CMD_TX_TEST_config_whitenMode                   2
#define _NBITS_CMD_TX_TEST_config_whitenMode                    2
#define _POSITION_CMD_TX_TEST_txWord                            16
#define _TYPE_CMD_TX_TEST_txWord                                uint16_t
#define _POSITION_CMD_TX_TEST_endTrigger                        19
#define _TYPE_CMD_TX_TEST_endTrigger                            uint8_t
#define _BITPOS_CMD_TX_TEST_endTrigger_triggerType              0
#define _NBITS_CMD_TX_TEST_endTrigger_triggerType               4
#define _BITPOS_CMD_TX_TEST_endTrigger_bEnaCmd                  4
#define _NBITS_CMD_TX_TEST_endTrigger_bEnaCmd                   1
#define _BITPOS_CMD_TX_TEST_endTrigger_triggerNo                5
#define _NBITS_CMD_TX_TEST_endTrigger_triggerNo                 2
#define _BITPOS_CMD_TX_TEST_endTrigger_pastTrig                 7
#define _NBITS_CMD_TX_TEST_endTrigger_pastTrig                  1
#define _POSITION_CMD_TX_TEST_syncWord                          20
#define _TYPE_CMD_TX_TEST_syncWord                              uint32_t
#define _POSITION_CMD_TX_TEST_endTime                           24
#define _TYPE_CMD_TX_TEST_endTime                               ratmr_t
#define _SIZEOF_CMD_TX_TEST                                     28

#define _POSITION_CMD_SYNC_STOP_RAT_rat0                        16
#define _TYPE_CMD_SYNC_STOP_RAT_rat0                            ratmr_t
#define _SIZEOF_CMD_SYNC_STOP_RAT                               20

#define _POSITION_CMD_SYNC_START_RAT_rat0                       16
#define _TYPE_CMD_SYNC_START_RAT_rat0                           ratmr_t
#define _SIZEOF_CMD_SYNC_START_RAT                              20

#define _POSITION_CMD_COUNT_counter                             14
#define _TYPE_CMD_COUNT_counter                                 uint16_t
#define _SIZEOF_CMD_COUNT                                       16

#define _POSITION_CMD_FS_POWERUP_pRegOverride                   16
#define _TYPE_CMD_FS_POWERUP_pRegOverride                       uint32_t*
#define _SIZEOF_CMD_FS_POWERUP                                  20

#define _SIZEOF_CMD_FS_POWERDOWN                                14

#define _POSITION_CMD_SCH_IMM_cmdrVal                           16
#define _TYPE_CMD_SCH_IMM_cmdrVal                               uint32_t
#define _POSITION_CMD_SCH_IMM_cmdstaVal                         20
#define _TYPE_CMD_SCH_IMM_cmdstaVal                             uint32_t
#define _SIZEOF_CMD_SCH_IMM                                     24

#define _POSITION_CMD_COUNT_BRANCH_counter                      14
#define _TYPE_CMD_COUNT_BRANCH_counter                          uint16_t
#define _POSITION_CMD_COUNT_BRANCH_pNextOpIfOk                  16
#define _TYPE_CMD_COUNT_BRANCH_pNextOpIfOk                      uint8_t*
#define _SIZEOF_CMD_COUNT_BRANCH                                20

#define _POSITION_CMD_PATTERN_CHECK_patternOpt                  14
#define _TYPE_CMD_PATTERN_CHECK_patternOpt                      uint16_t
#define _BITPOS_CMD_PATTERN_CHECK_patternOpt_operation          0
#define _NBITS_CMD_PATTERN_CHECK_patternOpt_operation           2
#define _BITPOS_CMD_PATTERN_CHECK_patternOpt_bByteRev           2
#define _NBITS_CMD_PATTERN_CHECK_patternOpt_bByteRev            1
#define _BITPOS_CMD_PATTERN_CHECK_patternOpt_bBitRev            3
#define _NBITS_CMD_PATTERN_CHECK_patternOpt_bBitRev             1
#define _BITPOS_CMD_PATTERN_CHECK_patternOpt_signExtend         4
#define _NBITS_CMD_PATTERN_CHECK_patternOpt_signExtend          5
#define _BITPOS_CMD_PATTERN_CHECK_patternOpt_bRxVal             9
#define _NBITS_CMD_PATTERN_CHECK_patternOpt_bRxVal              1
#define _POSITION_CMD_PATTERN_CHECK_pNextOpIfOk                 16
#define _TYPE_CMD_PATTERN_CHECK_pNextOpIfOk                     uint8_t*
#define _POSITION_CMD_PATTERN_CHECK_pValue                      20
#define _TYPE_CMD_PATTERN_CHECK_pValue                          uint8_t*
#define _POSITION_CMD_PATTERN_CHECK_mask                        24
#define _TYPE_CMD_PATTERN_CHECK_mask                            uint32_t
#define _POSITION_CMD_PATTERN_CHECK_compareVal                  28
#define _TYPE_CMD_PATTERN_CHECK_compareVal                      uint32_t
#define _SIZEOF_CMD_PATTERN_CHECK                               32

#define _SIZEOF_CMD_ABORT                                       2

#define _SIZEOF_CMD_STOP                                        2

#define _SIZEOF_CMD_GET_RSSI                                    2

#define _POSITION_CMD_UPDATE_RADIO_SETUP_pRegOverride           4
#define _TYPE_CMD_UPDATE_RADIO_SETUP_pRegOverride               uint32_t*
#define _SIZEOF_CMD_UPDATE_RADIO_SETUP                          8

#define _POSITION_CMD_TRIGGER_triggerNo                         2
#define _TYPE_CMD_TRIGGER_triggerNo                             uint8_t
#define _SIZEOF_CMD_TRIGGER                                     3

#define _POSITION_CMD_GET_FW_INFO_versionNo                     2
#define _TYPE_CMD_GET_FW_INFO_versionNo                         uint16_t
#define _POSITION_CMD_GET_FW_INFO_startOffset                   4
#define _TYPE_CMD_GET_FW_INFO_startOffset                       uint16_t
#define _POSITION_CMD_GET_FW_INFO_freeRamSz                     6
#define _TYPE_CMD_GET_FW_INFO_freeRamSz                         uint16_t
#define _POSITION_CMD_GET_FW_INFO_availRatCh                    8
#define _TYPE_CMD_GET_FW_INFO_availRatCh                        uint16_t
#define _SIZEOF_CMD_GET_FW_INFO                                 10

#define _SIZEOF_CMD_START_RAT                                   2

#define _SIZEOF_CMD_PING                                        2

#define _POSITION_CMD_ADD_DATA_ENTRY_pQueue                     4
#define _TYPE_CMD_ADD_DATA_ENTRY_pQueue                         dataQueue_t*
#define _POSITION_CMD_ADD_DATA_ENTRY_pEntry                     8
#define _TYPE_CMD_ADD_DATA_ENTRY_pEntry                         uint8_t*
#define _SIZEOF_CMD_ADD_DATA_ENTRY                              12

#define _POSITION_CMD_REMOVE_DATA_ENTRY_pQueue                  4
#define _TYPE_CMD_REMOVE_DATA_ENTRY_pQueue                      dataQueue_t*
#define _POSITION_CMD_REMOVE_DATA_ENTRY_pEntry                  8
#define _TYPE_CMD_REMOVE_DATA_ENTRY_pEntry                      uint8_t*
#define _SIZEOF_CMD_REMOVE_DATA_ENTRY                           12

#define _POSITION_CMD_FLUSH_QUEUE_pQueue                        4
#define _TYPE_CMD_FLUSH_QUEUE_pQueue                            dataQueue_t*
#define _POSITION_CMD_FLUSH_QUEUE_pFirstEntry                   8
#define _TYPE_CMD_FLUSH_QUEUE_pFirstEntry                       uint8_t*
#define _SIZEOF_CMD_FLUSH_QUEUE                                 12

#define _POSITION_CMD_CLEAR_RX_pQueue                           4
#define _TYPE_CMD_CLEAR_RX_pQueue                               dataQueue_t*
#define _SIZEOF_CMD_CLEAR_RX                                    8

#define _POSITION_CMD_REMOVE_PENDING_ENTRIES_pQueue             4
#define _TYPE_CMD_REMOVE_PENDING_ENTRIES_pQueue                 dataQueue_t*
#define _POSITION_CMD_REMOVE_PENDING_ENTRIES_pFirstEntry        8
#define _TYPE_CMD_REMOVE_PENDING_ENTRIES_pFirstEntry            uint8_t*
#define _SIZEOF_CMD_REMOVE_PENDING_ENTRIES                      12

#define _POSITION_CMD_SET_RAT_CMP_ratCh                         2
#define _TYPE_CMD_SET_RAT_CMP_ratCh                             uint8_t
#define _POSITION_CMD_SET_RAT_CMP_compareTime                   4
#define _TYPE_CMD_SET_RAT_CMP_compareTime                       ratmr_t
#define _SIZEOF_CMD_SET_RAT_CMP                                 8

#define _POSITION_CMD_SET_RAT_CPT_config                        2
#define _TYPE_CMD_SET_RAT_CPT_config                            uint16_t
#define _BITPOS_CMD_SET_RAT_CPT_config_inputSrc                 3
#define _NBITS_CMD_SET_RAT_CPT_config_inputSrc                  5
#define _BITPOS_CMD_SET_RAT_CPT_config_ratCh                    8
#define _NBITS_CMD_SET_RAT_CPT_config_ratCh                     4
#define _BITPOS_CMD_SET_RAT_CPT_config_bRepeated                12
#define _NBITS_CMD_SET_RAT_CPT_config_bRepeated                 1
#define _BITPOS_CMD_SET_RAT_CPT_config_inputMode                13
#define _NBITS_CMD_SET_RAT_CPT_config_inputMode                 2
#define _SIZEOF_CMD_SET_RAT_CPT                                 4

#define _POSITION_CMD_DISABLE_RAT_CH_ratCh                      2
#define _TYPE_CMD_DISABLE_RAT_CH_ratCh                          uint8_t
#define _SIZEOF_CMD_DISABLE_RAT_CH                              3

#define _POSITION_CMD_SET_RAT_OUTPUT_config                     2
#define _TYPE_CMD_SET_RAT_OUTPUT_config                         uint16_t
#define _BITPOS_CMD_SET_RAT_OUTPUT_config_outputSel             2
#define _NBITS_CMD_SET_RAT_OUTPUT_config_outputSel              3
#define _BITPOS_CMD_SET_RAT_OUTPUT_config_outputMode            5
#define _NBITS_CMD_SET_RAT_OUTPUT_config_outputMode             3
#define _BITPOS_CMD_SET_RAT_OUTPUT_config_ratCh                 8
#define _NBITS_CMD_SET_RAT_OUTPUT_config_ratCh                  4
#define _SIZEOF_CMD_SET_RAT_OUTPUT                              4

#define _POSITION_CMD_ARM_RAT_CH_ratCh                          2
#define _TYPE_CMD_ARM_RAT_CH_ratCh                              uint8_t
#define _SIZEOF_CMD_ARM_RAT_CH                                  3

#define _POSITION_CMD_DISARM_RAT_CH_ratCh                       2
#define _TYPE_CMD_DISARM_RAT_CH_ratCh                           uint8_t
#define _SIZEOF_CMD_DISARM_RAT_CH                               3

#define _POSITION_CMD_SET_TX_POWER_txPower                      2
#define _TYPE_CMD_SET_TX_POWER_txPower                          uint16_t
#define _BITPOS_CMD_SET_TX_POWER_txPower_IB                     0
#define _NBITS_CMD_SET_TX_POWER_txPower_IB                      6
#define _BITPOS_CMD_SET_TX_POWER_txPower_GC                     6
#define _NBITS_CMD_SET_TX_POWER_txPower_GC                      2
#define _BITPOS_CMD_SET_TX_POWER_txPower_tempCoeff              8
#define _NBITS_CMD_SET_TX_POWER_txPower_tempCoeff               8
#define _SIZEOF_CMD_SET_TX_POWER                                4

#define _POSITION_CMD_UPDATE_FS_frequency                       2
#define _TYPE_CMD_UPDATE_FS_frequency                           uint16_t
#define _POSITION_CMD_UPDATE_FS_fractFreq                       4
#define _TYPE_CMD_UPDATE_FS_fractFreq                           uint16_t
#define _SIZEOF_CMD_UPDATE_FS                                   6

#define _POSITION_CMD_BUS_REQUEST_bSysBusNeeded                 2
#define _TYPE_CMD_BUS_REQUEST_bSysBusNeeded                     uint8_t
#define _SIZEOF_CMD_BUS_REQUEST                                 3

#endif

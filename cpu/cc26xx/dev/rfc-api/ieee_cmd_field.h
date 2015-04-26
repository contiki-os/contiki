/******************************************************************************
*  Filename:       ieee_cmd_field.h
*  Revised:        $ $
*  Revision:       $ $
*
*  Description:    CC26xx/CC13xx API for IEEE 802.15.4 commands
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

#ifndef __IEEE_CMD_FIELD_H
#define __IEEE_CMD_FIELD_H

#include <stdint.h>
#include "mailbox.h"
#include "common_cmd.h"

#define _POSITION_CMD_IEEE_RX_channel                           14
#define _TYPE_CMD_IEEE_RX_channel                               uint8_t
#define _POSITION_CMD_IEEE_RX_rxConfig                          15
#define _TYPE_CMD_IEEE_RX_rxConfig                              uint8_t
#define _BITPOS_CMD_IEEE_RX_rxConfig_bAutoFlushCrc              0
#define _NBITS_CMD_IEEE_RX_rxConfig_bAutoFlushCrc               1
#define _BITPOS_CMD_IEEE_RX_rxConfig_bAutoFlushIgn              1
#define _NBITS_CMD_IEEE_RX_rxConfig_bAutoFlushIgn               1
#define _BITPOS_CMD_IEEE_RX_rxConfig_bIncludePhyHdr             2
#define _NBITS_CMD_IEEE_RX_rxConfig_bIncludePhyHdr              1
#define _BITPOS_CMD_IEEE_RX_rxConfig_bIncludeCrc                3
#define _NBITS_CMD_IEEE_RX_rxConfig_bIncludeCrc                 1
#define _BITPOS_CMD_IEEE_RX_rxConfig_bAppendRssi                4
#define _NBITS_CMD_IEEE_RX_rxConfig_bAppendRssi                 1
#define _BITPOS_CMD_IEEE_RX_rxConfig_bAppendCorrCrc             5
#define _NBITS_CMD_IEEE_RX_rxConfig_bAppendCorrCrc              1
#define _BITPOS_CMD_IEEE_RX_rxConfig_bAppendSrcInd              6
#define _NBITS_CMD_IEEE_RX_rxConfig_bAppendSrcInd               1
#define _BITPOS_CMD_IEEE_RX_rxConfig_bAppendTimestamp           7
#define _NBITS_CMD_IEEE_RX_rxConfig_bAppendTimestamp            1
#define _POSITION_CMD_IEEE_RX_pRxQ                              16
#define _TYPE_CMD_IEEE_RX_pRxQ                                  dataQueue_t*
#define _POSITION_CMD_IEEE_RX_pOutput                           20
#define _TYPE_CMD_IEEE_RX_pOutput                               uint8_t*
#define _POSITION_CMD_IEEE_RX_frameFiltOpt                      24
#define _TYPE_CMD_IEEE_RX_frameFiltOpt                          uint16_t
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_frameFiltEn            0
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_frameFiltEn             1
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_frameFiltStop          1
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_frameFiltStop           1
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_autoAckEn              2
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_autoAckEn               1
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_slottedAckEn           3
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_slottedAckEn            1
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_autoPendEn             4
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_autoPendEn              1
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_defaultPend            5
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_defaultPend             1
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_bPendDataReqOnly       6
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_bPendDataReqOnly        1
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_bPanCoord              7
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_bPanCoord               1
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_maxFrameVersion        8
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_maxFrameVersion         2
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_fcfReservedMask        10
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_fcfReservedMask         3
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_modifyFtFilter         13
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_modifyFtFilter          2
#define _BITPOS_CMD_IEEE_RX_frameFiltOpt_bStrictLenFilter       15
#define _NBITS_CMD_IEEE_RX_frameFiltOpt_bStrictLenFilter        1
#define _POSITION_CMD_IEEE_RX_frameTypes                        26
#define _TYPE_CMD_IEEE_RX_frameTypes                            uint8_t
#define _BITPOS_CMD_IEEE_RX_frameTypes_bAcceptFt0Beacon         0
#define _NBITS_CMD_IEEE_RX_frameTypes_bAcceptFt0Beacon          1
#define _BITPOS_CMD_IEEE_RX_frameTypes_bAcceptFt1Data           1
#define _NBITS_CMD_IEEE_RX_frameTypes_bAcceptFt1Data            1
#define _BITPOS_CMD_IEEE_RX_frameTypes_bAcceptFt2Ack            2
#define _NBITS_CMD_IEEE_RX_frameTypes_bAcceptFt2Ack             1
#define _BITPOS_CMD_IEEE_RX_frameTypes_bAcceptFt3MacCmd         3
#define _NBITS_CMD_IEEE_RX_frameTypes_bAcceptFt3MacCmd          1
#define _BITPOS_CMD_IEEE_RX_frameTypes_bAcceptFt4Reserved       4
#define _NBITS_CMD_IEEE_RX_frameTypes_bAcceptFt4Reserved        1
#define _BITPOS_CMD_IEEE_RX_frameTypes_bAcceptFt5Reserved       5
#define _NBITS_CMD_IEEE_RX_frameTypes_bAcceptFt5Reserved        1
#define _BITPOS_CMD_IEEE_RX_frameTypes_bAcceptFt6Reserved       6
#define _NBITS_CMD_IEEE_RX_frameTypes_bAcceptFt6Reserved        1
#define _BITPOS_CMD_IEEE_RX_frameTypes_bAcceptFt7Reserved       7
#define _NBITS_CMD_IEEE_RX_frameTypes_bAcceptFt7Reserved        1
#define _POSITION_CMD_IEEE_RX_ccaOpt                            27
#define _TYPE_CMD_IEEE_RX_ccaOpt                                uint8_t
#define _BITPOS_CMD_IEEE_RX_ccaOpt_ccaEnEnergy                  0
#define _NBITS_CMD_IEEE_RX_ccaOpt_ccaEnEnergy                   1
#define _BITPOS_CMD_IEEE_RX_ccaOpt_ccaEnCorr                    1
#define _NBITS_CMD_IEEE_RX_ccaOpt_ccaEnCorr                     1
#define _BITPOS_CMD_IEEE_RX_ccaOpt_ccaEnSync                    2
#define _NBITS_CMD_IEEE_RX_ccaOpt_ccaEnSync                     1
#define _BITPOS_CMD_IEEE_RX_ccaOpt_ccaCorrOp                    3
#define _NBITS_CMD_IEEE_RX_ccaOpt_ccaCorrOp                     1
#define _BITPOS_CMD_IEEE_RX_ccaOpt_ccaSyncOp                    4
#define _NBITS_CMD_IEEE_RX_ccaOpt_ccaSyncOp                     1
#define _BITPOS_CMD_IEEE_RX_ccaOpt_ccaCorrThr                   5
#define _NBITS_CMD_IEEE_RX_ccaOpt_ccaCorrThr                    2
#define _POSITION_CMD_IEEE_RX_ccaRssiThr                        28
#define _TYPE_CMD_IEEE_RX_ccaRssiThr                            int8_t
#define _POSITION_CMD_IEEE_RX_numExtEntries                     30
#define _TYPE_CMD_IEEE_RX_numExtEntries                         uint8_t
#define _POSITION_CMD_IEEE_RX_numShortEntries                   31
#define _TYPE_CMD_IEEE_RX_numShortEntries                       uint8_t
#define _POSITION_CMD_IEEE_RX_pExtEntryList                     32
#define _TYPE_CMD_IEEE_RX_pExtEntryList                         uint32_t*
#define _POSITION_CMD_IEEE_RX_pShortEntryList                   36
#define _TYPE_CMD_IEEE_RX_pShortEntryList                       uint32_t*
#define _POSITION_CMD_IEEE_RX_localExtAddr                      40
#define _TYPE_CMD_IEEE_RX_localExtAddr                          uint64_t
#define _POSITION_CMD_IEEE_RX_localShortAddr                    48
#define _TYPE_CMD_IEEE_RX_localShortAddr                        uint16_t
#define _POSITION_CMD_IEEE_RX_localPanID                        50
#define _TYPE_CMD_IEEE_RX_localPanID                            uint16_t
#define _POSITION_CMD_IEEE_RX_endTrigger                        55
#define _TYPE_CMD_IEEE_RX_endTrigger                            uint8_t
#define _BITPOS_CMD_IEEE_RX_endTrigger_triggerType              0
#define _NBITS_CMD_IEEE_RX_endTrigger_triggerType               4
#define _BITPOS_CMD_IEEE_RX_endTrigger_bEnaCmd                  4
#define _NBITS_CMD_IEEE_RX_endTrigger_bEnaCmd                   1
#define _BITPOS_CMD_IEEE_RX_endTrigger_triggerNo                5
#define _NBITS_CMD_IEEE_RX_endTrigger_triggerNo                 2
#define _BITPOS_CMD_IEEE_RX_endTrigger_pastTrig                 7
#define _NBITS_CMD_IEEE_RX_endTrigger_pastTrig                  1
#define _POSITION_CMD_IEEE_RX_endTime                           56
#define _TYPE_CMD_IEEE_RX_endTime                               ratmr_t
#define _SIZEOF_CMD_IEEE_RX                                     60

#define _POSITION_CMD_IEEE_ED_SCAN_channel                      14
#define _TYPE_CMD_IEEE_ED_SCAN_channel                          uint8_t
#define _POSITION_CMD_IEEE_ED_SCAN_ccaOpt                       15
#define _TYPE_CMD_IEEE_ED_SCAN_ccaOpt                           uint8_t
#define _BITPOS_CMD_IEEE_ED_SCAN_ccaOpt_ccaEnEnergy             0
#define _NBITS_CMD_IEEE_ED_SCAN_ccaOpt_ccaEnEnergy              1
#define _BITPOS_CMD_IEEE_ED_SCAN_ccaOpt_ccaEnCorr               1
#define _NBITS_CMD_IEEE_ED_SCAN_ccaOpt_ccaEnCorr                1
#define _BITPOS_CMD_IEEE_ED_SCAN_ccaOpt_ccaEnSync               2
#define _NBITS_CMD_IEEE_ED_SCAN_ccaOpt_ccaEnSync                1
#define _BITPOS_CMD_IEEE_ED_SCAN_ccaOpt_ccaCorrOp               3
#define _NBITS_CMD_IEEE_ED_SCAN_ccaOpt_ccaCorrOp                1
#define _BITPOS_CMD_IEEE_ED_SCAN_ccaOpt_ccaSyncOp               4
#define _NBITS_CMD_IEEE_ED_SCAN_ccaOpt_ccaSyncOp                1
#define _BITPOS_CMD_IEEE_ED_SCAN_ccaOpt_ccaCorrThr              5
#define _NBITS_CMD_IEEE_ED_SCAN_ccaOpt_ccaCorrThr               2
#define _POSITION_CMD_IEEE_ED_SCAN_ccaRssiThr                   16
#define _TYPE_CMD_IEEE_ED_SCAN_ccaRssiThr                       int8_t
#define _POSITION_CMD_IEEE_ED_SCAN_maxRssi                      18
#define _TYPE_CMD_IEEE_ED_SCAN_maxRssi                          int8_t
#define _POSITION_CMD_IEEE_ED_SCAN_endTrigger                   19
#define _TYPE_CMD_IEEE_ED_SCAN_endTrigger                       uint8_t
#define _BITPOS_CMD_IEEE_ED_SCAN_endTrigger_triggerType         0
#define _NBITS_CMD_IEEE_ED_SCAN_endTrigger_triggerType          4
#define _BITPOS_CMD_IEEE_ED_SCAN_endTrigger_bEnaCmd             4
#define _NBITS_CMD_IEEE_ED_SCAN_endTrigger_bEnaCmd              1
#define _BITPOS_CMD_IEEE_ED_SCAN_endTrigger_triggerNo           5
#define _NBITS_CMD_IEEE_ED_SCAN_endTrigger_triggerNo            2
#define _BITPOS_CMD_IEEE_ED_SCAN_endTrigger_pastTrig            7
#define _NBITS_CMD_IEEE_ED_SCAN_endTrigger_pastTrig             1
#define _POSITION_CMD_IEEE_ED_SCAN_endTime                      20
#define _TYPE_CMD_IEEE_ED_SCAN_endTime                          ratmr_t
#define _SIZEOF_CMD_IEEE_ED_SCAN                                24

#define _POSITION_CMD_IEEE_TX_txOpt                             14
#define _TYPE_CMD_IEEE_TX_txOpt                                 uint8_t
#define _BITPOS_CMD_IEEE_TX_txOpt_bIncludePhyHdr                0
#define _NBITS_CMD_IEEE_TX_txOpt_bIncludePhyHdr                 1
#define _BITPOS_CMD_IEEE_TX_txOpt_bIncludeCrc                   1
#define _NBITS_CMD_IEEE_TX_txOpt_bIncludeCrc                    1
#define _BITPOS_CMD_IEEE_TX_txOpt_payloadLenMsb                 3
#define _NBITS_CMD_IEEE_TX_txOpt_payloadLenMsb                  5
#define _POSITION_CMD_IEEE_TX_payloadLen                        15
#define _TYPE_CMD_IEEE_TX_payloadLen                            uint8_t
#define _POSITION_CMD_IEEE_TX_pPayload                          16
#define _TYPE_CMD_IEEE_TX_pPayload                              uint8_t*
#define _POSITION_CMD_IEEE_TX_timeStamp                         20
#define _TYPE_CMD_IEEE_TX_timeStamp                             ratmr_t
#define _SIZEOF_CMD_IEEE_TX                                     24

#define _POSITION_CMD_IEEE_CSMA_randomState                     14
#define _TYPE_CMD_IEEE_CSMA_randomState                         uint16_t
#define _POSITION_CMD_IEEE_CSMA_macMaxBE                        16
#define _TYPE_CMD_IEEE_CSMA_macMaxBE                            uint8_t
#define _POSITION_CMD_IEEE_CSMA_macMaxCSMABackoffs              17
#define _TYPE_CMD_IEEE_CSMA_macMaxCSMABackoffs                  uint8_t
#define _POSITION_CMD_IEEE_CSMA_csmaConfig                      18
#define _TYPE_CMD_IEEE_CSMA_csmaConfig                          uint8_t
#define _BITPOS_CMD_IEEE_CSMA_csmaConfig_initCW                 0
#define _NBITS_CMD_IEEE_CSMA_csmaConfig_initCW                  5
#define _BITPOS_CMD_IEEE_CSMA_csmaConfig_bSlotted               5
#define _NBITS_CMD_IEEE_CSMA_csmaConfig_bSlotted                1
#define _BITPOS_CMD_IEEE_CSMA_csmaConfig_rxOffMode              6
#define _NBITS_CMD_IEEE_CSMA_csmaConfig_rxOffMode               2
#define _POSITION_CMD_IEEE_CSMA_NB                              19
#define _TYPE_CMD_IEEE_CSMA_NB                                  uint8_t
#define _POSITION_CMD_IEEE_CSMA_BE                              20
#define _TYPE_CMD_IEEE_CSMA_BE                                  uint8_t
#define _POSITION_CMD_IEEE_CSMA_remainingPeriods                21
#define _TYPE_CMD_IEEE_CSMA_remainingPeriods                    uint8_t
#define _POSITION_CMD_IEEE_CSMA_lastRssi                        22
#define _TYPE_CMD_IEEE_CSMA_lastRssi                            int8_t
#define _POSITION_CMD_IEEE_CSMA_endTrigger                      23
#define _TYPE_CMD_IEEE_CSMA_endTrigger                          uint8_t
#define _BITPOS_CMD_IEEE_CSMA_endTrigger_triggerType            0
#define _NBITS_CMD_IEEE_CSMA_endTrigger_triggerType             4
#define _BITPOS_CMD_IEEE_CSMA_endTrigger_bEnaCmd                4
#define _NBITS_CMD_IEEE_CSMA_endTrigger_bEnaCmd                 1
#define _BITPOS_CMD_IEEE_CSMA_endTrigger_triggerNo              5
#define _NBITS_CMD_IEEE_CSMA_endTrigger_triggerNo               2
#define _BITPOS_CMD_IEEE_CSMA_endTrigger_pastTrig               7
#define _NBITS_CMD_IEEE_CSMA_endTrigger_pastTrig                1
#define _POSITION_CMD_IEEE_CSMA_lastTimeStamp                   24
#define _TYPE_CMD_IEEE_CSMA_lastTimeStamp                       ratmr_t
#define _POSITION_CMD_IEEE_CSMA_endTime                         28
#define _TYPE_CMD_IEEE_CSMA_endTime                             ratmr_t
#define _SIZEOF_CMD_IEEE_CSMA                                   32

#define _POSITION_CMD_IEEE_RX_ACK_seqNo                         14
#define _TYPE_CMD_IEEE_RX_ACK_seqNo                             uint8_t
#define _POSITION_CMD_IEEE_RX_ACK_endTrigger                    15
#define _TYPE_CMD_IEEE_RX_ACK_endTrigger                        uint8_t
#define _BITPOS_CMD_IEEE_RX_ACK_endTrigger_triggerType          0
#define _NBITS_CMD_IEEE_RX_ACK_endTrigger_triggerType           4
#define _BITPOS_CMD_IEEE_RX_ACK_endTrigger_bEnaCmd              4
#define _NBITS_CMD_IEEE_RX_ACK_endTrigger_bEnaCmd               1
#define _BITPOS_CMD_IEEE_RX_ACK_endTrigger_triggerNo            5
#define _NBITS_CMD_IEEE_RX_ACK_endTrigger_triggerNo             2
#define _BITPOS_CMD_IEEE_RX_ACK_endTrigger_pastTrig             7
#define _NBITS_CMD_IEEE_RX_ACK_endTrigger_pastTrig              1
#define _POSITION_CMD_IEEE_RX_ACK_endTime                       16
#define _TYPE_CMD_IEEE_RX_ACK_endTime                           ratmr_t
#define _SIZEOF_CMD_IEEE_RX_ACK                                 20

#define _SIZEOF_CMD_IEEE_ABORT_BG                               14

#define _POSITION_CMD_IEEE_MOD_CCA_newCcaOpt                    2
#define _TYPE_CMD_IEEE_MOD_CCA_newCcaOpt                        uint8_t
#define _BITPOS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaEnEnergy          0
#define _NBITS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaEnEnergy           1
#define _BITPOS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaEnCorr            1
#define _NBITS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaEnCorr             1
#define _BITPOS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaEnSync            2
#define _NBITS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaEnSync             1
#define _BITPOS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaCorrOp            3
#define _NBITS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaCorrOp             1
#define _BITPOS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaSyncOp            4
#define _NBITS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaSyncOp             1
#define _BITPOS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaCorrThr           5
#define _NBITS_CMD_IEEE_MOD_CCA_newCcaOpt_ccaCorrThr            2
#define _POSITION_CMD_IEEE_MOD_CCA_newCcaRssiThr                3
#define _TYPE_CMD_IEEE_MOD_CCA_newCcaRssiThr                    int8_t
#define _SIZEOF_CMD_IEEE_MOD_CCA                                4

#define _POSITION_CMD_IEEE_MOD_FILT_newFrameFiltOpt             2
#define _TYPE_CMD_IEEE_MOD_FILT_newFrameFiltOpt                 uint16_t
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_frameFiltEn   0
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_frameFiltEn    1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_frameFiltStop 1
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_frameFiltStop  1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_autoAckEn     2
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_autoAckEn      1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_slottedAckEn  3
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_slottedAckEn   1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_autoPendEn    4
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_autoPendEn     1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_defaultPend   5
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_defaultPend    1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_bPendDataReqOnly 6
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_bPendDataReqOnly 1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_bPanCoord     7
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_bPanCoord      1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_maxFrameVersion 8
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_maxFrameVersion 2
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_fcfReservedMask 10
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_fcfReservedMask 3
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_modifyFtFilter 13
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_modifyFtFilter 2
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_bStrictLenFilter 15
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameFiltOpt_bStrictLenFilter 1
#define _POSITION_CMD_IEEE_MOD_FILT_newFrameTypes               4
#define _TYPE_CMD_IEEE_MOD_FILT_newFrameTypes                   uint8_t
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt0Beacon 0
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt0Beacon 1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt1Data  1
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt1Data   1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt2Ack   2
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt2Ack    1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt3MacCmd 3
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt3MacCmd 1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt4Reserved 4
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt4Reserved 1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt5Reserved 5
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt5Reserved 1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt6Reserved 6
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt6Reserved 1
#define _BITPOS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt7Reserved 7
#define _NBITS_CMD_IEEE_MOD_FILT_newFrameTypes_bAcceptFt7Reserved 1
#define _SIZEOF_CMD_IEEE_MOD_FILT                               5

#define _POSITION_CMD_IEEE_MOD_SRC_MATCH_options                2
#define _TYPE_CMD_IEEE_MOD_SRC_MATCH_options                    uint8_t
#define _BITPOS_CMD_IEEE_MOD_SRC_MATCH_options_bEnable          0
#define _NBITS_CMD_IEEE_MOD_SRC_MATCH_options_bEnable           1
#define _BITPOS_CMD_IEEE_MOD_SRC_MATCH_options_srcPend          1
#define _NBITS_CMD_IEEE_MOD_SRC_MATCH_options_srcPend           1
#define _BITPOS_CMD_IEEE_MOD_SRC_MATCH_options_entryType        2
#define _NBITS_CMD_IEEE_MOD_SRC_MATCH_options_entryType         1
#define _POSITION_CMD_IEEE_MOD_SRC_MATCH_entryNo                3
#define _TYPE_CMD_IEEE_MOD_SRC_MATCH_entryNo                    uint8_t
#define _SIZEOF_CMD_IEEE_MOD_SRC_MATCH                          4

#define _SIZEOF_CMD_IEEE_ABORT_FG                               2

#define _SIZEOF_CMD_IEEE_STOP_FG                                2

#define _POSITION_CMD_IEEE_CCA_REQ_currentRssi                  2
#define _TYPE_CMD_IEEE_CCA_REQ_currentRssi                      int8_t
#define _POSITION_CMD_IEEE_CCA_REQ_maxRssi                      3
#define _TYPE_CMD_IEEE_CCA_REQ_maxRssi                          int8_t
#define _POSITION_CMD_IEEE_CCA_REQ_ccaInfo                      4
#define _TYPE_CMD_IEEE_CCA_REQ_ccaInfo                          uint8_t
#define _BITPOS_CMD_IEEE_CCA_REQ_ccaInfo_ccaState               0
#define _NBITS_CMD_IEEE_CCA_REQ_ccaInfo_ccaState                2
#define _BITPOS_CMD_IEEE_CCA_REQ_ccaInfo_ccaEnergy              2
#define _NBITS_CMD_IEEE_CCA_REQ_ccaInfo_ccaEnergy               2
#define _BITPOS_CMD_IEEE_CCA_REQ_ccaInfo_ccaCorr                4
#define _NBITS_CMD_IEEE_CCA_REQ_ccaInfo_ccaCorr                 2
#define _BITPOS_CMD_IEEE_CCA_REQ_ccaInfo_ccaSync                6
#define _NBITS_CMD_IEEE_CCA_REQ_ccaInfo_ccaSync                 1
#define _SIZEOF_CMD_IEEE_CCA_REQ                                5

#define _POSITION_ieeeRxOutput_nTxAck                           0
#define _TYPE_ieeeRxOutput_nTxAck                               uint8_t
#define _POSITION_ieeeRxOutput_nRxBeacon                        1
#define _TYPE_ieeeRxOutput_nRxBeacon                            uint8_t
#define _POSITION_ieeeRxOutput_nRxData                          2
#define _TYPE_ieeeRxOutput_nRxData                              uint8_t
#define _POSITION_ieeeRxOutput_nRxAck                           3
#define _TYPE_ieeeRxOutput_nRxAck                               uint8_t
#define _POSITION_ieeeRxOutput_nRxMacCmd                        4
#define _TYPE_ieeeRxOutput_nRxMacCmd                            uint8_t
#define _POSITION_ieeeRxOutput_nRxReserved                      5
#define _TYPE_ieeeRxOutput_nRxReserved                          uint8_t
#define _POSITION_ieeeRxOutput_nRxNok                           6
#define _TYPE_ieeeRxOutput_nRxNok                               uint8_t
#define _POSITION_ieeeRxOutput_nRxIgnored                       7
#define _TYPE_ieeeRxOutput_nRxIgnored                           uint8_t
#define _POSITION_ieeeRxOutput_nRxBufFull                       8
#define _TYPE_ieeeRxOutput_nRxBufFull                           uint8_t
#define _POSITION_ieeeRxOutput_lastRssi                         9
#define _TYPE_ieeeRxOutput_lastRssi                             int8_t
#define _POSITION_ieeeRxOutput_maxRssi                          10
#define _TYPE_ieeeRxOutput_maxRssi                              int8_t
#define _POSITION_ieeeRxOutput_beaconTimeStamp                  12
#define _TYPE_ieeeRxOutput_beaconTimeStamp                      ratmr_t
#define _SIZEOF_ieeeRxOutput                                    16

#define _POSITION_shortAddrEntry_shortAddr                      0
#define _TYPE_shortAddrEntry_shortAddr                          uint16_t
#define _POSITION_shortAddrEntry_panId                          2
#define _TYPE_shortAddrEntry_panId                              uint16_t
#define _SIZEOF_shortAddrEntry                                  4

#define _POSITION_ieeeRxCorrCrc_status                          0
#define _TYPE_ieeeRxCorrCrc_status                              uint8_t
#define _BITPOS_ieeeRxCorrCrc_status_corr                       0
#define _NBITS_ieeeRxCorrCrc_status_corr                        6
#define _BITPOS_ieeeRxCorrCrc_status_bIgnore                    6
#define _NBITS_ieeeRxCorrCrc_status_bIgnore                     1
#define _BITPOS_ieeeRxCorrCrc_status_bCrcErr                    7
#define _NBITS_ieeeRxCorrCrc_status_bCrcErr                     1
#define _SIZEOF_ieeeRxCorrCrc                                   1

#endif

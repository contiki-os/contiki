/******************************************************************************
*  Filename:       hw_spis.h
*  Revised:        2015-01-15 18:41:47 +0100 (to, 15 jan 2015)
*  Revision:       42400
*
* Copyright (c) 2015, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1) Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2) Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3) Neither the name of the ORGANIZATION nor the names of its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __HW_SPIS_H__
#define __HW_SPIS_H__


//*****************************************************************************
//
// This section defines the register offsets of
// SPIS component
//
//*****************************************************************************
// SPI Slave General Purpose Flags
#define SPIS_O_GPFLAGS                                              0x00000000

// SPI Slave General Purpose Flags Set
#define SPIS_O_GPFLAGSSET                                           0x00000004

// SPI Slave General Purpose Flags Mask
#define SPIS_O_GPFLAGSMASK                                          0x00000008

// SPI Slave Configuration
#define SPIS_O_CFG                                                  0x0000000C

// SPI Slave TX FIFO Flags Clear
#define SPIS_O_TXFFLAGSCLRN                                         0x00000010

// TX FIFO Flags Set
#define SPIS_O_TXFFLAGSSET                                          0x00000014

// SPI Slave TX FIFO Flags Mask
#define SPIS_O_TXFFLAGSMASK                                         0x00000018

// SPI Slave TX FIFO Status
#define SPIS_O_TXSTAT                                               0x0000001C

// SPI Slave TX FIFO Event Source
#define SPIS_O_TXFEVSRC                                             0x00000020

// SPI Slave TX FIFO Threshold Count
#define SPIS_O_TXFTHR                                               0x00000024

// SPI Slave TX FIFO Push
#define SPIS_O_TXFPUSH                                              0x00000028

// SPI Slave TX FIFO Flush
#define SPIS_O_TXFFLUSH                                             0x0000002C

// SPI Slave TX FIFO Memory Read Pointer
#define SPIS_O_TXFMEMRDPOS                                          0x00000040

// SPI Slave TX FIFO Memory Write Pointer
#define SPIS_O_TXMEMWRPOS                                           0x00000044

// SPI Slave TX FIFO Count
#define SPIS_O_TXFCNT                                               0x00000048

// SPI Slave RX FIFO Flag Clear
#define SPIS_O_RXFFLAGSCLRN                                         0x0000004C

// RX FIFO Flags Set
#define SPIS_O_RXFFLAGSSET                                          0x00000050

// SPI Slave RX FIFO Flags Mask
#define SPIS_O_RXFFLAGSMASK                                         0x00000054

// SPI Slave RX FIFO Status
#define SPIS_O_RXFSTAT                                              0x00000058

// SPI Slave RX FIFO Event Source
#define SPIS_O_RXFEVSRC                                             0x0000005C

// SPI Slave RX FIFO Threshold
#define SPIS_O_RXFTHR                                               0x00000060

// SPI Slave RX FIFO Pop
#define SPIS_O_RXFPOP                                               0x00000064

// SPI Slave RX FIFO Flush
#define SPIS_O_RXFFLUSH                                             0x00000068

// SPI Slave RX FIFO Memory Read Position
#define SPIS_O_RXFMEMRDPOS                                          0x00000080

// SPI Slave RX FIFO Memory Write Position
#define SPIS_O_RXFMEMWRPOS                                          0x00000084

// SPI Slave RX FIFO Byte Count
#define SPIS_O_RXCNT                                                0x00000088

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM0                                              0x00000400

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM1                                              0x00000404

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM2                                              0x00000408

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM3                                              0x0000040C

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM4                                              0x00000410

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM5                                              0x00000414

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM6                                              0x00000418

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM7                                              0x0000041C

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM8                                              0x00000420

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM9                                              0x00000424

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM10                                             0x00000428

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM11                                             0x0000042C

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM12                                             0x00000430

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM13                                             0x00000434

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM14                                             0x00000438

// SPI Slave TX FIFO Memory
#define SPIS_O_TXFMEM15                                             0x0000043C

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM0                                              0x00000800

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM1                                              0x00000804

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM2                                              0x00000808

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM3                                              0x0000080C

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM4                                              0x00000810

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM5                                              0x00000814

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM6                                              0x00000818

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM7                                              0x0000081C

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM8                                              0x00000820

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM9                                              0x00000824

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM10                                             0x00000828

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM11                                             0x0000082C

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM12                                             0x00000830

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM13                                             0x00000834

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM14                                             0x00000838

// SPI Slave RX FIFO Data
#define SPIS_O_RXFMEM15                                             0x0000083C

//*****************************************************************************
//
// Register: SPIS_O_GPFLAGS
//
//*****************************************************************************
// Field: [3]    BYTE_DONE
//
// 0
#define SPIS_GPFLAGS_BYTE_DONE                                      0x00000008
#define SPIS_GPFLAGS_BYTE_DONE_BITN                                 3
#define SPIS_GPFLAGS_BYTE_DONE_M                                    0x00000008
#define SPIS_GPFLAGS_BYTE_DONE_S                                    3

// Field: [2]    BYTE_RX_OVF
//
// Parallel receive data overflow event.
//
// 0: No overflow condition is detected.
// 1: An overflow condition of sampled SDI pin data is detected. The SPI Slave
// is able to buffer one byte of data sampled on the SDI pin.
//
#define SPIS_GPFLAGS_BYTE_RX_OVF                                    0x00000004
#define SPIS_GPFLAGS_BYTE_RX_OVF_BITN                               2
#define SPIS_GPFLAGS_BYTE_RX_OVF_M                                  0x00000004
#define SPIS_GPFLAGS_BYTE_RX_OVF_S                                  2

// Field: [1]    BYTE_ABORT
//
// Incomplete SPI transfer event.
//
// 0: No SPI incomplete transfer detected.
// 1: Chip select has been de-asserted during a byte transfer.
//
#define SPIS_GPFLAGS_BYTE_ABORT                                     0x00000002
#define SPIS_GPFLAGS_BYTE_ABORT_BITN                                1
#define SPIS_GPFLAGS_BYTE_ABORT_M                                   0x00000002
#define SPIS_GPFLAGS_BYTE_ABORT_S                                   1

// Field: [0]    CS
//
// Chip select event.
//
// 0: Chip select has not been asserted.
// 1: Chip select assertion is detected.
//
#define SPIS_GPFLAGS_CS                                             0x00000001
#define SPIS_GPFLAGS_CS_BITN                                        0
#define SPIS_GPFLAGS_CS_M                                           0x00000001
#define SPIS_GPFLAGS_CS_S                                           0

//*****************************************************************************
//
// Register: SPIS_O_GPFLAGSSET
//
//*****************************************************************************
// Field: [3]    BYTE_DONE
//
// 0: No effect.
// 1: Set GPFLAGS.BYTE_DONE high
#define SPIS_GPFLAGSSET_BYTE_DONE                                   0x00000008
#define SPIS_GPFLAGSSET_BYTE_DONE_BITN                              3
#define SPIS_GPFLAGSSET_BYTE_DONE_M                                 0x00000008
#define SPIS_GPFLAGSSET_BYTE_DONE_S                                 3

// Field: [2]    BYTE_RX_OVF
//
// 0: No effect.
// 1: Set GPFLAGS.BYTE_RX_OVF high
#define SPIS_GPFLAGSSET_BYTE_RX_OVF                                 0x00000004
#define SPIS_GPFLAGSSET_BYTE_RX_OVF_BITN                            2
#define SPIS_GPFLAGSSET_BYTE_RX_OVF_M                               0x00000004
#define SPIS_GPFLAGSSET_BYTE_RX_OVF_S                               2

// Field: [1]    BYTE_ABORT
//
// 0: No effect.
// 1: Set GPFLAGS.BYTE_ABORT high
#define SPIS_GPFLAGSSET_BYTE_ABORT                                  0x00000002
#define SPIS_GPFLAGSSET_BYTE_ABORT_BITN                             1
#define SPIS_GPFLAGSSET_BYTE_ABORT_M                                0x00000002
#define SPIS_GPFLAGSSET_BYTE_ABORT_S                                1

// Field: [0]    CS
//
// 0: No effect.
// 1: Set GPFLAGS.CS high
#define SPIS_GPFLAGSSET_CS                                          0x00000001
#define SPIS_GPFLAGSSET_CS_BITN                                     0
#define SPIS_GPFLAGSSET_CS_M                                        0x00000001
#define SPIS_GPFLAGSSET_CS_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_GPFLAGSMASK
//
//*****************************************************************************
// Field: [5]    RX_DMA_DONE
//
// Enable bit for DMA done, DMA channel with RX FIFO as source.
//
// 0: Not an event source.
// 1: Enabled as an event source.
#define SPIS_GPFLAGSMASK_RX_DMA_DONE                                0x00000020
#define SPIS_GPFLAGSMASK_RX_DMA_DONE_BITN                           5
#define SPIS_GPFLAGSMASK_RX_DMA_DONE_M                              0x00000020
#define SPIS_GPFLAGSMASK_RX_DMA_DONE_S                              5

// Field: [4]    TX_DMA_DONE
//
// 0
#define SPIS_GPFLAGSMASK_TX_DMA_DONE                                0x00000010
#define SPIS_GPFLAGSMASK_TX_DMA_DONE_BITN                           4
#define SPIS_GPFLAGSMASK_TX_DMA_DONE_M                              0x00000010
#define SPIS_GPFLAGSMASK_TX_DMA_DONE_S                              4

// Field: [3]    BYTE_DONE
//
// Enable bit for GPFLAGS.BYTE_DONE as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_GPFLAGSMASK_BYTE_DONE                                  0x00000008
#define SPIS_GPFLAGSMASK_BYTE_DONE_BITN                             3
#define SPIS_GPFLAGSMASK_BYTE_DONE_M                                0x00000008
#define SPIS_GPFLAGSMASK_BYTE_DONE_S                                3

// Field: [2]    BYTE_RX_OVF
//
// Enable bit for GPFLAGS.BYTE_RX_OVF as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_GPFLAGSMASK_BYTE_RX_OVF                                0x00000004
#define SPIS_GPFLAGSMASK_BYTE_RX_OVF_BITN                           2
#define SPIS_GPFLAGSMASK_BYTE_RX_OVF_M                              0x00000004
#define SPIS_GPFLAGSMASK_BYTE_RX_OVF_S                              2

// Field: [1]    BYTE_ABORT
//
// Enable bit for GPFLAGS.BYTE_ABORT as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_GPFLAGSMASK_BYTE_ABORT                                 0x00000002
#define SPIS_GPFLAGSMASK_BYTE_ABORT_BITN                            1
#define SPIS_GPFLAGSMASK_BYTE_ABORT_M                               0x00000002
#define SPIS_GPFLAGSMASK_BYTE_ABORT_S                               1

// Field: [0]    CS
//
// Enable bit for GPFLAGS.CS as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_GPFLAGSMASK_CS                                         0x00000001
#define SPIS_GPFLAGSMASK_CS_BITN                                    0
#define SPIS_GPFLAGSMASK_CS_M                                       0x00000001
#define SPIS_GPFLAGSMASK_CS_S                                       0

//*****************************************************************************
//
// Register: SPIS_O_CFG
//
//*****************************************************************************
// Field: [4]    RX_DMA_REQ_TYPE
//
// RX DMA request type select.
//
// 0: DMA request is chosen.
// 1: Single DMA transfer request is chosen.
#define SPIS_CFG_RX_DMA_REQ_TYPE                                    0x00000010
#define SPIS_CFG_RX_DMA_REQ_TYPE_BITN                               4
#define SPIS_CFG_RX_DMA_REQ_TYPE_M                                  0x00000010
#define SPIS_CFG_RX_DMA_REQ_TYPE_S                                  4

// Field: [3]    TX_DMA_REQ_TYPE
//
// TX DMA request type select.
//
// 0: DMA Request is chosen.
// 1: Single DMA transfer request is chosen.
#define SPIS_CFG_TX_DMA_REQ_TYPE                                    0x00000008
#define SPIS_CFG_TX_DMA_REQ_TYPE_BITN                               3
#define SPIS_CFG_TX_DMA_REQ_TYPE_M                                  0x00000008
#define SPIS_CFG_TX_DMA_REQ_TYPE_S                                  3

// Field: [2]    RX_BIT_ORDER
//
// MOSI first bit configuration.
//
// 0: First bit is LSB.
// 1: First bit is MSB.
#define SPIS_CFG_RX_BIT_ORDER                                       0x00000004
#define SPIS_CFG_RX_BIT_ORDER_BITN                                  2
#define SPIS_CFG_RX_BIT_ORDER_M                                     0x00000004
#define SPIS_CFG_RX_BIT_ORDER_S                                     2

// Field: [1]    TX_BIT_ORDER
//
// MISO first bit configuration.
//
// 0: First bit is LSB.
// 1: First bit is MSB.
#define SPIS_CFG_TX_BIT_ORDER                                       0x00000002
#define SPIS_CFG_TX_BIT_ORDER_BITN                                  1
#define SPIS_CFG_TX_BIT_ORDER_M                                     0x00000002
#define SPIS_CFG_TX_BIT_ORDER_S                                     1

// Field: [0]    POL
//
// SPI clock polarity.
//
// 0: CPOL=0	CPHA=0
// 1: CPOL=1	CPHA=0
#define SPIS_CFG_POL                                                0x00000001
#define SPIS_CFG_POL_BITN                                           0
#define SPIS_CFG_POL_M                                              0x00000001
#define SPIS_CFG_POL_S                                              0

//*****************************************************************************
//
// Register: SPIS_O_TXFFLAGSCLRN
//
//*****************************************************************************
// Field: [6]    OVF
//
// TX FIFO overflow flag.
//
// 0: No TX FIFO overflow.
// 1: TX FIFO overflow due to writing data into a full TX FIFO.
#define SPIS_TXFFLAGSCLRN_OVF                                       0x00000040
#define SPIS_TXFFLAGSCLRN_OVF_BITN                                  6
#define SPIS_TXFFLAGSCLRN_OVF_M                                     0x00000040
#define SPIS_TXFFLAGSCLRN_OVF_S                                     6

// Field: [5]    UNF
//
// TX FIFO underflow flag.
//
// 0: No TX FIFO underflow.
// 1: TX FIFO underflow due to data request from the SPI Slave while TX FIFO is
// empty.
#define SPIS_TXFFLAGSCLRN_UNF                                       0x00000020
#define SPIS_TXFFLAGSCLRN_UNF_BITN                                  5
#define SPIS_TXFFLAGSCLRN_UNF_M                                     0x00000020
#define SPIS_TXFFLAGSCLRN_UNF_S                                     5

// Field: [4]    NOT_EMPTY
//
// TX FIFO has one or more bytes.
//
// 0: Number of bytes in TX FIFO is zero.
// 1: Number of bytes in TX FIFO is greater than zero.
#define SPIS_TXFFLAGSCLRN_NOT_EMPTY                                 0x00000010
#define SPIS_TXFFLAGSCLRN_NOT_EMPTY_BITN                            4
#define SPIS_TXFFLAGSCLRN_NOT_EMPTY_M                               0x00000010
#define SPIS_TXFFLAGSCLRN_NOT_EMPTY_S                               4

// Field: [3]    LE_THR
//
// TX FIFO less than or equal TX FIFO threshold count set by TXFTHR.CNT.
//
// 0: Number of bytes in TX FIFO is greater than the TX FIFO threshold count.
// 1: Number of bytes in TX FIFO is equal or less than TX FIFO threshold count.
//
#define SPIS_TXFFLAGSCLRN_LE_THR                                    0x00000008
#define SPIS_TXFFLAGSCLRN_LE_THR_BITN                               3
#define SPIS_TXFFLAGSCLRN_LE_THR_M                                  0x00000008
#define SPIS_TXFFLAGSCLRN_LE_THR_S                                  3

// Field: [2]    GE_THR
//
// TX FIFO greater than or equal to TX FIFO threshold count set by TXFTHR.CNT.
//
// 0: Number of bytes in TX FIFO is less than TX FIFO threshold count.
// 1: Number of bytes in TX FIFO is equal or greater than TX FIFO threshold
// count.
#define SPIS_TXFFLAGSCLRN_GE_THR                                    0x00000004
#define SPIS_TXFFLAGSCLRN_GE_THR_BITN                               2
#define SPIS_TXFFLAGSCLRN_GE_THR_M                                  0x00000004
#define SPIS_TXFFLAGSCLRN_GE_THR_S                                  2

// Field: [1]    EMPTY
//
// TX FIFO empty flag.
//
// 0: TX FIFO not empty.
// 1: TX FIFO is empty. Attempting to clear the flag while the TX FIFO is empty,
// will result in the flag remaining set.
#define SPIS_TXFFLAGSCLRN_EMPTY                                     0x00000002
#define SPIS_TXFFLAGSCLRN_EMPTY_BITN                                1
#define SPIS_TXFFLAGSCLRN_EMPTY_M                                   0x00000002
#define SPIS_TXFFLAGSCLRN_EMPTY_S                                   1

// Field: [0]    FULL
//
// TX FIFO full flag.
//
// 0: TX FIFO has not become full.
// 1: TX FIFO is full. Attempting to clear this flag while the TX FIFO is full,
// results in the flag remaining set.
#define SPIS_TXFFLAGSCLRN_FULL                                      0x00000001
#define SPIS_TXFFLAGSCLRN_FULL_BITN                                 0
#define SPIS_TXFFLAGSCLRN_FULL_M                                    0x00000001
#define SPIS_TXFFLAGSCLRN_FULL_S                                    0

//*****************************************************************************
//
// Register: SPIS_O_TXFFLAGSSET
//
//*****************************************************************************
// Field: [6]    OVF
//
// TX FIFO overflow flag.
//
// 1: TX FIFO overflow event is forced to 1
#define SPIS_TXFFLAGSSET_OVF                                        0x00000040
#define SPIS_TXFFLAGSSET_OVF_BITN                                   6
#define SPIS_TXFFLAGSSET_OVF_M                                      0x00000040
#define SPIS_TXFFLAGSSET_OVF_S                                      6

// Field: [5]    UNF
//
// TX FIFO underflow flag.
//
// 1: TX FIFO underflow event is forced to 1
#define SPIS_TXFFLAGSSET_UNF                                        0x00000020
#define SPIS_TXFFLAGSSET_UNF_BITN                                   5
#define SPIS_TXFFLAGSSET_UNF_M                                      0x00000020
#define SPIS_TXFFLAGSSET_UNF_S                                      5

// Field: [4]    NOT_EMPTY
//
// TX FIFO has data  flag.
//
// 1: TX FIFO has data event is forced to 1
#define SPIS_TXFFLAGSSET_NOT_EMPTY                                  0x00000010
#define SPIS_TXFFLAGSSET_NOT_EMPTY_BITN                             4
#define SPIS_TXFFLAGSSET_NOT_EMPTY_M                                0x00000010
#define SPIS_TXFFLAGSSET_NOT_EMPTY_S                                4

// Field: [3]    LE_THR
//
// TX FIFO threshold count event.
//
// 1: TX FIFO lower than threshold count event is forced to 1
#define SPIS_TXFFLAGSSET_LE_THR                                     0x00000008
#define SPIS_TXFFLAGSSET_LE_THR_BITN                                3
#define SPIS_TXFFLAGSSET_LE_THR_M                                   0x00000008
#define SPIS_TXFFLAGSSET_LE_THR_S                                   3

// Field: [2]    GE_THR
//
// TX FIFO GE threshold count event.
//
// 1: TX FIFO greater than or equal event is forced to 1
#define SPIS_TXFFLAGSSET_GE_THR                                     0x00000004
#define SPIS_TXFFLAGSSET_GE_THR_BITN                                2
#define SPIS_TXFFLAGSSET_GE_THR_M                                   0x00000004
#define SPIS_TXFFLAGSSET_GE_THR_S                                   2

// Field: [1]    EMPTY
//
// TX FIFO empty event.
//
// 0: No effect
// 1: TX FIFO empty event is forced to 1
#define SPIS_TXFFLAGSSET_EMPTY                                      0x00000002
#define SPIS_TXFFLAGSSET_EMPTY_BITN                                 1
#define SPIS_TXFFLAGSSET_EMPTY_M                                    0x00000002
#define SPIS_TXFFLAGSSET_EMPTY_S                                    1

// Field: [0]    FULL
//
// TX FIFO full event.
//
// 1: TX FIFO full event is forced to 1
#define SPIS_TXFFLAGSSET_FULL                                       0x00000001
#define SPIS_TXFFLAGSSET_FULL_BITN                                  0
#define SPIS_TXFFLAGSSET_FULL_M                                     0x00000001
#define SPIS_TXFFLAGSSET_FULL_S                                     0

//*****************************************************************************
//
// Register: SPIS_O_TXFFLAGSMASK
//
//*****************************************************************************
// Field: [6]    OVF
//
// Enable bit for TXFFLAGSCLRN.OVF as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_TXFFLAGSMASK_OVF                                       0x00000040
#define SPIS_TXFFLAGSMASK_OVF_BITN                                  6
#define SPIS_TXFFLAGSMASK_OVF_M                                     0x00000040
#define SPIS_TXFFLAGSMASK_OVF_S                                     6

// Field: [5]    UNF
//
// Enable bit for TXFFLAGSCLRN.UNF as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_TXFFLAGSMASK_UNF                                       0x00000020
#define SPIS_TXFFLAGSMASK_UNF_BITN                                  5
#define SPIS_TXFFLAGSMASK_UNF_M                                     0x00000020
#define SPIS_TXFFLAGSMASK_UNF_S                                     5

// Field: [4]    NOT_EMPTY
//
// Enable bit for TXFFLAGSCLRN.NOT_EMPTY as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_TXFFLAGSMASK_NOT_EMPTY                                 0x00000010
#define SPIS_TXFFLAGSMASK_NOT_EMPTY_BITN                            4
#define SPIS_TXFFLAGSMASK_NOT_EMPTY_M                               0x00000010
#define SPIS_TXFFLAGSMASK_NOT_EMPTY_S                               4

// Field: [3]    LE_THR
//
// Enable bit for TXFFLAGSCLRN.LE_THR as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_TXFFLAGSMASK_LE_THR                                    0x00000008
#define SPIS_TXFFLAGSMASK_LE_THR_BITN                               3
#define SPIS_TXFFLAGSMASK_LE_THR_M                                  0x00000008
#define SPIS_TXFFLAGSMASK_LE_THR_S                                  3

// Field: [2]    GE_THR
//
// Enable bit for TXFFLAGSCLRN.GE_THR as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_TXFFLAGSMASK_GE_THR                                    0x00000004
#define SPIS_TXFFLAGSMASK_GE_THR_BITN                               2
#define SPIS_TXFFLAGSMASK_GE_THR_M                                  0x00000004
#define SPIS_TXFFLAGSMASK_GE_THR_S                                  2

// Field: [1]    EMPTY
//
// Enable bit for TXFFLAGSCLRN.EMPTY as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_TXFFLAGSMASK_EMPTY                                     0x00000002
#define SPIS_TXFFLAGSMASK_EMPTY_BITN                                1
#define SPIS_TXFFLAGSMASK_EMPTY_M                                   0x00000002
#define SPIS_TXFFLAGSMASK_EMPTY_S                                   1

// Field: [0]    FULL
//
// Enable bit for TXFFLAGSCLRN.FULL as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_TXFFLAGSMASK_FULL                                      0x00000001
#define SPIS_TXFFLAGSMASK_FULL_BITN                                 0
#define SPIS_TXFFLAGSMASK_FULL_M                                    0x00000001
#define SPIS_TXFFLAGSMASK_FULL_S                                    0

//*****************************************************************************
//
// Register: SPIS_O_TXSTAT
//
//*****************************************************************************
// Field: [4]    NOT_EMPTY
//
// TX FIFO has one or more bytes status.
//
// 0: Number of bytes in TX FIFO is zero (empty).
// 1: Number of bytes in TX FIFO is greater than zero (not empty).
#define SPIS_TXSTAT_NOT_EMPTY                                       0x00000010
#define SPIS_TXSTAT_NOT_EMPTY_BITN                                  4
#define SPIS_TXSTAT_NOT_EMPTY_M                                     0x00000010
#define SPIS_TXSTAT_NOT_EMPTY_S                                     4

// Field: [3]    LE_THR
//
// TX FIFO less than or equal to TX FIFO threshold count set by  TXFTHR.CNT.
//
// 0: Number of bytes in TX FIFO is greater than TX FIFO threshold count.
// 1: Number of bytes in TX FIFO is equal to or less than TX FIFO threshold
// count.
#define SPIS_TXSTAT_LE_THR                                          0x00000008
#define SPIS_TXSTAT_LE_THR_BITN                                     3
#define SPIS_TXSTAT_LE_THR_M                                        0x00000008
#define SPIS_TXSTAT_LE_THR_S                                        3

// Field: [2]    GE_THR
//
// TX FIFO greater than or equal to TX FIFO threshold count set by TXFTHR.CNT.
//
// 0: Number of bytes in TX FIFO is less than TX FIFO threshold count.
// 1: Number of bytes in TX FIFO is equal to or greater than TX FIFO threshold
// count.
#define SPIS_TXSTAT_GE_THR                                          0x00000004
#define SPIS_TXSTAT_GE_THR_BITN                                     2
#define SPIS_TXSTAT_GE_THR_M                                        0x00000004
#define SPIS_TXSTAT_GE_THR_S                                        2

// Field: [1]    EMPTY
//
// TX FIFO empty status.
//
// 0: TX FIFO is not empty.
// 1: TX FIFO is empty.
#define SPIS_TXSTAT_EMPTY                                           0x00000002
#define SPIS_TXSTAT_EMPTY_BITN                                      1
#define SPIS_TXSTAT_EMPTY_M                                         0x00000002
#define SPIS_TXSTAT_EMPTY_S                                         1

// Field: [0]    FULL
//
// TX FIFO full status.
//
// 0: TX FIFO is not full.
// 1: TX FIFO is full.
#define SPIS_TXSTAT_FULL                                            0x00000001
#define SPIS_TXSTAT_FULL_BITN                                       0
#define SPIS_TXSTAT_FULL_M                                          0x00000001
#define SPIS_TXSTAT_FULL_S                                          0

//*****************************************************************************
//
// Register: SPIS_O_TXFEVSRC
//
//*****************************************************************************
// Field: [2:0] SEL
//
// TX FIFO Status event source select
// ENUMs:
// FULL                 TXSTAT.FULL FIFO is full
// EMPTY                TXSTAT.EMPTY FIFO is empty
// GE_THR               TXSTAT.GE_THR FIFO has more data than threshold count
// LE_THR               TXSTAT.LE_THR FIFO has less data than threshold count
// NOT_EMPTY            TXSTAT.NOT_EMPTY FIFO has some data
// ONE                  None
// RESERVED             None
// ZERO                 None
//
#define SPIS_TXFEVSRC_SEL_M                                         0x00000007
#define SPIS_TXFEVSRC_SEL_S                                         0
#define SPIS_TXFEVSRC_SEL_FULL                                      0x00000000
#define SPIS_TXFEVSRC_SEL_EMPTY                                     0x00000001
#define SPIS_TXFEVSRC_SEL_GE_THR                                    0x00000002
#define SPIS_TXFEVSRC_SEL_LE_THR                                    0x00000003
#define SPIS_TXFEVSRC_SEL_NOT_EMPTY                                 0x00000004
#define SPIS_TXFEVSRC_SEL_ONE                                       0x00000005
#define SPIS_TXFEVSRC_SEL_RESERVED                                  0x00000006
#define SPIS_TXFEVSRC_SEL_ZERO                                      0x00000007

//*****************************************************************************
//
// Register: SPIS_O_TXFTHR
//
//*****************************************************************************
// Field: [3:0] CNT
//
// TXFIFO count threshold.
// TXSTAT.LE_THR and TXSTAT.GE_THR depend on this setting.
//
// 0x0: 0 bytes
// 0x1: 1 bytes
// 0x2: 2 bytes
// ...
// 0xE: 14 bytes
// 0xF: 15 bytes
#define SPIS_TXFTHR_CNT_M                                           0x0000000F
#define SPIS_TXFTHR_CNT_S                                           0

//*****************************************************************************
//
// Register: SPIS_O_TXFPUSH
//
//*****************************************************************************
// Field: [7:0] DATA
//
// Data to be pushed into TX FIFO.
#define SPIS_TXFPUSH_DATA_M                                         0x000000FF
#define SPIS_TXFPUSH_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFFLUSH
//
//*****************************************************************************
// Field: [0]    FLUSH
//
// Execute TX FIFO flush command.
//
// 0: TX FIFO is not flushed (untouched).
// 1: TX FIFO is flushed.
//
#define SPIS_TXFFLUSH_FLUSH                                         0x00000001
#define SPIS_TXFFLUSH_FLUSH_BITN                                    0
#define SPIS_TXFFLUSH_FLUSH_M                                       0x00000001
#define SPIS_TXFFLUSH_FLUSH_S                                       0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEMRDPOS
//
//*****************************************************************************
// Field: [3:0] POS
//
// Read pointer position.
// 0x0: FIFO element #0
// 0x1: FIFO element #1
// ...
// 0xE: FIFO element #14
// 0xF: FIFO element #15
//
//
#define SPIS_TXFMEMRDPOS_POS_M                                      0x0000000F
#define SPIS_TXFMEMRDPOS_POS_S                                      0

//*****************************************************************************
//
// Register: SPIS_O_TXMEMWRPOS
//
//*****************************************************************************
// Field: [3:0] POS
//
// Write pointer position.
// 0x0: FIFO element #0
// 0x1: FIFO element #1
// ...
// 0xE: FIFO element #14
// 0xF: FIFO element #15
//
//
#define SPIS_TXMEMWRPOS_POS_M                                       0x0000000F
#define SPIS_TXMEMWRPOS_POS_S                                       0

//*****************************************************************************
//
// Register: SPIS_O_TXFCNT
//
//*****************************************************************************
// Field: [4:0] CNT
//
// Number of bytes present in the TX FIFO.
//
// 0x00: 0 bytes in FIFO
// 0x01: 1 bytes in FIFO
// ...
// 0x0E: 14 bytes in FIFO
// 0x0F: 15 bytes in FIFO
// 0x10: 16 bytes in FIFO
// 0x11 to 0x1F: Unreachable values
//
//
#define SPIS_TXFCNT_CNT_M                                           0x0000001F
#define SPIS_TXFCNT_CNT_S                                           0

//*****************************************************************************
//
// Register: SPIS_O_RXFFLAGSCLRN
//
//*****************************************************************************
// Field: [6]    OVF
//
// RX FIFO overflow flag.
//
// 0: RX FIFO has no overflow.
// 1: RX FIFO overflow condition.
#define SPIS_RXFFLAGSCLRN_OVF                                       0x00000040
#define SPIS_RXFFLAGSCLRN_OVF_BITN                                  6
#define SPIS_RXFFLAGSCLRN_OVF_M                                     0x00000040
#define SPIS_RXFFLAGSCLRN_OVF_S                                     6

// Field: [5]    UNF
//
// RX FIFO underflow.
//
// 0: RX FIFO has no underflow.
// 1: RX FIFO underflow condition.
#define SPIS_RXFFLAGSCLRN_UNF                                       0x00000020
#define SPIS_RXFFLAGSCLRN_UNF_BITN                                  5
#define SPIS_RXFFLAGSCLRN_UNF_M                                     0x00000020
#define SPIS_RXFFLAGSCLRN_UNF_S                                     5

// Field: [4]    NOT_EMPTY
//
// RX FIFO has one or more bytes.
//
// 0: Number of bytes in RX FIFO equal zero.
// 1: Number of bytes in RX FIFO greater than zero.
//
#define SPIS_RXFFLAGSCLRN_NOT_EMPTY                                 0x00000010
#define SPIS_RXFFLAGSCLRN_NOT_EMPTY_BITN                            4
#define SPIS_RXFFLAGSCLRN_NOT_EMPTY_M                               0x00000010
#define SPIS_RXFFLAGSCLRN_NOT_EMPTY_S                               4

// Field: [3]    LE_THR
//
// RX FIFO less than or equal RX FIFO threshold count.
//
// 0: Number of bytes in RX FIFO not less than or equal RX FIFO threshold count.
// 1: Number of bytes in RX FIFO less than or equal RX FIFO threshold count.
#define SPIS_RXFFLAGSCLRN_LE_THR                                    0x00000008
#define SPIS_RXFFLAGSCLRN_LE_THR_BITN                               3
#define SPIS_RXFFLAGSCLRN_LE_THR_M                                  0x00000008
#define SPIS_RXFFLAGSCLRN_LE_THR_S                                  3

// Field: [2]    GE_THR
//
// RX FIFO greater than or equal to RX FIFO threshold count .
//
// 0: Number of bytes in RX FIFO not  greater than or equal RX FIFO threshold
// count.
// 1: Number of bytes in RX FIFO greater than or equal RX FIFO threshold count.
#define SPIS_RXFFLAGSCLRN_GE_THR                                    0x00000004
#define SPIS_RXFFLAGSCLRN_GE_THR_BITN                               2
#define SPIS_RXFFLAGSCLRN_GE_THR_M                                  0x00000004
#define SPIS_RXFFLAGSCLRN_GE_THR_S                                  2

// Field: [1]    EMPTY
//
// RX FIFO empty flag.
// This flag can not be cleared while the RX FIFO remains empty.
//
// 0: RX FIFO not empty.
// 1: RX FIFO  is empty.
#define SPIS_RXFFLAGSCLRN_EMPTY                                     0x00000002
#define SPIS_RXFFLAGSCLRN_EMPTY_BITN                                1
#define SPIS_RXFFLAGSCLRN_EMPTY_M                                   0x00000002
#define SPIS_RXFFLAGSCLRN_EMPTY_S                                   1

// Field: [0]    FULL
//
// RX FIFO full flag.
// This flag can not be cleared while the RX FIFO remains full.
//
// 0: RX FIFO not  full.
// 1: RX FIFO is full.
#define SPIS_RXFFLAGSCLRN_FULL                                      0x00000001
#define SPIS_RXFFLAGSCLRN_FULL_BITN                                 0
#define SPIS_RXFFLAGSCLRN_FULL_M                                    0x00000001
#define SPIS_RXFFLAGSCLRN_FULL_S                                    0

//*****************************************************************************
//
// Register: SPIS_O_RXFFLAGSSET
//
//*****************************************************************************
// Field: [6]    OVF
//
// RX FIFO overflow flag.
// 1: RX FIFO overflow event is forced to 1
#define SPIS_RXFFLAGSSET_OVF                                        0x00000040
#define SPIS_RXFFLAGSSET_OVF_BITN                                   6
#define SPIS_RXFFLAGSSET_OVF_M                                      0x00000040
#define SPIS_RXFFLAGSSET_OVF_S                                      6

// Field: [5]    UNF
//
// RX FIFO underflow flag.
// 1: RX FIFO underflow event is forced to 1
#define SPIS_RXFFLAGSSET_UNF                                        0x00000020
#define SPIS_RXFFLAGSSET_UNF_BITN                                   5
#define SPIS_RXFFLAGSSET_UNF_M                                      0x00000020
#define SPIS_RXFFLAGSSET_UNF_S                                      5

// Field: [4]    NOT_EMPTY
//
// RX FIFO has data  flag.
// 1: RX FIFO has data event is forced to 1
#define SPIS_RXFFLAGSSET_NOT_EMPTY                                  0x00000010
#define SPIS_RXFFLAGSSET_NOT_EMPTY_BITN                             4
#define SPIS_RXFFLAGSSET_NOT_EMPTY_M                                0x00000010
#define SPIS_RXFFLAGSSET_NOT_EMPTY_S                                4

// Field: [3]    LE_THR
//
// RX FIFO threshold count event.
// 1: RX FIFO lower than threshold count event is forced to 1
#define SPIS_RXFFLAGSSET_LE_THR                                     0x00000008
#define SPIS_RXFFLAGSSET_LE_THR_BITN                                3
#define SPIS_RXFFLAGSSET_LE_THR_M                                   0x00000008
#define SPIS_RXFFLAGSSET_LE_THR_S                                   3

// Field: [2]    GE_THR
//
// RX FIFO GE threshold count event.
// 1: RX FIFO greater than or equal event is forced to 1
#define SPIS_RXFFLAGSSET_GE_THR                                     0x00000004
#define SPIS_RXFFLAGSSET_GE_THR_BITN                                2
#define SPIS_RXFFLAGSSET_GE_THR_M                                   0x00000004
#define SPIS_RXFFLAGSSET_GE_THR_S                                   2

// Field: [1]    EMPTY
//
// RX FIFO empty event.
// 1: RX FIFO empty event is forced to 1
#define SPIS_RXFFLAGSSET_EMPTY                                      0x00000002
#define SPIS_RXFFLAGSSET_EMPTY_BITN                                 1
#define SPIS_RXFFLAGSSET_EMPTY_M                                    0x00000002
#define SPIS_RXFFLAGSSET_EMPTY_S                                    1

// Field: [0]    FULL
//
// RX FIFO full event.
//
// 1: RX FIFO full event is forced to 1
#define SPIS_RXFFLAGSSET_FULL                                       0x00000001
#define SPIS_RXFFLAGSSET_FULL_BITN                                  0
#define SPIS_RXFFLAGSSET_FULL_M                                     0x00000001
#define SPIS_RXFFLAGSSET_FULL_S                                     0

//*****************************************************************************
//
// Register: SPIS_O_RXFFLAGSMASK
//
//*****************************************************************************
// Field: [6]    OVF
//
// Enable bit for RXFFLAGSCLRN.OVF as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_RXFFLAGSMASK_OVF                                       0x00000040
#define SPIS_RXFFLAGSMASK_OVF_BITN                                  6
#define SPIS_RXFFLAGSMASK_OVF_M                                     0x00000040
#define SPIS_RXFFLAGSMASK_OVF_S                                     6

// Field: [5]    UNF
//
// Enable bit for RXFFLAGSCLRN.UNF as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_RXFFLAGSMASK_UNF                                       0x00000020
#define SPIS_RXFFLAGSMASK_UNF_BITN                                  5
#define SPIS_RXFFLAGSMASK_UNF_M                                     0x00000020
#define SPIS_RXFFLAGSMASK_UNF_S                                     5

// Field: [4]    NOT_EMPTY
//
// Enable bit for RXFFLAGSCLRN.NOT_EMPTY as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_RXFFLAGSMASK_NOT_EMPTY                                 0x00000010
#define SPIS_RXFFLAGSMASK_NOT_EMPTY_BITN                            4
#define SPIS_RXFFLAGSMASK_NOT_EMPTY_M                               0x00000010
#define SPIS_RXFFLAGSMASK_NOT_EMPTY_S                               4

// Field: [3]    LE_THR
//
// Enable bit for RXFFLAGSCLRN.LE_THR as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_RXFFLAGSMASK_LE_THR                                    0x00000008
#define SPIS_RXFFLAGSMASK_LE_THR_BITN                               3
#define SPIS_RXFFLAGSMASK_LE_THR_M                                  0x00000008
#define SPIS_RXFFLAGSMASK_LE_THR_S                                  3

// Field: [2]    GE_THR
//
// Enable bit for RXFFLAGSCLRN.GE_THR as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_RXFFLAGSMASK_GE_THR                                    0x00000004
#define SPIS_RXFFLAGSMASK_GE_THR_BITN                               2
#define SPIS_RXFFLAGSMASK_GE_THR_M                                  0x00000004
#define SPIS_RXFFLAGSMASK_GE_THR_S                                  2

// Field: [1]    EMPTY
//
// Enable bit for RXFFLAGSCLRN.EMPTY as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_RXFFLAGSMASK_EMPTY                                     0x00000002
#define SPIS_RXFFLAGSMASK_EMPTY_BITN                                1
#define SPIS_RXFFLAGSMASK_EMPTY_M                                   0x00000002
#define SPIS_RXFFLAGSMASK_EMPTY_S                                   1

// Field: [0]    FULL
//
// Enable bit for RXFFLAGSCLRN.FULL as event signal.
//
// 0: Flag is not an event source.
// 1: Flag is enabled as an event source.
#define SPIS_RXFFLAGSMASK_FULL                                      0x00000001
#define SPIS_RXFFLAGSMASK_FULL_BITN                                 0
#define SPIS_RXFFLAGSMASK_FULL_M                                    0x00000001
#define SPIS_RXFFLAGSMASK_FULL_S                                    0

//*****************************************************************************
//
// Register: SPIS_O_RXFSTAT
//
//*****************************************************************************
// Field: [4]    NOT_EMPTY
//
// RX FIFO has one or more bytes status.
//
// 0: Number of bytes in RX FIFO is zero (empty).
// 1: Number of bytes in RX FIFO is greater than zero (not empty).
#define SPIS_RXFSTAT_NOT_EMPTY                                      0x00000010
#define SPIS_RXFSTAT_NOT_EMPTY_BITN                                 4
#define SPIS_RXFSTAT_NOT_EMPTY_M                                    0x00000010
#define SPIS_RXFSTAT_NOT_EMPTY_S                                    4

// Field: [3]    LE_THR
//
// RX FIFO byte count less than or equal to RX FIFO threshold count  RXFTHR.CNT.
//
// 0: Number of bytes in RX FIFO is greater than RX FIFO threshold count.
// 1: Number of bytes in RX FIFO is equal to or less than RX FIFO threshold
// count.
#define SPIS_RXFSTAT_LE_THR                                         0x00000008
#define SPIS_RXFSTAT_LE_THR_BITN                                    3
#define SPIS_RXFSTAT_LE_THR_M                                       0x00000008
#define SPIS_RXFSTAT_LE_THR_S                                       3

// Field: [2]    GE_THR
//
// RX FIFO byte count greater than or equal to RX FIFO threshold count
// RXFTHR.CNT.
//
// 0: Number of bytes in RX FIFO is less than RX FIFO threshold count.
// 1: Number of bytes in RX FIFO is equal to or greater than RX FIFO threshold
// count.
#define SPIS_RXFSTAT_GE_THR                                         0x00000004
#define SPIS_RXFSTAT_GE_THR_BITN                                    2
#define SPIS_RXFSTAT_GE_THR_M                                       0x00000004
#define SPIS_RXFSTAT_GE_THR_S                                       2

// Field: [1]    EMPTY
//
// RX FIFO empty status.
//
// 0: RX FIFO is not empty.
// 1: RX FIFO is empty.
#define SPIS_RXFSTAT_EMPTY                                          0x00000002
#define SPIS_RXFSTAT_EMPTY_BITN                                     1
#define SPIS_RXFSTAT_EMPTY_M                                        0x00000002
#define SPIS_RXFSTAT_EMPTY_S                                        1

// Field: [0]    FULL
//
// RX FIFO full status.
//
// 0: RX FIFO is not full.
// 1: RX FIFO is full.
#define SPIS_RXFSTAT_FULL                                           0x00000001
#define SPIS_RXFSTAT_FULL_BITN                                      0
#define SPIS_RXFSTAT_FULL_M                                         0x00000001
#define SPIS_RXFSTAT_FULL_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFEVSRC
//
//*****************************************************************************
// Field: [2:0] SEL
//
// RX FIFO Status source select
// ENUMs:
// FULL                 RXFSTAT.FULL FIFO is full
// EMPTY                RXFSTAT.EMPTY FIFO is empty
// GE_THR               RXFSTAT.GE_THR FIFO has more data than threshold count
// LE_THR               RXFSTAT.LE_THR FIFO has less data than threshold count
// NOT_EMPTY            RXFSTAT.NOT_EMPTY FIFO not empty
// ONE                  None
// RESERVED             None
// ZERO                 None
//
#define SPIS_RXFEVSRC_SEL_M                                         0x00000007
#define SPIS_RXFEVSRC_SEL_S                                         0
#define SPIS_RXFEVSRC_SEL_FULL                                      0x00000000
#define SPIS_RXFEVSRC_SEL_EMPTY                                     0x00000001
#define SPIS_RXFEVSRC_SEL_GE_THR                                    0x00000002
#define SPIS_RXFEVSRC_SEL_LE_THR                                    0x00000003
#define SPIS_RXFEVSRC_SEL_NOT_EMPTY                                 0x00000004
#define SPIS_RXFEVSRC_SEL_ONE                                       0x00000005
#define SPIS_RXFEVSRC_SEL_RESERVED                                  0x00000006
#define SPIS_RXFEVSRC_SEL_ZERO                                      0x00000007

//*****************************************************************************
//
// Register: SPIS_O_RXFTHR
//
//*****************************************************************************
// Field: [3:0] CNT
//
// Threshold count.
//
// 0x0: 0 bytes
// 0x1: 1 bytes
// 0x2: 2 bytes
// ...
// 0xE: 14 bytes
// 0xF: 15 bytes
#define SPIS_RXFTHR_CNT_M                                           0x0000000F
#define SPIS_RXFTHR_CNT_S                                           0

//*****************************************************************************
//
// Register: SPIS_O_RXFPOP
//
//*****************************************************************************
// Field: [7:0] DATA
//
// Data read from RX FIFO.
#define SPIS_RXFPOP_DATA_M                                          0x000000FF
#define SPIS_RXFPOP_DATA_S                                          0

//*****************************************************************************
//
// Register: SPIS_O_RXFFLUSH
//
//*****************************************************************************
// Field: [0]    FLUSH
//
// Execute RX FIFO flush command.
//
// 0: RX FIFO is not flushed (untouched).
// 1: RX FIFO is flushed.
//
#define SPIS_RXFFLUSH_FLUSH                                         0x00000001
#define SPIS_RXFFLUSH_FLUSH_BITN                                    0
#define SPIS_RXFFLUSH_FLUSH_M                                       0x00000001
#define SPIS_RXFFLUSH_FLUSH_S                                       0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEMRDPOS
//
//*****************************************************************************
// Field: [3:0] POS
//
// RX FIFO read pointer value.
//
// 0x0: FIFO element #0
// 0x1: FIFO element #1
// ...
// 0xE: Read Pointer at FIFO element #14
// 0xF: Read Pointer at FIFO element #15
//
//
#define SPIS_RXFMEMRDPOS_POS_M                                      0x0000000F
#define SPIS_RXFMEMRDPOS_POS_S                                      0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEMWRPOS
//
//*****************************************************************************
// Field: [3:0] POS
//
// FIFO write pointer value.
// 0x0: FIFO element #0
// 0x1: FIFO element #1
// ...
// 0xE: FIFO element #14
// 0xF: FIFO element #15
//
//
#define SPIS_RXFMEMWRPOS_POS_M                                      0x0000000F
#define SPIS_RXFMEMWRPOS_POS_S                                      0

//*****************************************************************************
//
// Register: SPIS_O_RXCNT
//
//*****************************************************************************
// Field: [4:0] CNT
//
// Number of bytes in present in RX FIFO.
//
// 0x00: 0 bytes in FIFO
// 0x01: 1 bytes in FIFO
// ...
// 0x0E: 14 bytes in FIFO
// 0x0F: 15 bytes in FIFO
// 0x10: 16 bytes in FIFO
// 0x11 to 0x1F: Unreachable values
#define SPIS_RXCNT_CNT_M                                            0x0000001F
#define SPIS_RXCNT_CNT_S                                            0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM0
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM0_DATA_M                                         0x000000FF
#define SPIS_TXFMEM0_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM1
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM1_DATA_M                                         0x000000FF
#define SPIS_TXFMEM1_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM2
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM2_DATA_M                                         0x000000FF
#define SPIS_TXFMEM2_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM3
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM3_DATA_M                                         0x000000FF
#define SPIS_TXFMEM3_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM4
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM4_DATA_M                                         0x000000FF
#define SPIS_TXFMEM4_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM5
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM5_DATA_M                                         0x000000FF
#define SPIS_TXFMEM5_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM6
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM6_DATA_M                                         0x000000FF
#define SPIS_TXFMEM6_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM7
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM7_DATA_M                                         0x000000FF
#define SPIS_TXFMEM7_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM8
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM8_DATA_M                                         0x000000FF
#define SPIS_TXFMEM8_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM9
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM9_DATA_M                                         0x000000FF
#define SPIS_TXFMEM9_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM10
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM10_DATA_M                                        0x000000FF
#define SPIS_TXFMEM10_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM11
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM11_DATA_M                                        0x000000FF
#define SPIS_TXFMEM11_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM12
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM12_DATA_M                                        0x000000FF
#define SPIS_TXFMEM12_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM13
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM13_DATA_M                                        0x000000FF
#define SPIS_TXFMEM13_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM14
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM14_DATA_M                                        0x000000FF
#define SPIS_TXFMEM14_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_TXFMEM15
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data element
#define SPIS_TXFMEM15_DATA_M                                        0x000000FF
#define SPIS_TXFMEM15_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM0
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM0_DATA_M                                         0x000000FF
#define SPIS_RXFMEM0_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM1
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM1_DATA_M                                         0x000000FF
#define SPIS_RXFMEM1_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM2
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM2_DATA_M                                         0x000000FF
#define SPIS_RXFMEM2_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM3
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM3_DATA_M                                         0x000000FF
#define SPIS_RXFMEM3_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM4
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM4_DATA_M                                         0x000000FF
#define SPIS_RXFMEM4_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM5
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM5_DATA_M                                         0x000000FF
#define SPIS_RXFMEM5_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM6
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM6_DATA_M                                         0x000000FF
#define SPIS_RXFMEM6_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM7
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM7_DATA_M                                         0x000000FF
#define SPIS_RXFMEM7_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM8
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM8_DATA_M                                         0x000000FF
#define SPIS_RXFMEM8_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM9
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM9_DATA_M                                         0x000000FF
#define SPIS_RXFMEM9_DATA_S                                         0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM10
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM10_DATA_M                                        0x000000FF
#define SPIS_RXFMEM10_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM11
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM11_DATA_M                                        0x000000FF
#define SPIS_RXFMEM11_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM12
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM12_DATA_M                                        0x000000FF
#define SPIS_RXFMEM12_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM13
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM13_DATA_M                                        0x000000FF
#define SPIS_RXFMEM13_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM14
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM14_DATA_M                                        0x000000FF
#define SPIS_RXFMEM14_DATA_S                                        0

//*****************************************************************************
//
// Register: SPIS_O_RXFMEM15
//
//*****************************************************************************
// Field: [7:0] DATA
//
// FIFO data.
#define SPIS_RXFMEM15_DATA_M                                        0x000000FF
#define SPIS_RXFMEM15_DATA_S                                        0

#endif // __HW_SPIS_H__

/******************************************************************************
*  Filename:       hw_trng.h
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

#ifndef __HW_TRNG_H__
#define __HW_TRNG_H__


//*****************************************************************************
//
// This section defines the register offsets of
// TRNG component
//
//*****************************************************************************
// Internal register
#define TRNG_O_OUT0                                                 0x00000000

// Internal register
#define TRNG_O_OUT1                                                 0x00000004

// Internal register
#define TRNG_O_IRQFLAGSTAT                                          0x00000008

// Internal register
#define TRNG_O_IRQFLAGMASK                                          0x0000000C

// Internal register
#define TRNG_O_IRQFLAGCLR                                           0x00000010

// Internal register
#define TRNG_O_CTL                                                  0x00000014

// Internal register
#define TRNG_O_CFG0                                                 0x00000018

// Internal register
#define TRNG_O_ALARMCNT                                             0x0000001C

// Internal register
#define TRNG_O_FROEN                                                0x00000020

// Internal register
#define TRNG_O_FRODETUNE                                            0x00000024

// Internal register
#define TRNG_O_ALARMMASK                                            0x00000028

// Internal register
#define TRNG_O_ALARMSTOP                                            0x0000002C

// Internal register
#define TRNG_O_LFSR0                                                0x00000030

// Internal register
#define TRNG_O_LFSR1                                                0x00000034

// Internal register
#define TRNG_O_LFSR2                                                0x00000038

// Internal register
#define TRNG_O_HWOPT                                                0x00000078

// Internal register
#define TRNG_O_HWVER0                                               0x0000007C

// Internal register
#define TRNG_O_IRQSTATMASK                                          0x00001FD8

// Internal register
#define TRNG_O_HWVER1                                               0x00001FE0

// Internal register
#define TRNG_O_IRQSET                                               0x00001FEC

// Internal register
#define TRNG_O_SWRESET                                              0x00001FF0

// Internal register
#define TRNG_O_IRQSTAT                                              0x00001FF8

//*****************************************************************************
//
// Register: TRNG_O_OUT0
//
//*****************************************************************************
// Field: [31:0] VALUE_31_0
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_OUT0_VALUE_31_0_M                                      0xFFFFFFFF
#define TRNG_OUT0_VALUE_31_0_S                                      0

//*****************************************************************************
//
// Register: TRNG_O_OUT1
//
//*****************************************************************************
// Field: [31:0] VALUE_63_32
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_OUT1_VALUE_63_32_M                                     0xFFFFFFFF
#define TRNG_OUT1_VALUE_63_32_S                                     0

//*****************************************************************************
//
// Register: TRNG_O_IRQFLAGSTAT
//
//*****************************************************************************
// Field: [31]    NEED_CLOCK
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQFLAGSTAT_NEED_CLOCK                                 0x80000000
#define TRNG_IRQFLAGSTAT_NEED_CLOCK_BITN                            31
#define TRNG_IRQFLAGSTAT_NEED_CLOCK_M                               0x80000000
#define TRNG_IRQFLAGSTAT_NEED_CLOCK_S                               31

// Field: [1]    SHUTDOWN_OVF
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQFLAGSTAT_SHUTDOWN_OVF                               0x00000002
#define TRNG_IRQFLAGSTAT_SHUTDOWN_OVF_BITN                          1
#define TRNG_IRQFLAGSTAT_SHUTDOWN_OVF_M                             0x00000002
#define TRNG_IRQFLAGSTAT_SHUTDOWN_OVF_S                             1

// Field: [0]    RDY
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQFLAGSTAT_RDY                                        0x00000001
#define TRNG_IRQFLAGSTAT_RDY_BITN                                   0
#define TRNG_IRQFLAGSTAT_RDY_M                                      0x00000001
#define TRNG_IRQFLAGSTAT_RDY_S                                      0

//*****************************************************************************
//
// Register: TRNG_O_IRQFLAGMASK
//
//*****************************************************************************
// Field: [1]    SHUTDOWN_OVF
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQFLAGMASK_SHUTDOWN_OVF                               0x00000002
#define TRNG_IRQFLAGMASK_SHUTDOWN_OVF_BITN                          1
#define TRNG_IRQFLAGMASK_SHUTDOWN_OVF_M                             0x00000002
#define TRNG_IRQFLAGMASK_SHUTDOWN_OVF_S                             1

// Field: [0]    RDY
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQFLAGMASK_RDY                                        0x00000001
#define TRNG_IRQFLAGMASK_RDY_BITN                                   0
#define TRNG_IRQFLAGMASK_RDY_M                                      0x00000001
#define TRNG_IRQFLAGMASK_RDY_S                                      0

//*****************************************************************************
//
// Register: TRNG_O_IRQFLAGCLR
//
//*****************************************************************************
// Field: [1]    SHUTDOWN_OVF
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQFLAGCLR_SHUTDOWN_OVF                                0x00000002
#define TRNG_IRQFLAGCLR_SHUTDOWN_OVF_BITN                           1
#define TRNG_IRQFLAGCLR_SHUTDOWN_OVF_M                              0x00000002
#define TRNG_IRQFLAGCLR_SHUTDOWN_OVF_S                              1

// Field: [0]    RDY
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQFLAGCLR_RDY                                         0x00000001
#define TRNG_IRQFLAGCLR_RDY_BITN                                    0
#define TRNG_IRQFLAGCLR_RDY_M                                       0x00000001
#define TRNG_IRQFLAGCLR_RDY_S                                       0

//*****************************************************************************
//
// Register: TRNG_O_CTL
//
//*****************************************************************************
// Field: [31:16] STARTUP_CYCLES
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_CTL_STARTUP_CYCLES_M                                   0xFFFF0000
#define TRNG_CTL_STARTUP_CYCLES_S                                   16

// Field: [10]    TRNG_EN
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_CTL_TRNG_EN                                            0x00000400
#define TRNG_CTL_TRNG_EN_BITN                                       10
#define TRNG_CTL_TRNG_EN_M                                          0x00000400
#define TRNG_CTL_TRNG_EN_S                                          10

// Field: [2]    NO_LFSR_FB
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_CTL_NO_LFSR_FB                                         0x00000004
#define TRNG_CTL_NO_LFSR_FB_BITN                                    2
#define TRNG_CTL_NO_LFSR_FB_M                                       0x00000004
#define TRNG_CTL_NO_LFSR_FB_S                                       2

// Field: [1]    TEST_MODE
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_CTL_TEST_MODE                                          0x00000002
#define TRNG_CTL_TEST_MODE_BITN                                     1
#define TRNG_CTL_TEST_MODE_M                                        0x00000002
#define TRNG_CTL_TEST_MODE_S                                        1

//*****************************************************************************
//
// Register: TRNG_O_CFG0
//
//*****************************************************************************
// Field: [31:16] MAX_REFILL_CYCLES
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_CFG0_MAX_REFILL_CYCLES_M                               0xFFFF0000
#define TRNG_CFG0_MAX_REFILL_CYCLES_S                               16

// Field: [11:8] SMPL_DIV
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_CFG0_SMPL_DIV_M                                        0x00000F00
#define TRNG_CFG0_SMPL_DIV_S                                        8

// Field: [7:0] MIN_REFILL_CYCLES
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_CFG0_MIN_REFILL_CYCLES_M                               0x000000FF
#define TRNG_CFG0_MIN_REFILL_CYCLES_S                               0

//*****************************************************************************
//
// Register: TRNG_O_ALARMCNT
//
//*****************************************************************************
// Field: [29:24] SHUTDOWN_CNT
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_ALARMCNT_SHUTDOWN_CNT_M                                0x3F000000
#define TRNG_ALARMCNT_SHUTDOWN_CNT_S                                24

// Field: [20:16] SHUTDOWN_THR
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_ALARMCNT_SHUTDOWN_THR_M                                0x001F0000
#define TRNG_ALARMCNT_SHUTDOWN_THR_S                                16

// Field: [7:0] ALARM_THR
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_ALARMCNT_ALARM_THR_M                                   0x000000FF
#define TRNG_ALARMCNT_ALARM_THR_S                                   0

//*****************************************************************************
//
// Register: TRNG_O_FROEN
//
//*****************************************************************************
// Field: [23:0] FRO_MASK
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_FROEN_FRO_MASK_M                                       0x00FFFFFF
#define TRNG_FROEN_FRO_MASK_S                                       0

//*****************************************************************************
//
// Register: TRNG_O_FRODETUNE
//
//*****************************************************************************
// Field: [23:0] FRO_MASK
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_FRODETUNE_FRO_MASK_M                                   0x00FFFFFF
#define TRNG_FRODETUNE_FRO_MASK_S                                   0

//*****************************************************************************
//
// Register: TRNG_O_ALARMMASK
//
//*****************************************************************************
// Field: [23:0] FRO_MASK
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_ALARMMASK_FRO_MASK_M                                   0x00FFFFFF
#define TRNG_ALARMMASK_FRO_MASK_S                                   0

//*****************************************************************************
//
// Register: TRNG_O_ALARMSTOP
//
//*****************************************************************************
// Field: [23:0] FRO_FLAGS
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_ALARMSTOP_FRO_FLAGS_M                                  0x00FFFFFF
#define TRNG_ALARMSTOP_FRO_FLAGS_S                                  0

//*****************************************************************************
//
// Register: TRNG_O_LFSR0
//
//*****************************************************************************
// Field: [31:0] LFSR_31_0
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_LFSR0_LFSR_31_0_M                                      0xFFFFFFFF
#define TRNG_LFSR0_LFSR_31_0_S                                      0

//*****************************************************************************
//
// Register: TRNG_O_LFSR1
//
//*****************************************************************************
// Field: [31:0] LFSR_63_32
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_LFSR1_LFSR_63_32_M                                     0xFFFFFFFF
#define TRNG_LFSR1_LFSR_63_32_S                                     0

//*****************************************************************************
//
// Register: TRNG_O_LFSR2
//
//*****************************************************************************
// Field: [16:0] LFSR_80_64
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_LFSR2_LFSR_80_64_M                                     0x0001FFFF
#define TRNG_LFSR2_LFSR_80_64_S                                     0

//*****************************************************************************
//
// Register: TRNG_O_HWOPT
//
//*****************************************************************************
// Field: [11:6] NR_OF_FROS
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_HWOPT_NR_OF_FROS_M                                     0x00000FC0
#define TRNG_HWOPT_NR_OF_FROS_S                                     6

//*****************************************************************************
//
// Register: TRNG_O_HWVER0
//
//*****************************************************************************
// Field: [27:24] HW_MAJOR_VER
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_HWVER0_HW_MAJOR_VER_M                                  0x0F000000
#define TRNG_HWVER0_HW_MAJOR_VER_S                                  24

// Field: [23:20] HW_MINOR_VER
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_HWVER0_HW_MINOR_VER_M                                  0x00F00000
#define TRNG_HWVER0_HW_MINOR_VER_S                                  20

// Field: [19:16] HW_PATCH_LVL
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_HWVER0_HW_PATCH_LVL_M                                  0x000F0000
#define TRNG_HWVER0_HW_PATCH_LVL_S                                  16

// Field: [15:8] EIP_NUM_COMPL
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_HWVER0_EIP_NUM_COMPL_M                                 0x0000FF00
#define TRNG_HWVER0_EIP_NUM_COMPL_S                                 8

// Field: [7:0] EIP_NUM
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_HWVER0_EIP_NUM_M                                       0x000000FF
#define TRNG_HWVER0_EIP_NUM_S                                       0

//*****************************************************************************
//
// Register: TRNG_O_IRQSTATMASK
//
//*****************************************************************************
// Field: [1]    SHUTDOWN_OVF
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQSTATMASK_SHUTDOWN_OVF                               0x00000002
#define TRNG_IRQSTATMASK_SHUTDOWN_OVF_BITN                          1
#define TRNG_IRQSTATMASK_SHUTDOWN_OVF_M                             0x00000002
#define TRNG_IRQSTATMASK_SHUTDOWN_OVF_S                             1

// Field: [0]    RDY
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQSTATMASK_RDY                                        0x00000001
#define TRNG_IRQSTATMASK_RDY_BITN                                   0
#define TRNG_IRQSTATMASK_RDY_M                                      0x00000001
#define TRNG_IRQSTATMASK_RDY_S                                      0

//*****************************************************************************
//
// Register: TRNG_O_HWVER1
//
//*****************************************************************************
// Field: [7:0] REV
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_HWVER1_REV_M                                           0x000000FF
#define TRNG_HWVER1_REV_S                                           0

//*****************************************************************************
//
// Register: TRNG_O_IRQSET
//
//*****************************************************************************
//*****************************************************************************
//
// Register: TRNG_O_SWRESET
//
//*****************************************************************************
// Field: [0]    RESET
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_SWRESET_RESET                                          0x00000001
#define TRNG_SWRESET_RESET_BITN                                     0
#define TRNG_SWRESET_RESET_M                                        0x00000001
#define TRNG_SWRESET_RESET_S                                        0

//*****************************************************************************
//
// Register: TRNG_O_IRQSTAT
//
//*****************************************************************************
// Field: [0]    STAT
//
// Internal Register. Customers can control this through TI provided API
#define TRNG_IRQSTAT_STAT                                           0x00000001
#define TRNG_IRQSTAT_STAT_BITN                                      0
#define TRNG_IRQSTAT_STAT_M                                         0x00000001
#define TRNG_IRQSTAT_STAT_S                                         0

#endif // __HW_TRNG_H__

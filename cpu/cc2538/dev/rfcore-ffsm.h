/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 *
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
 * \addtogroup cc2538-rfcore
 * @{
 *
 * \file
 * Header with declarations of the RF Core FFSM registers.
 */
#ifndef RFCORE_FFSM_H_
#define RFCORE_FFSM_H_
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM register offsets
 * @{
 */
#define RFCORE_FFSM_SRCRESMASK0     0x40088580 /**< Src addr matching result */
#define RFCORE_FFSM_SRCRESMASK1     0x40088584 /**< Src addr matching result */
#define RFCORE_FFSM_SRCRESMASK2     0x40088588 /**< Src addr matching result */
#define RFCORE_FFSM_SRCRESINDEX     0x4008858C /**< Src addr matching result */
#define RFCORE_FFSM_SRCEXTPENDEN0   0x40088590 /**< Src addr matching control */
#define RFCORE_FFSM_SRCEXTPENDEN1   0x40088594 /**< Src addr matching control */
#define RFCORE_FFSM_SRCEXTPENDEN2   0x40088598 /**< Src addr matching control */
#define RFCORE_FFSM_SRCSHORTPENDEN0 0x4008859C /**< Src addr matching control */
#define RFCORE_FFSM_SRCSHORTPENDEN1 0x400885A0 /**< Src addr matching control */
#define RFCORE_FFSM_SRCSHORTPENDEN2 0x400885A4 /**< Src addr matching control */
#define RFCORE_FFSM_EXT_ADDR0       0x400885A8 /**< Local address information */
#define RFCORE_FFSM_EXT_ADDR1       0x400885AC /**< Local address information */
#define RFCORE_FFSM_EXT_ADDR2       0x400885B0 /**< Local address information */
#define RFCORE_FFSM_EXT_ADDR3       0x400885B4 /**< Local address information */
#define RFCORE_FFSM_EXT_ADDR4       0x400885B8 /**< Local address information */
#define RFCORE_FFSM_EXT_ADDR5       0x400885BC /**< Local address information */
#define RFCORE_FFSM_EXT_ADDR6       0x400885C0 /**< Local address information */
#define RFCORE_FFSM_EXT_ADDR7       0x400885C4 /**< Local address information */
#define RFCORE_FFSM_PAN_ID0         0x400885C8 /**< Local address information */
#define RFCORE_FFSM_PAN_ID1         0x400885CC /**< Local address information */
#define RFCORE_FFSM_SHORT_ADDR0     0x400885D0 /**< Local address information */
#define RFCORE_FFSM_SHORT_ADDR1     0x400885D4 /**< Local address information */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM_SRCRESMASK[0:2] register bit masks
 * @{
 */
#define RFCORE_FFSM_SRCRESMASK0_SRCRESMASK0 0x000000FF /**< Ext addr match */
#define RFCORE_FFSM_SRCRESMASK1_SRCRESMASK1 0x000000FF /**< Short addr match */
#define RFCORE_FFSM_SRCRESMASK2_SRCRESMASK2 0x000000FF /**< 24-bit mask */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM_SRCRESINDEX register bit masks
 * @{
 */
#define RFCORE_FFSM_SRCRESINDEX_SRCRESINDEX 0x000000FF /**< LS Entry bit index */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM_SRCEXTPENDEN[0:2] register bit masks
 * @{
 */
#define RFCORE_FFSM_SRCEXTPENDEN0_SRCEXTPENDEN0 0x000000FF /**< 8 LSBs */
#define RFCORE_FFSM_SRCEXTPENDEN1_SRCEXTPENDEN1 0x000000FF /**< 8 middle bits */
#define RFCORE_FFSM_SRCEXTPENDEN2_SRCEXTPENDEN2 0x000000FF /**< 8 MSBs */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM_SRCSHORTPENDEN[0:2] register bit masks
 * @{
 */
#define RFCORE_FFSM_SRCSHORTPENDEN0_SRCSHORTPENDEN0 0x000000FF /**< 8 LSBs */
#define RFCORE_FFSM_SRCSHORTPENDEN1_SRCSHORTPENDEN1 0x000000FF /**< 8 middle */
#define RFCORE_FFSM_SRCSHORTPENDEN2_SRCSHORTPENDEN2 0x000000FF /**< 8 MSBs */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM_EXT_ADDR[0:7] register bit masks
 * @{
 */
#define RFCORE_FFSM_EXT_ADDR0_EXT_ADDR0 0x000000FF /**< EXT_ADDR[7:0] */
#define RFCORE_FFSM_EXT_ADDR1_EXT_ADDR1 0x000000FF /**< EXT_ADDR[15:8] */
#define RFCORE_FFSM_EXT_ADDR2_EXT_ADDR2 0x000000FF /**< EXT_ADDR[23:16] */
#define RFCORE_FFSM_EXT_ADDR3_EXT_ADDR3 0x000000FF /**< EXT_ADDR[31:24] */
#define RFCORE_FFSM_EXT_ADDR4_EXT_ADDR4 0x000000FF /**< EXT_ADDR[39:32] */
#define RFCORE_FFSM_EXT_ADDR5_EXT_ADDR5 0x000000FF /**< EXT_ADDR[47:40] */
#define RFCORE_FFSM_EXT_ADDR6_EXT_ADDR6 0x000000FF /**< EXT_ADDR[55:48] */
#define RFCORE_FFSM_EXT_ADDR7_EXT_ADDR7 0x000000FF /**< EXT_ADDR[63:56] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM_PAN_ID[0:1] register bit masks
 * @{
 */
#define RFCORE_FFSM_PAN_ID0_PAN_ID0 0x000000FF /**< PAN_ID[7:0] */
#define RFCORE_FFSM_PAN_ID1_PAN_ID1 0x000000FF /**< PAN_ID[15:8] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name RFCORE_FFSM_SHORT_ADDR[0:1] register bit masks
 * @{
 */
#define RFCORE_FFSM_SHORT_ADDR0_SHORT_ADDR0 0x000000FF /**< SHORT_ADDR[7:0] */
#define RFCORE_FFSM_SHORT_ADDR1_SHORT_ADDR1 0x000000FF /**< SHORT_ADDR[15:8] */
/** @} */

#endif /* RFCORE_FFSM_H_ */
/** @} */

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
 * \addtogroup cc2538
 * @{
 *
 * \defgroup cc2538-smwdthrosc cc2538 Sleep Timer and Watchdog
 *
 * Register declarations for the cc2538 Sleep Timer and Watchdog
 * @{
 *
 * \file
 * Header file with register declarations and bit masks for the cc2538
 * Sleep Timer and Watchdog
 */
#ifndef SMWDTHROSC_H_
#define SMWDTHROSC_H_
/*---------------------------------------------------------------------------*/
/** \name ST and WDT Register offset declarations
 * @{
 */
#define SMWDTHROSC_WDCTL        0x400D5000 /**< Watchdog Control */
#define SMWDTHROSC_ST0          0x400D5040 /**< ST count/compare value 0 */
#define SMWDTHROSC_ST1          0x400D5044 /**< ST count/compare value 1 */
#define SMWDTHROSC_ST2          0x400D5048 /**< ST count/compare value 2 */
#define SMWDTHROSC_ST3          0x400D504C /**< ST count/compare value 3 */
#define SMWDTHROSC_STLOAD       0x400D5050 /**< Compare value load status */
#define SMWDTHROSC_STCC         0x400D5054 /**< ST capture control */
#define SMWDTHROSC_STCS         0x400D5058 /**< ST capture status */
#define SMWDTHROSC_STCV0        0x400D505C /**< ST capture value 0 */
#define SMWDTHROSC_STCV1        0x400D5060 /**< ST capture value 1 */
#define SMWDTHROSC_STCV2        0x400D5064 /**< ST capture value 2 */
#define SMWDTHROSC_STCV3        0x400D5068 /**< ST capture value 3 */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SMWDTHROSC_WDCTL register bit masks
 * @{
 */
#define SMWDTHROSC_WDCTL_CLR     0x000000F0  /**< Clear timer mask */
#define SMWDTHROSC_WDCTL_CLR_3   0x00000080  /**< Clear timer mask[3] */
#define SMWDTHROSC_WDCTL_CLR_2   0x00000040  /**< Clear timer mask[2] */
#define SMWDTHROSC_WDCTL_CLR_1   0x00000020  /**< Clear timer mask[1] */
#define SMWDTHROSC_WDCTL_CLR_0   0x00000010  /**< Clear timer mask[0] */
#define SMWDTHROSC_WDCTL_EN      0x00000008  /**< Enable mask */
#define SMWDTHROSC_WDCTL_MODE    0x00000004  /**< Mode select mask */
#define SMWDTHROSC_WDCTL_INT     0x00000003  /**< Interval Select mask */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SMWDTHROSC_ST[0:3] register bit masks
 * @{
 */
#define SMWDTHROSC_ST0_ST0       0x000000FF  /**< ST count/compare bits [7:0] */
#define SMWDTHROSC_ST1_ST1       0x000000FF  /**< ST count/compare bits [15:8] */
#define SMWDTHROSC_ST2_ST2       0x000000FF  /**< ST count/compare bits [23:16] */
#define SMWDTHROSC_ST3_ST3       0x000000FF  /**< ST count/compare bits [31:24] */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SMWDTHROSC_STLOAD register bit masks
 * @{
 */
#define SMWDTHROSC_STLOAD_STLOAD 0x00000001  /**< STx upload status signal */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SMWDTHROSC_STCC register bit masks
 * @{
 */
#define SMWDTHROSC_STCC_PORT     0x00000038  /**< Port select */
#define SMWDTHROSC_STCC_PIN      0x00000007  /**< Pin select */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SMWDTHROSC_STCS register bit masks
 * @{
 */
#define SMWDTHROSC_STCS_VALID    0x00000001  /**< Capture valid flag */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name SMWDTHROSC_STCV[0:3] register bit masks
 * @{
 */
#define SMWDTHROSC_STCV0_STCV0   0x000000FF  /**< ST capture bits [7:0] */
#define SMWDTHROSC_STCV1_STCV1   0x000000FF  /**< ST capture bits [15:8] */
#define SMWDTHROSC_STCV2_STCV2   0x000000FF  /**< ST capture bits [23:16] */
#define SMWDTHROSC_STCV3_STCV3   0x000000FF  /**< ST capture bits [32:24] */
/** @} */

#endif /* SMWDTHROSC_H_ */

/**
 * @}
 * @}
 */

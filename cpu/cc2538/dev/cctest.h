/*
 * Original file:
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Port to Contiki:
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
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
 * Header with declarations of CCTEST module registers.
 */
#ifndef CCTEST_H
#define CCTEST_H

/*---------------------------------------------------------------------------*/
/**
 * \name CCTEST register offsets
 * @{
 */
#define CCTEST_IO               0x44010000 /**< Output strength control */
#define CCTEST_OBSSEL0          0x44010014 /**< Observation output 0 */
#define CCTEST_OBSSEL1          0x44010018 /**< Observation output 1 */
#define CCTEST_OBSSEL2          0x4401001C /**< Observation output 2 */
#define CCTEST_OBSSEL3          0x44010020 /**< Observation output 3 */
#define CCTEST_OBSSEL4          0x44010024 /**< Observation output 4 */
#define CCTEST_OBSSEL5          0x44010028 /**< Observation output 5 */
#define CCTEST_OBSSEL6          0x4401002C /**< Observation output 6 */
#define CCTEST_OBSSEL7          0x44010030 /**< Observation output 7 */
#define CCTEST_TR0              0x44010034 /**< Test register 0 */
#define CCTEST_USBCTRL          0x44010050 /**< USB PHY stand-by control */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name CCTEST_IO register bit fields
 * @{
 */
#define CCTEST_IO_SC            0x00000001 /**< I/O strength control */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name CCTEST_OBSSELx registers bit fields
 * @{
 */
#define CCTEST_OBSSEL_EN        0x00000080 /**< Observation output enable */
#define CCTEST_OBSSEL_SEL_M     0x0000007F /**< n - obs_sigs[n] output selection mask */
#define CCTEST_OBSSEL_SEL_S     0          /**< n - obs_sigs[n] output selection shift */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name CCTEST_TR0 register bit fields
 * @{
 */
#define CCTEST_TR0_ADCTM        0x00000002 /**< Connect temperature sensor to ADC */
/** @} */
/*---------------------------------------------------------------------------*/
/** \name CCTEST_USBCTRL register bit fields
 * @{
 */
#define CCTEST_USBCTRL_USB_STB  0x00000001 /**< USB PHY stand-by override */
/** @} */

#endif /* CCTEST_H */
/** @} */

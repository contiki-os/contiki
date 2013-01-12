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
 * Header with declarations of ANA_REGS module registers.
 */
#ifndef ANA_REGS_H
#define ANA_REGS_H

/*---------------------------------------------------------------------------*/
/**
 * \name ANA_REGS register offsets
 * @{
 */
#define ANA_REGS_IVCTRL       0x00000004
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name ANA_REGS_IVCTRL register bit masks
 * @{
 */
#define ANA_REGS_IVCTRL_DAC_CURR_CTRL    0x00000030 /**< Controls bias current to DAC */
#define ANA_REGS_IVCTRL_LODIV_BIAS_CTRL  0x00000008 /**< Controls bias current to LODIV */
#define ANA_REGS_IVCTRL_TXMIX_DC_CTRL    0x00000004 /**< Controls DC bias in TXMIX */
#define ANA_REGS_IVCTRL_PA_BIAS_CTRL     0x00000003 /**< Controls bias current to PA */
/** @} */

#endif /* ANA_REGS_H */
/** @} */

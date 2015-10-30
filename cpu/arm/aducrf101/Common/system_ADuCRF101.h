/**
 * Copyright (c) 2014, Analog Devices, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Analog Devices, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
@file     system_ADuCRF101.h
@brief:   CMSIS Cortex-M3 Device Peripheral Access Layer Header File
          for the ADuCRF101
@version  v0.2
@author   PAD CSE group, Analog Devices Inc
@date     March 09th 2012
**/


#ifndef __SYSTEM_ADUCRF101_H__
#define __SYSTEM_ADUCRF101_H__

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @brief  Initialize the system
 *
 * @return none
 *
 * Setup the microcontroller system.
 * Initialize the System and update the SystemCoreClock variable.
 */
extern void SystemInit (void);

/**
 * @brief  Update internal SystemCoreClock variable
 *
 * @return none
 *
 * Updates the internal SystemCoreClock with current core
 * Clock retrieved from cpu registers.
 */
extern void SystemCoreClockUpdate (void);


/**
 * @brief  Sets the system external clock frequency
 *
 * @param  ExtClkFreq   External clock frequency in Hz
 * @return none
 *
 * Sets the clock frequency of the source connected to P0.5 clock input source
 */
extern void SetSystemExtClkFreq (uint32_t ExtClkFreq);


/**
 * @brief  Gets the system external clock frequency
 *
 * @return External Clock frequency
 *
 * Gets the clock frequency of the source connected to P0.5 clock input source
 */
extern uint32_t GetSystemExtClkFreq (void);


#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_ADUCRF101_H__ */


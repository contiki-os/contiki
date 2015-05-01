/*
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx
 * @{
 *
 * \defgroup cc26xx-oscillators CC26XX oscillator control
 *
 * Wrapper around those CC26xxware OSC functions that we need in Contiki.
 *
 * All CC26xxware OSC control requires access to the semaphore module within
 * AUX. Thus, in addition to enabling the oscillator interface, we need to
 * start the clock to SMPH and restore it to its previous state when we are
 * done.
 * @{
 *
 * \file
 * Header file for the CC26XX oscillator control
 */
/*---------------------------------------------------------------------------*/
#ifndef OSCILLATORS_H_
#define OSCILLATORS_H_
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the LF clock source to be the LF XOSC
 *
 * This function is only called once as soon as the system starts.
 *
 * Do not switch the LF clock source to the RC OSC for normal system operation
 * See CC26xx Errata (swrz058)
 */
void oscillators_select_lf_xosc(void);

/**
 * \brief Set the LF clock source to be the LF RCOSC
 *
 * This function is only called once, when the systen transitions to a full
 * shutdown
 *
 * Do not switch the LF clock source to the RC OSC for normal system operation
 * See CC26xx Errata (swrz058)
 */
void oscillators_select_lf_rcosc(void);

/**
 * \brief Requests the HF XOSC as the source for the HF clock, but does not
 * perform the actual switch.
 *
 * This triggers the startup sequence of the HF XOSC and returns so the CPU
 * can perform other tasks while the XOSC is starting.
 *
 * The XOSC is requested as the source for the HF as well as the MF clock.
 */
void oscillators_request_hf_xosc(void);

/**
 * \brief Performs the switch to the XOSC
 *
 * This function must be preceded by a call to oscillators_request_hf_xosc()
 */
void oscillators_switch_to_hf_xosc(void);

/**
 * \brief Switches MF and HF clock source to be the HF RC OSC
 */
void oscillators_switch_to_hf_rc(void);
/*---------------------------------------------------------------------------*/
#endif /* OSCILLATORS_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Power mode switching functions for the K60 CPU.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#ifndef CPU_ARM_K60_POWER_MODES_H_
#define CPU_ARM_K60_POWER_MODES_H_

#ifdef __cplusplus
extern "C" {
#endif

/** \todo Handle VLLSx modes */
/** \todo Handle VLPR, VLPW modes */

void power_modes_init(void);

/* void power_mode_run(void); */
void power_mode_wait(void);
void power_mode_stop(void);
/* void power_mode_vlpr(void); */
/* void power_mode_vlpw(void); */
void power_mode_vlps(void);
void power_mode_lls(void);
/* void power_mode_vlls1(void); */
/* void power_mode_vlls2(void); */
/* void power_mode_vlls3(void); */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CPU_ARM_K60_POWER_MODES_H_ */

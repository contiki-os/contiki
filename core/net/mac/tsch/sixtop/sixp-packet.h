/*
 * Copyright (c) 2016, Centre for Development of Advanced Computing (C-DAC).
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
 * \addtogroup sixtop
 * @{
 */
/**
 * \file
 *         6top Protocol (6P) Packet Manipulation APIs
 * \author
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 *         Yasuyuki Tanaka <yasuyuki.tanaka@inf.ethz.ch>
 */
#ifndef _SIXTOP_6P_PACKET_H_
#define _SIXTOP_6P_PACKET_H_

#include "sixp.h"

/**
 * \brief Create 6P packet in packetbuf
 * \param ie 6top IE contents to be stored in the packet
 * \return 0 on success, -1 on failure
 */
int sixp_packet_create(const sixtop_ie_t *ie);

/**
 * \brief Parse 6P packet in the specified buffer
 * \param buf The pointer to a buffer having 6P packet
 * \param len Length of the 6P packet
 * \param ie The pointer to a sixtop_ie_t object to store 6P packet contents
 * \return 0 on success, -1 on failure
 */
int sixp_packet_parse(const uint8_t *buf, uint16_t len,
                      sixtop_ie_t *ie);

#endif /* !_SIXP_PACKET_H_ */
/** @} */

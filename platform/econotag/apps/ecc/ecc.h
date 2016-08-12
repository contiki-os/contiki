/*
 * Copyright (c) 2015, Lars Schmertmann <SmallLars@t-online.de>,
 * Jens Trillmann <jtrillma@informatik.uni-bremen.de>.
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
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *      Calculations on elliptic curve secp256r1
 *
 *      This is a efficient ECC implementation on the secp256r1 curve for
 *      32 Bit CPU architectures. It provides basic operations on the
 *      secp256r1 curve and support for ECDH and ECDSA.
 *
 * \author
 *      Lars Schmertmann <SmallLars@t-online.de>
 *      Jens Trillmann <jtrillma@informatik.uni-bremen.de>
 */

#ifndef ECC_H_
#define ECC_H_

#include <stdint.h>

/**
 * \brief  Checks if a (random) number is valid as scalar on elliptic curve secp256r1
 *
 *         A (random) number is only usable as scalar on elliptic curve secp256r1 if
 *         it is lower than the order of the curve. For the check, you need to provide
 *         the order of elliptic curve secp256r1.
 *
 *         uint32_t order[8] = {0xFC632551, 0xF3B9CAC2, 0xA7179E84, 0xBCE6FAAD, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF};
 *
 * \param  key    The (random) number to check for usability
 * \param  order  The order of elliptic curve secp256r1
 *
 * \return 1 if key is valid
 */
#define ecc_is_valid_key(key, order) (ecc_compare(order, key) == 1)

/**
 * \brief  Compares the value of a with the value of b
 *
 *         This function is only public because its needed for the macro ecc_is_valid_key.
 *         It does a comparison of two 256 bit numbers. The return values are 1, 0 or -1.
 *
 * \param  a  First number
 * \param  b  Second number
 *
 * \return  1 if a is greater than b
            0 if a is equal to b
           -1 if a is less than b
 */
int32_t ecc_compare(const uint32_t *a, const uint32_t *b);

/**
 * \brief  ECC scalar multiplication on elliptic curve secp256r1
 *
 *         This function does a scalar multiplication on elliptic curve secp256r1.
 *         For an Elliptic curve Diffie–Hellman you need two multiplications. First one
 *         with the base point of elliptic curve secp256r1 you need to provide.
 *
 *         uint32_t base_x[8] = {0xd898c296, 0xf4a13945, 0x2deb33a0, 0x77037d81, 0x63a440f2, 0xf8bce6e5, 0xe12c4247, 0x6b17d1f2};
 *         uint32_t base_y[8] = {0x37bf51f5, 0xcbb64068, 0x6b315ece, 0x2bce3357, 0x7c0f9e16, 0x8ee7eb4a, 0xfe1a7f9b, 0x4fe342e2};
 *
 * \param  resultx  Pointer to memory to store the x-coordinate of the result
 * \param  resulty  Pointer to memory to store the y-coordinate of the result
 * \param  px       x-coordinate of the point to multiply with scalar
 * \param  py       y-coordinate of the point to multiply with scalar
 * \param  secret   Scalar for multiplication with elliptic curve point
 */
void ecc_ec_mult(uint32_t *resultx, uint32_t *resulty, const uint32_t *px, const uint32_t *py, const uint32_t *secret);

#endif /* ECC_H_ */

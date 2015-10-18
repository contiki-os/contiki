/*
 * Copyright (c) 2014, Institute for Pervasive Computing, ETH Zurich.
 * All rights reserved.
 *
 * Author: Andreas Dröscher <contiki@anticat.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/**
 * \addtogroup c2538-ecc-algo
 * @{
 *
 * \file
 * Implementation of the cc2538 ECC Algorithms
 */
#include <contiki.h>
#include <process.h>

#include <limits.h>
#include <stdio.h>

#include "ecc-algorithm.h"
#include "ecc-driver.h"
#include "pka.h"

#define CHECK_RESULT(...)                                                  \
  state->result = __VA_ARGS__;                                             \
  if(state->result) {                                                      \
    printf("Line: %u Error: %u\n", __LINE__, (unsigned int)state->result); \
    PT_EXIT(&state->pt);                                                   \
  }

PT_THREAD(ecc_compare(ecc_compare_state_t *state)) {
  PT_BEGIN(&state->pt);

  CHECK_RESULT(bignum_cmp_start(state->a, state->b, state->size, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  state->result = bignum_cmp_get_result();

  PT_END(&state->pt);
}

PT_THREAD(ecc_multiply(ecc_multiply_state_t *state)) {
  PT_BEGIN(&state->pt);

  CHECK_RESULT(ecc_mul_start(state->secret, &state->point_in, state->curve_info, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(ecc_mul_get_result(&state->point_out, state->rv));

  PT_END(&state->pt);
}

PT_THREAD(ecc_dsa_sign(ecc_dsa_sign_state_t *state)) {
  /* Executed Every Time */
  uint8_t size = state->curve_info->size;
  const uint32_t *ord = state->curve_info->n;

  ec_point_t point;
  memcpy(point.x, state->curve_info->x, sizeof(point.x));
  memcpy(point.y, state->curve_info->y, sizeof(point.y));

  PT_BEGIN(&state->pt);

  /* Invert k_e mod n */
  CHECK_RESULT(bignum_inv_mod_start(state->k_e, size, ord, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(bignum_inv_mod_get_result(state->k_e_inv, size, state->rv));

  /* Calculate Point R = K_e * GeneratorPoint */
  CHECK_RESULT(ecc_mul_start(state->k_e, &point, state->curve_info, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(ecc_mul_get_result(&state->point_r, state->rv));

  /* Calculate signature using big math functions
   * d*r (r is the x coordinate of PointR) */
  CHECK_RESULT(bignum_mul_start(state->secret, size, state->point_r.x, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  state->len = 24;
  CHECK_RESULT(bignum_mul_get_result(state->signature_s, &state->len, state->rv));

  /* d*r mod n */
  CHECK_RESULT(bignum_mod_start(state->signature_s, state->len, ord, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(bignum_mod_get_result(state->signature_s, size, state->rv));

  /* hash + d*r */
  CHECK_RESULT(bignum_add_start(state->hash, size, state->signature_s, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  state->len = 24;
  CHECK_RESULT(bignum_add_get_result(state->signature_s, &state->len, state->rv));

  /* hash + d*r mod n */
  CHECK_RESULT(bignum_mod_start(state->signature_s, state->len, ord, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(bignum_mod_get_result(state->signature_s, size, state->rv));

  /* k_e_inv * (hash + d*r) */
  CHECK_RESULT(bignum_mul_start(state->k_e_inv, size, state->signature_s, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  state->len = 24;
  CHECK_RESULT(bignum_mul_get_result(state->signature_s, &state->len, state->rv));

  /* k_e_inv * (hash + d*r) mod n */
  CHECK_RESULT(bignum_mod_start(state->signature_s, state->len, ord, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(bignum_mod_get_result(state->signature_s, size, state->rv));

  PT_END(&state->pt);
}

PT_THREAD(ecc_dsa_verify(ecc_dsa_verify_state_t *state)) {
  /* Executed Every Time */
  uint8_t size = state->curve_info->size;
  const uint32_t *ord = state->curve_info->n;

  ec_point_t point;
  memcpy(point.x, state->curve_info->x, sizeof(point.x));
  memcpy(point.y, state->curve_info->y, sizeof(point.y));

  PT_BEGIN(&state->pt);

  /* Invert s mod n */
  CHECK_RESULT(bignum_inv_mod_start(state->signature_s, size, ord, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(bignum_inv_mod_get_result(state->s_inv, size, state->rv));

  /* Calculate u1 = s_inv * hash */
  CHECK_RESULT(bignum_mul_start(state->s_inv, size, state->hash, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  state->len = 24;
  CHECK_RESULT(bignum_mul_get_result(state->u1, &state->len, state->rv));

  /* Calculate u1 = s_inv * hash mod n */
  CHECK_RESULT(bignum_mod_start(state->u1, state->len, ord, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(bignum_mod_get_result(state->u1, size, state->rv));

  /* Calculate u2 = s_inv * r */
  CHECK_RESULT(bignum_mul_start(state->s_inv, size, state->signature_r, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  state->len = 24;
  CHECK_RESULT(bignum_mul_get_result(state->u2, &state->len, state->rv));

  /* Calculate u2 = s_inv * r mod n */
  CHECK_RESULT(bignum_mod_start(state->u2, state->len, ord, size, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(bignum_mod_get_result(state->u2, size, state->rv));

  /* Calculate p1 = u1 * A */
  CHECK_RESULT(ecc_mul_start(state->u1, &point, state->curve_info, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(ecc_mul_get_result(&state->p1, state->rv));

  /* Calculate p2 = u1 * B */
  CHECK_RESULT(ecc_mul_start(state->u2, &state->public, state->curve_info, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(ecc_mul_get_result(&state->p2, state->rv));

  /* Calculate P = p1 + p2 */
  CHECK_RESULT(ecc_add_start(&state->p1, &state->p2, state->curve_info, &state->rv, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  CHECK_RESULT(ecc_add_get_result(&state->p1, state->rv));

  /* Verify Result */
  CHECK_RESULT(bignum_cmp_start(state->signature_r, state->p1.x, size, state->process));
  PT_WAIT_UNTIL(&state->pt, pka_check_status());
  state->result = bignum_cmp_get_result();
  if((state->result == PKA_STATUS_A_GR_B) || (state->result == PKA_STATUS_A_LT_B)) {
    state->result = PKA_STATUS_SIGNATURE_INVALID;
  }

  PT_END(&state->pt);
}

/**
 * @}
 * @}
 */

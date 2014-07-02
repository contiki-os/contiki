#include "contikiecc/ecc/ecc.h"
#include "contikiecc/ecc/ecc_sha1.h"
#include <string.h>
#include <stdio.h>

// enable to not overflow stack on stm32w with owaecc_perf_eval
//#define REDUCE_LOCAL_VARIABLES
/*---------------------------------------------------------------------------*/
#ifdef REDUCE_LOCAL_VARIABLES

void
owaecc_add_key(point_t * acc, point_t * pb_key)
{
  uint16_t len = NUMWORDS * NN_DIGIT_LEN;
  uint8_t hash_x[len];
  uint8_t hash_y[len];
  uint8_t hash[len];
  NN_DIGIT hash_tmp[NUMWORDS];
  point_t acc_copy;

  /* Convert point_t -> uint8_t (or byte[]) */
  memset(hash_x, 0, len);
  memset(hash_y, 0, len);
  memset(hash, 0, len);
  memset(hash_tmp, 0, NUMWORDS);
  /* hash(hash (pb_key.x) + hash(pb_key.y)) */
  contikiecc_sha1((uint8_t *) pb_key->x, len, hash_x);
  contikiecc_sha1((uint8_t *) pb_key->y, len, hash_y);
  NN_Add(hash_tmp, (NN_DIGIT *) hash_x, (NN_DIGIT *) hash_y, NUMWORDS);
  contikiecc_sha1((uint8_t *) hash_tmp, len, hash);

  p_copy(&acc_copy, acc);
  ecc_mul(acc, &acc_copy, (NN_DIGIT *) hash);
}

#else
void
owaecc_add_key(point_t * acc, point_t * pb_key)
{
  uint16_t len = NUMWORDS * NN_DIGIT_LEN;
  uint8_t hash_1[len];
  uint8_t hash_2[len];
  uint8_t hash[len];
  uint8_t msg_1[len];
  uint8_t msg_2[len];
  uint8_t msg[len];
  NN_DIGIT hash_1_tmp[NUMWORDS];
  NN_DIGIT hash_2_tmp[NUMWORDS];
  NN_DIGIT hash_tmp[NUMWORDS];
  point_t acc_copy;

  // Convert point_t -> uint8_t (or byte[])
  memset(hash_1, 0, len);
  memset(hash_2, 0, len);
  memset(hash, 0, len);
  memset(msg_1, 0, len);
  memset(msg_2, 0, len);
  memset(msg, 0, len);
  memset(hash_1_tmp, 0, NUMWORDS);
  memset(hash_2_tmp, 0, NUMWORDS);
  memset(hash_tmp, 0, NUMWORDS);

  // hash(hash (pb_key.x) + hash(pb_key.y))
  NN_Encode(msg_1, len, pb_key->x, NUMWORDS);
  contikiecc_sha1(msg_1, len, hash_1);
  NN_Decode(hash_1_tmp, NUMWORDS, hash_1, len);

  NN_Encode(msg_2, len, pb_key->y, NUMWORDS);
  contikiecc_sha1(msg_2, len, hash_2);
  NN_Decode(hash_2_tmp, NUMWORDS, hash_2, len);

  /* NN_DIGIT NN_Add(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, 
   * NN_UINT digits); */
  // Computes hash_tmp = hash_1_tmp + hash_2_tmp
  NN_Add(hash_tmp, hash_1_tmp, hash_2_tmp, NUMWORDS);
  NN_Encode(msg, len, hash_tmp, NUMWORDS);
  contikiecc_sha1(msg, len, hash);

  NN_Decode(hash_tmp, NUMWORDS, hash, len);
  /* P0 = n * P1, P0 and P1 can not be same pointer.
   * void ecc_mul(point_t * P0, point_t * P1, NN_DIGIT * n); */
  p_copy(&acc_copy, acc);
  ecc_mul(acc, &acc_copy, hash_tmp);
}
#endif
/*---------------------------------------------------------------------------*/
uint8_t
owaecc_check_key(point_t * pb_key, point_t * partial_acc, point_t * full_acc)
{
  point_t partial_acc_copy;
  p_copy(&partial_acc_copy, partial_acc);
  owaecc_add_key(&partial_acc_copy, pb_key);
  return p_equal(full_acc, &partial_acc_copy);
}
/*---------------------------------------------------------------------------*/

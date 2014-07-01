/**
 * \addtogroup ecc
 *
 * @{
 */

/**
 * \file
 * 			Interface to Diffie–Hellman functions
 * \author
 * 			Kasun Hewage <kasun.ch@gmail.com>, port to Contiki
 *			Vilhelm Jutvik <ville@imorgon.se>, created the interface and reshuffled some stuff
 *
 */
#include <string.h>
#include "contiki.h"
#include "nn.h"
#include "ecc.h"

/**
  * Encodes the public key beginning at start.
  */
uint8_t *ecdh_encode_public_key(uint32_t *start, NN_DIGIT *myPrvKey)
{
  uint8_t *ptr = (uint8_t *) start;
  uint16_t len = KEYDIGITS * NN_DIGIT_LEN;
  point_t pubKey;

  ecc_gen_public_key(&pubKey, myPrvKey);
  NN_Encode(ptr, len, pubKey.x, KEYDIGITS);
  ptr += len;
  NN_Encode(ptr, len, pubKey.y, KEYDIGITS);
  
  return ptr + len;
}


/**
  * Calculate the shared key
  *
  * \parameter shared_key Pointer to the shared key (the X coordinate of the resulting point). Must be IKE_DH_SCALAR_LEN bytes long.\
  * \parameter peerKeData The public key (commonly that of the other party). IKE_DH_POINT_LEN. Big endian.
  * \parameter myPrvKey The private key (commonly ours). 24 bytes long. Small endian ContikiECC-format.
  */
void ecdh_get_shared_secret(uint8_t *shared_key, uint8_t *peerKeData, NN_DIGIT *myPrvKey) 
{
  // Multiplicate
  
  // Projective optimization disabled as of now since the *Z-functions have not been ported -Ville 111006
  //#ifdef PROJECTIVE
  //    call ECC.win_precompute_Z(PublicKey, baseArray, ZList);
  //    call ECC.win_mul_Z(&tempP, PrivateKey, baseArray, ZList);
  //#else
  
  // Decode peerKeData into point_t (and from big endian network byte order to small endian ContikiECC)
  point_t peerPubPoint;

  /*
  // 519A1216 80E00454 66BA21DF 2EEE47F5 973B5005 77EF13D5
  peerPubPoint.x[24] = 0x0;
  peerPubPoint.x[23] = 0x51;
  peerPubPoint.x[22] = 0x9A;
  peerPubPoint.x[21] = 0x12;
  peerPubPoint.x[20] = 0x16;
  peerPubPoint.x[19] = 0x80;
  peerPubPoint.x[18] = 0xE0;
  peerPubPoint.x[17] = 0x04;
  peerPubPoint.x[16] = 0x54;
  peerPubPoint.x[15] = 0x66;
  peerPubPoint.x[14] = 0xBA;
  peerPubPoint.x[13] = 0x21;
  peerPubPoint.x[12] = 0xDF;
  peerPubPoint.x[11] = 0x2E;
  peerPubPoint.x[10] = 0xEE;
  peerPubPoint.x[9] = 0x47;
  peerPubPoint.x[8] = 0xF5;
  peerPubPoint.x[7] = 0x97;
  peerPubPoint.x[6] = 0x3B;
  peerPubPoint.x[5] = 0x50;
  peerPubPoint.x[4] = 0x05;
  peerPubPoint.x[3] = 0x77;
  peerPubPoint.x[2] = 0xEF;
  peerPubPoint.x[1] = 0x13;
  peerPubPoint.x[0] = 0xD5;
  
  // FF613AB4 D64CEE3A 20875BDB 10F953F6 B30CA072 C60AA57F
  peerPubPoint.y[24] = 0x0;
  peerPubPoint.y[23] = 0xFF;
  peerPubPoint.y[22] = 0x61;
  peerPubPoint.y[21] = 0x3A;
  peerPubPoint.y[20] = 0xB4;
  peerPubPoint.y[19] = 0xD6;
  peerPubPoint.y[18] = 0x4C;
  peerPubPoint.y[17] = 0xEE;
  peerPubPoint.y[16] = 0x3A;
  peerPubPoint.y[15] = 0x20;
  peerPubPoint.y[14] = 0x87;
  peerPubPoint.y[13] = 0x5B;
  peerPubPoint.y[12] = 0xDB;
  peerPubPoint.y[11] = 0x10;
  peerPubPoint.y[10] = 0xF9;
  peerPubPoint.y[9] = 0x53;
  peerPubPoint.y[8] = 0xF6;
  peerPubPoint.y[7] = 0xB3;
  peerPubPoint.y[6] = 0x0C;
  peerPubPoint.y[5] = 0xA0;
  peerPubPoint.y[4] = 0x72;
  peerPubPoint.y[3] = 0xC6;
  peerPubPoint.y[2] = 0x0A;
  peerPubPoint.y[1] = 0xA5;
  peerPubPoint.y[0] = 0x7F;
  */
  
  //uint16_t len = KEYDIGITS * NN_DIGIT_LEN;
  NN_Decode(peerPubPoint.x, MAX_NN_DIGITS, peerKeData, KEYDIGITS * NN_DIGIT_LEN);  
  peerKeData += KEYDIGITS * NN_DIGIT_LEN;
  NN_Decode(peerPubPoint.y, MAX_NN_DIGITS, peerKeData, KEYDIGITS * NN_DIGIT_LEN);  
  
  point_t tempP;
  #ifdef SLIDING_WIN
    ecc_win_precompute(peerKeData, baseArray);
    ecc_win_mul(&tempP, myPrvKey, baseArray);
  //#endif //PROJECTIVE
  #else  //SLIDING_WIN
    ecc_mul(&tempP, &peerPubPoint, myPrvKey);
  #endif  //SLIDING_WIN
  
  // Encode the shared key to string. Big endian.
  NN_Encode(shared_key, KEYDIGITS * NN_DIGIT_LEN, tempP.x, KEYDIGITS);
}

/** @} */

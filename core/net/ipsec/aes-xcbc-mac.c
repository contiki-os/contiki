/**
 * \file
 *         AES-XCBC Message Authentication Code mode of operation
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include <string.h>
#include "contiki-conf.h"
#include "net/uip.h"

#if WITH_IPSEC

#include "ipsec.h"
#include "aes-moo.h"

/*---------------------------------------------------------------------------*/
/* Pre-shared AES-XCBC-MAC keys, obtained from the main AES key as follows (from RFC 3566):
   Derive 3 128-bit keys (K1, K2 and K3) from the 128-bit secret key K, as follows:
        K1 = 0x01010101010101010101010101010101 encrypted with Key K
        K2 = 0x02020202020202020202020202020202 encrypted with Key K
        K3 = 0x03030303030303030303030303030303 encrypted with Key K

   We provide a php script doing this in scripts/calculate_xcbc_keys.php
 */
static const unsigned char aesxcbc_k1[IPSEC_KEYSIZE] =
{0x3b,0xda,0x5b,0x6c,0x05,0x59,0x5d,0xe5,0x64,0x2b,0xf6,0x13,0xf8,0xd1,0xaf,0xd4};
static const unsigned char aesxcbc_k2[IPSEC_KEYSIZE] =
{0x74,0xf8,0x30,0xba,0x3a,0x1a,0x18,0x52,0x0d,0x2f,0xae,0x03,0x50,0xbe,0xb0,0x53};
static const unsigned char aesxcbc_k3[IPSEC_KEYSIZE] =
{0xca,0x6c,0x1f,0x23,0x64,0x70,0x37,0x07,0xb5,0xd1,0x8d,0x9b,0xc0,0x65,0x6b,0x43};

/*---------------------------------------------------------------------------*/
struct aes_xcbc_mac {
  u8_t prev[IPSEC_KEYSIZE];
};
/*---------------------------------------------------------------------------*/
static void
aes_xcbc_mac_init(struct aes_xcbc_mac *axcbc, const unsigned char *key)
{
  /* Set key */
  CRYPTO_AES.init(key);
  /* No previous block, set to 0 */
  memset(axcbc->prev, 0, IPSEC_KEYSIZE);
}
/*---------------------------------------------------------------------------*/
static void
aes_xcbc_mac_step(struct aes_xcbc_mac *axcbc, unsigned char *buff)
{
  int i;
  /* prev ^= buff */
  for(i=0; i<IPSEC_KEYSIZE; i++) {
    axcbc->prev[i] ^= buff[i];
  }
  /* AES encrypt prev */
  CRYPTO_AES.encrypt(axcbc->prev);
}
/*---------------------------------------------------------------------------*/
static void
aes_xcbc_mac_final_step(struct aes_xcbc_mac *axcbc, unsigned char *buff, int len,
    const unsigned char *key2, const unsigned char *key3)
{
  int i;
  unsigned char tmp[IPSEC_KEYSIZE];
  /* the key is not the same if the last block isn't full */
  const unsigned char *key = (len == IPSEC_KEYSIZE) ? key2 : key3;
  /* tmp = buff */
  memcpy(tmp, buff, IPSEC_KEYSIZE);
  /* add padding if needed */
  for(i=0; i<IPSEC_KEYSIZE-len; i++) {
    tmp[len+i] = (i == 0) ? 0x80 : 0x00;
  }
  /* lastinput ^= key */
  for(i=0; i<IPSEC_KEYSIZE; i++) {
    tmp[i] ^= key[i];
  }
  /* run normal step on tmp */
  aes_xcbc_mac_step(axcbc, tmp);
}
/*---------------------------------------------------------------------------*/
static void
aes_xcbc_mac_process(unsigned char *out, unsigned char *buff, int bufflen,
    const unsigned char *key, const unsigned char *key2, const unsigned char *key3)
{
  int i;
  int len;
  struct aes_xcbc_mac axcbc;
  aes_xcbc_mac_init(&axcbc, key);
  for(i=0; i< (bufflen-1)/IPSEC_KEYSIZE; i++) {
    aes_xcbc_mac_step(&axcbc, buff + i * IPSEC_KEYSIZE);
  }
  len = bufflen % IPSEC_KEYSIZE;
  aes_xcbc_mac_final_step(&axcbc, buff + i * IPSEC_KEYSIZE,
           len == 0 ? IPSEC_KEYSIZE : len, key2, key3);
  memcpy(out, axcbc.prev, IPSEC_MACSIZE);
}
/*---------------------------------------------------------------------------*/
void
auth(unsigned char *out, unsigned char *buff, int bufflen)
{
  aes_xcbc_mac_process(out, buff, bufflen, aesxcbc_k1, aesxcbc_k2, aesxcbc_k3);
}
/*---------------------------------------------------------------------------*/
struct ipsec_mac_implem aesxcbc_mac = {
    auth,
};
/*---------------------------------------------------------------------------*/

#endif /* WITH_IPSEC */

/**
 * \file
 *         AES-CTR block cipher mode of operation
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
/* pre-shared AES key */
static const unsigned char aes_key[IPSEC_KEYSIZE] =
{0xcf,0x5f,0xaa,0xca,0x70,0xee,0x5e,0xc4,0xc8,0xf4,0x31,0x58,0xa4,0x5c,0x03,0x63};

/* pre-shared AES-CTR nonce */
static const unsigned char aes_ctr_nonce[] =
{0x69,0xbb,0xc0,0xc9};

#define AESCTR_NONCESIZE 4

/*---------------------------------------------------------------------------*/
struct aes_ctr {
  u8_t counter[IPSEC_KEYSIZE];
} aes_ctr;
/*---------------------------------------------------------------------------*/
static void
aes_ctr_init(struct aes_ctr *actr, const unsigned char *key,
    const unsigned char *iv, const unsigned char *nonce)
{
  /* Set key */
  CRYPTO_AES.init(key);
  /* Initialize counter block */
  memcpy(actr->counter, nonce, AESCTR_NONCESIZE);
  memcpy(actr->counter + AESCTR_NONCESIZE, iv, IPSEC_IVSIZE);
  *((u32_t *)(actr->counter + AESCTR_NONCESIZE + IPSEC_IVSIZE)) = UIP_HTONL(1);
}
/*---------------------------------------------------------------------------*/
static void
aes_ctr_step(struct aes_ctr *actr, unsigned char *buff, int len)
{
  int i;
  unsigned char tmp[IPSEC_KEYSIZE];
  u32_t count;
  if(len == 0) {
    return;
  }
  /* tmp = counter */
  memcpy(tmp, actr->counter, IPSEC_KEYSIZE);
  /* AES encrypt tmp */
  CRYPTO_AES.encrypt(tmp);
  /* buff ^= tmp */
  for (i=0; i<len; i++) {
    buff[i] ^= tmp[i];
  }
  /* counter++ */
  count = UIP_HTONL(*((uint32_t*)(actr->counter + 12)));
  *((uint32_t*)(actr->counter + 12)) = UIP_HTONL(count+1);
}
/*---------------------------------------------------------------------------*/
static void
aes_ctr_process(unsigned char *buff, int bufflen,
    const unsigned char *iv, const unsigned char *key, const unsigned char *nonce)
{
  int i;
  struct aes_ctr actr;
  aes_ctr_init(&actr, key, iv, nonce);
  for(i=0; i<bufflen/IPSEC_KEYSIZE; i++) {
    aes_ctr_step(&actr, buff + i * IPSEC_KEYSIZE, IPSEC_KEYSIZE);
  }
  aes_ctr_step(&actr, buff + i * IPSEC_KEYSIZE, bufflen % IPSEC_KEYSIZE);
}
/*---------------------------------------------------------------------------*/
void
encrypt_decrypt(unsigned char *buff, int bufflen, const unsigned char *iv)
{
  aes_ctr_process(buff, bufflen, iv, aes_key, aes_ctr_nonce);
}
/*---------------------------------------------------------------------------*/
struct ipsec_encrypt_implem aesctr = {
    encrypt_decrypt,
    encrypt_decrypt,
};
/*---------------------------------------------------------------------------*/

#endif /* WITH_IPSEC */

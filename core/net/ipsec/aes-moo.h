/**
 * \file
 *         Generic interfaces for AES and modes of operation
 *         (block cipher, MAC)
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __AES_MOO_H__
#define __AES_MOO_H__

#if WITH_IPSEC

#ifdef CRYPTO_CONF_AES
#define CRYPTO_AES CRYPTO_CONF_AES
#else
#define CRYPTO_AES default_aes
#endif

#ifdef IPSEC_CONF_BLOCK
#define IPSEC_BLOCK IPSEC_CONF_BLOCK
#else
#define IPSEC_BLOCK aesctr
#endif

#ifdef IPSEC_CONF_MAC
#define IPSEC_MAC IPSEC_CONF_MAC
#else
#define IPSEC_MAC aesxcbc_mac
#endif

struct aes_implem {
  void (*init)(const unsigned char *key);
  void (*encrypt)(unsigned char *buff);
  void (*decrypt)(unsigned char *buff);
};

struct ipsec_encrypt_implem {
  void (*encrypt)(unsigned char *buff, int bufflen, const unsigned char *iv);
  void (*decrypt)(unsigned char *buff, int bufflen, const unsigned char *iv);
};

struct ipsec_mac_implem {
  void (*auth)(unsigned char *out, unsigned char *buff, int bufflen);
};

extern struct aes_implem CRYPTO_AES;
extern struct ipsec_encrypt_implem IPSEC_BLOCK;
extern struct ipsec_mac_implem IPSEC_MAC;

#endif /* WITH_IPSEC */

#endif /* __AES_MOO_H__ */

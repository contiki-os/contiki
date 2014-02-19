/**
 * \file
 *         Mapping the cc2420 crypto functions
 *         to the generic aes_implem interface
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 */

#include "contiki-conf.h"

#if WITH_IPSEC

#include "dev/cc2420-aes.h"
#include "ipsec.h"
#include "aes-moo.h"

/*---------------------------------------------------------------------------*/
static void
aes_init(const unsigned char *key)
{
  cc2420_aes_set_key(key, 0);
}
/*---------------------------------------------------------------------------*/
static void
aes_encrypt(unsigned char *buff)
{
  cc2420_aes_cipher(buff, IPSEC_KEYSIZE, 0);
}
/*---------------------------------------------------------------------------*/
struct aes_implem cc2420_aes = {
    aes_init,
    aes_encrypt,
    NULL,
};
/*---------------------------------------------------------------------------*/

#endif /* WITH_IPSEC */

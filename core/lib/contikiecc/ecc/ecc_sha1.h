/**
 * \addtogroup ecc
 *
 * @{
 */

/**
 * \file
 * 			SHA1 implementation
 * \author
 * 			Kasun Hewage <kasun.ch@gmail.com>, port to Contiki
 *			Vilhelm Jutvik <ville@imorgon.se>, bug fixes, adaption to IKEv2
 *
 * \details
 *  ecc_sha1.h
 *
 *  Description:
 *      This is the header file for code which implements the Secure
 *      Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
 *      April 17, 1995.
 *
 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *
 *      Please read the file sha1.c for more information.
 *
 * This SHA-1 implementation is ported by An Liu from the example code
 * in RFC 3174
 */

#ifndef _ECC_SHA1_H_
#define _ECC_SHA1_H_

#include <stdint.h>
/*
 * If you do not have the ISO standard stdint.h header file, then you
 * must typdef the following:
 *    name              meaning
 *  uint32_t         unsigned 32 bit integer
 *  uint8_t          unsigned 8 bit integer (i.e., unsigned char)
 *  int_least16_t    integer of >= 16 bits
 *
 */

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};
#endif
#define SHA1HashSize 20

/*
 *  This structure will hold context information for the SHA-1
 *  hashing operation
 */
typedef struct SHA1Context
{
    uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

    uint32_t Length_Low;            /* Message length in bits      */
    uint32_t Length_High;           /* Message length in bits      */

                               /* Index into message block array   */
    uint16_t Message_Block_Index;
    uint8_t Message_Block[64];      /* 512-bit message blocks      */

    int Computed;               /* Is the digest computed?         */
    int Corrupted;             /* Is the message digest corrupted? */
} SHA1Context;

int sha1_reset(SHA1Context *context);
int sha1_digest(SHA1Context *context, uint8_t Message_Digest[SHA1HashSize]);
int sha1_update(SHA1Context *context, const uint8_t *message_array, uint32_t length);
void contikiecc_sha1(uint8_t *msg, uint16_t len, uint8_t *hash);
#endif


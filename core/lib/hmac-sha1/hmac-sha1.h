/*
 * hmac-sha1-96.h
 *
 *  Created on: Aug 4, 2010
 *      Author: dogan
 *
 * Removed the 96 bit limit. - Ville
 */

#ifndef HMACSHA196_H_
#define HMACSHA196_H_

#define HMAC_SHA1_OUTPUT_LEN 20

/**
 * Performs HMAC-SHA1. Call this function from your program.
 *
 * @param out     Output from HMAC-SHA1 (of length HMAC_SHA1_OUTPUT_LEN)
 * @param key     The key to use
 * @param keylen  The length of the key. HMAC_SHA1_OUTPUT_LEN bytes is the "preferred" key length (RFC 5996, p. 46, second paragraph).
 * @param data    The data to hash
 * @param datalen The length of the data
 */
typedef struct {
  uint8_t *out; 
  const uint8_t *key; 
  uint8_t keylen;
  uint8_t *data; 
  uint16_t datalen;
} hmac_data_t;

void hmac_sha1(hmac_data_t *hmac_data);

#endif /* HMACSHA196_H_ */

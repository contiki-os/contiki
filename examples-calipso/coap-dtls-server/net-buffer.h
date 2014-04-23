/*
 * net-buffer.h
 *
 *  Created on: Mar 12, 2014
 *      Author: root
 */

#ifndef NET_BUFFER_H_
#define NET_BUFFER_H_

#include "dtls.h"

#define DEF_BUFFER_SIZE 50

typedef struct buf_record_t {
  struct buf_record_t *next;
  session_t session;		/**< remote address */
  size_t length;		/**< actual length of data */
  unsigned char data[DTLS_MAX_BUF];		/**< the datagram to send */
} buf_record_t;

#endif /* NET_BUFFER_H_ */

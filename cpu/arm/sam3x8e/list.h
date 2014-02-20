#include "skbuff.h"
#include <stdint-gcc.h>

/*
 * list.h
 *
 * Created: 4/13/2013 7:16:14 PM
 *  Author: Ioannis Glaropoulos
 */ 


#ifndef _LIST_H_
#define _LIST_H_


struct ar9170_send_list;

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct _list_head {
	struct _list_head *next, *prev;
};

/*
 * Implementation of a single linked list for pending commands / chunks
 */
struct ar9170_send_list {
	uint8_t* buffer;
	uint16_t send_chunk_len;
	struct ar9170_send_list* next_send_chunk;
} __attribute__ ((packed));



#endif /* _LIST_H_ */
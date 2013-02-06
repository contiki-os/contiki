/************************************************************************************/
/* The MIT License (MIT)							    */
/* 										    */
/* Copyright (c) 2012 Jeff Ciesielski <jeffciesielski@gmail.com>		    */
/*                    Andrey Smirnov <andrew.smirnov@gmail.com>			    */
/* 										    */
/* Permission is hereby granted, free of charge, to any person obtaining a copy of  */
/* this software and associated documentation files (the "Software"), to deal in    */
/* the Software without restriction, including without limitation the rights to	    */
/* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies    */
/* of the Software, and to permit persons to whom the Software is furnished to do   */
/* so, subject to the following conditions:					    */
/* 										    */
/* -The above copyright notice and this permission notice shall be included in all  */
/*  copies or substantial portions of the Software.				    */
/* 										    */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR	    */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS */
/* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR   */
/* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER   */
/* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN	    */
/* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.	    */
/************************************************************************************/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

struct generic_queue {
	volatile void *head;
	volatile void *tail;
	size_t item_size;
	size_t len;
	size_t max_capacity;
	volatile uint8_t memory[0];
};

#define DECLARE_QUEUE(element_type, name, max_size)		\
        struct name {                                           \
                struct generic_queue gq;                        \
                element_type __elements[max_size];              \
        } name = {                                              \
                .gq={                                           \
                        .len		= 0,                    \
                        .item_size	= sizeof(element_type), \
                        .max_capacity	= max_size,             \
                },                                              \
        }


static inline bool queue_is_empty(volatile void *q)
{
	volatile struct generic_queue *gq = q;

	return (gq->len == 0);
}
static inline int queue_get_len(volatile void *q)
{
	volatile struct generic_queue *gq = q;
	return gq->len;
}
static inline bool queue_is_full(volatile void *q)
{
	volatile struct generic_queue *gq = q;
	return (gq->len >= gq->max_capacity);
}
void queue_enqueue(volatile void *q, const void *elt) __attribute__((nonnull));
void queue_dequeue(volatile void *q, void *elt) __attribute__((nonnull));

#endif

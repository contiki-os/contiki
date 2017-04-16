/*
 * Copyright (c) 2012, Alex Barclay.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 *
 * Author: Alex Barclay <alex@planet-barclay.com>
 *
 */

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../freebuf.h"

void
dump_buf(void *base, size_t len)
{
  uint8_t *t = (uint8_t *)base;

  while(len > 0) {
    printf("%016llx    ", (unsigned long long)t);
    int i;
    for(i = 0; i < 16 && len > 0; ++i, --len) {
      printf("%02x ", *t++);
    }
    printf("\n");
  }
}
static void *freebuf_head;

int
main(int argc, char **argv)
{
  void *test_storage[51];

  void *buf = malloc(510);

  dump_buf(buf, 510);

  freebuf_init(&freebuf_head, buf, 10, 51);

  dump_buf(buf, 510);

  {
    int i;
    for(i = 0; i < 51; ++i) {
      test_storage[i] = freebuf_pop(&freebuf_head);
      assert(test_storage[i] != 0);
    }

    dump_buf(buf, 510);

    assert(freebuf_head == 0);
    assert(freebuf_pop(&freebuf_head) == 0);

    for(i = 0; i < 51; ++i) {
      freebuf_push(&freebuf_head, test_storage[i]);
      assert(freebuf_head != 0);
    }
  }

  dump_buf(buf, 510);

  return 0;
}

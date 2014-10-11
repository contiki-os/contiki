/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Device I/O helpers for a no-op device, implementations.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include <reent.h>
#include <errno.h>
#include <string.h>
#include "devio-null.h"

int  devnull_open_r(struct _reent *r, const char *path, int flags, int mode) {
  return 0;
}

int  devnull_close_r(struct _reent *r, int fd) {
  return 0;
}

long devnull_write_r(struct _reent *r, int fd, const char *ptr, int len) {
  /* Aaand... it's gone! */
  return len;
}

long devnull_read_r(struct _reent *r, int fd, char *ptr, int len) {
  /* Read null bytes */
  memset(ptr, '\0', len);
  return len;
}

long devnull_lseek_r(struct _reent *r, int fd, int ptr, int dir) {
  r->_errno = ENOSYS;
  return -1;
}

long devnull_fstat_r(struct _reent *r, int fd, char *ptr, int len) {
  r->_errno = ENOSYS;
  return -1;
}



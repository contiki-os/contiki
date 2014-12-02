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
 *         Device I/O mappings for the Mulle platform.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */
#ifndef DEVICEMAP_H_
#define DEVICEMAP_H_

#include "devopttab.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of file descriptors allocated to hardware devices. All fd's
 * above this number will be remapped to CFS accesses. */
#define MAX_OPEN_DEVICES 16 /* Arbitrarily chosen */

/** Number of IO devices in this platform implementation */
#define NUM_IO_DEVICES 16 /* Arbitrarily chosen */

/* This table maps the standard streams to device operations table entries. */
extern const devoptab_t *devoptab_list[MAX_OPEN_DEVICES];

/* This table maps filenames to devices */
typedef struct {
  const char *name;
  const devoptab_t *devoptab;
} devoptab_name_t;

typedef struct {
  unsigned int len;
  const devoptab_name_t* data;
} devoptab_name_list_t;

extern const devoptab_name_list_t devoptab_name_list;

void devicemap_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* !defined(DEVICEMAP_H_) */

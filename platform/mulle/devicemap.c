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

#include <stddef.h> /* for NULL */
#include "devopttab.h"
#include "devicemap.h"
#include "devio-uart.h"
#include "devio-null.h"
#include "K60.h"

static const devoptab_t dotab_uart0 = {
  "UART0", /* name */
  1, /* isatty */
  S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO, /* Character device, 0777 perms (crwxrwxrwx) */
  devnull_open_r,
  devnull_close_r,
  uart0_write_r,
  NULL, /* Not yet implemented */
  NULL, /* No seeking on UART */
  NULL, /* No fstat on UART */
};

static const devoptab_t dotab_uart1 = {
  "UART1", /* name */
  1, /* isatty */
  S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO, /* Character device, 0777 perms (crwxrwxrwx) */
  devnull_open_r,
  devnull_close_r,
  uart1_write_r,
  NULL, /* Not yet implemented */
  NULL, /* No seeking on UART */
  NULL, /* No fstat on UART */
};

static const devoptab_t dotab_uart2 = {
  "UART2", /* name */
  1, /* isatty */
  S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO, /* Character device, 0777 perms (crwxrwxrwx) */
  devnull_open_r,
  devnull_close_r,
  uart2_write_r,
  NULL, /* Not yet implemented */
  NULL, /* No seeking on UART */
  NULL, /* No fstat on UART */
};

static const devoptab_t dotab_uart3 = {
  "UART3", /* name */
  1, /* isatty */
  S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO, /* Character device, 0777 perms (crwxrwxrwx) */
  devnull_open_r,
  devnull_close_r,
  uart3_write_r,
  NULL, /* Not yet implemented */
  NULL, /* No seeking on UART */
  NULL, /* No fstat on UART */
};

static const devoptab_t dotab_uart4 = {
  "UART4", /* name */
  1, /* isatty */
  S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO, /* Character device, 0777 perms (crwxrwxrwx) */
  devnull_open_r,
  devnull_close_r,
  uart4_write_r,
  NULL, /* Not yet implemented */
  NULL, /* No seeking on UART */
  NULL, /* No fstat on UART */
};

static const devoptab_t dotab_stdin = {
  "stdin", /* name */
  1, /* isatty */
  S_IFCHR | S_IRUSR | S_IRGRP | S_IROTH, /* Character device, 0444 perms (cr--r--r--) */
  devnull_open_r,
  devnull_close_r,
  NULL,
  NULL, /* Not yet implemented */
  NULL, /* No seeking on UART */
  NULL, /* No fstat on UART */
};

static const devoptab_t dotab_stdout = {
  "stdout", /* name */
  1, /* isatty */
  S_IFCHR | S_IWUSR | S_IWGRP | S_IWOTH, /* Character device, 0222 perms (c-w--w--w-) */
  devnull_open_r,
  devnull_close_r,
  uart1_write_r,
  NULL, /* Not yet implemented */
  NULL, /* No seeking on UART */
  NULL, /* No fstat on UART */
};



/* This table maps the standard streams to device operations table entries. */
const devoptab_t *devoptab_list[MAX_OPEN_DEVICES] = { 0 };
   //~ &dotab_com1, /* standard input */
   //~ &dotab_com1, /* standard output */
   //~ &dotab_com1, /* standard error */
   //~ 0, /* another device */
   //~ 0, /* and so on... */
   //~ 0 /* terminates the list */
//~ };

static const devoptab_name_t devoptab_names[] = {
  {"UART0", &dotab_uart0},
  {"UART1", &dotab_uart1},
  {"UART2", &dotab_uart2},
  {"UART3", &dotab_uart3},
  {"UART4", &dotab_uart4},
};

const devoptab_name_list_t devoptab_name_list = {
  sizeof(devoptab_names) / sizeof(devoptab_names[0]), /* len */
  &devoptab_names[0], /* data */
};

void devicemap_init(void) {
  /* Set up stdin, stdout and stderr */
  devoptab_list[STDIN_FILENO] = &dotab_stdin;
  devoptab_list[STDOUT_FILENO] = &dotab_stdout;
  devoptab_list[STDERR_FILENO] = &dotab_stdout;
}

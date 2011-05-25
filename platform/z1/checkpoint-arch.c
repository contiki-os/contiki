/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *  Checkpoint library implementation for the Tmote Sky platform.
 * \author
 *  Fredrik Osterlind <fros@sics.se>
 */

#include "contiki.h"
#include "lib/checkpoint.h"

#include "sys/rtimer.h"
#include "sys/mt.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "dev/leds.h"
#include "dev/watchdog.h"

#ifdef __IAR_SYSTEMS_ICC__
#include <msp430.h>
#else
#include <io.h>
#include <signal.h>
#endif


#include <stdio.h>

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define COMMAND_ROLLBACK 1
#define COMMAND_CHECKPOINT 2
#define COMMAND_TBR 3

#define DATA_AS_HEX 0 /* If false, store binary data */

#define INCLUDE_RAM 1 /* Less then 10240 bytes */
#define INCLUDE_TIMERS 1 /* 16 bytes */
#define INCLUDE_LEDS 1 /* 1 bytes */

// 8kB memory 
#define RAM_START 0x1100
#define RAM_END 0x30FF

#define STOP_TIMERS() TACTL &= ~(MC1); TBCTL &= ~(MC1); watchdog_stop();
#define START_TIMERS() watchdog_start(); TACTL |= MC1; TBCTL |= MC1;

static struct mt_thread checkpoint_thread;
static uint8_t preset_cmd;
static int preset_fd;

typedef union {
  unsigned char u8[2];
  unsigned short u16;
} word_union_t;

/*---------------------------------------------------------------------------*/
static void
write_byte(int fd, uint8_t c)
{
#if DATA_AS_HEX
  uint8_t hex[2];
  sprintf(hex, "%02x", c);
  if(cfs_write(fd, hex, 2) != 2) {
    printf("err #1\n");
  }
#else /* DATA_AS_HEX */
  if(cfs_write(fd, &c, 1) != 1) {
    printf("err #2\n");
  }
#endif /* DATA_AS_HEX */
}/*---------------------------------------------------------------------------*/
#if 0
static void
write_array(int fd, unsigned char *mem, uint16_t len)
{
#if DATA_AS_HEX
  int i;
  for(i = 0; i < len; i++) {
    write_byte(fd, mem[i]);
  }
#else /* DATA_AS_HEX */
  cfs_write(fd, mem, len);
#endif /* DATA_AS_HEX */
}
#endif /* 0 */
/*---------------------------------------------------------------------------*/
static void
write_word(int fd, uint16_t w)
{
  word_union_t tmp;
  tmp.u16 = w;
  write_byte(fd, tmp.u8[0]);
  write_byte(fd, tmp.u8[1]);
}
/*---------------------------------------------------------------------------*/
static uint8_t
read_byte(int fd)
{
#if DATA_AS_HEX
  uint8_t hex[2];

  cfs_read(fd, hex, 2);

  if(hex[0] >= 'A' && hex[0] <= 'F') {
    hex[0] = (hex[0] - 'A' + 0xa);
  } else if(hex[0] >= 'a' && hex[0] <= 'f') {
    hex[0] = (hex[0] - 'a' + 0xa);
  } else {
    hex[0] = (hex[0] - '0');
  }
  if(hex[1] >= 'A' && hex[1] <= 'F') {
    hex[1] = (hex[1] - 'A' + 0xa);
  } else if(hex[1] >= 'a' && hex[1] <= 'f') {
    hex[1] = (hex[1] - 'a' + 0xa);
  } else {
    hex[1] = (hex[1] - '0');
  }
  return (uint8_t)((hex[0]<<4)&0xf0) | (hex[1]&0x0f);
#else /* DATA_AS_HEX */
  uint8_t c;
  cfs_read(fd, &c, 1);
  return c;
#endif /* DATA_AS_HEX */
}
/*---------------------------------------------------------------------------*/
static uint16_t
read_word(int fd)
{
  word_union_t tmp;
  tmp.u8[0] = read_byte(fd);
  tmp.u8[1] = read_byte(fd);
  return tmp.u16;
}
/*---------------------------------------------------------------------------*/
static void
thread_checkpoint(int fd)
{
#if INCLUDE_RAM
  unsigned char *addr;
  uint16_t size = 0;
  unsigned char *thread_mem_start = (unsigned char *)&checkpoint_thread.thread.stack;
  unsigned char *thread_mem_end = thread_mem_start + sizeof(checkpoint_thread.thread.stack) - 1;
  unsigned char *coffee_mem_start = cfs_coffee_get_protected_mem(&size);
  unsigned char *coffee_mem_end = coffee_mem_start + size - 1;
#endif /* INCLUDE_RAM */

  /*printf("protected thread memory: %u, size=%u\n", (uint16_t) thread_mem_start, sizeof(checkpoint_thread.thread.stack));*/
  /*printf("protected coffee memory: %u, size=%u\n", (uint16_t) coffee_mem_start, size);*/

  /* RAM */
#if INCLUDE_RAM
  for(addr = (unsigned char *)RAM_START;
      addr < (unsigned char *)RAM_END;
      addr++) {

    if((addr >= thread_mem_start && addr <= thread_mem_end)) {
      /* Writing dummy memory */
      /*write_byte(fd, 1);*/
      continue;
    }

    if((addr >= coffee_mem_start && addr <= coffee_mem_end)) {
      /* Writing dummy memory */
      /*write_byte(fd, 2);*/
      continue;
    }

    /* TODO Use write_array() */
    write_byte(fd, *addr);

    if(((int)addr % 512) == 0) {
      PRINTF(".");
    }
  }

#endif /* INCLUDE_RAM */

  /* Timers */
#if INCLUDE_TIMERS
  write_word(fd, TACTL);
  write_word(fd, TACCTL1);
  write_word(fd, TACCR1);
  write_word(fd, TAR);

  write_word(fd, TBCTL);
  write_word(fd, TBCCTL1);
  write_word(fd, TBCCR1);
  write_word(fd, TBR);
#endif /* INCLUDE_TIMERS */

  /* LEDs */
#if INCLUDE_LEDS
  write_byte(fd, leds_arch_get());
#endif /* INCLUDE_LEDS */

  /* Radio */
  /* ADC */
  /* ... */

  write_byte(fd, -1); /* Coffee padding byte */
}
/*---------------------------------------------------------------------------*/
static void
thread_rollback(int fd)
{
#if INCLUDE_RAM
  unsigned char *addr;
  uint16_t size = 0;
  unsigned char *thread_mem_start = (unsigned char *)&checkpoint_thread.thread.stack;
  unsigned char *thread_mem_end = thread_mem_start + sizeof(checkpoint_thread.thread.stack) - 1;
  unsigned char *coffee_mem_start = cfs_coffee_get_protected_mem(&size);
  unsigned char *coffee_mem_end = coffee_mem_start + size - 1;
#endif /* INCLUDE_RAM */

  /*printf("protected thread memory: %u, size=%u\n", (uint16_t) thread_mem_start, sizeof(checkpoint_thread.thread.stack));*/
  /*printf("protected coffee memory: %u, size=%u\n", (uint16_t) coffee_mem_start, size);*/

  /* RAM */
#if INCLUDE_RAM
  for(addr = (unsigned char *)RAM_START;
      addr < (unsigned char *)RAM_END;
      addr++) {
    if((addr >= thread_mem_start && addr <= thread_mem_end)) {
      /* Ignoring incoming memory */
      /*read_byte(fd);*/
      continue;
    }

    if((addr >= coffee_mem_start && addr <= coffee_mem_end)) {
      /* Ignoring incoming memory */
      /*read_byte(fd);*/
      continue;
    }

    *addr = read_byte(fd);

    if(((int)addr % 512) == 0) {
      PRINTF(".");
    }
  }

#endif /* INCLUDE_RAM */

  /* Timers */
#if INCLUDE_TIMERS
  TACTL = read_word(fd);
  TACCTL1 = read_word(fd);
  TACCR1 = read_word(fd);
  TAR = read_word(fd);

  TBCTL = read_word(fd);
  TBCCTL1 = read_word(fd);
  TBCCR1 = read_word(fd);
  TBR = read_word(fd);
#endif /* INCLUDE_TIMERS */

  /* LEDs */
#if INCLUDE_LEDS
  leds_arch_set(read_byte(fd));
#endif /* INCLUDE_LEDS */

  /* Radio */
  /* ADC */
  /* ... */

  read_byte(fd); /* Coffee padding byte */
}
/*---------------------------------------------------------------------------*/
static void
thread_loop(void *data)
{
  uint8_t cmd;
  int fd;

  while(1) {
    /* Store command and file descriptor on stack */
    cmd = preset_cmd;
    fd = preset_fd;

    /* Handle command */
    if(cmd == COMMAND_ROLLBACK) {
      PRINTF("Rolling back");
      thread_rollback(fd);
      PRINTF(" done!\n");
    } else if(cmd == COMMAND_CHECKPOINT) {
      PRINTF("Checkpointing");
      thread_checkpoint(fd);
      PRINTF(" done!\n");
    } else if(cmd == COMMAND_TBR) {
      PRINTF("Writing TBR");
      write_word(fd, TBR);
      PRINTF(" done!\n");
    } else {
      printf("Error: unknown command: %u\n", cmd);
    }

    /* Return to main Contiki thread */
    mt_yield();
  }
}
/*---------------------------------------------------------------------------*/
int
checkpoint_arch_size()
{
  return 10258;
}
/*---------------------------------------------------------------------------*/
void
checkpoint_arch_checkpoint(int fd)
{
  STOP_TIMERS();

  preset_cmd = COMMAND_CHECKPOINT;
  preset_fd = fd;
  mt_exec(&checkpoint_thread);

  START_TIMERS();
}
/*---------------------------------------------------------------------------*/
void
checkpoint_arch_rollback(int fd)
{
  STOP_TIMERS();

  preset_cmd = COMMAND_ROLLBACK;
  preset_fd = fd;
  mt_exec(&checkpoint_thread);

  START_TIMERS();
}
/*---------------------------------------------------------------------------*/
void
checkpoint_arch_init(void)
{
  mt_init();
  mt_start(&checkpoint_thread, thread_loop, NULL);

  /*mt_stop(&checkpoint_thread);*/
  /*mt_remove();*/
}
/*---------------------------------------------------------------------------*/

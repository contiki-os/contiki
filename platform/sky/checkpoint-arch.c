/*
 * Copyright (c) 2010, Swedish Institute of Computer Science
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
 *
 * \author
 *  Fredrik Osterlind <fros@sics.se>
 */

#include "contiki.h"

#include "lib/crc16.h"
#include "lib/checkpoint.h"

#include "sys/rtimer.h"
#include "sys/mt.h"
#include "sys/energest.h"
#include "sys/compower.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/serial-line.h"
#include "dev/uart1.h"
#include "dev/cc2420.h"
#include "dev/button-sensor.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef CHECKPOINT_ROLLBACK_BUTTON
#define CHECKPOINT_ROLLBACK_BUTTON 1 /* rollback "cp_wdt" on button click */
#endif /* CHECKPOINT_ROLLBACK_BUTTON */

#if CHECKPOINT_ROLLBACK_BUTTON
PROCESS(checkpoint_button_process, "Rollback on button");
#endif /* CHECKPOINT_ROLLBACK_BUTTON */

#define WITH_SERIAL_COMMANDS 0 /* checkpoint via serial port */
#if WITH_SERIAL_COMMANDS
#if UART1_CONF_TX_WITH_INTERRUPT
#error TX_WITH_INTERRUPTS must be 0
#endif /* UART1_CONF_TX_WITH_INTERRUPT */
#define PRINTF_COMMAND(...) printf(__VA_ARGS__)
#else /* WITH_SERIAL_COMMANDS */
#define PRINTF_COMMAND(...)
#endif /* WITH_SERIAL_COMMANDS */

#define COMMAND_ROLLBACK 1
#define COMMAND_CHECKPOINT 2
#define COMMAND_METRICS 3

#define INCLUDE_RAM 1 /* Less then 10240 bytes */
#define INCLUDE_TIMERS 1 /* 16 bytes */
#define INCLUDE_LEDS 1 /* 1 bytes */
/* ... */

/* 10kb memory */
#define RAM_START 0x1100
#define RAM_END 0x3900

#define PAUSE_TIME() \
  TACTL &= ~(MC1); \
  TBCTL &= ~(MC1); \
  watchdog_stop();
#define RESUME_TIME() \
  TACTL |= MC1; \
  TBCTL |= MC1; \
  TACCR1 = clock_fine_max(); \
  watchdog_start();
#define PAUSE_TIME_INT() \
  dint(); \
  PAUSE_TIME();
#define RESUME_TIME_INT() \
  RESUME_TIME(); \
  eint();

static struct mt_thread checkpoint_thread;
static uint8_t preset_cmd;
static int preset_fd;

/* bookkeeping */
#if WITH_SERIAL_COMMANDS
static int nr_pongs=0;
#endif /* WITH_SERIAL_COMMANDS */
static int nr_checkpoints=0, nr_rollbacks=0, nr_metrics=0;

/*---------------------------------------------------------------------------*/
typedef union {
  unsigned char u8[2];
  unsigned short u16;
} word_union_t;
/*---------------------------------------------------------------------------*/
static int
write_byte(int fd, uint8_t c)
{
  return cfs_write(fd, &c, 1);
}
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
  uint8_t c;
  cfs_read(fd, &c, 1);
  return c;
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

  /*PRINTF("protected thread memory: %u, size=%u\n", (uint16_t) thread_mem_start, sizeof(checkpoint_thread.thread.stack));*/
  /*PRINTF("protected coffee memory: %u, size=%u\n", (uint16_t) coffee_mem_start, size);*/

  /* RAM */
#if INCLUDE_RAM
  for(addr = (unsigned char *)RAM_START;
  addr < (unsigned char *)RAM_END;
  addr++) {

    if((addr >= thread_mem_start && addr <= thread_mem_end)) {
      /* Skip */
      continue;
    }

    if((addr >= coffee_mem_start && addr <= coffee_mem_end)) {
      /* Skip */
      continue;
    }

    /* TODO Use write_array() */
    write_byte(fd, *addr);

    /*if(((int)addr % 512) == 0) {
      PRINTF(".");
    }*/
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

  /*PRINTF("protected thread memory: %u, size=%u\n", (uint16_t) thread_mem_start, sizeof(checkpoint_thread.thread.stack));*/
  /*PRINTF("protected coffee memory: %u, size=%u\n", (uint16_t) coffee_mem_start, size);*/

  /* RAM */
#if INCLUDE_RAM
  for(addr = (unsigned char *)RAM_START;
  addr < (unsigned char *)RAM_END;
  addr++) {
    if((addr >= thread_mem_start && addr <= thread_mem_end)) {
      /* Skip */
      continue;
    }

    if((addr >= coffee_mem_start && addr <= coffee_mem_end)) {
      /* Skip */
      continue;
    }

    *addr = read_byte(fd);
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
#if WITH_SERIAL_COMMANDS
static uint32_t
thread_metric_tx(void)
{
  energest_flush();
  return energest_type_time(ENERGEST_TYPE_TRANSMIT);
}
/*---------------------------------------------------------------------------*/
static uint32_t
thread_metric_rx(void)
{
  energest_flush();
  return energest_type_time(ENERGEST_TYPE_LISTEN);
}
#endif /* WITH_SERIAL_COMMANDS */
/*---------------------------------------------------------------------------*/
static void
thread_metrics(void)
{
  PRINTF_COMMAND("METRICS:START\n");
  PRINTF_COMMAND("M:RTIMER_NOW:%u\n", RTIMER_NOW()); /* TODO extract */
  PRINTF_COMMAND("M:ENERGY_TX:%lu\n", thread_metric_tx());
  PRINTF_COMMAND("M:ENERGY_RX:%lu\n", thread_metric_rx());
  PRINTF_COMMAND("M:RTIMER_NOW2:%u\n", RTIMER_NOW());
  nr_metrics++;
  PRINTF_COMMAND("METRICS:DONE %u\n", nr_metrics);
}
/*---------------------------------------------------------------------------*/
static void
checkpoint_thread_loop(void *data)
{
  uint8_t cmd;
  int fd;

  while(1) {
    /* Store command and file descriptor on stack */
    cmd = preset_cmd;
    fd = preset_fd;

    /* Handle command */
    if(cmd == COMMAND_ROLLBACK) {
      PRINTF_COMMAND("RB:START\n");
      thread_rollback(fd);
      nr_rollbacks++;
      PRINTF_COMMAND("RB:DONE %u\n", nr_rollbacks);
      /* TODO Synch before leaving this thread. */
    } else if(cmd == COMMAND_CHECKPOINT) {
      PRINTF_COMMAND("CP:START\n");
      thread_checkpoint(fd);
      thread_metrics();
      nr_checkpoints++;
      PRINTF_COMMAND("CP:DONE %u\n", nr_checkpoints);
    } else if(cmd == COMMAND_METRICS) {
      thread_metrics();
    } else {
      printf("ERROR: Unknown thread command: %u\n", cmd);
    }

    /* Return to Contiki */
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
  PAUSE_TIME_INT();

  preset_cmd = COMMAND_CHECKPOINT;
  preset_fd = fd;
  mt_exec(&checkpoint_thread);

  RESUME_TIME_INT();
}
/*---------------------------------------------------------------------------*/
void
checkpoint_arch_rollback(int fd)
{
  PAUSE_TIME_INT();

  preset_cmd = COMMAND_ROLLBACK;
  preset_fd = fd;
  mt_exec(&checkpoint_thread);

  RESUME_TIME_INT();
}
/*---------------------------------------------------------------------------*/
static uint8_t inited = 0;
void
checkpoint_arch_init(void)
{
  if(inited) {
    return;
  }

  mt_init();
  mt_start(&checkpoint_thread, checkpoint_thread_loop, NULL);
  inited = 1;

#if CHECKPOINT_ROLLBACK_BUTTON
  process_start(&checkpoint_button_process, NULL);
#endif /* CHECKPOINT_ROLLBACK_BUTTON */

  /*mt_stop(&checkpoint_thread);*/
  /*mt_remove();*/
}
/*---------------------------------------------------------------------------*/
struct power_log {
  uint32_t transmit;
  uint32_t listen;
};
/*---------------------------------------------------------------------------*/
#if WITH_SERIAL_COMMANDS
static void
serial_interrupt_checkpoint()
{
  int fd = 0;
  PAUSE_TIME();

  if(SPI_IS_ENABLED()) {
    /* SPI is busy, abort */
    PRINTF_COMMAND("CP:SPIBUSY\n");
    RESUME_TIME();
    return;
  }

  /* Open file */
  cfs_remove("cp");
  cfs_coffee_reserve("cp", checkpoint_arch_size());
  fd = cfs_open("cp", CFS_WRITE);

  if(fd < 0) {
    printf("ERROR: No file access (cp)\n");
    RESUME_TIME();
    return;
  }

  /* Checkpoint */
  preset_cmd = COMMAND_CHECKPOINT;
  preset_fd = fd;
  mt_exec(&checkpoint_thread);

  /* Close file */
  cfs_close(fd);

  RESUME_TIME();
}
/*---------------------------------------------------------------------------*/
static void
serial_interrupt_rollback()
{
  int fd = 0;
  PAUSE_TIME();

  if(SPI_IS_ENABLED()) {
    /* SPI is busy, abort */
    PRINTF_COMMAND("RB:SPIBUSY\n");
    RESUME_TIME();
    return;
  }

  /* Open file */
  fd = cfs_open("cp", CFS_READ);

  if(fd < 0) {
    printf("ERROR: No file access (rb)\n");
    RESUME_TIME();
    return;
  }

  /* Rollback */
  preset_cmd = COMMAND_ROLLBACK;
  preset_fd = fd;
  mt_exec(&checkpoint_thread);

  /* Close file */
  cfs_close(fd);

  RESUME_TIME();
}
/*---------------------------------------------------------------------------*/
static void
serial_interrupt_metrics()
{
  PAUSE_TIME();

  preset_cmd = COMMAND_METRICS;
  preset_fd = -1;
  mt_exec(&checkpoint_thread);

  RESUME_TIME();
}
/*---------------------------------------------------------------------------*/
static const unsigned char command_checkpoint[] = { 'c', 'p', '\n' };
static const unsigned char command_rollback[] = { 'r', 'b', '\n' };
static const unsigned char command_metrics[] = { 'm', 't', '\n' };
static volatile int command_checkpoint_state = 0;
static volatile int command_rollback_state = 0;
static volatile int command_metrics_state = 0;
/*---------------------------------------------------------------------------*/
static int
serial_input_byte_intercept(unsigned char c)
{
  /* Detect checkpoint request */
  if(command_checkpoint[command_checkpoint_state] == c) {
    command_checkpoint_state++;

    if(command_checkpoint_state == sizeof(command_checkpoint)) {
      serial_interrupt_checkpoint();
      command_checkpoint_state = 0;
    }
  } else {
    command_checkpoint_state = 0;
  }

  /* Detect rollback request */
  if(command_rollback[command_rollback_state] == c) {
    command_rollback_state++;

    if(command_rollback_state == sizeof(command_rollback)) {
      serial_interrupt_rollback();
      command_rollback_state = 0;
    }
  } else {
    command_rollback_state = 0;
  }

  /* Detect metrics request */
  if(command_metrics[command_metrics_state] == c) {
    command_metrics_state++;

    if(command_metrics_state == sizeof(command_metrics)) {
      serial_interrupt_metrics();
      command_metrics_state = 0;
    }
  } else {
    command_metrics_state = 0;
  }

  /* Forward to serial line input byte */
  return serial_line_input_byte(c);
}
/*---------------------------------------------------------------------------*/
static void
handle_get_command(void)
{
  int fd = 0;
  fd = cfs_open("cp", CFS_READ);
  if(fd < 0) {
    printf("ERROR: No file access (get)\n");
  } else {
    PRINTF_COMMAND("GET:START\n");
    char data[8];
    int offset=0, size=0, read=8;
    unsigned short crc = 0;
    cfs_seek(fd, offset, CFS_SEEK_SET);

    while (read == 8) {
      int i;

      /*if(offset != cfs_seek(fd, offset, CFS_SEEK_SET)) {
        printf("bad seek, breaking\n");
        break;
      }*/
      read = cfs_read(fd, data, 8);
      size += read;

      printf("%04i: ", offset); /*REMOVE*/
      for (i=0; i < read; i++) {
        crc = crc16_add((uint8_t) data[i], crc);
        printf("%02x", (uint8_t) (0xff&data[i]));
      }
      printf("\n");

      offset += 8;
    }

    PRINTF_COMMAND("GET:DONE CRC=%u\n", crc);
    cfs_close(fd);
  }
}
/*---------------------------------------------------------------------------*/
static int
hex_decode_char(char c)
{
  if(c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if(c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if(c >= '0' && c <= '9') {
    return c - '0';
  } else {
    printf("WARN: bad hex: %c\n", c);
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(checkpoint_serial_process, "Checkpoint via serial commands");
PROCESS_THREAD(checkpoint_serial_process, ev, data)
{
  static int set_fd = -1;
  static int set_count = -1;

  PROCESS_BEGIN();

  /* Note: 'cp', 'rb', and 'mt' commands are intercepted */
  PROCESS_PAUSE();
  uart1_set_input(serial_input_byte_intercept);

  /* Format Coffee? */
  PRINTF("Formatting Coffee\n");
  cfs_coffee_format();
  PRINTF("Formatting Coffee... done!\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && data != NULL);

    if(strcmp("set", data) == 0) {
      /* TODO Handle set command */
      /* Open file */
      cfs_remove("cp");
      cfs_coffee_reserve("cp", checkpoint_arch_size());
      set_fd = cfs_open("cp", CFS_WRITE);
      set_count = 0;
      if(set_fd < 0) {
        printf("SET:FSBUSY\n");
      } else {
        printf("SET:LINE\n");
      }
    } else if(set_fd >= 0 && strcmp("set:done", data) == 0) {
        cfs_close(set_fd);
        set_fd = -1;
        if(set_count == 9862) {
          printf("SET:DONE\n");
        } else {
          printf("SET:WRONGSIZE\n");
        }
    } else if(set_fd >= 0) {
      /* We are ready for another line */
      printf("SET:LINE\n");
      /* Set command: parse hex data */
      int len = strlen((char*)data);
      if(len > 16 || (len%2)!=0) {
        printf("WARN: bad set data: %s\n", (char*)data);
      } else {
        int i;
        for (i=0; i < len; i+=2) {
          uint8_t b =
            (hex_decode_char(((char*)data)[i]) << 4) +
            (hex_decode_char(((char*)data)[i+1]));

          PRINTF("Parsing set command: writing to CFS: %02x\n", b);
          write_byte(set_fd, b); /* TODO Check return value */
          set_count++;
        }
      }
    } else if(strcmp("", data) == 0 ||
        strcmp("cp", data) == 0 ||
        strcmp("rb", data) == 0 ||
        strcmp("mt", data) == 0) {
      /* ignore commands: handled by interrupt */
    } else if(strcmp("ping", data) == 0) {
      nr_pongs++;
      printf("pong %u\n", nr_pongs);
    } else if(strcmp("get", data) == 0) {
      handle_get_command();
    } else {
      printf("WARN: Unknown command: '%s'\n", (char*)data);
    }
  }

  PROCESS_END();
}
#endif /* WITH_SERIAL_COMMANDS */
/*---------------------------------------------------------------------------*/
#if CHECKPOINT_ROLLBACK_BUTTON
PROCESS_THREAD(checkpoint_button_process, ev, data)
{
  PROCESS_BEGIN();

  button_sensor.configure(SENSORS_ACTIVE, 1);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == sensors_event && data == &button_sensor) {
      int fd = 0;

      /* Rollback from Coffee file "cp_wdt" */
      fd = cfs_open("cp_wdt", CFS_READ);
      if(fd >= 0) {
        checkpoint_rollback(fd);
        cfs_close(fd);
      }
    }
  }

  PROCESS_END();
}
#endif /* CHECKPOINT_ROLLBACK_BUTTON */

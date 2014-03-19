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

#ifndef SPIDRV_H_
#define SPIDRV_H_

#include <string.h>

#include "pt-sem.h"

void spi_init();

/* Return is 1 meaning work needed to be done, else 0 */
#define NOWORK 0
#define WORKPEND 1
typedef int (*CmdSetup)(void *state);
typedef int (*CmdCallback)(void *state);

typedef struct DMACmdS {
  CmdSetup setupCmd;
  CmdCallback completeCB;
  void *state;            /* To be cast to the correct type in the action commands */

  struct DMACmdS *next;
} DMACmd;

extern volatile unsigned char *crntDMABuf;
extern volatile unsigned char *altDMABuf;

extern volatile unsigned char spiCmdPend;

extern struct pt_sem spiBusSem;

extern struct process *spiOwningProcess;

/* Hi = 10Mbit/s, Lo = 400Kbit/s (only used for SD card init sequence) */
#define SPI_HISPD 0
#define SPI_LOSPD 1
#define SPI_NOCONTCLK 0
#define SPI_CONTCLK 1
void setup_spi(int lowSpd, int continuousClock);

void send_spi_cmd(uint16_t count);

void push_spi_cmd(CmdSetup sc, CmdCallback ec, void *st);
void raw_push_spi_cmd(CmdSetup sc, CmdCallback ec, void *st); /* No interrupt protection - for call from ISR */
void invoke_spi_cmd();

void init_dma_mem();

void set_alt_dma_buf(volatile uint8_t *addr);

#define NUM_DMA_CMD_BUFS 4
#define DMA_CMD_BUF_SZ 32
#define NUM_DMA_PKT_BUFS 4
#define DMA_PKT_BUF_SZ 256

struct DMABufDescr {
  volatile uint8_t *buf;
  struct DMABufDescr *next;
  uint16_t inUse;
};
struct DMABufDescr *allocDMAPktBuf();
struct DMABufDescr *allocDMAPktBuf_unsafe();
void freeDMABuf_unsafe(struct DMABufDescr *d);

/* Return the total length of the chain */
static inline int
getBufDescrLen(const struct DMABufDescr *d)
{
  int l = 0;
  while(d != 0) {
    l += d->inUse;
    d = d->next;
  }
  return l;
}
/* Copy the chain into a single uint8_t array, upto the max len */
static inline int
copyPktBuf(uint8_t *dst, int maxLen, const struct DMABufDescr *d)
{
  uint8_t *st = dst;
  while(maxLen > 0 && d != 0) {
    memcpy(dst, (const void *)d->buf, d->inUse);
    maxLen -= d->inUse;
    dst += d->inUse;
    d = d->next;
  }
  return dst - st;
}
#endif /* SPIDRV_H_ */

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

/* #define DEBUG_DMABUF 1 */
/* #define DEBUG 1 */

#ifdef DEBUG
#define DBGPRINTF(...) printf(__VA_ARGS__)
#else
#define DBGPRINTF(...)
#endif

#include <stdio.h>
#include <p33Fxxxx.h>
#include <../peripheral_30F_24H_33F/spi.h>
#include <string.h>

#include "contiki.h"
#include "spidrv.h"
#include "intsafe.h"

static DMACmd cmdQueue[16];
static DMACmd *freeCmd;
static DMACmd *todoCmd;

volatile uint8_t genDMABuf[32] __attribute__((space(dma)));
/* volatile unsigned char genDMABuf2[16] __attribute__((space(dma))); */

volatile uint8_t dummyTXDMA[1] __attribute__((space(dma)));
volatile uint8_t dummyRXDMA[1] __attribute__((space(dma)));

volatile uint8_t dmaPktBuffer[DMA_PKT_BUF_SZ * NUM_DMA_PKT_BUFS] __attribute__((space(dma)));
volatile uint8_t dmaCmdBuffer[DMA_CMD_BUF_SZ * NUM_DMA_CMD_BUFS] __attribute__((space(dma)));

struct DMABufDescr dmaPktBufDescr[NUM_DMA_PKT_BUFS];
struct DMABufDescr dmaCmdBufDescr[NUM_DMA_CMD_BUFS];
static struct DMABufDescr *dmaPktBufDescrFree;
static struct DMABufDescr *dmaCmdBufDescrFree;

volatile uint8_t *crntDMABuf;
/* volatile unsigned char* altDMABuf; */

volatile unsigned char spiCmdPend; /* Command queued but not completed */

static unsigned int crntDMAOff;
/* static unsigned int altDMAOff; */

static unsigned char continuousClock;
static unsigned char isIdleing;

struct pt_sem spiBusSem;
struct process *spiOwningProcess;

void init_dma_buf();
void init_cmd_queue();

void
spi_init()
{
  PT_SEM_INIT(&spiBusSem, 1); /* Allow only one device driver access at a time */
  init_dma_buf();
  init_cmd_queue();
  setup_spi(SPI_HISPD, SPI_NOCONTCLK);
}
void
setup_spi(int lowSpd, int cc_)
{
  continuousClock = cc_; /* If necessary transmit 0xff continually if nothing to transmit */
  isIdleing = 0;
  /* Max CLK = 14MHz - imposed by ENC424J600 */
  /* Map RP21 = MCLK */
  /* Map RP5 = MOSI */
  /* Map RP6 = MISO */
  /* Map RP19/ RC3 = Eth CS (default) */
  SPI1STATbits.SPIEN = 0;
  /*SPI1CON1bits.DISSCK = 0;
        SPI1CON1bits.DISSDO = 0;
        SPI1CON1bits.MODE16 = 0;
        SPI1CON1bits.SMP = 1;
        SPI1CON1bits.CKE = 1;
        SPI1CON1bits.SSEN = 0;
        SPI1CON1bits.CKP = 0;*/
  if(lowSpd == SPI_LOSPD) {
    DBGPRINTF("Selecting low speed\n");
    /* 40 MHz / (7 * 16) = 357Kbit/s for SDcard init */
    SPI1CON1 = ENABLE_SCK_PIN & ENABLE_SDO_PIN & SPI_MODE16_OFF & SPI_SMP_ON
      & SPI_CKE_ON & SLAVE_ENABLE_OFF & CLK_POL_ACTIVE_HIGH & MASTER_ENABLE_ON
      & SEC_PRESCAL_7_1 & PRI_PRESCAL_16_1;
    /* 40 MHz / (7 * 16) = 357Kbit/s for SDcard init */
  } else {
    DBGPRINTF("Selecting hi speed\n");
    /* 40 MHz / (4 * 1) = 10Mbit/s for general use */
    SPI1CON1 = ENABLE_SCK_PIN & ENABLE_SDO_PIN & SPI_MODE16_OFF & SPI_SMP_ON
      & SPI_CKE_ON & SLAVE_ENABLE_OFF & CLK_POL_ACTIVE_HIGH & MASTER_ENABLE_ON
      & SEC_PRESCAL_1_1 & PRI_PRESCAL_4_1;

    /* SPI1CON1bits.SPRE = 0b111; // 1 */
    /* SPI1CON1bits.PPRE = 0b10; // 4 - giving 40MHz / 4 = 10Mbit/s */
  }
  RPOR10bits.RP21R = 0x08; /* RP21 = SCK1OUT */
  RPOR2bits.RP5R = 0x07; /* RP5 = SDO1 */
  RPINR20bits.SDI1R = 0x06; /* RP6 = SDI1 */
  RPOR9bits.RP19R = 0x00; /* RP19 = default (RC3) */

  SPI1STATbits.SPISIDL = 0;
  /* SPI1CON1bits.MSTEN = 1; */
  SPI1STATbits.SPIEN = 1;
}
void
init_dma_free_buffers(struct DMABufDescr **head, struct DMABufDescr *bufArr, volatile uint8_t *array, int cnt, size_t sz)
{
  *head = bufArr; /* Setting up head */

  struct DMABufDescr *d = *head;
  volatile uint8_t *t = array;
  volatile uint8_t *te = array + cnt * sz;
  while(t < te) {
    d->buf = t;
    d->inUse = 0;
    t += sz;
    d->next = (t < te) ? d + 1 : 0;
    d = d->next;
  }

#ifdef DEBUG_DMABUF
  d = *head;
  while(d != 0) {
    DBGPRINTF("addr: %04x, buf: %04x, next: %04x\n",
              (uint16_t)d, (uint16_t)d->buf, (uint16_t)d->next);
    d = d->next;
  }
#endif
}
void
init_dma_buf()
{
  set_alt_dma_buf(genDMABuf);

  init_dma_free_buffers(&dmaPktBufDescrFree, dmaPktBufDescr, dmaPktBuffer, NUM_DMA_PKT_BUFS, DMA_PKT_BUF_SZ);
  init_dma_free_buffers(&dmaCmdBufDescrFree, dmaCmdBufDescr, dmaCmdBuffer, NUM_DMA_CMD_BUFS, DMA_CMD_BUF_SZ);
}
struct DMABufDescr *
allocDMACmdBuf_unsafe()
{
  struct DMABufDescr *d = dmaCmdBufDescrFree;
  if(d != 0) {
    dmaPktBufDescrFree = d->next;
    d->next = 0; /* Now out of the chain */
    memset((void *)d->buf, 0, DMA_PKT_BUF_SZ);
#ifdef DEBUG_DMABUF
    DBGPRINTF("alloc dmabuf: %04x, buf: %04x\n", (uint16_t)d, (uint16_t)d->buf);
#endif
  }
  return d;
}
struct DMABufDescr *
allocDMAPktBuf_unsafe()
{
  struct DMABufDescr *d = dmaPktBufDescrFree;
  if(d != 0) {
    dmaPktBufDescrFree = d->next;
    d->next = 0; /* Now out of the chain */
    memset((void *)d->buf, 0, DMA_PKT_BUF_SZ);
#ifdef DEBUG_DMABUF
    DBGPRINTF("alloc dmabuf: %04x, buf: %04x\n", (uint16_t)d, (uint16_t)d->buf);
#endif
  }
  return d;
}
struct DMABufDescr *
allocDMAPktBuf()
{
  uint16_t osr = disable_int();
  struct DMABufDescr *d = allocDMAPktBuf_unsafe();
  enable_int(osr);
  return d;
}
void
freeDMABuf_unsafe(struct DMABufDescr *d)
{
#ifdef DEBUG_DMABUF
  DBGPRINTF("free  dmabuf: %04x, buf: %04x\n", (uint16_t)d, (uint16_t)d->buf);
#endif
  d->next = dmaPktBufDescrFree;
  dmaPktBufDescrFree = d;
}
void
set_alt_dma_buf(volatile uint8_t *addr)
{
  if(!addr) {  /* Going back to standard */
    crntDMABuf = genDMABuf;
    crntDMAOff = (unsigned int)crntDMABuf - (unsigned int)&_DMA_BASE;
  } else {
    crntDMABuf = addr;
    crntDMAOff = (unsigned int)crntDMABuf - (unsigned int)&_DMA_BASE;
  }
}
void
init_cmd_queue()
{
  spiCmdPend = 0;
  continuousClock = 0;

  DMACmd *t;
  uint16_t osr = disable_int();
  for(t = cmdQueue; t < cmdQueue + 15; ++t) {
    t->next = t + 1;
  }
  t->next = 0;
  todoCmd = 0; /* Queue is empty */
  freeCmd = cmdQueue; /* Everything is available */

  enable_int(osr);
}
void
invoke_spi_cmd()
{
  /* If idleing is happening the interrupt will kick off the transfer */
  uint16_t osr = disable_int();
  if(todoCmd) {
    spiCmdPend = 1;
    if(!isIdleing) {
      (*todoCmd->setupCmd)(todoCmd->state);
    }
  }
  enable_int(osr);
}
void
raw_push_spi_cmd(CmdSetup sc, CmdCallback ec, void *st)
{
  if(freeCmd) {
    DMACmd *t = freeCmd;
    freeCmd = freeCmd->next;

    t->setupCmd = sc;
    t->completeCB = ec;
    t->state = st;
    t->next = 0;

    if(todoCmd) {
      DMACmd *s = todoCmd;
      while(s->next) s = s->next;
      s->next = t;
    } else {
      todoCmd = t;
    }
  }
}
void
push_spi_cmd(CmdSetup sc, CmdCallback ec, void *st)
{
  uint16_t osr = disable_int();
  raw_push_spi_cmd(sc, ec, st);
  enable_int(osr);
}
void
popCmd()
{
  uint16_t osr = disable_int();
  if(todoCmd) {
    DMACmd *t = todoCmd;
    todoCmd = todoCmd->next;
    t->next = freeCmd;
    freeCmd = t;
  }
  enable_int(osr);
}
/*
 * Newer packaged command series
 */
struct DMABufDescr *
startCmdFrag(void (*cb)())
{
  return allocDMACmdBuf_unsafe();
}
void
invokeFrag()
{
}
/*
 *      Send a command to the target. When the command is complete spiCmdPend = 0.
 *      There is no protection for overlapping commands, so don't do that!
 */

void __attribute__((__interrupt__, no_auto_psv))
_DMA1Interrupt()
{
  IFS0bits.DMA1IF = 0; /* Acknowledge the interrupt */
  uint8_t cmdBusy = NOWORK;

  if(todoCmd) {
    if(!isIdleing) {
      /* The interrupt occurred because a command completed */
      if(todoCmd->completeCB) {
        cmdBusy = todoCmd->completeCB(todoCmd->state);
      }
    } else {
      isIdleing = 0; /* We're starting a real command sequence */
    }
    if(cmdBusy == NOWORK) {
      popCmd();

      while(todoCmd && todoCmd->setupCmd) {
        if(todoCmd->setupCmd(todoCmd->state) == WORKPEND) {
          break;
        }
        popCmd();
      }
      if(!todoCmd) {
        spiCmdPend = 0;
      }
    }
  } else {
    spiCmdPend = 0;
  }
  if(!spiCmdPend && continuousClock) {
    isIdleing = 1;
    *dummyTXDMA = 0xff;
    /* Need to setup for another transfer */
    DMA0CON = 0x6011;
    DMA0CNT = 64;
    DMA0REQ = 0x0a;
    DMA0PAD = (unsigned int)&SPI1BUF;
    DMA0STA = __builtin_dmaoffset(dummyTXDMA);
    DMA0CONbits.CHEN = 1;

    DMA1CON = 0x4011;
    DMA1CNT = 64;
    DMA1REQ = 0x0a;
    DMA1PAD = (unsigned int)&SPI1BUF;
    DMA1STA = __builtin_dmaoffset(dummyRXDMA);
    DMA1CONbits.CHEN = 1;

    IFS0bits.DMA1IF = 0;
    IEC0bits.DMA1IE = 1;

    DMA0REQbits.FORCE = 1; /* Kick the transfer off */
  }
  if(!spiCmdPend && spiOwningProcess != 0) {
    process_poll(spiOwningProcess);
  }
}
void
send_spi_cmd(uint16_t count)
{
  --count; /* 0 means transfer 1 byte */

  DMA0CON = 0x6001;
  DMA0CNT = count;
  DMA0REQ = 0x0a;
  DMA0PAD = (uint16_t)&SPI1BUF;
  DMA0STA = crntDMAOff;
  DMA0CONbits.CHEN = 1;

  DMA1CON = 0x4001;
  DMA1CNT = count;
  DMA1REQ = 0x0a;
  DMA1PAD = (uint16_t)&SPI1BUF;
  DMA1STA = crntDMAOff;
  DMA1CONbits.CHEN = 1;

  IFS0bits.DMA1IF = 0;
  IEC0bits.DMA1IE = 1;

  DMA0REQbits.FORCE = 1; /* Kick the transfer off */
}

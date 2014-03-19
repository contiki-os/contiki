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

#include <stdio.h>
#include <string.h>
#include <p33Fxxxx.h>

#include "contiki.h"
#include "contiki-net.h"
#include "enc424mac.h"
#include "spidrv.h"
#include "net/ip/uip.h"
#include "freebuf.h"
#include "intsafe.h"
#include "net/ip/uip-debug.h"

/* #define DEBUG 1 */
/* #define DEBUG_PKTBUF 1 */

#ifdef DEBUG
#define DBGPRINTF(...) printf(__VA_ARGS__)
#else
#define DBGPRINTF(...)
#endif

#define ETXST 0x00
#define ETXLEN 0x02
#define ERXST 0x04
#define ERXTAIL 0x06
#define ERXHEAD 0x08
#define ETXSTAT 0x12
#define ETXWIRE 0x14
#define EUDAST 0x16
#define ESTAT 0x1a
#define EIR 0x1c
#define ECON1 0x1e
#define ERXFCON 0x34
#define MACON2 0x42
#define MABBIPG 0x44
#define MICMD 0x52
#define MIREGADR 0x54
#define MAADR3 0x60
#define MAADR2 0x62
#define MAADR1 0x64
#define MIWR 0x66
#define MIRD 0x68
#define MISTAT 0x6a
#define ECON2 0x6e
#define EIE 0x72
#define EGPDATA 0x80
#define ERXDATA 0x82
#define EGPRDPT 0x86
#define EGPWRPT 0x88
#define ERXRDPT 0x8a
#define ERXWRPT 0x8c
#define EUDARDPT 0x8e
#define EUDAWRPT 0x90

/* EIR */
#define PKTIF 0x0040
#define LINKIF 0x0800

/* EIE */
#define TXABIF 0x0004
#define TXIF 0x0008
#define PKTIE 0x0040
#define LINKIE 0x0800
#define INTIE 0x8000

/* ECON1 */
#define RXEN 0x0001
#define TXRTS 0x0002
#define PKTDEC 0x0100

#define ETHRST 0x10
#define MIIRD 0x0001

/* ESTAT */
#define CLKRDY 0x1000
#define PHYLNK 0x0100
#define PHYDPX  0x0400

/* MACON2 */
#define FULDPX 0x0001

/* MABBIPG */
#define MABBIPG_FD_DFLT 0x15
#define MABBIPG_HD_DFLT 0x12

#define CRCEN   0x0040
#define RUNTEN  0x0010
#define UCEN    0x0008

#define PHCON1 0x0000
#define PHSTAT1 0x0001

#define ENC424_RCRU 0x20
#define ENC424_WCRU 0x22
#define ENC424_BFSU 0x24
#define ENC424_BFCU 0x26
#define ENC424_RGPDATA 0x28
#define ENC424_WGPDATA 0x2a
#define ENC424_RRXDATA 0x2c
#define ENC424_WRXDATA 0x2e

/* Buffer setup for general purpose (used for TX) and rx buffers */
#define MAC_RXBUF_OFFSET 0x3000
#define MAC_RXBUF_END 0x6000
#define MAC_TXBUF_OFFSET 0x0000
#define MAC_TXBUF_END 0x3000

/* Outgoing SPI cmd + 2 next pkt ptr + 6 rx descriptor */
#define RXDESCR_HDRLEN 9
/* DUMMY is because the first byte is actually the outgoing SPI command */
#define RXDESCR_DUMMY 0x00
#define RXDESCR_NXTPKTL 0x01
#define RXDESCR_NXTPKTH 0x02
#define RXDESCR_LENL 0x03
#define RXDESCR_LENH 0x04
#define ETHER_MINUSERDAT 46
#define ETHER_HDRLEN 14
#define ETHER_CRCLEN 4

/* Standard defines used to access ethernet and IP header */
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#define ETHCMDNUMELEM 10
static struct EtherCmdState ethCmdState[ETHCMDNUMELEM];
void *ethCmdStateFree;

#define GET_CMD_STATE() ((struct EtherCmdState *)freebuf_pop(&ethCmdStateFree))
#define FREE_CMD_STATE(x) freebuf_push(&ethCmdStateFree, x)

#define ETHCS(x) LATCbits.LATC3 = x

void bzero(void *a, size_t cnt);

static struct etimer timer;

static volatile uint16_t statVal;
static volatile union EtherDevDrvState devstat;

static volatile uint16_t ethIntrStat; /* Where to store the ethernet status */
static volatile uint16_t ethIntrEIR;
static volatile uint16_t etxStat;
static volatile uint16_t etxWire;

static volatile uint16_t rRXDataPtr;
static volatile uint16_t rRXDataPtr_onchip; /* Where we read out the chip address to */
static volatile uint16_t wTXDataPtr;

process_event_t EVENT_ETH_SEND;
process_event_t EVENT_ETH_STARTTCP;

void
dumpPacket(const struct DMABufDescr *descr)
{
#ifdef DEBUG
  DBGPRINTF("Packet:\n");
  int offset = 0;
  while(descr != 0) {
    const volatile uint8_t *t = descr->buf;
    int len = descr->inUse;

    while(len > 0) {
      if(offset % 16 == 0) {
        DBGPRINTF("%04x  ", offset);
      }
      DBGPRINTF("%02x ", *t++);
      ++offset;
      if(offset % 16 == 0) {
        DBGPRINTF("\n");
      }
      --len;
    }
    descr = descr->next;
  }
#endif
}
#ifdef DEBUG_PKTBUF
#define dumpPktBuffer(...) dumpPktBuffer_impl(__VA_ARGS__)

void
dumpPktBuffer_impl()
{
#ifdef DEBUG_PKTBUF
  printf("TCPIP Packet Buffer:\n");
  int offset = 0;
  uint8_t *x = uip_buf;

  while(offset < uip_len) {
    if(offset % 16 == 0) {
      printf("%04x  ", offset);
    }
    printf("%02x ", *x++);
    ++offset;
    if(offset % 16 == 0) {
      printf("\n");
    }
  }
#endif
}
#else
#define dumpPktBuffer(...)
#endif

static void
doBFSU(unsigned char addr, unsigned int mask)
{
  crntDMABuf[0] = 0x24; /* BFSU */
  crntDMABuf[1] = addr;
  crntDMABuf[2] = mask & 0xff;
  crntDMABuf[3] = (mask >> 8) & 0xff;
  send_spi_cmd(4);
}
static void
doBFCU(unsigned char addr, unsigned int mask)
{
  crntDMABuf[0] = 0x26; /* BFCU */
  crntDMABuf[1] = addr;
  crntDMABuf[2] = mask & 0xff;
  crntDMABuf[3] = (mask >> 8) & 0xff;
  send_spi_cmd(4);
}
void
doEthMemRead(unsigned char addr, unsigned char count)
{
  crntDMABuf[0] = 0x20;
  crntDMABuf[1] = addr;
  send_spi_cmd(count + 2);
}
void
doEthMemSnglWrite(unsigned char addr, unsigned int val)
{
  crntDMABuf[0] = 0x22;
  crntDMABuf[1] = addr;
  crntDMABuf[2] = val & 0xff;
  crntDMABuf[3] = (val >> 8) & 0xff;
  send_spi_cmd(4);
}
void
doEthMemWrite(unsigned char addr, unsigned char count)
{
  crntDMABuf[0] = 0x22;
  crntDMABuf[1] = addr;
  send_spi_cmd(count + 2);
}
void
doEthRXMemRead(unsigned int count)
{
  crntDMABuf[0] = 0x2c; /* RRXDATA */
  send_spi_cmd(count + 1);
}
int
deassertCS_CB(void *p)
{
  struct EtherCmdState *data = (struct EtherCmdState *)p;
  ETHCS(1);

  FREE_CMD_STATE(data); /* Put it back on the free list */
  return NOWORK; /* No more work to do */
}
int
writeEthRegPtrSetup(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  crntDMABuf[2] = *phase->cs.wrPtrRegStat.regVal & 0xff;
  crntDMABuf[3] = (*phase->cs.wrPtrRegStat.regVal >> 8) & 0xff;
  ETHCS(0);
  doEthMemWrite(phase->cs.wrPtrRegStat.regAddr, 2);
  return WORKPEND;
}
int
writeEthRegSetup(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  crntDMABuf[2] = phase->cs.wrRegStat.regVal & 0xff;
  crntDMABuf[3] = (phase->cs.wrRegStat.regVal >> 8) & 0xff;
  ETHCS(0);
  doEthMemWrite(phase->cs.wrRegStat.regAddr, 2);
  return WORKPEND;
}
#if 0
void
writeEthReg(uint8_t addr, volatile uint16_t *val)
{
  struct EtherCmdState *cmdState = GET_CMD_STATE();
  cmdState->cs.rwRegStat.regAddr = addr;
  cmdState->cs.rwRegStat.reg = val;
  push_spi_cmd(writeEthRegSetup, deassertCS_CB, cmdState);
  invoke_spi_cmd();
}
#endif

int
readEthRegSetup(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  ETHCS(0);

  doEthMemRead(phase->cs.rdRegStat.regAddr, 2);
  return WORKPEND;
}
int
readEthRegCB(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  ETHCS(1);

  *phase->cs.rdRegStat.regVal = ((unsigned int)crntDMABuf[3] << 8) | (unsigned int)crntDMABuf[2];

  FREE_CMD_STATE(phase);

  return NOWORK;
}
#if 0
void
readEthReg(uint8_t addr, volatile uint16_t *val)
{
  struct EtherCmdState *cmdState = GET_CMD_STATE();
  cmdState->cs.rwRegStat.regAddr = addr;
  cmdState->cs.rwRegStat.reg = val;
  push_spi_cmd(readEthRegSetup, readEthRegCB, cmdState);
  invoke_spi_cmd();
}
#endif

int
setBitSetup(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  ETHCS(0);
  doBFSU(phase->cs.wrRegStat.regAddr, phase->cs.wrRegStat.regVal & 0xffff);
  return WORKPEND;
}
int
clrBitSetup(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  ETHCS(0);
  doBFCU(phase->cs.wrRegStat.regAddr, phase->cs.wrRegStat.regVal & 0xffff);
  return WORKPEND;
}
#if 0
void
setBit(uint8_t addr, uint16_t clear, volatile uint16_t *val)
{
  struct EtherCmdState *cmdState = GET_CMD_STATE();
  cmdState->cs.rwRegStat.regAddr = addr;
  cmdState->cs.rwRegStat.reg = val;
  if(clear) {
    push_spi_cmd(clrBitSetup, deassertCS_CB, cmdState);
  } else {
    push_spi_cmd(setBitSetup, deassertCS_CB, cmdState);
  } invoke_spi_cmd();
}
#endif

int
sramReadSetup(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  ETHCS(0);
  set_alt_dma_buf(phase->cs.rwSRAM.tail->buf);

  if(phase->cs.rwSRAM.count != -1) {
    phase->cs.rwSRAM.tail->inUse = MINVAL(phase->cs.rwSRAM.count, DMA_PKT_BUF_SZ);
    phase->cs.rwSRAM.count -= phase->cs.rwSRAM.tail->inUse;
  } else {
    phase->cs.rwSRAM.tail->inUse = RXDESCR_HDRLEN; /* We're going to reuse the same block in a bit */
  }
  rRXDataPtr = rRXDataPtr + phase->cs.rwSRAM.tail->inUse - 1;  /* -1 because need to take off SPI command */
  if(rRXDataPtr >= MAC_RXBUF_END) {
    rRXDataPtr = rRXDataPtr - MAC_RXBUF_END + MAC_RXBUF_OFFSET;
  }

  DBGPRINTF("setup recv %04x, %04x remain\n", phase->cs.rwSRAM.tail->inUse, phase->cs.rwSRAM.count);
  send_spi_cmd(phase->cs.rwSRAM.tail->inUse);

  return WORKPEND;
}
int
sramReadCB(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  if(phase->cs.rwSRAM.count == -1 && phase->cs.rwSRAM.head == phase->cs.rwSRAM.tail) {
    /* Reading the first block of a received packet. Figure out how long it is and why we received it */
    /* Then update the count to bring in the next block (if necessary) */
    uint16_t pktLen = (phase->cs.rwSRAM.head->buf[RXDESCR_LENL] | ((uint16_t)phase->cs.rwSRAM.head->buf[RXDESCR_LENH] << 8));
    pktLen = (pktLen + 1) & 0xfffe; /* Force back to next higher even transfer, pg 87, sec 9.2.2 */

    dumpPacket(phase->cs.rwSRAM.head);
    DBGPRINTF("LEN = %04x\n", pktLen);
    if(pktLen > 1500 + RXDESCR_HDRLEN) {
      DBGPRINTF("Unable to receive packet too long, just rx rxdescr\n");
      goto error;
    } else {
      phase->cs.rwSRAM.count = pktLen;
      phase->cs.rwSRAM.head->inUse = 0; /* Recycle the current buffer */
      DBGPRINTF("Total RX pkt len %04x, remain %04x\n", pktLen, phase->cs.rwSRAM.count);
    }
  }

  if(phase->cs.rwSRAM.count) {
    /* More work to do */
    if(phase->cs.rwSRAM.tail->inUse != 0) {
      struct DMABufDescr *d = allocDMAPktBuf_unsafe();
      if(!d) {
        DBGPRINTF("BOOM! Out of DMA memory!\n");
        goto error;
      }
      phase->cs.rwSRAM.tail->next = d;
      phase->cs.rwSRAM.tail = d;
    }

    phase->cs.rwSRAM.tail->inUse = MINVAL(phase->cs.rwSRAM.count, DMA_PKT_BUF_SZ);
    phase->cs.rwSRAM.count -= phase->cs.rwSRAM.tail->inUse;
    set_alt_dma_buf(phase->cs.rwSRAM.tail->buf);
    DBGPRINTF("cb recv %04x, %04x remain\n", phase->cs.rwSRAM.tail->inUse, phase->cs.rwSRAM.count);
    rRXDataPtr += phase->cs.rwSRAM.tail->inUse;
    if(rRXDataPtr >= MAC_RXBUF_END) {
      rRXDataPtr = rRXDataPtr - MAC_RXBUF_END + MAC_RXBUF_OFFSET;
    }
    send_spi_cmd(phase->cs.rwSRAM.tail->inUse);

    return WORKPEND;
  }
error:  /* eventually error needs to reset the MAC buffers to restore sanity and log the issue */
  set_alt_dma_buf(0); /* Back to default DMA buffer */
  ETHCS(1);
  FREE_CMD_STATE(phase);
  return NOWORK;
}
int
sramWriteSetup(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;
  ETHCS(0);
  set_alt_dma_buf(phase->cs.rwSRAM.head->buf);
  DBGPRINTF("setup send %04x\n", phase->cs.rwSRAM.head->inUse);
  send_spi_cmd(phase->cs.rwSRAM.head->inUse);

  return WORKPEND;
}
int
sramWriteCB(void *p)
{
  struct EtherCmdState *phase = (struct EtherCmdState *)p;

  struct DMABufDescr *d = phase->cs.rwSRAM.head;
  phase->cs.rwSRAM.head = d->next;
  freeDMABuf_unsafe(d);

  if(phase->cs.rwSRAM.head) {
    /* Another transfer so set it off */
    set_alt_dma_buf(phase->cs.rwSRAM.head->buf);
    DBGPRINTF("isr send %04x\n", phase->cs.rwSRAM.head->inUse);
    send_spi_cmd(phase->cs.rwSRAM.head->inUse);
    return WORKPEND;
  }
  DBGPRINTF("send isr end\n");
  set_alt_dma_buf(0); /* Back to default DMA buffer */
  ETHCS(1);
  FREE_CMD_STATE(phase);
  return NOWORK;
}
void
pushRegPtrWrite(uint8_t reg, volatile uint16_t *val)
{
  struct EtherCmdState *s;
  s = GET_CMD_STATE();
  s->cs.wrPtrRegStat.regAddr = reg;
  s->cs.wrPtrRegStat.regVal = val;
  push_spi_cmd(writeEthRegPtrSetup, deassertCS_CB, s);
}
void
pushRegWrite(uint8_t reg, uint16_t val)
{
  struct EtherCmdState *s;
  s = GET_CMD_STATE();
  s->cs.wrRegStat.regAddr = reg;
  s->cs.wrRegStat.regVal = val;
  push_spi_cmd(writeEthRegSetup, deassertCS_CB, s);
}
void
pushRegRead(uint8_t reg, volatile uint16_t *val)
{
  struct EtherCmdState *s;
  s = GET_CMD_STATE();
  s->cs.rdRegStat.regAddr = reg;
  s->cs.rdRegStat.regVal = val;
  push_spi_cmd(readEthRegSetup, readEthRegCB, s);
}
void
pushClrBit(uint8_t reg, uint16_t val)
{
  struct EtherCmdState *s;
  s = GET_CMD_STATE();
  s->cs.wrRegStat.regAddr = reg;
  s->cs.wrRegStat.regVal = val;
  push_spi_cmd(clrBitSetup, deassertCS_CB, s);
}
void
pushSetBit(uint8_t reg, uint16_t val)
{
  struct EtherCmdState *s;
  s = GET_CMD_STATE();
  s->cs.wrRegStat.regAddr = reg;
  s->cs.wrRegStat.regVal = val;
  push_spi_cmd(setBitSetup, deassertCS_CB, s);
}
void
pushBufferWrite(uint8_t cmd, struct DMABufDescr *data)
{
  struct EtherCmdState *s;
  s = GET_CMD_STATE();
  s->cs.rwSRAM.head = data;
  s->cs.rwSRAM.tail = data;
  while(s->cs.rwSRAM.tail->next)
    s->cs.rwSRAM.tail = s->cs.rwSRAM.tail->next;

  s->cs.rwSRAM.count = 0; /* Not used on write because lengths are in each buffer */
  data->buf[0] = cmd;
  push_spi_cmd(sramWriteSetup, sramWriteCB, s);
}
/*
 * If len == 0 then it's a receive buffer so read based on the packet descriptor.
 * Alternatively if len > 0 then read exactly len bytes
 */
void
pushBufferRead(uint8_t cmd, struct DMABufDescr **data, int len)
{
  *data = allocDMAPktBuf();
  (*data)->inUse = 0; /* DMA_PKT_BUF_SZ; */
  (*data)->buf[0] = cmd; /* First block will be 1 byte smaller because of this */

  struct EtherCmdState *s;
  s = GET_CMD_STATE();
  s->cs.rwSRAM.head = *data;
  s->cs.rwSRAM.tail = *data;
  s->cs.rwSRAM.count = len ? len + 1 : -1; /* count = -1 is the use RX descriptor for rest of read */

  push_spi_cmd(sramReadSetup, sramReadCB, s);
}
/*
 * New command series to reduce the number of discrete DMA requests and interrupts required for packet transmission
 * also allow for packet transmission during packet reception
 */
void
pushCmdReadFrag(struct DMABufDescr *bd, uint16_t regAddr)
{
  volatile uint8_t *idx = &bd->buf[bd->inUse];
  *idx++ = 42; /* TODO */
}
void
enableEthMacInt()
{
  /* Split memory into 2 for now */
  pushRegWrite(ERXST, MAC_RXBUF_OFFSET);
  pushRegWrite(ERXHEAD, MAC_RXBUF_OFFSET);
  pushRegWrite(ERXTAIL, MAC_RXBUF_END - 4); /* -4 on section 9.2 of datasheet */
  pushRegWrite(ERXRDPT, MAC_RXBUF_OFFSET);
  rRXDataPtr = MAC_RXBUF_OFFSET;
  pushRegWrite(EGPWRPT, MAC_TXBUF_OFFSET);
  wTXDataPtr = MAC_TXBUF_OFFSET;
  pushRegWrite(EGPRDPT, MAC_TXBUF_OFFSET);
  pushRegWrite(ERXFCON, 0x800a);  /* hash, multicast, unicast, goog crc & len,  0x80af = Pretty much anything goes */
  pushClrBit(EIR, 0xffff);
  pushSetBit(EIE, INTIE | LINKIE | PKTIE | TXIF | TXABIF);

  invoke_spi_cmd();
}
void
clearEthMacInt(volatile uint16_t *eir, volatile uint16_t *status)
{
  pushClrBit(EIE, INTIE);
  pushRegRead(EIR, eir);
  pushRegRead(ESTAT, status);
  pushClrBit(EIR, 0xffff);
  pushSetBit(EIE, INTIE);
  invoke_spi_cmd();
}
static void
setupEthernet()
{

  /* Initialize the free list for the cmd state stuctures */
  freebuf_init(&ethCmdStateFree, ethCmdState, sizeof(struct EtherCmdState), ETHCMDNUMELEM);

  TRISCbits.TRISC3 = 0; /* RC3 = output = #CS */
  ETHCS(1);

  /* Interrupt is tied to RP4, configure this as input on INT1 */
  devstat.val = 0;
  devstat.bits.isInitializing = 1;

  RPINR0bits.INT1R = 0x04; /* RP4 -> INT1 */
  INTCON2bits.INT1EP = 1; /* Negative edge */
  IFS1bits.INT1IF = 0;
  IEC1bits.INT1IE = 1;
}
void __attribute__((__interrupt__, no_auto_psv))
_INT1Interrupt()
{
  IFS1bits.INT1IF = 0;
  devstat.bits.ethIntReceived = 1; /* Let the main loop know */
  process_poll(&enc424mac_process);
}
PROCESS(enc424mac_process, "Microchip ENC424 MAC");

static void
do_send(struct DMABufDescr *descr)
{
  process_post(&enc424mac_process, EVENT_ETH_SEND, descr);

  dumpPacket(descr);
}
static uint8_t
enc424mac_send(const uip_lladdr_t *lladdr)
{
  DBGPRINTF("packet send\n");

  /*
   * If L3 dest is multicast, build L2 multicast address
   * as per RFC 2464 section 7
   * else fill with th eaddrsess in argument
   */
  struct DMABufDescr *descr = allocDMAPktBuf();
  struct DMABufDescr *tail = descr; /* Shortcut if adding subsequent buffer */
  if(descr == 0) {
    DBGPRINTF("Unable to allocate buffer, dropping packet\n");
    return 0;
  }
  /*
   * Take the first byte for the DMA command
   * not needed for subsequent buffers because we won't deassert CS
   */

  /* Copy destination address */
  if(lladdr == NULL) {
    DBGPRINTF("no lladdr\n");
    /* the dest must be multicast */
    tail->buf[1] = 0x33;
    tail->buf[2] = 0x33;
    tail->buf[3] = IPBUF->destipaddr.u8[12];
    tail->buf[4] = IPBUF->destipaddr.u8[13];
    tail->buf[5] = IPBUF->destipaddr.u8[14];
    tail->buf[6] = IPBUF->destipaddr.u8[15];
    /*tail->buf[1] = 0xff;
       tail->buf[2] = 0xff;
       tail->buf[3] = 0xff;
       tail->buf[4] = 0xff;
       tail->buf[5] = 0xff;
       tail->buf[6] = 0xff;*/
  } else {
    DBGPRINTF("add lladdr\n");
    memcpy((void *)&tail->buf[1], lladdr, UIP_LLADDR_LEN);
  }
  /* DBGPRINTF("using dmabuf %04x: %04x %04x %04x %04x\n", (uint16_t)tail, (uint16_t)(tail->buf), (uint16_t)(&tail->buf[7]), (uint16_t)&uip_lladdr, (uint16_t)UIP_LLADDR_LEN); */
  memcpy((uint8_t *)&tail->buf[7], &uip_lladdr, UIP_LLADDR_LEN);
  tail->buf[13] = UIP_ETHTYPE_IPV6 >> 8;
  tail->buf[14] = UIP_ETHTYPE_IPV6 & 0xff;

  volatile uint8_t *st = &tail->buf[15];
  tail->inUse = 15;
  uint8_t *uip_crnt_pos = &uip_buf[UIP_LLH_LEN];

  int srctocopy = MINVAL(UIP_TCPIP_HLEN, uip_len);
  int spaceremain = DMA_PKT_BUF_SZ - 15; /* TODO: length left in first buf, cleanup */

  DBGPRINTF("Total packet length %04x\n", uip_len + 15);
  /* Copy the rest of the tcpip headers into the buffer */
  /* max offset will be UIP_TCPIP_HLEN */
  while(srctocopy > 0) {
    int tocopy = MINVAL(srctocopy, spaceremain);
    /* DBGPRINTF("tc=%d\n", tocopy); */
    memcpy((uint8_t *)st, uip_crnt_pos, tocopy);
    st += tocopy;
    tail->inUse += tocopy;
    uip_len -= tocopy;
    srctocopy -= tocopy;
    uip_crnt_pos += tocopy;
    spaceremain -= tocopy;
    /*(DBGPRINTF("%04x %d %d %04x %d\n", (uint16_t)st,
       tail->inUse, uip_len, (uint16_t)uip_crnt_pos,
       spaceremain);*/
    if(uip_len > 0) {  /* More data to come */
      if(srctocopy == 0) {
        DBGPRINTF("switching to uip_appdata\n");
        /* Can only happen after the headers are copied and there's still more data */
        srctocopy = uip_len;
        uip_crnt_pos = uip_appdata;
      }

      if(spaceremain == 0) {
        DBGPRINTF("allocate another dma buffer\n");
        /* Allocate another buffer */
        tail->next = allocDMAPktBuf();
        tail = tail->next;
        tail->inUse = 0;
        spaceremain = DMA_PKT_BUF_SZ;
        st = tail->buf;
      }
    }
  }

  uip_len = 0; /* Make sure stack sees buffer as free */

  do_send(descr);
  return 0;
}
static void
process_link(uint16_t stat)
{
  devstat.bits.linkPending = 1;
  if(stat & PHYLNK) {
    DBGPRINTF("link up,");
    if(stat & PHYDPX) {
      DBGPRINTF(" full duplex\n");
      pushSetBit(MACON2, FULDPX);
      pushRegWrite(MABBIPG, MABBIPG_FD_DFLT);
    } else {
      DBGPRINTF(" half duplex\n");
      pushClrBit(MACON2, FULDPX);
      pushRegWrite(MABBIPG, MABBIPG_HD_DFLT);
    }
    pushSetBit(ECON1, RXEN);
    devstat.bits.linkUp = 1;
  } else {
    DBGPRINTF("link down\n");
    devstat.bits.linkUp = 0;
    pushClrBit(ECON1, RXEN);
  }
  DBGPRINTF("Ethstat %04x\n", stat);
  devstat.bits.linkPending = 1;
  invoke_spi_cmd();
}
static struct etimer timer;
static struct etimer tcp_start_delay;
static struct DMABufDescr *rxBuf; /* To hold an incoming packet chain */

static void
pollhandler()
{
  /* spiCmdPend means the spi bus is free */
  /* ethIntReceived means we've just had the ISR trigger */
  /* ethIntPending means we've acknowledged the ISR but are still processing the result */
  if(!devstat.bits.isInitializing) {
    DBGPRINTF("poll spi=%d ethint=%d ethintpend=%d linkpend=%d\n",
              spiCmdPend, devstat.bits.ethIntReceived,
              devstat.bits.ethIntPending, devstat.bits.linkPending);

    /* Handle any received interrupt status first */
    if(devstat.bits.ethIntPending) {
      /* Figure out which interrupts we have going on */
      if(ethIntrEIR & LINKIF) {
        devstat.bits.linkIntPending = 1;
      } else if(ethIntrEIR & TXIF) {
        devstat.bits.txIntPending = 1;
      } else if(ethIntrEIR & PKTIF) {
        devstat.bits.rxIntPending = 1;
      }
      devstat.bits.ethIntPending = 0;
      DBGPRINTF("interrupt eth int status %04x\n", ethIntrStat);
    }

    /* Handle any commands that are pending next */
    if(!spiCmdPend && devstat.bits.linkPending) {
      /* To get here the interrupt was already determined to be a link status change */
      devstat.bits.linkPending = 0;
      if(devstat.bits.linkUp) {
        DBGPRINTF("nearly starting tcpip\n");
        devstat.bits.tcp_start = 1;
        etimer_set(&tcp_start_delay, CLOCK_SECOND * 1);
      } else {
        DBGPRINTF("stopping tcpip\n");
        process_exit(&tcpip_process);
      }
    }

    if(!spiCmdPend && devstat.bits.txStatPending) {
      DBGPRINTF("ETXSTAT %04x    ETXWIRE %04x\n", etxStat, etxWire);
      devstat.bits.txStatPending = 0;
      /* pushSetBit(ECON1, TXRTS); */
      /* invoke_spi_cmd(); */
    }

    if(!spiCmdPend && devstat.bits.rxPktPending) {
      devstat.bits.rxPktPending = 0;
      if(rxBuf != 0) {
        /* Now copy this packet into */
        /* Strip off the MAC layer as we don't need it */
        uip_len = copyPktBuf(uip_buf, UIP_BUFSIZE, rxBuf);
        struct DMABufDescr *r;
        uint16_t osr = disable_int();
        while(rxBuf) {
          r = rxBuf->next;
          freeDMABuf_unsafe(rxBuf);
          rxBuf = r;
        }
        enable_int(osr);
        dumpPktBuffer();

        if((((uint16_t)uip_buf[12] << 8) | uip_buf[13]) == 0x86dd) {
          DBGPRINTF("IPv6 packet\n");
          /* dumpPktBuffer(); */
          tcpip_input(); /* Tell UIP that a packet arrived */
          if(uip_len > 0) {
            printf("Outgoing packet needed\n");
            dumpPktBuffer();
          }
        } else {
          uip_len = 0; /* No IPv6 packet so prevent reception */
        }
      } else {
        printf("No buffer\n");
      }
    }

    /* */
    /* Handle any specific pending interrupt notifications into commands */
    /* */
    if(!spiCmdPend && devstat.bits.linkIntPending) {
      devstat.bits.linkIntPending = 0; /* Interrupt handled */
      process_link(ethIntrStat); /* Will cause spiCmdPend to go busy again */
    }

    if(!spiCmdPend && devstat.bits.txIntPending) {
      devstat.bits.txIntPending = 0;
      DBGPRINTF("transmit interrupt");
      pushRegRead(ETXSTAT, &etxStat);
      pushRegRead(ETXWIRE, &etxWire);
      devstat.bits.txStatPending = 1;
      invoke_spi_cmd();
    }

    if(!spiCmdPend && devstat.bits.rxIntPending) {
      devstat.bits.rxIntPending = 0;
      DBGPRINTF("receive interrupt\n");
      /* Read the packet and decrement the counter */
      /* 0 length means "figure it out from the RX packet descriptor */
      pushBufferRead(ENC424_RRXDATA, (struct DMABufDescr **)&rxBuf, 0);
      pushSetBit(ECON1, PKTDEC);
      pushRegPtrWrite(ERXTAIL, &rRXDataPtr_onchip);
      pushRegRead(ERXRDPT, &rRXDataPtr_onchip);
      invoke_spi_cmd();
      devstat.bits.rxPktPending = 1;
    }

    if(!spiCmdPend && devstat.bits.ethIntReceived && !devstat.bits.ethIntPending) {
      DBGPRINTF("int poll\n");
      devstat.bits.ethIntReceived = 0;
      devstat.bits.ethIntPending = 1;

      /* The device is idle so we can read what caused the interrupt */
      clearEthMacInt(&ethIntrEIR, &ethIntrStat);
    }
  }
}
PROCESS_THREAD(enc424mac_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();
  DBGPRINTF("Starting ENC424MAC\n");
  EVENT_ETH_SEND = process_alloc_event();
  EVENT_ETH_STARTTCP = process_alloc_event();

  devstat.bits.isInitializing = 1;

  setupEthernet();

  DBGPRINTF("setupEthernet done\n");
  spiOwningProcess = PROCESS_CURRENT();
  /* ACB TODO PT_SEM_WAIT(process_pt, &spiBusSem); */
  DBGPRINTF("Ether lock SPI\n");

  do {
    pushRegWrite(EUDAST, 0x1234);
    invoke_spi_cmd();
    PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend);
    pushRegRead(EUDAST, &statVal);
    invoke_spi_cmd();
    PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend);
  } while(statVal != 0x1234);

  DBGPRINTF("MAC alive\n");

  do {
    pushRegRead(ESTAT, &statVal);
    invoke_spi_cmd();
    PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend);
  } while((statVal & CLKRDY) != CLKRDY);
  DBGPRINTF("Clock Ready\n");

  pushSetBit(ECON2, ETHRST);
  invoke_spi_cmd();
  etimer_set(&timer, CLOCK_SECOND / 100);

  do {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer) && !spiCmdPend);
    pushRegRead(EUDAST, &statVal);
    invoke_spi_cmd();
  } while(statVal != 0x0000);

  etimer_set(&timer, CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

  /* Reading the MAC address takes 3 reads */
  pushRegRead(MAADR3, &statVal);
  invoke_spi_cmd();
  PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend);
  uip_lladdr.addr[5] = (statVal >> 8) & 0xff;
  uip_lladdr.addr[4] = statVal & 0xff;
  pushRegRead(MAADR2, &statVal);
  invoke_spi_cmd();
  PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend);
  uip_lladdr.addr[3] = (statVal >> 8) & 0xff;
  uip_lladdr.addr[2] = statVal & 0xff;
  pushRegRead(MAADR1, &statVal);
  invoke_spi_cmd();
  PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend);
  uip_lladdr.addr[1] = (statVal >> 8) & 0xff;
  uip_lladdr.addr[0] = statVal & 0xff;
  printf("MAC Address :");
  uip_debug_lladdr_print(&uip_lladdr);
  printf("\n");

  /*  struct DMABufDescr* bd = startCmdFrag(gotMAC); */
  /* pushCmdReadFrag(bd, MAADR3, TRUE); */
  /* pushCmdReadFrag(MAADR2); */
  /* pushCmdReadFrag(MAADR1); */
  /* invokeFrag(bd); */
  /* PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend); */

  tcpip_set_outputfunc(enc424mac_send);

  enableEthMacInt();
  PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend);

  /* Interrupts are enabled now, read ESTAT to get the link status */
  pushRegRead(ESTAT, &statVal);
  invoke_spi_cmd();
  PROCESS_WAIT_EVENT_UNTIL(!spiCmdPend);

  devstat.bits.isInitializing = 0;
  DBGPRINTF("ether init done %04x\n", statVal);
  process_link(statVal);

  while(1) {
    PROCESS_WAIT_EVENT();
    DBGPRINTF("ENC424MAC: Event received %d\n", ev);
    if(ev == PROCESS_EVENT_TIMER) {
      if(devstat.bits.tcp_start && etimer_expired(&tcp_start_delay)) {
        devstat.bits.tcp_start = 0;
        process_start(&tcpip_process, NULL);
        DBGPRINTF("ENC424MAC: TCP started\n");
      }
    }
    if(ev == EVENT_ETH_SEND) {
      DBGPRINTF("send packet\n");
      if(spiCmdPend) {
        /* defer the packet by sending it back to ourselves */
        /* TODO the process should queue this up and use the poll to act on it */
        process_post(PROCESS_CURRENT(), ev, data);
      } else {
        /* SPI is free, we can queue up the request now */
        uint16_t outgoingPktLen = 0;
        struct DMABufDescr *d = (struct DMABufDescr *)data;
        while(d != 0) {
          outgoingPktLen += d->inUse;
          d = d->next;
        }
        if(outgoingPktLen > 0) {
          --outgoingPktLen;                 /* Account for DMA command octet */
        }
        DBGPRINTF("sending packet len %d\n", outgoingPktLen);
        pushBufferWrite(ENC424_WGPDATA, (struct DMABufDescr *)data);
        pushRegWrite(ETXST, wTXDataPtr);
        pushRegWrite(ETXLEN, outgoingPktLen);
        pushSetBit(ECON1, TXRTS);
        wTXDataPtr += outgoingPktLen;
        if(wTXDataPtr >= MAC_TXBUF_END) {
          wTXDataPtr = wTXDataPtr - MAC_TXBUF_END + MAC_TXBUF_OFFSET;
        }

        /* pushBufferRead(ENC424_RGPDATA, (struct DMAPktBufDescr**)&rxBuf, outgoingPktLen); */
        /* devstat.bits.rxPktPending = 1; // Hack to get the packet dumped */
        invoke_spi_cmd();
      }
    }
  }
  PROCESS_END();
}

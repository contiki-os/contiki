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

#ifndef ENC424MAC_H_
#define ENC424MAC_H_

PROCESS_NAME(enc424mac_process);

/* Short on memory so reuse, reuse, reuse! */
struct EtherCmdState {
  /* volatile unsigned int cmdState; // Shared state counter for multi-stage commands */

  union CmdState {
    struct RdRegBlock {
      volatile uint8_t regAddr;
      volatile uint8_t *data;
      volatile uint8_t count;
    } rdRegBlock;

    struct RdRegStatus { /* Used to read */
      volatile uint8_t regAddr; /* May be used if different addr for use */
      volatile uint16_t *regVal; /* Where to put the reg value */
    } rdRegStat;
    struct WrPtrRegStatus {
      volatile uint8_t regAddr;
      volatile uint16_t *regVal;
    } wrPtrRegStat;
    struct WrRegStatus {
      volatile uint8_t regAddr;
      volatile uint16_t regVal;
    } wrRegStat;
    struct RwSRAMState {
      struct DMABufDescr *head;
      struct DMABufDescr *tail; /* For quickly adding additional buffers */
      volatile int16_t count; /* -1 means reading packet descriptor */
    } rwSRAM;
  } cs;
};

union EtherDevDrvState {
  struct Bits {
    volatile uint16_t ethIntReceived : 1;
    volatile uint16_t ethIntPending : 1;
    volatile uint16_t linkIntPending : 1;
    volatile uint16_t linkPending : 1;
    volatile uint16_t txIntPending : 1;
    volatile uint16_t rxIntPending : 1;
    volatile uint16_t isInitializing : 1;
    volatile uint16_t linkUp : 1;
    volatile uint16_t txStatPending : 1;
    volatile uint16_t tcp_start : 1;
    volatile uint16_t rxPktPending : 1;
  } bits;
  volatile uint16_t val;
};
#endif /* ENC424MAC_H_ */

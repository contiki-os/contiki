/*
 * Copyright (c) 2010, Loughborough University - Computer Science
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
 */

/**
 * \file
 *         Header file for the Disco server
 *         (embedded part of the DISCOBALL project)
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef DISCO_H_
#define DISCO_H_

#include "contiki.h"
#include "contiki-net.h"
/*---------------------------------------------------------------------------*/
#define DISCO_UDP_PORT 60002
#define DISCO_DESCRIPTORS_LOC /* In external Flash */

#define DATA_CHUNK_LEN 64

/* Intervals - Timeouts */
#define DISCO_TIMEOUT_PREPARE  (CLOCK_SECOND / 2)
#define DISCO_TIMEOUT_ABORT    (CLOCK_SECOND * 10)
#define DISCO_TIMEOUT_REBOOT   CLOCK_SECOND

/* Disco State Machine */
#define DISCO_STATE_LISTENING  0x00 /* Waiting for a transaction to start */
#define DISCO_STATE_PREPARING  0x01 /* Erasing Sectors */
#define DISCO_STATE_READY      0x02
#define DISCO_STATE_REBOOTING  0x03 /* Reboot to BooTTY and copy new image */

/* Instructions */
#define DISCO_CMD_INIT      0x00 /* Prepare flash area for writes */
#define DISCO_CMD_SWITCH    0x01 /* Copy image from ext. to int. flash */
#define DISCO_CMD_WRITE     0x02 /* Write Image to Ext Flash */
#define DISCO_CMD_DONE      0x03 /* All Done */

/* Error Codes */
#define DISCO_ERR_GENERIC     0xFF /* Generic Error */
#define DISCO_ERR_BAD_LEN     0xFE /* Incorrect Length */
#define DISCO_ERR_NOT_READY   0xFD /* Not Initialised */
#define DISCO_ERR_BAD_OFFSET  0xFC /* Bad Offset */
#define DISCO_ERR_PROTECTED   0xFB /* Target sector is protected */
#define DISCO_ERR_INIT_DONE   0xFA /* Already Initialized */

/* Message Sizes */
#define DISCO_FLEN_CMD   1
#define DISCO_FLEN_IMG   1
#define DISCO_FLEN_ADDR  3
#define DISCO_FLEN_DATA 64

/* Request Lengths */
#define DISCO_LEN_INIT    (DISCO_FLEN_CMD + DISCO_FLEN_IMG)
#define DISCO_LEN_DONE    DISCO_FLEN_CMD
#define DISCO_LEN_WRITE   (DISCO_FLEN_CMD + DISCO_FLEN_ADDR + DISCO_FLEN_DATA)
#define DISCO_LEN_SWITCH  (DISCO_FLEN_CMD + DISCO_FLEN_IMG)

/* Response Lengths */
#define DISCO_RESPONSE_NONE    0
#define DISCO_RESP_LEN_ERR     DISCO_FLEN_CMD
#define DISCO_RESP_LEN_INIT    DISCO_FLEN_CMD
#define DISCO_RESP_LEN_DONE    DISCO_FLEN_CMD
#define DISCO_RESP_LEN_WRITE   (DISCO_FLEN_CMD + DISCO_FLEN_ADDR)
#define DISCO_RESP_LEN_SWITCH  (DISCO_FLEN_CMD + DISCO_FLEN_IMG)

/* Tell BooTTy! what to do after we jump:
 * BOOTY_CMD
 * [7:5]: Command
 * [5:4]: Reserved
 * [4:0]: Image number
 */
#define BOOTTY_CMD_LOCATION    0xFEFF

#define BOOTTY_CMD_JUMP_TO_APP   0x80
#define BOOTTY_CMD_COPY_IMAGE    0x40

#define SECTOR_UNPROTECTED 0
#define SECTOR_PROTECTED   1
/*---------------------------------------------------------------------------*/
PROCESS_NAME(disco_process);
/*---------------------------------------------------------------------------*/
struct disco_request_pdu {
  uint8_t cmd;
  uint8_t addr[3];
  uint8_t data[DATA_CHUNK_LEN];
};

struct disco_response_pdu {
  uint8_t status;
  uint8_t addr[3];
};

struct disco_seed {
  uip_ipaddr_t addr;
  uint16_t port;
};
/*---------------------------------------------------------------------------*/
#endif /* DISCO_H_ */

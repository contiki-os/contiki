/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 */

/**
 * \file
 *         Device driver for the numonyx N25Q128 108MHz 16Mbyte external memory.
 * \author
 *         Francisco Acosta <francisco.acosta@inria.fr>
 *
 *         Data is written bit inverted (~-operator) to flash so that
 *         unwritten data will read as zeros (UNIX style).
 */

#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/xmem.h"
#include "dev/watchdog.h"
#include "n25xxx.h"
#include "periph/n25xxx/n25xxx_regs.h"

#if 0
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif
/*---------------------------------------------------------------------------*/
static unsigned
read_status_register(void)
{
    unsigned char rx;
    uint8_t status;

    status = n25xxx_read_status();
    rx = status;

    return rx;
}
/*---------------------------------------------------------------------------*/
static void
write_enable(void)
{
    n25xxx_write_enable();
}

/*---------------------------------------------------------------------------*/
/*
 * Wait for a write/erase operation to finish.
 */
static unsigned
wait_ready(void)
{
    unsigned u;
    do {
        u = read_status_register();
    } while(u & 0x01);        /* WIP=1, write in progress */
    return u;
}
/*---------------------------------------------------------------------------*/
/*
 * Erase 64k bytes of data
 */
static void
erase_sector(unsigned long offset)
{
    write_enable();
    n25xxx_erase_sector(offset);
}
/*---------------------------------------------------------------------------*/
/*
 * Initialize external flash
 */
void
xmem_init(void)
{
    /* Initialized by the platform */
}
/*---------------------------------------------------------------------------*/
int
xmem_pread(void *_p, int size, unsigned long offset)
{
    unsigned char *p = _p;
    const unsigned char *end = p + size;

    wait_ready();

    _n25xxx_cs_clear();

    _n25xxx_rw_byte(N25XXX_INS__READ);

    _n25xxx_rw_byte(offset >> 16);    /* MSB */
    _n25xxx_rw_byte(offset >> 8);
    _n25xxx_rw_byte(offset >> 0);    /* LSB */

    for(; p < end; p++) {
        unsigned char u;
        u = _n25xxx_rw_byte(0);
        *p = ~u;
    }

    _n25xxx_cs_set();

    // Wait for the WIP bit to be cleared
    while (read_status_register() & 0x1)
    {
        ;
    }

    return size;
}
/*---------------------------------------------------------------------------*/
static const unsigned char *
program_page(unsigned long offset, const unsigned char *p, int nbytes)
{
    const unsigned char *end = p + nbytes;

    wait_ready();

    write_enable();

    // Start the SPI transfer
    _n25xxx_cs_clear();

    // Send instruction
    _n25xxx_rw_byte(N25XXX_INS__PP);

    // Send the address
    _n25xxx_rw_byte(offset >> 16);    /* MSB */
    _n25xxx_rw_byte(offset >> 8);
    _n25xxx_rw_byte(offset >> 0);    /* LSB */

    // Write the data

    for(; p < end; p++) {
        _n25xxx_rw_byte(~*p);
    }

    // End the SPI transfer
    _n25xxx_cs_set();

    // Wait for the WIP bit to be cleared
    while (read_status_register() & 0x1)
    {
        ;
    }

    return p;
}

/*---------------------------------------------------------------------------*/
int
xmem_pwrite(const void *_buf, int size, unsigned long addr)
{
    const unsigned char *p = _buf;
    const unsigned long end = addr + size;
    unsigned long i, next_page;

    for(i = addr; i < end;) {
        next_page = (i | 0xff) + 1;
        if(next_page > end) {
            next_page = end;
        }
        p = program_page(i, p, next_page - i);
        i = next_page;
    }

    return size;
}
/*---------------------------------------------------------------------------*/
int
xmem_erase(long size, unsigned long addr)
{
    PRINTF("Ereasing flash...\n");

    unsigned long end = addr + size;

    if(size % XMEM_ERASE_UNIT_SIZE != 0) {
        PRINTF("xmem_erase: bad size\n");
        return -1;
    }

    if(addr % XMEM_ERASE_UNIT_SIZE != 0) {
        PRINTF("xmem_erase: bad offset\n");
        return -1;
    }

    for (; addr < end; addr += XMEM_ERASE_UNIT_SIZE) {
        erase_sector(addr);
        // erasing all sectors take around 3 minutes
        // so watchdog must be restarted
        watchdog_periodic();
    }

    return size;
}

/*---------------------------------------------------------------------------*/

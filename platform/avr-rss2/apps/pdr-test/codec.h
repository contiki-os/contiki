// -*- mode: contiki-c-mode; c-basic-offset: 4; c-indent-level: 4 -*-
#ifndef CODEC_H
#define CODEC_H

#include <stdint.h>

//
// COding and DECoding routines
//

// ------------------------------------------------------
// CRC stuff

// Polynomial ^8 + ^5 + ^4 + 1
static inline uint8_t crc8Add(uint8_t acc, uint8_t byte)
{
    int i;
    acc ^= byte;
    for (i = 0; i < 8; i++) {
        if (acc & 1) {
            acc = (acc >> 1) ^ 0x8c;
        } else {
            acc >>= 1;
        }
    }

    return acc;
}

static inline uint8_t crc8(const void *data, uint16_t len)
{
    uint16_t i;
    uint8_t crc = 0;
    const uint8_t *_data = data;

    for (i = 0; i < len; ++i) {
        crc = crc8Add(crc, *_data++);
    }

    return crc;
}

static inline uint8_t crc8Str(const char *data)
{
    uint8_t crc = 0;
    while (*data) {
        crc = crc8Add(crc, *data++);
    }
    return crc;
}


// Convert half-a-byte (so called "nibble") to a hexadecimal character
static inline char to_hex(uint8_t nibble) {
    switch (nibble) {
    case 0 ... 9:
        return '0' + nibble;
    case 10 ... 15:
    default:
        return 'a' + nibble - 10;
    }
}


#endif

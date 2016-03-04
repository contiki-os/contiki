// -*- mode: contiki-c-mode; c-basic-offset: 4; c-indent-level: 4 -*-
#ifndef PATTERN_H
#define PATTERN_H

#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------
// a fast inline randomizer

extern uint32_t fastrandomKey;

static inline void randomSeed(uint16_t seed) {
    fastrandomKey = seed + 1234567890ul;
}

static inline uint16_t randomNumber(void)
{
    //
    // The numbers are from "Numerical Recipes"
    //
    fastrandomKey = fastrandomKey * 1664525ul + 1013904223ul;
    return (uint16_t) (fastrandomKey >> 16);
}

// ----------------------------------------

static inline void zeroFill(uint16_t *buffer, uint8_t len, uint16_t seed, uint8_t skip)
{
    memset(buffer, 0, len);
}

static inline void sequentialFill(uint16_t *buffer, uint8_t len, uint16_t seed, uint8_t skip)
{
    uint8_t i;
    uint16_t symbol = 0;

    // we process the data as uint16_t array
    len /= 2;
    skip /= 2;

    for (i = skip; i < len; ++i) {
        buffer[i] = (symbol << 12) + ((symbol + 1) << 8) + ((symbol + 2) << 4) + (symbol + 3);
        symbol = (symbol + 4) % 16;
    }
}

static inline void randomFill(uint16_t *buffer, uint8_t len, uint16_t seed, uint8_t skip)
{
    uint8_t i;

    randomSeed(seed);

    // we process the data as uint16_t array
    len /= 2;
    skip /= 2;

    for (i = skip; i < len; ++i) {
        buffer[i] = randomNumber();
    }
}

static inline void patternFill(uint16_t *buffer, uint8_t len, uint16_t seed, uint8_t skip)
{
#if PATTERN_FILL == FILL_ZERO
    zeroFill(buffer, len, seed, skip);

#elif PATTERN_FILL == FILL_SEQUENTIAL
    sequentialFill(buffer, len, seed, skip);
  
#elif PATTERN_FILL == FILL_RANDOM
    randomFill(buffer, len, seed, skip);

#endif
}

// ----------------------------------------

static inline uint8_t countBitErrors(uint8_t b1, uint8_t b2)
{
    const unsigned char oneBits[] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
    uint8_t x = b1 ^ b2;
    uint8_t n1 = x & 0xf;
    uint8_t n2 = (x >> 4) & 0xf;
    return oneBits[n1] + oneBits[n2];
}

static inline uint8_t countNibbleErrors(uint16_t x1, uint16_t x2,
                                        uint16_t symbolErrors[],
                                        uint16_t confusionMatrix[],
                                        uint32_t correctCounts[],
                                        uint8_t startIdx)
{
    uint8_t r = 0;

    uint8_t n11 = x1 >> 12;
    uint8_t n12 = (x1 >> 8) & 0xf;
    uint8_t n13 = (x1 >> 4) & 0xf;
    uint8_t n14 = x1 & 0xf;
    uint8_t n21 = x2 >> 12;
    uint8_t n22 = (x2 >> 8) & 0xf;
    uint8_t n23 = (x2 >> 4) & 0xf;
    uint8_t n24 = x2 & 0xf;

#if TRACK_ERRORS
    if (n11 ^ n21) {
        r++;
        symbolErrors[startIdx + 0]++;
        confusionMatrix[n11 * 16 + n21]++;
    } else {
        correctCounts[n11]++;
    }
    if (n12 ^ n22) {
        r++;
        symbolErrors[startIdx + 1]++;
        confusionMatrix[n12 * 16 + n22]++;
    } else {
        correctCounts[n12]++;
    }
    if (n13 ^ n23) {
        r++;
        symbolErrors[startIdx + 2]++;
        confusionMatrix[n13 * 16 + n23]++;
    } else {
        correctCounts[n13]++;
    }
    if (n14 ^ n24) {
        r++;
        symbolErrors[startIdx + 3]++;
        confusionMatrix[n14 * 16 + n24]++;
    } else {
        correctCounts[n14]++;
    }
#else
    if (n11 ^ n21) r++;
    if (n12 ^ n22) r++;
    if (n13 ^ n23) r++;
    if (n14 ^ n24) r++;
#endif


    return r;
}

static inline uint16_t patternCheck(uint16_t *buffer, uint8_t len, uint16_t seed,
                                    uint8_t skip, uint16_t symbolErrors[],
                                    uint16_t confusionMatrix[],
                                    uint32_t correctCounts[])
{
    uint16_t numErrors = 0;
    uint8_t i;
    uint16_t symbol = 0;

    // we process the data as uint16_t array
    len /= 2;
    skip /= 2;

    randomSeed(seed);

    for (i = skip; i < len; ++i) {
        uint16_t correct;

#if PATTERN_FILL == FILL_ZERO
        correct = 0;

#elif PATTERN_FILL == FILL_SEQUENTIAL
        correct = (symbol << 12) + ((symbol + 1) << 8) + ((symbol + 2) << 4) + (symbol + 3);
        symbol = (symbol + 4) % 16;

#elif PATTERN_FILL == FILL_RANDOM
        correct = randomNumber();

#endif

        numErrors += countNibbleErrors(correct, buffer[i], symbolErrors, 
                                       confusionMatrix, correctCounts, (i - skip) * 4);
    }
    return numErrors;
}

#endif

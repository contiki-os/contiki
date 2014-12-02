#include <stdint.h>
#include <stdio.h>

/*
 * Taken from
 *  http://www.barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
 *
 * Minor modification by Joakim Gebart <joakim.gebart@eistec.se> for computing
 * LSB first using reversed polynomials.
 */

typedef uint8_t crc_t;
#define WIDTH  (8 * sizeof(crc_t))

#define REVERSED 1

#if REVERSED
#define POLYNOMIAL 0x8C  /* 1-wire bus polynomial, reversed, LSB first */
#else
#define POLYNOMIAL 0x31  /* 1-wire bus polynomial */
#define TOPBIT (1 << (WIDTH - 1))
#endif

crc_t  crcTable[256];

void
crcInit(void)
{
    crc_t  remainder;
    int dividend;

    /*
     * Compute the remainder of each possible dividend.
     */
    for (dividend = 0; dividend < 256; ++dividend)
    {
        /*
         * Start with the dividend followed by zeros.
         */
        remainder = dividend << (WIDTH - 8);

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            #if REVERSED
            if (remainder & 0x01)
            {
                remainder = (remainder >> 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder >> 1);
            }
            #else
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
            #endif
        }

        /*
         * Store the result into the table.
         */
        crcTable[dividend] = remainder;
    }

}   /* crcInit() */

int main(int argc, char ** argv)
{
    int i;
    crcInit();
    printf("static const uint8_t crcTable[] = {");
    for (i = 0; i < sizeof(crcTable); ++i)
    {
        if (i % 8 == 0)
        {
            printf("\n");
        }
        printf("0x%02x, ", crcTable[i]);
    }
    printf("\n};");
    /*
    for (i = 0; i < sizeof(crcTable); ++i)
    {
        printf("%d, ", crcTable[i]);
    }
    printf("\n");*/
    return 0;
}

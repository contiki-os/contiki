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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Implementation of GCR coding/decoding
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *
 */

/* GCR conversion table - used for converting ordinary byte to 10-bits */
/* (or 4 bits to 5) */
static const unsigned char GCR_encode[16] = {
    0x0a, 0x0b, 0x12, 0x13,
    0x0e, 0x0f, 0x16, 0x17,
    0x09, 0x19, 0x1a, 0x1b,
    0x0d, 0x1d, 0x1e, 0x15
};

/*  5 bits > 4 bits (0xff => invalid) */
static const unsigned char GCR_decode[32] = {
    0xff, 0xff, 0xff, 0xff, // 0 - 3invalid...
    0xff, 0xff, 0xff, 0xff, // 4 - 7 invalid...
    0xff, 0x08, 0x00, 0x01, // 8 invalid... 9 = 8, a = 0, b = 1
    0xff, 0x0c, 0x04, 0x05, // c invalid... d = c, e = 4, f = 5

    0xff, 0xff, 0x02, 0x03, // 10-11 invalid...
    0xff, 0x0f, 0x06, 0x07, // 14 invalid...
    0xff, 0x09, 0x0a, 0x0b, // 18 invalid...
    0xff, 0x0d, 0x0e, 0xff, // 1c, 1f invalid...
  };

static unsigned char gcr_bits = 0;
static unsigned short gcr_val = 0;

/* Call before starting encoding or decoding */
void gcr_init(void) {
  gcr_val = 0;
  gcr_bits = 0;
}

/* Use this to check if encoding / decoding is complete for now */
unsigned char gcr_finished(void) {
  return gcr_bits == 0;
}

/* Encode one character - and store in bits - get encoded with get_encoded */
void gcr_encode(unsigned char raw_data) {
  gcr_val |=
    ((GCR_encode[raw_data >> 4u] << 5u ) |
     GCR_encode[raw_data & 0xf]) << gcr_bits;
  gcr_bits += 10;
}

/* Gets the current char of the encoded stream */
unsigned char gcr_get_encoded(unsigned char *raw_data) {
  if (gcr_bits >= 8) {
    *raw_data = (unsigned char) (gcr_val & 0xff);
    gcr_val = gcr_val >> 8u;
    gcr_bits = gcr_bits - 8;
    return 1;
  }
  return 0;
}

/* Decode one char - result can be get from get_decoded */
void gcr_decode(unsigned char gcr_data) {
  gcr_val |= gcr_data << gcr_bits;
  gcr_bits += 8;
}

/* check if the current decoded stream is correct */
unsigned char gcr_valid(void) {
  if (gcr_bits >= 10) {
    unsigned short val = gcr_val & 0x3ff;
    if ((GCR_decode[val >> 5u] << 4u) == 0xff ||
	(GCR_decode[val & 0x1f]) == 0xff) {
      return 0;
    }
  }
  return 1;
}

/* gets the decoded stream - if any char is available */
unsigned char gcr_get_decoded(unsigned char *raw_data) {
  if (gcr_bits >= 10) {
    unsigned short val = gcr_val & 0x3ff;
    *raw_data = (unsigned char) ((GCR_decode[val >> 5] << 4) |
				 (GCR_decode[val & 0x1f]));
    gcr_val = gcr_val >> 10;
    gcr_bits = gcr_bits - 10;
    return 1;
  }
  return 0;
}

/*
static const char encoded[] = {
  0x4a, 0x25, 0xa5, 0xfc, 0x96, 0xff, 0xff, 0xb5, 0xd4, 0x5a, 0xea, 0xff, 0xff, 0xaa, 0xd3, 0xff
};

int main(int argc, char **argv) {
  // unsigned char c[] = "testing gcr 1 2 3 4 5 6...";
  unsigned char c[] = { 0, 8, 0xe0, 0x2b, 0xac, 0x10, 0x01, 0x11, 0x50, 0xff, 0xf4, 0xa4, 0x00 };
  unsigned char c2[200];
  int pos = 0, pos2 = 0, i = 0;

  printf("Testing GCR on: %s \n", c);

  gcr_init();
  for (i = 0; i < sizeof(c); i++) {
    gcr_encode(c[i]);
    while(gcr_get_encoded(&c2[pos])) {
      printf("%02x=>%02x ", c[i], c2[pos]);
      pos++;
    }
  }
  printf("\n");
  printf("Encoded result %d chars (from %d) \n", pos, i);
  gcr_init();
  for (i = 0; i < pos; i++) {
    gcr_decode(c2[i]);
    if(!gcr_valid()) {
      printf("GCR: not valid\n");
    }
    while(gcr_get_decoded(&c[pos2])) {
      pos2++;
    }
  }
  printf("GCR: %s\n",c);
}
*/

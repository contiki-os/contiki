/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: GCRCoder.java,v 1.1 2008/02/07 14:54:16 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import org.apache.log4j.Logger;

/**
 * Ported from contiki-2.x/core/lib/gcr.[ch].
 *
 * @author Fredrik Osterlind
 */
public class GCRCoder {
  private static Logger logger = Logger.getLogger(GCRCoder.class);

  /*
   * GCR conversion table - used for converting ordinary byte to 10-bits (or 4
   * bits to 5)
   */
  static final int[] GCR_encode = new int[] { 0x0a, 0x0b, 0x12, 0x13, 0x0e,
      0x0f, 0x16, 0x17, 0x09, 0x19, 0x1a, 0x1b, 0x0d, 0x1d, 0x1e, 0x15 };

  /* 5 bits > 4 bits (0xff => invalid) */
  static final int[] GCR_decode = new int[] { 0xff, 0xff, 0xff, 0xff, // 0 -
      // 3invalid...
      0xff, 0xff, 0xff, 0xff, // 4 - 7 invalid...
      0xff, 0x08, 0x00, 0x01, // 8 invalid... 9 = 8, a = 0, b = 1
      0xff, 0x0c, 0x04, 0x05, // c invalid... d = c, e = 4, f = 5

      0xff, 0xff, 0x02, 0x03, // 10-11 invalid...
      0xff, 0x0f, 0x06, 0x07, // 14 invalid...
      0xff, 0x09, 0x0a, 0x0b, // 18 invalid...
      0xff, 0x0d, 0x0e, 0xff, // 1c, 1f invalid...
  };

  private int gcr_bits = 0;

  private int gcr_val = 0;

  public GCRCoder() {
  }

  /* Call before starting encoding or decoding */
  public void gcr_init() {
    gcr_val = 0;
    gcr_bits = 0;
  }

  /* Use this to check if encoding / decoding is complete for now */
  public boolean gcr_finished() {
    return gcr_bits == 0;
  }

  /* Encode one character - and store in bits - get encoded with get_encoded */
  void gcr_encode(int raw_data) {
    gcr_val |= ((GCR_encode[raw_data >> 4] << 5) | GCR_encode[raw_data & 0xf]) << gcr_bits;
    gcr_bits += 10;
  }

  /* Gets the current char of the encoded stream */
  boolean gcr_get_encoded(int[] raw_data, int current_pos) {
    if (gcr_bits >= 8) {
      raw_data[current_pos] = (gcr_val & 0xff);
      gcr_val = gcr_val >> 8;
      gcr_bits = gcr_bits - 8;
      return true;
    }
    return false;
  }

  /* Decode one char - result can be get from get_decoded */
  void gcr_decode(int gcr_data) {
    gcr_val |= gcr_data << gcr_bits;
    gcr_bits += 8;
  }

  /* check if the current decoded stream is correct */
  boolean gcr_valid() {
    if (gcr_bits >= 10) {
      int val = gcr_val & 0x3ff;
      if ((GCR_decode[val >> 5] << 4) == 0xff
          || (GCR_decode[val & 0x1f]) == 0xff) {
        return false;
      }
    }
    return true;
  }

  /* gets the decoded stream - if any char is available */
  boolean gcr_get_decoded(int[] raw_data, int current_pos) {
    if (gcr_bits >= 10) {
      int val = gcr_val & 0x3ff;
      raw_data[current_pos] = ((GCR_decode[val >> 5] << 4) | (GCR_decode[val & 0x1f]));
      gcr_val = gcr_val >> 10;
      gcr_bits = gcr_bits - 10;
      return true;
    }
    return false;
  }

  /**
   * Decodes given data. If decoding fails null is returned.
   *
   * WARNING! Decoding encoded data may differ from original data due to
   * appended zeroes during encoding.
   *
   * @param data
   *          Data
   * @param dataLength
   *          Data length to decode
   * @return Decoded data or null
   */
  public byte[] gcrDecode(byte[] data, int dataLength) {
    // Reset GCR (de)coder
    gcr_init();

    // Length of decoded data
    int convertedLength = 0;

    // Temporary decoded data storage
    int[] convertedInts = new int[dataLength]; // Maximum length

    for (int i = 0; i < dataLength; i++) {

      // Try decode byte
      gcr_decode(0xff & data[i]);
      if (!gcr_valid()) {
        logger.fatal("GCR decoding failed, dropping packet");
        return null;
      }

      // If new byte decoded, store it
      if (gcr_get_decoded(convertedInts, convertedLength)) {
        convertedLength++;
      }
    }

    // Convert to byte array
    byte[] convertedBytes = new byte[convertedLength];
    for (int i = 0; i < convertedLength; i++) {
      convertedBytes[i] = (byte) (0xff & convertedInts[i]);
    }

    return convertedBytes;
  }

  /**
   * Encodes given data. If encoding fails null is returned.
   *
   * WARNING! May append data by extra zeroes if needed by GCR.
   *
   * @param data
   *          Data
   * @param dataLength
   *          Data length to decode
   * @return Encoded data or null
   */
  public byte[] gcrEncode(byte[] data, int dataLength) {
    // Reset GCR (en)coder
    gcr_init();

    // Length of decoded data
    int convertedLength = 0;

    // Temporary encoded data storage
    int[] convertedInts = new int[dataLength * 2]; // Maximum length

    for (int i = 0; i < dataLength; i++) {

      // Try encode byte
      gcr_encode(0xff & data[i]);

      // Store encoded bytes
      while (gcr_get_encoded(convertedInts, convertedLength)) {
        convertedLength++;
      }
    }

    // Append extra 0 if GCR not finished
    if (!gcr_finished()) {
      gcr_encode(0);

      while (gcr_get_encoded(convertedInts, convertedLength)) {
        convertedLength++;
      }
    }

    // Convert to byte array
    byte[] convertedBytes = new byte[convertedLength];
    for (int i = 0; i < convertedLength; i++) {
      convertedBytes[i] = (byte) (0xff & convertedInts[i]);
    }

    return convertedBytes;
  }

}
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
 */

package se.sics.cooja.emulatedmote;
import org.apache.log4j.Logger;

/**
 * Ported from contiki-2.x/core/lib/crc16.[ch].
 *
 * @author Fredrik Osterlind
 */
public class CRCCoder {

    private static Logger logger = Logger.getLogger(CRCCoder.class);

    /**
     * Updates given accumulated CRC16 checksum with given byte.
     *
     * @param b Byte to be added to CRC
     * @param acc Accumulated CRC that is to be updated
     * @return New accumulated CRC
     */
    public static short crc16Add(byte b, short acc) {
        acc ^= 0xff & b;
        acc  = (short) ((0xff & (acc >> 8)) | (0xff00 & (acc << 8)));
        acc ^= 0xffff & ((acc & 0xff00) << 4);
        acc ^= (0xffff & (0xff & (acc >> 8)) >> 4);
        acc ^= 0xffff & ((0xffff & (acc & 0xff00)) >>> 5);
        return acc;
    }
}
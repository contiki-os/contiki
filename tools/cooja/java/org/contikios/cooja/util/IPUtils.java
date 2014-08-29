/*
 * Copyright (c) 2014, TU Braunschweig
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
 */
package org.contikios.cooja.util;

/**
 * Holds some IP byte to string conversion functions etc.
 *
 * @author Enrico Jorns
 */
public class IPUtils {
  
  /**
   * Convert IPv6 Byte-array in compressed IPv6-Address String.
   *
   * @param ip byte array holding IPv6 address information
   * @return String representation
   */
  public static String getCompressedIPv6AddressString(byte[] ip) {
    StringBuilder build = new StringBuilder();
    IPUtils.getCompressedIPv6AddressString(build, ip);
    return build.toString();
  }

  /**
   * Convert IPv6 Byte-array in compressed IPv6-Address String.
   *
   * @param builder Buffer to append to
   * @param ip byte array holding IPv6 address information
   */
  public static void getCompressedIPv6AddressString(StringBuilder builder, byte[] ip) {
    int startMax = 0, startCurr = 0, zeroMax = 0, zeroCurr = 0;

    if (ip.length != 16) {
      throw new IllegalArgumentException("Invalid array length: " + ip.length);
    }

    for (int i = 0; i < 16; i += 2) {
      if ((ip[i] | ip[i + 1]) == 0x00) {
        if (zeroCurr == 0) {
          startCurr = i;
        }
        zeroCurr++;
      }
      else {
        if (zeroCurr > zeroMax) {
          zeroMax = zeroCurr;
          startMax = startCurr;
          zeroCurr = 0;
        }
      }
    }
    if (zeroCurr > zeroMax) {
      zeroMax = zeroCurr;
      startMax = startCurr;
    }

    short a;
    for (int i = 0, f = 0; i < 16; i += 2) {
      a = (short) (((ip[i] & 0xFF) << 8) + (ip[i + 1] & 0xFF));
      if ((i >= startMax) && (i < startMax + zeroMax * 2)) {
        if (f++ == 0) {
          builder.append("::");
        }
      }
      else {
        if (f > 0) {
          f = -1;
        }
        else if (i > 0) {
          builder.append(':');
        }
        builder.append(String.format("%x", a));
      }
    }
  }

  /**
   * Convert IPv6 Byte-array in uncompressed IPv6-Address String.
   *
   * @param ip byte array holding IPv6 address information
   * @return uncompressed IPv6 representation string
   */
  public static String getUncompressedIPv6AddressString(byte[] ip) {
    StringBuilder ipBuilder = new StringBuilder();
    IPUtils.getUncompressedIPv6AddressString(ipBuilder, ip);
    return ipBuilder.toString();
  }
  
  /**
   * Convert IPv6 Byte-array in uncompressed IPv6-Address String.
   *
   * @param builder StringBuilder to append address to
   * @param ip byte array holding IPv6 address information
   */
  public static void getUncompressedIPv6AddressString(StringBuilder builder, byte[] ip) {
    for (int i = 0; i < 14; i += 2) {
      builder.append(String.format("%02x%02x:", 0xFF & ip[i + 0], 0xFF & ip[i + 1]));
    }
    builder.append(String.format("%02x%02x", 0xFF & ip[14], 0xFF & ip[15]));
  }

  /**
   * Convert IPv4 Byte-array to IPv4-Address String.
   *
   * @param ip byte array holding IPv4 address information
   * @return IPv4 representation string
   */
  public static String getIPv4AddressString(byte[] ip) {

    if (ip.length != 4) {
      throw new IllegalArgumentException("Invalid array length: " + ip.length);
    }

    StringBuilder ipBuilder = new StringBuilder();
    for (int i = 0; i < 3; i++) {
      ipBuilder.append(0xFF & ip[i]);
      ipBuilder.append('.');
    }
    ipBuilder.append(0xFF & ip[3]);
    return ipBuilder.toString();
  }

}

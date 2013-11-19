/*
 * Copyright (c) 2009, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package org.contikios.cooja.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.URL;
import java.util.zip.GZIPInputStream;

/**
 * Some utility methods for generating hex dumps.
 *
 * @author Niclas Finne, Fredrik Osterlind
 */
public class StringUtils {

  private static final char[] HEX = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
  };

  private StringUtils() {
    // Prevent instances of this class
  }

  public static String toHex(byte data) {
      return "" + HEX[(data >> 4) & 0xf] + HEX[data & 0xf];
  }
  
  public static String toHex(byte[] data) {
    char[] buf = new char[data.length * 2];
    for (int i = 0, j = 0, n = data.length; i < n; i++, j += 2) {
      buf[j] = HEX[(data[i] >> 4) & 0xf];
      buf[j + 1] = HEX[data[i] & 0xf];
    }
    return new String(buf);
  }

  public static String toHex(byte[] data, int bytesPerGroup) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0, n = data.length; i < n; i++) {
      if ((i % bytesPerGroup) == 0 && i > 0) {
        sb.append(' ');
      }
      sb.append(HEX[(data[i] >> 4) & 0xf]);
      sb.append(HEX[data[i] & 0xf]);
    }
    return sb.toString();
  }

  public static String hexDump(byte[] data) {
    return hexDump(data, 5, 4);
  }

  public static byte[] fromHex(String[] data) {
    StringBuilder sb = new StringBuilder();
    for (String s: data) {
      sb.append(s);
    }
    return fromHex(sb.toString());
  }
  
  /**
   * This method is compatible with the output from {@link #toHex(byte)}.
   * 
   * @param data Hexadecimal data
   * @return Binary data
   * @see #toHex(byte[], int)
   */
  public static byte[] fromHex(String data) {
    data = data.replace(" ", "");
    if (data.length() % 2 != 0) {
      throw new RuntimeException("Bad hex string: " + data);
    }
    byte[] bin = new byte[data.length()/2];
    for (int i=0; i < bin.length; i++) {
      bin[i] = (byte) (0xff&Integer.parseInt(data.substring(i*2, i*2+2), 16));
    }
    return bin;
  }
  
  public static String hexDump(byte[] data, int groupsPerLine, int bytesPerGroup) {
    if (bytesPerGroup <= 0) {
      throw new IllegalArgumentException("0 bytes per group");
    }
    if (groupsPerLine <= 0) {
      groupsPerLine = 1;
    }
    final int bytesPerLine = groupsPerLine * bytesPerGroup;
    StringBuilder sb = new StringBuilder();
    for (int j = 0; j < data.length; j += bytesPerLine) {
      int n = data.length - j;
      if (n > bytesPerLine) {
        n = bytesPerLine;
      }
      for (int i = 0; i < bytesPerLine; i++) {
        if ((i % bytesPerGroup) == 0 && i > 0) {
          sb.append(' ');
        }
        if (i < n) {
          sb.append(HEX[(data[j + i] >> 4) & 0xf]);
          sb.append(HEX[data[j + i] & 0xf]);
        } else {
          sb.append("  ");
        }
      }
      sb.append("  ");
      for (int i = 0; i < n; i++) {
        if (data[j + i] >= 32) {
          sb.append((char)(data[j + i] & 0xff));
        } else {
          sb.append('.');
        }
      }
      sb.append('\n');
    }
    return sb.toString();
  }

  public static String loadFromURL(URL url) {
    if (url == null) {
      return null;
    }
    try {
      InputStreamReader reader = new InputStreamReader(url.openStream());
      StringBuilder sb = new StringBuilder();
      char[] buf = new char[4096];
      int read;
      while ((read = reader.read(buf)) > 0) {
        sb.append(buf, 0, read);
      };

      reader.close();
      return sb.toString();
    } catch (IOException e) {
      return null;
    }
  }

  public static String loadFromFile(File file) {
    if (file == null) {
      return null;
    }
    StringBuilder sb = new StringBuilder();
    InputStreamReader reader = null;
    
    try {
      if (file.getName().endsWith(".gz")) {
        reader = new InputStreamReader(new GZIPInputStream(new FileInputStream(file)));
      } else {
        reader = new InputStreamReader(new FileInputStream(file));
      }

      char[] buf = new char[4096];
      int read;
      while ((read = reader.read(buf)) > 0) {
        sb.append(buf, 0, read);
      };

      reader.close();
      return sb.toString();
    } catch (IOException e) {
      e.printStackTrace();
      
      if (reader != null) {
        try {
          reader.close();
        } catch (IOException e1) {
        }
      }
      
      if (sb != null && sb.length() > 0) {
        return sb.toString();
      }
      return null;
    }
  }
  
  public static boolean saveToFile(File file, String text) {
    try {
      PrintWriter outStream = new PrintWriter(new FileWriter(file));
      outStream.print(text);
      outStream.close();
      return true;
    } catch (Exception ex) {
      return false;
    }
  }
}

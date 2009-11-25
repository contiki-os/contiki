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
 * $Id: ArrayUtils.java,v 1.4 2009/11/25 15:43:45 fros4943 Exp $
 */

package se.sics.cooja.util;

import java.awt.Container;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;

import javax.swing.JFileChooser;

/**
 * Some utility methods for managing arrays.
 *
 * @author Niclas Finne, Fredrik Osterlind
 */
public class ArrayUtils {

  @SuppressWarnings("unchecked")
  public static <T> T[] add(T[] array, T value) {
    T[] tmp = (T[]) java.lang.reflect.Array.newInstance(
        ((Class<? extends T>)array.getClass()).getComponentType(),
        array.length + 1);
    System.arraycopy(array, 0, tmp, 0, array.length);
    tmp[array.length] = value;
    return tmp;
  }

  @SuppressWarnings("unchecked")
  public static <T> T[] remove(T[] array, int index) {
    if ((index < 0) || (index >= array.length)) {
      throw new ArrayIndexOutOfBoundsException(index);
    }

    T[] tmp = (T[]) java.lang.reflect.Array.newInstance(
        ((Class<? extends T>)array.getClass()).getComponentType(),
        array.length - 1);
    if (index > 0) {
      System.arraycopy(array, 0, tmp, 0, index);
    }
    if (index < tmp.length) {
      System.arraycopy(array, index + 1, tmp, index, tmp.length - index);
    }
    return tmp;
  }

  public static <T> T[] remove(T[] array, T element) {
    int index = indexOf(array, element);
    return (index >= 0) ? remove(array, index) : array;
  }

  public static <T> int indexOf(T[] array, T element) {
    if (array != null) {
      if (element == null) {
        for (int i = 0, n = array.length; i < n; i++) {
          if (array[i] == null) {
            return i;
          }
        }
      } else {
        for (int i = 0, n = array.length; i < n; i++) {
          if (element.equals(array[i])) {
            return i;
          }
        }
      }
    }
    return -1;
  }
  
  public static boolean writeToFile(File dest, byte[] data) {
    try {
      FileOutputStream writer = new FileOutputStream(dest);
      writer.write(data);
      writer.flush();
      writer.close();
      return true;
    } catch (IOException e) {
      return false;
    }
  }
  
  public static byte[] readFromFile(File file) {
    long fileSize = file.length();
    byte[] fileData = new byte[(int) fileSize];

    FileInputStream fileIn;
    DataInputStream dataIn;
    int offset = 0;
    int numRead = 0;
    try {
      fileIn = new FileInputStream(file);
      dataIn = new DataInputStream(fileIn);
      while (offset < fileData.length
          && (numRead = dataIn.read(fileData, offset, fileData.length - offset)) >= 0) {
        offset += numRead;
      }

      dataIn.close();
      fileIn.close();
      return fileData;
    } catch (Exception ex) {
      return null;
    }
  }

  public static byte[] readFromStream(InputStream input) {
    try {
      int numRead = 0;
      int offset = 0;
      byte data[] = new byte[input.available()*2];
      DataInputStream dataIn = new DataInputStream(input);
      while ((numRead = dataIn.read(data, offset, data.length-offset)) >= 0) {
        offset += numRead;
      }
      byte[] streamData = new byte[offset];
      System.arraycopy(data, 0, streamData, 0, offset);
      return streamData;
    } catch (Exception ex) {
      return null;
    }
  }
}

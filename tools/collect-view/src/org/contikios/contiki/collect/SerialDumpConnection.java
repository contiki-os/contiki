/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *
 * -----------------------------------------------------------------
 *
 * SerialDumpConnection
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 5 oct 2010
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package org.contikios.contiki.collect;

/**
 *
 */
public class SerialDumpConnection extends CommandConnection {

  public static final String SERIALDUMP_WINDOWS = "./tools/serialdump-windows.exe";
  public static final String SERIALDUMP_LINUX = "./tools/serialdump-linux";
  public static final String SERIALDUMP_MACOS = "./tools/serialdump-macos";

  public SerialDumpConnection(SerialConnectionListener listener) {
    super(listener);
  }

  @Override
  public boolean isMultiplePortsSupported() {
    return true;
  }

  @Override
  public String getConnectionName() {
    return comPort;
  }

  @Override
  public void open(String comPort) {
    if (comPort == null) {
      throw new IllegalStateException("no com port");
    }

    /* Connect to COM using external serialdump application */
    String osName = System.getProperty("os.name").toLowerCase();
    String fullCommand;
    if (osName.startsWith("win")) {
      fullCommand = SERIALDUMP_WINDOWS + " " + "-b115200" + " " + getMappedComPortForWindows(comPort);
    } else if (osName.startsWith("mac")) {
      fullCommand = SERIALDUMP_MACOS + " " + "-b115200" + " " + comPort;
    } else {
      fullCommand = SERIALDUMP_LINUX + " " + "-b115200" + " " + comPort;
    }
    setCommand(fullCommand);
    super.open(comPort);
  }

  @Override
  protected void standardData(String line) {
    serialData(line);
  }

  @Override
  protected void errorData(String line) {
    if (!isOpen && line.startsWith("connecting") && line.endsWith("[OK]")) {
      isOpen = true;
      serialOpened();
    } else {
      super.errorData(line);
    }
  }

  private String getMappedComPortForWindows(String comPort) {
    if (comPort.startsWith("COM")) {
      comPort = "/dev/com" + comPort.substring(3);
    }
    return comPort;
  }

}

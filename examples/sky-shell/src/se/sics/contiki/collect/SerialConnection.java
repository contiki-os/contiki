/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: SerialConnection.java,v 1.3 2008/09/03 13:35:21 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * SerialConnection
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 5 jul 2008
 * Updated : $Date: 2008/09/03 13:35:21 $
 *           $Revision: 1.3 $
 */

package se.sics.contiki.collect;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

/**
 *
 */
public abstract class SerialConnection {

  public static final String SERIALDUMP_WINDOWS = "./tools/serialdump-windows.exe";
  public static final String SERIALDUMP_LINUX = "./tools/serialdump-linux";

  private String comPort;
  private Process serialDumpProcess;
  private PrintWriter serialOutput;
  protected boolean isOpen;
  protected boolean isClosed = true;
  protected String lastError;

  public boolean isOpen() {
    return isOpen;
  }

  public String getComPort() {
    return comPort;
  }

  public void setComPort(String comPort) {
    this.comPort = comPort;
  }

  public String getLastError() {
    return lastError;
  }

  public void open(String comPort) {
    if (comPort == null) {
      throw new IllegalStateException("no com port");
    }
    close();
    this.comPort = comPort;

    /* Connect to COM using external serialdump application */
    String osName = System.getProperty("os.name").toLowerCase();
    String fullCommand;
    if (osName.startsWith("win")) {
      fullCommand = SERIALDUMP_WINDOWS + " " + "-b115200" + " " + getMappedComPortForWindows(comPort);
    } else {
      fullCommand = SERIALDUMP_LINUX + " " + "-b115200" + " " + comPort;
    }

    isClosed = false;
    try {
      String[] cmd = fullCommand.split(" ");

      serialDumpProcess = Runtime.getRuntime().exec(cmd);
      final BufferedReader input = new BufferedReader(new InputStreamReader(serialDumpProcess.getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(serialDumpProcess.getErrorStream()));
      serialOutput = new PrintWriter(new OutputStreamWriter(serialDumpProcess.getOutputStream()));

      /* Start thread listening on stdout */
      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = input.readLine()) != null) {
              serialData(line);
            }
            input.close();
            System.out.println("Serialdump process terminated.");
            closeConnection();
          } catch (IOException e) {
            lastError = "Error when reading from serialdump process: " + e;
            System.err.println(lastError);
            if (!isClosed) {
              e.printStackTrace();
              closeConnection();
            }
          }
        }
      }, "read input stream thread");

      /* Start thread listening on stderr */
      Thread readError = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = err.readLine()) != null) {
              if (!isOpen && line.startsWith("connecting") && line.endsWith("[OK]")) {
                isOpen = true;
                serialOpened();
              } else {
                System.err.println("Serialdump error stream> " + line);
              }
            }
            err.close();
          } catch (IOException e) {
            if (!isClosed) {
              System.err.println("Error when reading from serialdump process: " + e);
              e.printStackTrace();
            }
          }
        }
      }, "read error stream thread");

      readInput.start();
      readError.start();
    } catch (Exception e) {
      lastError = "Failed to execute '" + fullCommand + "': " + e;
      System.err.println(lastError);
      e.printStackTrace();
      closeConnection();
    }
  }

  private String getMappedComPortForWindows(String comPort) {
    if (comPort.startsWith("COM")) {
      comPort = "/dev/com" + comPort.substring(3);
    }
    return comPort;
  }

  public void writeSerialData(String data) {
    PrintWriter serialOutput = this.serialOutput;
    if (serialOutput != null) {
      serialOutput.println(data);
      serialOutput.flush();
    }
  }

  public void close() {
    isClosed = true;
    lastError = null;
    closeConnection();
  }

  protected void closeConnection() {
    isOpen = false;
    if (serialOutput != null) {
      serialOutput.close();
      serialOutput = null;
    }
    if (serialDumpProcess != null) {
      serialDumpProcess.destroy();
      serialDumpProcess = null;
    }
    serialClosed();
  }

  protected abstract void serialData(String line);

  protected abstract void serialOpened();

  protected abstract void serialClosed();

}

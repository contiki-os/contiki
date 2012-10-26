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
 * CommandConnection
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 5 oct 2010
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
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
public class CommandConnection extends SerialConnection {

  protected Process commandProcess;
  protected String command;

  public CommandConnection(SerialConnectionListener listener) {
    super(listener);
  }

  public CommandConnection(SerialConnectionListener listener, String command) {
    super(listener);
    this.command = command;
  }

  @Override
  public String getConnectionName() {
    return command;
  }

  public String getCommand() {
    return command;
  }

  public void setCommand(String command) {
    this.command = command;
  }

  @Override
  public void open(String comPort) {
    close();
    this.comPort = comPort == null ? "" : comPort;

    String fullCommand = getCommand();

    isClosed = false;
    try {
      String[] cmd = fullCommand.split(" ");
      System.err.println("Running '" + fullCommand + '\'');

      commandProcess = Runtime.getRuntime().exec(cmd);
      final BufferedReader input = new BufferedReader(new InputStreamReader(commandProcess.getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(commandProcess.getErrorStream()));
      setSerialOutput(new PrintWriter(new OutputStreamWriter(commandProcess.getOutputStream())));

      /* Start thread listening on standard out */
      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = input.readLine()) != null) {
              standardData(line);
            }
            input.close();
            System.err.println("SerialConnection command terminated.");
            closeConnection();
          } catch (IOException e) {
            lastError = "Error when reading from SerialConnection command: " + e;
            System.err.println(lastError);
            if (!isClosed) {
              e.printStackTrace();
              closeConnection();
            }
          }
        }
      }, "read input stream thread");

      /* Start thread listening on standard err */
      Thread readError = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = err.readLine()) != null) {
              errorData(line);
            }
            err.close();
          } catch (IOException e) {
            if (!isClosed) {
              System.err.println("Error when reading from SerialConnection command: " + e);
              e.printStackTrace();
            }
          }
        }
      }, "read error stream thread");

      if (!isOpen) {
        isOpen = true;
        serialOpened();
      }
      readInput.start();
      readError.start();
    } catch (Exception e) {
      lastError = "Failed to execute '" + fullCommand + "': " + e;
      System.err.println(lastError);
      e.printStackTrace();
      closeConnection();
    }
  }

  protected void standardData(String line) {
    serialData(line);
  }

  protected void errorData(String line) {
    System.err.println("SerialConnection error stream> " + line);
  }

  @Override
  protected void doClose() {
    if (commandProcess != null) {
      commandProcess.destroy();
      commandProcess = null;
    }
  }

}

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
 * StdinConnection
 *
 * Authors : Niclas Finne
 * Created : 5 oct 2010
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package org.contikios.contiki.collect;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

/**
 *
 */
public class StdinConnection extends SerialConnection {

  private PrintWriter stdout;

  public StdinConnection(SerialConnectionListener listener) {
    super(listener);
    // Redirect standard out as standard err to use standard out for serial output
    stdout = new PrintWriter(new OutputStreamWriter(System.out));
    System.setOut(System.err);
  }

  @Override
  public String getConnectionName() {
    return "<stdin>";
  }

  @Override
  public void open(String comPort) {
    close();
    this.comPort = comPort == null ? "" : comPort;

    isClosed = false;
    try {
      final BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
      setSerialOutput(stdout);

      /* Start thread listening on standard in */
      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = input.readLine()) != null) {
              serialData(line);
              // Do not send data too fast
              try {
                Thread.sleep(100);
              } catch (InterruptedException e) {
                e.printStackTrace();
              }
            }
            input.close();
            System.out.println("SerialConnection stdin terminated.");
            closeConnection();
          } catch (IOException e) {
            lastError = "Error when reading from SerialConnection stdin: " + e;
            System.err.println(lastError);
            if (!isClosed) {
              e.printStackTrace();
              closeConnection();
            }
          }
        }
      }, "read input stream thread");

      isOpen = true;
      serialOpened();
      readInput.start();

    } catch (Exception e) {
      lastError = "Failed to open stdin for reading: " + e;
      System.err.println(lastError);
      e.printStackTrace();
      closeConnection();
    }
  }

  @Override
  protected void doClose() {
  }

}

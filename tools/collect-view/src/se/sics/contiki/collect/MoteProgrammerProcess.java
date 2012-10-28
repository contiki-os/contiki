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
 *
 * -----------------------------------------------------------------
 *
 * MoteProgrammerProcess
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 10 jul 2008
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 *
 */
public class MoteProgrammerProcess {

  public static final String BSL_WINDOWS = "./tools/msp430-bsl-windows.exe";
  public static final String BSL_LINUX = "./tools/msp430-bsl-linux";

  private final String moteID;
  private final String firmwareFile;
  private final String[][] commandSet;
  private int retry = 3;

  private Process currentProcess;
  private Thread commandThread;
  private boolean isRunning;
  private boolean hasError;

  public MoteProgrammerProcess(String moteID, String firmwareFile) {
    this.moteID = moteID;
    this.firmwareFile = firmwareFile;
    String osName = System.getProperty("os.name").toLowerCase();
    String bslCommand;
    if (osName.startsWith("win")) {
      bslCommand = BSL_WINDOWS;
    } else {
      bslCommand = BSL_LINUX;
    }
    commandSet = new String[][] {
        { bslCommand, "--telosb", "-c", moteID, "-e" },
        { bslCommand, "--telosb", "-c", moteID, "-I", "-p", firmwareFile },
        { bslCommand, "--telosb", "-c", moteID, "-r" }
    };
  }

  public String getMoteID() {
    return moteID;
  }

  public String getFirmwareFile() {
    return firmwareFile;
  }

  public int getRetry() {
    return retry;
  }

  public void setRetry(int retry) {
    this.retry = retry;
  }

  public boolean isRunning() {
    return isRunning;
  }

  public boolean hasError() {
    return hasError;
  }

  public void start() {
    if (isRunning) {
      // Already running
      return;
    }
    isRunning = true;
    commandThread = new Thread(new Runnable() {
      public void run() {
        try {
          int count = 0;
          do {
            if (count > 0) {
              logLine("An error occurred. Retrying.", true, null);
            }
            count++;
            hasError = false;
            for (int j = 0, m = commandSet.length; j < m && isRunning && !hasError; j++) {
              runCommand(commandSet[j]);
              Thread.sleep(2000);
            }
          } while (isRunning && hasError && count < retry);
        } catch (Exception e) {
          e.printStackTrace();
        } finally {
          isRunning = false;
          processEnded();
        }
      }
    });
    commandThread.start();
  }

  public void stop() {
    isRunning = false;
    Process process = currentProcess;
    if (process != null) {
      process.destroy();
    }
  }

  public void waitForProcess() throws InterruptedException {
    if (isRunning && commandThread != null) {
      commandThread.join();
    }
  }

  protected void processEnded() {
  }

  private void runCommand(String[] cmd) throws IOException, InterruptedException {
    if (currentProcess != null) {
      currentProcess.destroy();
    }
    currentProcess = Runtime.getRuntime().exec(cmd);
    final BufferedReader input = new BufferedReader(new InputStreamReader(currentProcess.getInputStream()));
    final BufferedReader err = new BufferedReader(new InputStreamReader(currentProcess.getErrorStream()));

    /* Start thread listening on stdout */
    Thread readInput = new Thread(new Runnable() {
      public void run() {
        String line;
        try {
          while ((line = input.readLine()) != null) {
            handleLine(line, false);
          }
          input.close();
        } catch (IOException e) {
          logLine("Error reading from command", false, e);
        }
      }
    }, "read stdout thread");

    /* Start thread listening on stderr */
    Thread readError = new Thread(new Runnable() {
      public void run() {
        String line;
        try {
          while ((line = err.readLine()) != null) {
            handleLine(line, true);
          }
          err.close();
        } catch (IOException e) {
          logLine("Error reading from command", true, e);
        }
      }
    }, "read stderr thread");

    readInput.start();
    readError.start();

    // Wait for the bsl program to finish executing
    readInput.join();
    currentProcess = null;
  }

  private void handleLine(String line, boolean stderr) {
    if (line.toLowerCase().contains("error")) {
      hasError = true;
    }
    logLine(line, stderr, null);
  }

  protected void logLine(String line, boolean stderr, Throwable e) {
    if (stderr) {
      System.err.println("Programmer@" + moteID + "> " + line);
    } else {
      System.out.println("Programmer@" + moteID + "> " + line);
    }
    if (e != null) {
      e.printStackTrace();
    }
  }

  protected String toString(String[] cmd) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0, n = cmd.length; i < n; i++) {
      if (i > 0) sb.append(' ');
      sb.append(cmd[i]);
    }
    return sb.toString();
  }

}

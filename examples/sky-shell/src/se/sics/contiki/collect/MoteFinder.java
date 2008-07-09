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
 * $Id: MoteFinder.java,v 1.1 2008/07/09 23:18:06 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * Motelist
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 4 jul 2008
 * Updated : $Date: 2008/07/09 23:18:06 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect;
import java.awt.Component;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.JOptionPane;

/**
 *
 */
public class MoteFinder {

  public static final String MOTELIST_WINDOWS = "./tools/motelist-windows.exe";
  public static final String MOTELIST_LINUX = "./tools/motelist-linux";

  private final Pattern motePattern;
  private Process moteListProcess;
//  private boolean hasVerifiedProcess;
  private ArrayList<String> comList = new ArrayList<String>();
  private int[] moteList = new int[10];
  private int moteCount = 0;

  public MoteFinder() {
    motePattern = Pattern.compile("\\s(COM|/dev/[a-zA-Z]+)(\\d+)\\s");
  }

  public int[] getMotes() throws IOException {
    searchForMotes();
    return getMoteList();
  }

  public String[] getComPorts() throws IOException {
    searchForMotes();
    return getComList();
  }

  private void searchForMotes() throws IOException {
    comList.clear();
    moteCount = 0;
//    hasVerifiedProcess = false;

    /* Connect to COM using external serialdump application */
    String osName = System.getProperty("os.name").toLowerCase();
    String fullCommand;
    if (osName.startsWith("win")) {
      fullCommand = MOTELIST_WINDOWS;
    } else {
      fullCommand = MOTELIST_LINUX;
    }

    try {
      String[] cmd = new String[] { fullCommand };
      moteListProcess = Runtime.getRuntime().exec(cmd);
      final BufferedReader input = new BufferedReader(new InputStreamReader(moteListProcess.getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(moteListProcess.getErrorStream()));

      /* Start thread listening on stdout */
      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = input.readLine()) != null) {
              parseIncomingLine(line);
            }
            input.close();
          } catch (IOException e) {
            System.err.println("Exception when reading from motelist");
            e.printStackTrace();
          }
        }
      }, "read motelist thread");

      /* Start thread listening on stderr */
      Thread readError = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = err.readLine()) != null) {
              System.err.println("Motelist error stream> " + line);
            }
            err.close();
          } catch (IOException e) {
            System.err.println("Exception when reading from motelist");
            e.printStackTrace();
          }
        }
      }, "read motelist error stream thread");

      readInput.start();
      readError.start();

      // Wait for the motelist program to finish executing
      readInput.join();
    } catch (Exception e) {
      throw (IOException) new IOException("Failed to execute '" + fullCommand + "'").initCause(e);
    }
  }

  private String[] getComList() {
    return comList.toArray(new String[comList.size()]);
  }

  private int[] getMoteList() {
    if (moteCount < moteList.length) {
      int[] tmp = new int[moteCount];
      System.arraycopy(moteList, 0, tmp, 0, tmp.length);
      moteList = tmp;
    }
    return moteList;
  }

  public void close() {
    if (moteListProcess != null) {
      moteListProcess.destroy();
      moteListProcess = null;
    }
  }

  protected void parseIncomingLine(String line) {
    if (line.contains("No devices found") || line.startsWith("Reference")) {
      // No Sky connected or title before connected motes
//      hasVerifiedProcess = true;
    } else if (line.startsWith("-------")) {
      // Separator
    } else {
      Matcher matcher = motePattern.matcher(line);
      if (matcher.find()) {
        if (moteCount == moteList.length) {
          int[] tmp = new int[moteCount + 10];
          System.arraycopy(moteList, 0, tmp, 0, moteCount);
          moteList = tmp;
        }
        comList.add(matcher.group(1) + matcher.group(2));
        moteList[moteCount++] = Integer.parseInt(matcher.group(2));
      } else {
        System.err.println("Motelist> " + line);
      }
    }
  }

  public static String selectComPort(Component parent) {
    MoteFinder finder = new MoteFinder();
    try {
      String[] motes = finder.getComPorts();
      if (motes == null || motes.length == 0) {
        JOptionPane.showMessageDialog(parent, "Could not find any connected motes.", "No mote found", JOptionPane.ERROR_MESSAGE);
        return null;
      } else if (motes.length == 1) {
        // Only one node found
        return motes[0];
      } else {
        // Several motes found
        return (String) JOptionPane.showInputDialog(
            parent, "Found multiple connected motes. Please select serial port:",
            "Select serial port", JOptionPane.QUESTION_MESSAGE, null, motes, motes[0]);
      }
    } catch (IOException e) {
      e.printStackTrace();
      JOptionPane.showMessageDialog(parent, "Failed to search for connected motes:\n" + e, "Error", JOptionPane.ERROR_MESSAGE);
      return null;
    } finally {
      finder.close();
    }
  }

  public static void main(String[] args) throws IOException {
    MoteFinder finder = new MoteFinder();
    String[] motes = finder.getComPorts();
    finder.close();
    if (motes == null || motes.length == 0) {
      System.out.println("No motes connected");
    } else {
      for(String port: motes) {
        System.out.println("Found Sky at " + port);
      }
    }
  }

}

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: Serial2Pipe.java,v 1.1 2007/02/26 13:28:14 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.GridLayout;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Observable;
import java.util.Observer;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.interfaces.ContikiRS232;

@ClassDescription("Serial 2 Pipe")
@PluginType(PluginType.MOTE_PLUGIN)
public class Serial2Pipe extends VisPlugin {
  private static Logger logger = Logger.getLogger(Serial2Pipe.class);

  private static final long serialVersionUID = 1L;

  private static int pipeNrCounter = 1;

  private Mote mote;

  private ContikiRS232 serialInterface;

  private String inPipeName;

  private String outPipeName;

  private int pipeNr = pipeNrCounter++; // Increase pipe number

  private Thread pipeListener;

  private Observer serialObserver;

  public Serial2Pipe(Mote moteToView, Simulation simulation, GUI gui) {
    super("Serial 2 Pipe (" + moteToView + ")", gui);
    mote = moteToView;

    // Determine pipe names
    inPipeName = "s2p" + pipeNr + "in";
    outPipeName = "s2p" + pipeNr + "out";

    // Create pipes
    try {
      Process subProcess = Runtime.getRuntime().exec(
          new String[] { "mkfifo", inPipeName });
      subProcess.waitFor();
//      logger.debug("Creating in pipe returned: " + subProcess.exitValue());
      subProcess = Runtime.getRuntime().exec(
          new String[] { "mkfifo", outPipeName });
      subProcess.waitFor();
//      logger.debug("Creating out pipe returned: " + subProcess.exitValue());
    } catch (Exception e) {
      logger.fatal("Error when creating pipes: " + e);
    }

    // Forward serial interface to out pipe
    serialInterface = mote.getInterfaces().getInterfaceOfType(
        ContikiRS232.class);
    serialInterface.addObserver(serialObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        // Start shell process which forwards the data
        try {
          Runtime.getRuntime().exec(
              new String[] {
                  "sh",
                  "-c",
                  "echo " + serialInterface.getSerialMessages() + " > "
                      + outPipeName });
//          logger.debug("Forwarding from serial "
//              + serialInterface.getSerialMessages());
        } catch (Exception ex) {
          logger.fatal("Error when writing to out pipe: " + ex);
        }
      }
    });

    // Forward incoming data from in pipe (separate thread)
    pipeListener = new Thread(new Runnable() {
      public void run() {
        try {
          while (true) {
            // Start shell process which reads the data
            Process subProcess = Runtime.getRuntime().exec(
                new String[] { "sh", "-c", "cat " + inPipeName });
            BufferedReader in = new BufferedReader(new InputStreamReader(
                subProcess.getInputStream()));
            char[] buffer = new char[512];
            int bytes_read;
            bytes_read = in.read(buffer);
            if (bytes_read > 0) {
              String msg = String.copyValueOf(buffer, 0, bytes_read);
//              logger.debug("Forwarding to serial: " + msg);
              serialInterface.sendSerialMessage(msg);
            }
            in.close();
            subProcess.waitFor();
          }
        } catch (Exception e) {
          logger.fatal("Error while reading from in pipe: " + e);
        }
      }
    });
    pipeListener.start();

    // Add GUI labels
    setLayout(new GridLayout(2, 2));

    add(new JLabel("In pipe:"));
    add(new JLabel(inPipeName));

    add(new JLabel("Out pipe:"));
    add(new JLabel(outPipeName));

    setSize(300, 70);

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  public void closePlugin() {
    // Stop listening to serial port
    if (serialInterface != null && serialObserver != null) {
      serialInterface.deleteObserver(serialObserver);
      serialObserver = null;
    }

    // Stop listening to in pipe
    if (pipeListener != null)
      pipeListener.interrupt();

    // Remove earlier created pipes
    try {
      Runtime.getRuntime().exec(new String[] { "rm", inPipeName });
      Runtime.getRuntime().exec(new String[] { "rm", outPipeName });
    } catch (Exception e) {
      logger.fatal("Error when deleting pipes");
    }
  }

}

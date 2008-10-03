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
 * $Id: ContikiLog.java,v 1.3 2008/10/03 10:36:39 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.awt.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.Log;


/**
 * The class Log is an abstract interface to a mote's logging output. It needs
 * read access to the following core variables:
 * <ul>
 * <li>char simLoggedFlag
 * (1=mote has new outgoing log messages, else no new)
 * <li>int simLoggedLength
 * (length of new log message)
 * <li>byte[] simLoggedData (data of new log messages)
 * </ul>
 * <p>
 * Dependency core interfaces are:
 * <ul>
 * <li>simlog_interface
 * </ul>
 * <p>
 * This observable is changed and notifies observers whenever a log message has
 * been received from the core (checked after each tick). The public method
 * getLastLogMessages gives access to the last log message(s).
 *
 * @author      Fredrik Osterlind
 */
public class ContikiLog extends Log implements ContikiMoteInterface {
  private static Logger logger = Logger.getLogger(ContikiLog.class);
  private Mote mote = null;
  private SectionMoteMemory moteMem = null;

  private String lastLogMessage = null;

  /**
   * Creates an interface to mote's logging output.
   *
   * @param mote    Log's mote.
   * @see             Mote
   * @see             se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiLog(Mote mote) {
    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[] { "simlog_interface" };
  }

  public void doActionsBeforeTick() {
    // Nothing to do
  }

  public void doActionsAfterTick() {
    if (moteMem.getByteValueOf("simLoggedFlag") == 1) {
      int totalLength = moteMem.getIntValueOf("simLoggedLength");
      byte[] bytes = moteMem.getByteArray("simLoggedData", totalLength);
      char[] chars = new char[bytes.length];
      for (int i=0; i < chars.length; i++) {
        chars[i] = (char) bytes[i];
      }

      moteMem.setByteValueOf("simLoggedFlag", (byte) 0);
      moteMem.setIntValueOf("simLoggedLength", 0);

      String fullMessage[] = String.valueOf(chars).split("\n");
      for (String message: fullMessage) {
        lastLogMessage = message;

        this.setChanged();
        this.notifyObservers(mote);
      }
    }
  }

  public String getLastLogMessages() {
    return lastLogMessage;
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BorderLayout());
    final JTextArea logTextPane = new JTextArea();
    logTextPane.setOpaque(false);
    logTextPane.setEditable(false);

    if (lastLogMessage == null) {
      logTextPane.setText("");
    } else {
      logTextPane.append(lastLogMessage);
    }

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        logTextPane.append(lastLogMessage);
        logTextPane.setCaretPosition(logTextPane.getDocument().getLength());
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    JScrollPane scrollPane = new JScrollPane(logTextPane);
    scrollPane.setPreferredSize(new Dimension(100,100));
    panel.add(BorderLayout.NORTH, new JLabel("Last log messages:"));
    panel.add(BorderLayout.CENTER, scrollPane);
    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public double energyConsumptionPerTick() {
    // Does not require energy
    return 0.0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}

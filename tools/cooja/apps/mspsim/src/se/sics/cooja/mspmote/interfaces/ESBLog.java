/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: ESBLog.java,v 1.4 2008/12/03 13:13:07 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.awt.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.mspsim.core.*;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.mspmote.ESBMote;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Serial port")
public class ESBLog extends Log implements USARTListener {
  private static Logger logger = Logger.getLogger(ESBLog.class);

  private Mote myMote;
  private String lastLogMessage = "";
  private String newMessage = "";

  public ESBLog(ESBMote mote) {
    myMote = mote;

    /* Listen to port writes */
    IOUnit usart = mote.getCPU().getIOUnit("USART 1");
    if (usart instanceof USART) {
      ((USART) usart).setUSARTListener(this);
    }
  }

  public String getLastLogMessage() {
    return lastLogMessage;
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BorderLayout());
    final JTextArea logTextPane = new JTextArea();
    logTextPane.setOpaque(false);
    logTextPane.setEditable(false);

    if (getLastLogMessage() == null) {
      logTextPane.setText("");
    } else {
      logTextPane.append(getLastLogMessage());
    }

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        logTextPane.append(getLastLogMessage());
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

  public double energyConsumption() {
    return 0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  public void dataReceived(USART source, int data) {
    newMessage += (char) data;
    if (data == '\n') {
      lastLogMessage = newMessage;
      newMessage = "";
      this.setChanged();
      this.notifyObservers(myMote);
    }
  }

  public void stateChanged(int state) {
  }
}

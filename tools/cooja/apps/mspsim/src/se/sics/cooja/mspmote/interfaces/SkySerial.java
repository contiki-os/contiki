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
 * $Id: SkySerial.java,v 1.2 2008/09/17 12:05:45 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.mspsim.core.*;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.interfaces.SerialPort;
import se.sics.cooja.mspmote.SkyMote;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Serial port")
public class SkySerial extends Log implements SerialPort, USARTListener {
  private static Logger logger = Logger.getLogger(SkySerial.class);

  private Mote myMote;
  private String lastLogMessage = "";
  private String newMessage = "";

  private JTextArea logTextPane = null;
  private USART usart;

  private class SerialDataObservable extends Observable {
    private void notifyNewData() {
      setChanged();
      notifyObservers(SkySerial.this);
    }
  }
  private SerialDataObservable serialDataObservable = new SerialDataObservable();
  private byte lastSerialData = 0;

  private Vector<Byte> incomingData = new Vector<Byte>();

  public SkySerial(SkyMote mote) {
    myMote = mote;

    /* Listen to port writes */
    IOUnit ioUnit = mote.getCPU().getIOUnit("USART 1");
    if (ioUnit instanceof USART) {
      usart = (USART) ioUnit;
      usart.setUSARTListener(this);
    }
  }

  public void writeByte(byte b) {
    incomingData.add(b);
  }

  public void writeString(String s) {
    for (int i=0; i < s.length(); i++) {
      writeByte((byte) s.charAt(i));
    }
    writeByte((byte) 10);
  }

  public void writeArray(byte[] s) {
    for (byte element : s) {
      writeByte(element);
    }
  }

  public String getLastLogMessages() {
    return lastLogMessage;
  }

  public void addSerialDataObserver(Observer o) {
    serialDataObservable.addObserver(o);
  }
  public void deleteSerialDataObserver(Observer o) {
    serialDataObservable.deleteObserver(o);
  }

  public byte getLastSerialData() {
    return lastSerialData;
  }

  public void doActionsBeforeTick() {
    /* Send bytes */
    if (!incomingData.isEmpty()) {
      if (usart.isReceiveFlagCleared()) {
        byte b = incomingData.remove(0);
        usart.byteReceived(b);
      }
    }
  }

  public void doActionsAfterTick() {
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BorderLayout());

    if (logTextPane == null) {
      logTextPane = new JTextArea();
    }

    // Send RS232 data visualizer
    JPanel sendPane = new JPanel();
    final JTextField sendTextField = new JTextField(15);
    JButton sendButton = new JButton("Send data");
    sendButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        writeString(sendTextField.getText());
      }
    });
    sendPane.add(BorderLayout.WEST, sendTextField);
    sendPane.add(BorderLayout.EAST, sendButton);

    // Receive RS232 data visualizer
    logTextPane.setOpaque(false);
    logTextPane.setEditable(false);

    if (getLastLogMessages() == null) {
      logTextPane.setText("");
    } else {
      logTextPane.append(getLastLogMessages());
    }

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        logTextPane.append("< " + getLastLogMessages() + "\n");
        logTextPane.setCaretPosition(logTextPane.getDocument().getLength());
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    JScrollPane scrollPane = new JScrollPane(logTextPane);
    scrollPane.setPreferredSize(new Dimension(100, 100));
    panel.add(BorderLayout.NORTH, new JLabel("Last serial data:"));
    panel.add(BorderLayout.CENTER, scrollPane);
    panel.add(BorderLayout.SOUTH, sendPane);
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
    return 0.0;
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

    lastSerialData = (byte) data;
    serialDataObservable.notifyNewData();
  }

  public void close() {
  }

  public void flushInput() {
  }

}

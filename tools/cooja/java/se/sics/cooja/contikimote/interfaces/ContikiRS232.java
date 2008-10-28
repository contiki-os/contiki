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
 * $Id: ContikiRS232.java,v 1.5 2008/10/28 12:55:20 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.PolledAfterActiveTicks;

/**
 * Serial port mote interface.
 * Only supports printable characters.
 * Note that by default the serial interface is not equal to the log interface:
 * printf(...) are not forwarded to this interface.
 *
 * Contiki variables:
 * <ul>
 * <li>char simSerialSendingFlag (1=mote is sending data)
 * <li>int simSerialSendingLength (length of data being sent from mote)
 * <li>byte[] simSerialSendingData (data being sent from mote)
 * <li>char simSerialRecevingFlag (1=mote is receving data)
 * <li>int simSerialReceivingLength (length of data being received at mote)
 * <li>byte[] simSerialReceivingData (data being received at mote)
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>rs232_interface
 * </ul>
 * <p>
 *
 * This observable notifies observers when a serial message is sent from the mote.
 *
 * @see #getSerialMessages()
 *
 * @author Fredrik Österlind
 */
@ClassDescription("Serial port (RS232)")
public class ContikiRS232 extends MoteInterface implements ContikiMoteInterface, PolledAfterActiveTicks {
  private static Logger logger = Logger.getLogger(ContikiRS232.class);

  private Mote mote = null;
  private SectionMoteMemory moteMem = null;

  private String lastSerialMessage = null;

  private JTextArea logTextPane = null;

  /**
   * Approximate energy consumption of every sent character over RS232 (mQ).
   */
  public final double ENERGY_CONSUMPTION_PER_CHAR_mQ;

  private double myEnergyConsumption = 0.0;

  /**
   * Creates an interface to the RS232 at mote.
   *
   * @param mote
   *          RS232's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiRS232(Mote mote) {
    // Read class configurations of this mote type
    ENERGY_CONSUMPTION_PER_CHAR_mQ = mote.getType().getConfig()
        .getDoubleValue(ContikiRS232.class, "CONSUMPTION_PER_CHAR_mQ");

    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"rs232_interface"};
  }

  public void doActionsAfterTick() {
    if (moteMem.getByteValueOf("simSerialSendingFlag") == 1) {
      int totalLength = moteMem.getIntValueOf("simSerialSendingLength");
      byte[] bytes = moteMem.getByteArray("simSerialSendingData", totalLength);
      char[] chars = new char[bytes.length];
      for (int i = 0; i < chars.length; i++) {
        chars[i] = (char) bytes[i];
      }

      myEnergyConsumption = ENERGY_CONSUMPTION_PER_CHAR_mQ * totalLength;

      String message = String.valueOf(chars);
      lastSerialMessage = message;

      moteMem.setByteValueOf("simSerialSendingFlag", (byte) 0);
      moteMem.setIntValueOf("simSerialSendingLength", 0);

      this.setChanged();
      this.notifyObservers(mote);
    } else {
      myEnergyConsumption = 0.0;
    }
  }

  /**
   * Returns all serial messages sent by mote the last tick that anything was
   * sent.
   *
   * @return Last serial messages sent by mote.
   */
  public String getSerialMessages() {
    return lastSerialMessage;
  }

  /**
   * Send a serial message to mote.
   *
   * @param message
   *          Message that mote should receive
   */
  public void sendSerialMessage(String message) {

    if (logTextPane != null) {
      logTextPane.append("> " + message + "\n");
    }

    // Flag for incoming data
    moteMem.setByteValueOf("simSerialReceivingFlag", (byte) 1);

    byte[] dataToAppend = message.getBytes();

    // Increase receiving size
    int oldSize = moteMem.getIntValueOf("simSerialReceivingLength");
    moteMem.setIntValueOf("simSerialReceivingLength", oldSize
        + dataToAppend.length);
    int newSize = moteMem.getIntValueOf("simSerialReceivingLength");

    // Write buffer characters
    byte[] oldData = moteMem.getByteArray("simSerialReceivingData", oldSize);
    byte[] newData = new byte[newSize];

    for (int i = 0; i < oldData.length; i++) {
      newData[i] = oldData[i];
    }

    for (int i = 0; i < message.length(); i++) {
      newData[i + oldSize] = dataToAppend[i];
    }

    moteMem.setByteArray("simSerialReceivingData", newData);

    mote.setState(Mote.State.ACTIVE);

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
        sendSerialMessage(sendTextField.getText());
      }
    });
    sendPane.add(BorderLayout.WEST, sendTextField);
    sendPane.add(BorderLayout.EAST, sendButton);

    // Receive RS232 data visualizer
    logTextPane.setOpaque(false);
    logTextPane.setEditable(false);

    if (lastSerialMessage == null) {
      logTextPane.setText("");
    } else {
      logTextPane.append(lastSerialMessage);
    }

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        logTextPane.append("< " + lastSerialMessage + "\n");
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

  public double energyConsumption() {
    return myEnergyConsumption;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}

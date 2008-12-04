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
 * $Id: SkySerial.java,v 1.8 2008/12/04 14:03:41 joxe Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.awt.*;
import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.TimeEvent;
import se.sics.mspsim.core.*;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.interfaces.SerialPort;
import se.sics.cooja.mspmote.SkyMote;

/**
 * @author Fredrik Österlind
 */
@ClassDescription("Serial port")
public class SkySerial extends Log implements SerialPort, USARTListener {
  private static Logger logger = Logger.getLogger(SkySerial.class);

  private Mote mote;
  private String lastLogMessage = "";
  private StringBuilder newMessage = new StringBuilder();

  private JTextArea logTextPane = null;
  private USART usart;
  private JTextField commandField;
  private String[] history = new String[50];
  private int historyPos = 0;
  private int historyCount = 0;

  private class SerialDataObservable extends Observable {
    private void notifyNewData() {
      if (this.countObservers() == 0) {
        return;
      }

      setChanged();
      notifyObservers(SkySerial.this);
    }
  }
  private SerialDataObservable serialDataObservable = new SerialDataObservable();
  private byte lastSerialData = 0;

  private Vector<Byte> incomingData = new Vector<Byte>();

  public SkySerial(SkyMote mote) {
    this.mote = mote;

    /* Listen to port writes */
    IOUnit ioUnit = mote.getCPU().getIOUnit("USART 1");
    if (ioUnit instanceof USART) {
      usart = (USART) ioUnit;
      usart.setUSARTListener(this);
    }
  }

  public void stateChanged(int state) {
    if (state == USARTListener.RXFLAG_CLEARED) {
      tryWriteNextByte();
    }
  }

  public void writeByte(byte b) {
    incomingData.add(b);
    tryWriteNextByte();
    mote.getSimulation().scheduleEvent(writeDataEvent, mote.getSimulation().getSimulationTime());
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

  public String getLastLogMessage() {
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

  private void tryWriteNextByte() {
    byte b;

    synchronized (incomingData) {
      if (!usart.isReceiveFlagCleared()) {
        return;
      }
      if (incomingData.isEmpty()) {
        return;
      }

      /* Write byte to serial port */
      b = incomingData.remove(0);
    }
    usart.byteReceived(b);
  }

  private TimeEvent writeDataEvent = new TimeEvent(0) {
    public void execute(long t) {
      /* TODO Implement MSPSim callback - better timing */
      tryWriteNextByte();
      if (!incomingData.isEmpty()) {
        mote.getSimulation().scheduleEvent(this, t+1);
      }
    }
  };

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BorderLayout());

    if (logTextPane == null) {
      logTextPane = new JTextArea();
    }

    // Send RS232 data visualizer
    JPanel sendPane = new JPanel(new BorderLayout());
    commandField = new JTextField(15);
    JButton sendButton = new JButton("Send data");
    ActionListener action = new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        String command = trim(commandField.getText());
        if (command != null) {
          try {
            int previous = historyCount - 1;
            if (previous < 0) {
              previous += history.length;
            }
            if (!command.equals(history[previous])) {
              history[historyCount] = command;
              historyCount = (historyCount + 1) % history.length;
            }
            historyPos = historyCount;
            addToLog("> " + command);
            writeString(command);
            commandField.setText("");
          } catch (Exception ex) {
            System.err.println("could not send '" + command + "':");
            ex.printStackTrace();
            JOptionPane.showMessageDialog(logTextPane,
                                          "could not send '" + command + "':\n"
                                          + ex, "ERROR",
                                          JOptionPane.ERROR_MESSAGE);
          }
        } else {
          commandField.getToolkit().beep();
        }
      }

    };
    commandField.addKeyListener(new KeyAdapter() {

      @Override
      public void keyPressed(KeyEvent e) {
        switch (e.getKeyCode()) {
        case KeyEvent.VK_UP: {
          int nextPos = (historyPos + history.length - 1) % history.length;
          if (nextPos == historyCount || history[nextPos] == null) {
            commandField.getToolkit().beep();
          } else {
            String cmd = trim(commandField.getText());
            if (cmd != null) {
              history[historyPos] = cmd;
            }
            historyPos = nextPos;
            commandField.setText(history[historyPos]);
          }
          break;
        }
        case KeyEvent.VK_DOWN: {
          int nextPos = (historyPos + 1) % history.length;
          if (nextPos == historyCount) {
            historyPos = nextPos;
            commandField.setText("");
          } else if (historyPos == historyCount || history[nextPos] == null) {
            commandField.getToolkit().beep();
          } else {
            String cmd = trim(commandField.getText());
            if (cmd != null) {
              history[historyPos] = cmd;
            }
            historyPos = nextPos;
            commandField.setText(history[historyPos]);
          }
          break;
        }
        }
      }

    });

    commandField.addActionListener(action);
    sendButton.addActionListener(action);
    sendPane.add(BorderLayout.CENTER, commandField);
    sendPane.add(BorderLayout.EAST, sendButton);

    // Receive RS232 data visualizer
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
        final String logMessage = getLastLogMessage();
        EventQueue.invokeLater(new Runnable() {
          public void run() {
            addToLog(logMessage);
          }
        });
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

  protected void addToLog(String text) {
    String current = logTextPane.getText();
    int len = current.length();
    if (len > 8192) {
      current = current.substring(len - 8192);
    }
    current = len > 0 ? (current + '\n' + text) : text;
    logTextPane.setText(current);
    logTextPane.setCaretPosition(current.length());
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
    if (data == '\n') {
      lastLogMessage = newMessage.toString();
      newMessage.setLength(0);
      this.setChanged();
      this.notifyObservers(mote);
    } else {
      newMessage.append((char) data);
    }
    lastSerialData = (byte) data;
    serialDataObservable.notifyNewData();
  }

  public void close() {
  }

  public void flushInput() {
  }

  private String trim(String text) {
    return (text != null) && ((text = text.trim()).length() > 0) ? text : null;
  }

}

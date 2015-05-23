/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 */

package org.contikios.cooja.dialogs;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JButton;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.Mote;
import org.contikios.cooja.interfaces.Log;
import org.contikios.cooja.interfaces.SerialPort;

public abstract class SerialUI extends Log implements SerialPort {
  private static Logger logger = Logger.getLogger(SerialUI.class);

  private final static int MAX_LENGTH = 1024;

  private byte lastSerialData = 0; /* SerialPort */
  private String lastLogMessage = ""; /* Log */
  private StringBuilder newMessage = new StringBuilder(); /* Log */

  /* Command history */
  private final static int HISTORY_SIZE = 15;
  private ArrayList<String> history = new ArrayList<String>();
  private int historyPos = -1;

  /* Log */
  public String getLastLogMessage() {
    return lastLogMessage;
  }

  /* SerialPort */
  private abstract class SerialDataObservable extends Observable {
    public abstract void notifyNewData();
  }
  private SerialDataObservable serialDataObservable = new SerialDataObservable() {
    public void notifyNewData() {
      if (this.countObservers() == 0) {
        return;
      }
      setChanged();
      notifyObservers(SerialUI.this);
    }
  };
  public void addSerialDataObserver(Observer o) {
    serialDataObservable.addObserver(o);
  }
  public void deleteSerialDataObserver(Observer o) {
    serialDataObservable.deleteObserver(o);
  }
  public byte getLastSerialData() {
    return lastSerialData;
  }
  public void dataReceived(int data) {
    if (data == '\n') {
      /* Notify observers of new log */
      lastLogMessage = newMessage.toString();
      lastLogMessage = lastLogMessage.replaceAll("[^\\p{Print}\\p{Blank}]", "");
      newMessage.setLength(0);
      this.setChanged();
      this.notifyObservers(getMote());
    } else {
      newMessage.append((char) data);
      if (newMessage.length() > MAX_LENGTH) {
        /*logger.warn("Dropping too large log message (>" + MAX_LENGTH + " bytes).");*/
        lastLogMessage = "# [1024 bytes, no line ending]: " + newMessage.substring(0, 20) + "...";
        lastLogMessage = lastLogMessage.replaceAll("[^\\p{Print}\\p{Blank}]", "");
        newMessage.setLength(0);
        this.setChanged();
        this.notifyObservers(getMote());
      }
    }

    /* Notify observers of new serial character */
    lastSerialData = (byte) data;
    serialDataObservable.notifyNewData();
  }


  /* Mote interface visualizer */
  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel(new BorderLayout());
    JPanel commandPane = new JPanel(new BorderLayout());

    final JTextArea logTextPane = new JTextArea();
    final JTextField commandField = new JTextField(15);
    JButton sendButton = new JButton("Send data");

    ActionListener sendCommandAction = new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        final String command = trim(commandField.getText());
        if (command == null) {
          commandField.getToolkit().beep();
          return;
        }

        try {
          /* Add to history */
          if (history.size() > 0 && command.equals(history.get(0))) {
            /* Ignored */
          } else {
            history.add(0, command);
            while (history.size() > HISTORY_SIZE) {
              history.remove(HISTORY_SIZE-1);
            }
          }
          historyPos = -1;

          appendToTextArea(logTextPane, "> " + command);
          commandField.setText("");
          if (getMote().getSimulation().isRunning()) {
            getMote().getSimulation().invokeSimulationThread(new Runnable() {
              public void run() {
                writeString(command);
              }
            });
          } else {
            writeString(command);
          }
        } catch (Exception ex) {
          logger.error("could not send '" + command + "':", ex);
          JOptionPane.showMessageDialog(
              logTextPane,
              "Could not send '" + command + "':\n" + ex.getMessage(), "Error sending message",
              JOptionPane.ERROR_MESSAGE);
        }
      }
    };
    commandField.addActionListener(sendCommandAction);
    sendButton.addActionListener(sendCommandAction);

    /* History */
    commandField.addKeyListener(new KeyAdapter() {
      public void keyPressed(KeyEvent e) {
        switch (e.getKeyCode()) {
        case KeyEvent.VK_UP: {
          historyPos++;
          if (historyPos >= history.size()) {
            historyPos = history.size() - 1;
            commandField.getToolkit().beep();
          }
          if (historyPos >= 0 && historyPos < history.size()) {
            commandField.setText(history.get(historyPos));
          } else {
            commandField.setText("");
          }
          break;
        }
        case KeyEvent.VK_DOWN: {
          historyPos--;
          if (historyPos < 0) {
            historyPos = -1;
            commandField.setText("");
            commandField.getToolkit().beep();
            break;
          }
          if (historyPos >= 0 && historyPos < history.size()) {
            commandField.setText(history.get(historyPos));
          } else {
            commandField.setText("");
          }
          break;
        }
        }
      }
    });

    commandPane.add(BorderLayout.CENTER, commandField);
    commandPane.add(BorderLayout.EAST, sendButton);

    logTextPane.setOpaque(false);
    logTextPane.setEditable(false);
    logTextPane.addKeyListener(new KeyAdapter() {
      public void keyPressed(KeyEvent e) {
        if ((e.getModifiers() & (MouseEvent.SHIFT_MASK|MouseEvent.CTRL_MASK)) != 0) {
          return;
        }
        commandField.requestFocusInWindow();
      }
    });

    /* Mote interface observer */
    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        final String logMessage = getLastLogMessage();
        EventQueue.invokeLater(new Runnable() {
          public void run() {
            appendToTextArea(logTextPane, logMessage);
          }
        });
      }
    });
    panel.putClientProperty("intf_obs", observer);

    JScrollPane scrollPane = new JScrollPane(logTextPane);
    scrollPane.setPreferredSize(new Dimension(100, 100));
    panel.add(BorderLayout.CENTER, scrollPane);
    panel.add(BorderLayout.SOUTH, commandPane);
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

  private static final String HISTORY_SEPARATOR = "~;";
  public Collection<Element> getConfigXML() {
    StringBuilder sb = new StringBuilder();
    for (String s: history) {
      if (s == null) {
        continue;
      }
      sb.append(s + HISTORY_SEPARATOR);
    }
    if (sb.length() == 0) {
      return null;
    }

    ArrayList<Element> config = new ArrayList<Element>();
    Element element = new Element("history");
    element.setText(sb.toString());
    config.add(element);

    return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("history")) {
        String[] history = element.getText().split(HISTORY_SEPARATOR);
        for (String h: history) {
          this.history.add(h);
        }
        historyPos = -1;
      }
    }
  }

  public void close() {
  }

  public void flushInput() {
  }

  public abstract Mote getMote();


  protected static void appendToTextArea(JTextArea textArea, String text) {
    String current = textArea.getText();
    int len = current.length();
    if (len > 8192) {
      current = current.substring(len - 8192);
    }
    current = len > 0 ? (current + '\n' + text) : text;
    textArea.setText(current);
    textArea.setCaretPosition(current.length());

    Rectangle visRect = textArea.getVisibleRect();
    if (visRect.x > 0) {
      visRect.x = 0;
      textArea.scrollRectToVisible(visRect);
    }
  }

  private static String trim(String text) {
    return (text != null) && ((text = text.trim()).length() > 0) ? text : null;
  }
}

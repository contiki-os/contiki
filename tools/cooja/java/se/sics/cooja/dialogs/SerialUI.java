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
   * $Id: SerialUI.java,v 1.7 2010/10/07 13:09:28 joxe Exp $
   */

package se.sics.cooja.dialogs;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import org.jdom.Element;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import org.apache.log4j.Logger;

import se.sics.cooja.Mote;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.interfaces.SerialPort;
import se.sics.cooja.plugins.SLIP;

public abstract class SerialUI extends Log implements SerialPort {
  private static Logger logger = Logger.getLogger(SerialUI.class);

  private final static byte SLIP_END = (byte)0300;
  private final static byte SLIP_ESC = (byte)0333;
  private final static byte SLIP_ESC_END = (byte)0334;
  private final static byte SLIP_ESC_ESC = (byte)0335;

  
  private final static int MAX_LENGTH = 1024;

  private String lastLogMessage = "";
  private StringBuilder newMessage = new StringBuilder();

  private JTextArea logTextPane = null;
  private JTextField commandField;
  private JCheckBox slipCheckbox;
  private String[] history = new String[50];
  private int historyPos = 0;
  private int historyCount = 0;

  private class SerialDataObservable extends Observable {
      private void notifyNewData() {
        if (this.countObservers() == 0) {
          return;
        }
        setChanged();
        notifyObservers(SerialUI.this);
      }
    }
  
  private SerialDataObservable serialDataObservable = new SerialDataObservable();
  private byte lastSerialData = 0;

  public String getLastLogMessage() {
    return lastLogMessage;
  }

  public abstract Mote getMote();
  
  public void addSerialDataObserver(Observer o) {
    serialDataObservable.addObserver(o);
  }
  
  public void deleteSerialDataObserver(Observer o) {
    serialDataObservable.deleteObserver(o);
  }

  public byte getLastSerialData() {
    return lastSerialData;
  }

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

            if (slipCheckbox.isSelected()) {
              addToLog("SLIP> " + command);
              command += "\n";
              writeArray(SLIP.asSlip(command.getBytes()));
            } else {
              addToLog("> " + command);
              writeString(command);
            }
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

    slipCheckbox = new JCheckBox("", false);
    slipCheckbox.setToolTipText("Wrap data as SLIP");

    commandField.addActionListener(action);
    sendButton.addActionListener(action);
    sendPane.add(BorderLayout.WEST, slipCheckbox);
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
    
    Rectangle visRect = logTextPane.getVisibleRect();
    if (visRect.x > 0) {
      visRect.x = 0;
      logTextPane.scrollRectToVisible(visRect);
    }
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  private int tosChars = 0;
  boolean tosMode = false;
  private int tosPos = 0;
  private int tosLen = 0;

  private int slipCounter = 0;
  private int totalTOSChars = 0;
  
  public void dataReceived(int data) {
    if (data == SLIP_END || data == SLIP_ESC) {
        slipCounter++;
    }
    if (tosMode) {
      int tmpData = data;
      /* needs to add checks to CRC */
      //    System.out.println("Received: " + Integer.toString(data, 16) + " = " + (char) data + "  tosPos: " + tosPos);
      if (data == 0x7e) {
        if (tosPos > 6) {
          lastLogMessage = "TinyOS: " + newMessage.toString();
          newMessage.setLength(0);
          this.setChanged();
          this.notifyObservers(getMote());
          // System.out.println("*** Printing TOS String: " + lastLogMessage);
          tosPos = 0;
          tosLen = 0;
        } else {
          /* start of new message! */
          tosPos = 0;
          tosLen = 0;
        }
      }
      if (tosPos == 7) {
        tosLen = data;
        // System.out.println("TOS Payload len: " + tosLen);
      }
      if (tosPos > 9 && tosPos < 10 + tosLen) {
        if (data < 32) {
          tmpData = 32;
        }
        newMessage.append((char) tmpData);
      }
    } else {
      if (data == 0x7e) {
        tosChars++;
        totalTOSChars = 0; /* XXX Disabled TOS mode due to error */
        totalTOSChars++;
        if (tosChars == 2) {
          if (totalTOSChars > slipCounter) {
              tosMode = true;
              tosMode = false; /* XXX Disabled TOS mode due to error */
              /* already read one char here */
              tosPos = 1;
          } else {
              tosChars = 0;
          }
        }
      } else {
        tosChars = 0;
      }
      if (data == '\n') {
        lastLogMessage = newMessage.toString();
        lastLogMessage = lastLogMessage.replaceAll("[^\\p{Print}]", ""); 
        newMessage.setLength(0);
        this.setChanged();
        this.notifyObservers(getMote());
      } else {
        newMessage.append((char) data);
        if (newMessage.length() > MAX_LENGTH) {
          /*logger.warn("Dropping too large log message (>" + MAX_LENGTH + " bytes).");*/
        	lastLogMessage = "# [1024 bytes binary data]";
          this.setChanged();
          this.notifyObservers(getMote());
          newMessage.setLength(0);
        }
      }
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

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
 * $Id: SerialConsole.java,v 1.1 2010/11/03 14:53:05 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * SerialConsole
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 4 jul 2008
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class SerialConsole implements Visualizer {

  private final CollectServer server;
  private final String category;
  private JPanel panel;
  private JTextArea logArea;
  private JTextField commandField;
  private String[] history = new String[50];
  private int historyPos = 0;
  private int historyCount = 0;

  public SerialConsole(CollectServer server, String category) {
    this.server = server;
    this.category = category;
    panel = new JPanel(new BorderLayout());
    logArea = new JTextArea(4, 30);
    logArea.setEditable(false);
    panel.add(new JScrollPane(logArea), BorderLayout.CENTER);

    JPopupMenu popupMenu = new JPopupMenu();
    JMenuItem clearItem = new JMenuItem("Clear");
    clearItem.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        logArea.setText("");
      }

    });
    popupMenu.add(clearItem);
    logArea.setComponentPopupMenu(popupMenu);

    commandField = new JTextField();
    commandField.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        String command = trim(commandField.getText());
        if (command != null) {
          try {
            int previous = historyCount - 1;
            if (previous < 0) previous += history.length;
            if (!command.equals(history[previous])) {
              history[historyCount] = command;
              historyCount = (historyCount + 1) % history.length;
            }
            historyPos = historyCount;
            if (SerialConsole.this.server.sendToNode(command)) {
              commandField.setText("");
            } else {
              addSerialData("*** failed to send command ***");
            }
          } catch (Exception ex) {
            System.err.println("could not send '" + command + "':");
            ex.printStackTrace();
            JOptionPane.showMessageDialog(panel,
                                          "could not send '" + command + "':\n"
                                          + ex, "ERROR",
                                          JOptionPane.ERROR_MESSAGE);
          }
        } else {
          commandField.getToolkit().beep();
        }
      }

    });
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
    panel.add(commandField, BorderLayout.SOUTH);
  }

  @Override
  public Component getPanel() {
    return panel;
  }

  @Override
  public String getCategory() {
    return category;
  }

  @Override
  public String getTitle() {
    return "Serial Console";
  }

  @Override
  public void nodeAdded(Node node) {
    // Ignore
  }

  @Override
  public void nodeDataReceived(SensorData sensorData) {
    // Ignore
  }

  @Override
  public void clearNodeData() {
    // Ignore
  }

  @Override
  public void nodesSelected(Node[] node) {
    // Ignore
  }

  public void addSerialData(final String text) {
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        String current = logArea.getText();
        int len = current.length();
        if (len > 4096) {
          current = current.substring(len - 4096);
        }
        current = len > 0 ? (current + '\n' + text) : text;
        logArea.setText(current);
        logArea.setCaretPosition(current.length());
      }
    });
  }

  private String trim(String text) {
    return (text != null) && ((text = text.trim()).length() > 0) ? text : null;
  }

}

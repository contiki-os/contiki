/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: NodeControl.java,v 1.1 2010/11/03 14:53:05 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * NodeControl
 *
 * Authors : Niclas Finne
 * Created : 27 sep 2010
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JTextPane;
import javax.swing.border.LineBorder;

import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class NodeControl implements Visualizer {

  private final static String SET_TIME_COMMAND = "time %TIME% | null";

  private final CollectServer server;
  private final String category;
  private final JPanel panel;
  private final JLabel statusLabel;
  private final JSeparator statusSeparator;

  public NodeControl(CollectServer server, String category) {
    this.server = server;
    this.category = category;
    this.panel = new JPanel(new BorderLayout());

    final JFormattedTextField intervalField = new JFormattedTextField(new Integer(60));
    final JFormattedTextField randomField = new JFormattedTextField(new Integer(60));
    final JFormattedTextField reportsField = new JFormattedTextField(new Integer(0));
    final JFormattedTextField rexmitsField = new JFormattedTextField(new Integer(31));
    statusLabel = new JLabel("", JLabel.CENTER);
    statusLabel.setOpaque(true);
    statusLabel.setBackground(Color.white);
    statusLabel.setBorder(LineBorder.createBlackLineBorder());
    statusLabel.setVisible(false);
    statusSeparator = new JSeparator();
    statusSeparator.setVisible(false);

    JButton stopButton = createCommandButton("Send stop to nodes", "netcmd killall");

    JButton sendButton = new JButton("Send command to nodes");
    sendButton.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        int interval = (Integer)intervalField.getValue();
        int random = (Integer)randomField.getValue();
        int reports = (Integer)reportsField.getValue();
        int rexmits = (Integer)rexmitsField.getValue();

        sendCommand("netcmd { repeat " + reports + " " + interval
            + " { randwait " + random + " collect-view-data | send " + rexmits + " } }");
      }

    });

    JButton collectButton = createCommandButton("Start Collect",
                                                "~K", "killall",
                                                "mac 0", SET_TIME_COMMAND,
                                                "collect | timestamp | binprint &");
    JButton stopCollectButton = createCommandButton("Stop Collect", "~K", "killall");

    JPanel controlPanel = new JPanel(new GridBagLayout());

    GridBagConstraints c = new GridBagConstraints();
    c.fill = GridBagConstraints.HORIZONTAL;
    c.weightx = 0.5;
    c.insets.left = c.insets.right = c.insets.bottom = 3;
    c.anchor = GridBagConstraints.WEST;
    c.gridy = 0;

    c.gridwidth = 3;
    controlPanel.add(statusLabel, c);
    c.gridy++;
    controlPanel.add(statusSeparator, c);
    c.insets.top = 10;

    c.gridy++;
    c.gridwidth = 1;
    controlPanel.add(new JLabel("Program Connected Nodes", JLabel.RIGHT), c);
    c.gridwidth = 3;
    c.fill = GridBagConstraints.NONE;
    controlPanel.add(new JButton(server.getMoteProgramAction()), c);
    c.fill = GridBagConstraints.HORIZONTAL;

    c.gridy++;
    c.gridwidth = 1;
    controlPanel.add(new JLabel("Serial Connection", JLabel.RIGHT), c);
    c.gridwidth = 3;
    c.fill = GridBagConstraints.NONE;
    controlPanel.add(new JButton(server.getConnectSerialAction()), c);
    c.fill = GridBagConstraints.HORIZONTAL;

    c.gridy++;
    controlPanel.add(new JSeparator(), c);

    c.gridy++;
    c.gridwidth = 1;
    controlPanel.add(new JLabel("Base Station Control", JLabel.RIGHT), c);
    c.gridwidth = 2;
    JPanel basePanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 5, 5));
    basePanel.add(collectButton);
    basePanel.add(stopCollectButton);
    c.insets.left -= 5;
    controlPanel.add(basePanel, c);
    c.insets.left += 5;

    c.gridy++;
    c.gridwidth = 3;
    controlPanel.add(new JSeparator(), c);

    c.gridy++;
    JLabel label = new JLabel("Collect Settings", JLabel.CENTER);
    controlPanel.add(label, c);
    c.gridwidth = 1;

    c.gridy++;
    controlPanel.add(label = new JLabel("Report interval", JLabel.RIGHT), c);
    label.setLabelFor(intervalField);
    controlPanel.add(intervalField, c);
    controlPanel.add(new JLabel("seconds"), c);

    c.insets.top = 3;
    c.gridy++;
    controlPanel.add(label = new JLabel("Report randomness", JLabel.RIGHT), c);
    label.setLabelFor(randomField);
    controlPanel.add(randomField, c);
    controlPanel.add(new JLabel("seconds"), c);

    c.gridy++;
    controlPanel.add(label = new JLabel("Hop-by-hop retransmissions", JLabel.RIGHT), c);
    label.setLabelFor(rexmitsField);
    controlPanel.add(rexmitsField, c);
    controlPanel.add(new JLabel("retransmissions (0 - 31)"), c);

    c.gridy++;
    controlPanel.add(new JLabel("Number of reports", JLabel.RIGHT), c);
    label.setLabelFor(reportsField);
    controlPanel.add(reportsField, c);
    controlPanel.add(new JLabel("(0 = report forever)"), c);

    c.gridy++;
    c.gridwidth = 3;
    c.insets.bottom = 10;
    JPanel nodePanel = new JPanel();
    nodePanel.add(sendButton);
    nodePanel.add(stopButton);
    controlPanel.add(nodePanel, c);

    c.gridy++;
    controlPanel.add(new JSeparator(), c);
    panel.add(controlPanel, BorderLayout.NORTH);

    JTextPane helpPane = new JTextPane();
    helpPane.setContentType("text/html");
    helpPane.setEditable(false);
    helpPane.setText("<html>" +
 "<h3>Quick Startup Instructions</h3>" +
 "<lu>" +
 "<li> Connect nodes to USB. Press the <strong>Program Nodes...</strong> button." +
 "<li> Disconnect all except one node. " +
 "Press the <strong>Connect to Serial</strong> button." +
 "<li> Press the <strong>Start Collect</strong> button." +
 "<li> Press the <strong>Send command to nodes</strong> button." +
 "</lu>" + 
 "</html>");
    helpPane.setBackground(panel.getBackground());
    JScrollPane helpScroll = new JScrollPane(helpPane,
        JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
        JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    helpScroll.setBorder(BorderFactory.createEmptyBorder(3, 10, 10, 10));
    panel.add(helpScroll, BorderLayout.CENTER);
  }

  private JButton createCommandButton(String name, final String... command) {
    JButton button = new JButton(name);
    button.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        try {
          // TODO Should use separate thread to send commands
          panel.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
          for(int i = 0, n = command.length; i < n; i++) {
            if (i > 0) {
              try {
                // Do not send multiple commands too fast
                Thread.sleep(1000);
              } catch (InterruptedException e1) {
              }
            }
            String cmd = command[i];
            if (cmd == SET_TIME_COMMAND) {
              cmd = "time " + (System.currentTimeMillis() / 1000) + " | null";
            }
            if (!sendCommand(cmd)) {
              break;
            }
          }
        } finally {
          panel.setCursor(Cursor.getDefaultCursor());
        }
      }

    });
    return button;
  }

  protected boolean sendCommand(String command) {
    if (server.sendToNode(command)) {
      setStatus("Sent command '" + command + "'", false);
      return true;
    }
    setStatus("Failed to send command. No serial connection.", true);
    return false;
  }

  private void setStatus(String text, boolean isWarning) {
    statusLabel.setForeground(isWarning ? Color.red : Color.black);
    statusLabel.setText(text);
    statusLabel.setVisible(true);
    statusSeparator.setVisible(true);
  }

  public String getCategory() {
    return category;
  }

  public String getTitle() {
    return "Node Control";
  }

  public Component getPanel() {
    return panel;
  }

  public void nodesSelected(Node[] node) {
  }

  public void nodeAdded(Node node) {
  }

  public void nodeDataReceived(SensorData sensorData) {
  }

  public void clearNodeData() {
  }

}

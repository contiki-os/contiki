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
 * $Id: NodeControl.java,v 1.3 2010/10/03 20:19:37 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * NodeControl
 *
 * Authors : Niclas Finne
 * Created : 27 sep 2010
 * Updated : $Date: 2010/10/03 20:19:37 $
 *           $Revision: 1.3 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.LineBorder;

import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class NodeControl implements Visualizer {

  private final CollectServer server;
  private final String category;
  private final JPanel panel;
  private final JLabel statusLabel;

  public NodeControl(CollectServer server, String category) {
    this.server = server;
    this.category = category;
    this.panel = new JPanel(new BorderLayout());

    final JFormattedTextField intervalField = new JFormattedTextField(new Integer(60));
    final JFormattedTextField randomField = new JFormattedTextField(new Integer(2));
    final JFormattedTextField reportsField = new JFormattedTextField(new Integer(0));
    final JFormattedTextField rexmitsField = new JFormattedTextField(new Integer(15));
    statusLabel = new JLabel("", JLabel.CENTER);
    statusLabel.setOpaque(true);

    JButton stopButton = new JButton("Send stop to nodes");
    stopButton.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        sendCommand("netcmd killall");
      }

    });

    JButton sendButton = new JButton("Send command to nodes");
    sendButton.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        int interval = (Integer)intervalField.getValue();
        int random = (Integer)randomField.getValue();
        int reports = (Integer)reportsField.getValue();
        int rexmits = (Integer)rexmitsField.getValue();

        sendCommand("netcmd { repeat " + reports + " " + interval
            + " { randwait " + random + " sky-alldata | blink | send " + rexmits + " } }");
      }

    });

    JPanel controlPanel = new JPanel(new GridBagLayout());

    GridBagConstraints c = new GridBagConstraints();
    c.fill = GridBagConstraints.HORIZONTAL;
    c.weightx = 0.5;
    c.insets.left = c.insets.right = c.insets.bottom = 3;
    c.insets.top = 10;
    c.gridy = 0;

    c.gridwidth = 3;
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
    c.gridx = 1;
    c.weightx = 0;
    c.fill = GridBagConstraints.NONE;
    c.insets.bottom = 50;
    controlPanel.add(sendButton, c);

    c.gridx = 0;
    c.gridy++;
    c.gridwidth = 3;
    c.ipadx = c.ipady = 6;
    controlPanel.add(statusLabel, c);

    panel.add(controlPanel, BorderLayout.NORTH);

    controlPanel = new JPanel();
    controlPanel.add(stopButton);
    panel.add(controlPanel, BorderLayout.SOUTH);
  }

  protected void sendCommand(String command) {
    statusLabel.setBackground(Color.white);
    statusLabel.setBorder(LineBorder.createBlackLineBorder());
    if (server.sendToNode(command)) {
      statusLabel.setForeground(Color.black);
      statusLabel.setText("Sent command '" + command + "'");
    } else {
      statusLabel.setForeground(Color.red);
      statusLabel.setText("Failed to send command. No serial connection.");
    }
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

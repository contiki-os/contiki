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
 * $Id: NodeInfoPanel.java,v 1.3 2010/09/14 10:38:12 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * NodeInfoPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 6 sep 2010
 * Updated : $Date: 2010/09/14 10:38:12 $
 *           $Revision: 1.3 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.SensorDataAggregator;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class NodeInfoPanel extends JPanel implements Visualizer {

  private static final long serialVersionUID = -1060893468047793431L;

  private JTextArea infoArea;
  private Node[] selectedNodes;

  public NodeInfoPanel() {
    super(new BorderLayout());
    infoArea = new JTextArea(4, 30);
    infoArea.setEditable(false);
    add(new JScrollPane(infoArea), BorderLayout.CENTER);
  }

  @Override
  public Component getPanel() {
    return this;
  }

  @Override
  public String getTitle() {
    return "Node Info";
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
  public void nodesSelected(Node[] nodes) {
    this.selectedNodes = nodes;
    if (isVisible()) {
      updateInfoArea();
    }
  }

  private StringBuilder addTime(StringBuilder sb, long time) {
    time /= 1000;
    if (time > 24 * 60 * 60) {
      long days = time / (24 * 60 * 60);
      sb.append(days).append(" days, ");
      time -= days * 24 * 60 * 60;
    }
    if (time > 60 * 60) {
      long hours = time / (60 * 60);
      sb.append(hours).append(" hours, ");
      time -= hours * 60 * 60;
    }
    sb.append(time / 60).append(" min, ").append(time % 60).append(" sec");
    return sb;
  }

  private void updateInfoArea() {
    StringBuilder sb = new StringBuilder();
    if (selectedNodes != null) {
      for(Node node : selectedNodes) {
        SensorDataAggregator sda = node.getSensorDataAggregator();
        long longest = sda.getLongestPeriod();
        sb.append("Node " + node.getName() + '\n'
            + "  Packets Received: \t" + sda.getPacketCount() + '\n'
            + "  Duplicates:       \t" + sda.getDuplicateCount() + '\n');
        if (longest > 0) {
          sb.append("  Average period:\t");
          addTime(sb, sda.getAveragePeriod()).append('\n');
          sb.append("  Shortest period:\t");
          addTime(sb, sda.getShortestPeriod()).append('\n');
          sb.append("  Longest period:\t");
          addTime(sb, longest).append('\n');
        }
        sb.append("--------------------------------------------------------\n");
      }
    }
    infoArea.setText(sb.toString());
  }

  public void setVisible(boolean visible) {
    if (visible) {
      updateInfoArea();
    } else {
      infoArea.setText("");
    }
    super.setVisible(visible);
  }

}

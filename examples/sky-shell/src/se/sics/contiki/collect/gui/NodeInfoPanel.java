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
 * $Id: NodeInfoPanel.java,v 1.5 2010/09/15 16:15:10 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * NodeInfoPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 6 sep 2010
 * Updated : $Date: 2010/09/15 16:15:10 $
 *           $Revision: 1.5 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.MouseEvent;

import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.JTableHeader;

import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class NodeInfoPanel extends JPanel implements Visualizer {

  private static final long serialVersionUID = -1060893468047793431L;

  private final CollectServer server;
  private final String category;
  private final JTable nodeTable;
  private final NodeModel nodeModel;
  private Node[] nodes;

  public NodeInfoPanel(CollectServer server, String category) {
    super(new BorderLayout());
    this.server = server;
    this.category = category;
    
    nodeModel = new NodeModel();
    nodeTable = new JTable(nodeModel) {
      private static final long serialVersionUID = 1L;

      public Dimension getPreferredScrollableViewportSize() {
        return getPreferredSize();
      } 

      protected JTableHeader createDefaultTableHeader() {
          return new JTableHeader(columnModel) {
            private static final long serialVersionUID = 1L;

            public String getToolTipText(MouseEvent e) {
              int index = columnModel.getColumnIndexAtX(e.getPoint().x);
              int realIndex = index < 0 ? index : columnModel.getColumn(index).getModelIndex();
              return realIndex < 0 ? null : nodeModel.getColumnToolTip(realIndex);
            }
          };
      }
    };
    nodeTable.setFillsViewportHeight(true);
    nodeTable.setAutoCreateRowSorter(true);

    // Add right aligned renderer for node name
    DefaultTableCellRenderer renderer = new DefaultTableCellRenderer();
    renderer.setHorizontalAlignment(JLabel.RIGHT);
    nodeTable.setDefaultRenderer(Node.class, renderer);

    // Add renderer for time
    renderer = new DefaultTableCellRenderer() {
      private static final long serialVersionUID = 1L;

      public void setValue(Object value) {
        long time = (Long) value;
        setText(time > 0 ? getTimeAsString(time) : null);
      }
    };
    nodeTable.setDefaultRenderer(Long.class, renderer);

    // Add renderer for double
    renderer = new DefaultTableCellRenderer() {
      private static final long serialVersionUID = 1L;

      public void setValue(Object value) {
        if (value == null) {
          setText(null);
        }
        double v = (Double) value + 0.0005;
        int dec = ((int)(v * 1000)) % 1000;
        setText((long)v + "." + (dec > 99 ? "" : "0") + (dec > 9 ? "" : "0") + dec);
      }
    };
    renderer.setHorizontalAlignment(JLabel.RIGHT);
    nodeTable.setDefaultRenderer(Double.class, renderer);

    add(new JScrollPane(nodeTable), BorderLayout.CENTER);
  }

  @Override
  public Component getPanel() {
    return this;
  }

  @Override
  public String getCategory() {
    return category;
  }

  @Override
  public String getTitle() {
    return "Node Info";
  }

  @Override
  public void nodeAdded(Node node) {
    updateNodes();
  }

  @Override
  public void nodeDataReceived(SensorData sensorData) {
    updateNode(sensorData.getNode());
  }

  @Override
  public void clearNodeData() {
    updateNodes();
  }

  @Override
  public void nodesSelected(Node[] nodes) {
    // Ignore
  }

  @Override
  public void setVisible(boolean visible) {
    if (visible) {
      updateNodes();
    } else {
      clearNodes();
    }
    super.setVisible(visible);
  }

  private void clearNodes() {
    if (this.nodes != null && this.nodes.length > 0) {
      nodeModel.fireTableRowsDeleted(0, this.nodes.length - 1);
    }
    this.nodes = null;
  }

  private void updateNodes() {
    clearNodes();
    this.nodes = server.getNodes();
    if (this.nodes != null && this.nodes.length > 0) {
      nodeModel.fireTableRowsInserted(0, this.nodes.length - 1);
    }
  }

  private void updateNode(Node node) {
    if (this.nodes != null) {
      for(int row = 0; row < this.nodes.length; row++) {
        if (this.nodes[row] == node) {
          nodeModel.fireTableRowsUpdated(row, row);
          break;
        }
      }
    }
  }

  private String getTimeAsString(long time) {
    StringBuilder sb = new StringBuilder();
    time /= 1000;
    if (time > 24 * 60 * 60) {
      long days = time / (24 * 60 * 60);
      sb.append(days).append(days > 1 ? " days, " : " day, ");
      time -= days * 24 * 60 * 60;
    }
    if (time > 60 * 60) {
      long hours = time / (60 * 60);
      sb.append(hours).append(hours > 1 ? " hours, " : " hour, ");
      time -= hours * 60 * 60;
    }
    sb.append(time / 60).append(" min, ").append(time % 60).append(" sec");
    return sb.toString();
  }

  private class NodeModel extends AbstractTableModel {

    private static final long serialVersionUID = 1692207305977527004L;

    private final String[] COLUMN_NAMES = {
        "Node", "Node",
        "Packets", "Packets Received",
        "Duplicates", "Duplicate Packets Received",
        "Estimated Lost", "Estimated Lost Packets",
        "Average Hops", "Average Hops to Sink",
        "Next Hop Changes", "Next Hop Change Count",
        "Average Inter-packet time", "Average Inter-packet Time",
        "Shortest Inter-packet Time", "Shortest Inter-packet Time",
        "Longest Inter-packet Time", "Longest Inter-packet Time"
    };
    private final Class<?>[] COLUMN_TYPES = {
        Node.class,
        Integer.class,
        Integer.class,
        Integer.class,
        Double.class,
        Integer.class,
        Long.class,
        Long.class,
        Long.class
    };

    public Object getValueAt(int row, int col) {
      Node node = nodes[row];
      switch (col) {
      case 0:
        return node;
      case 1:
        return node.getSensorDataAggregator().getPacketCount();
      case 2:
        return node.getSensorDataAggregator().getDuplicateCount();
      case 3:
        return node.getSensorDataAggregator().getEstimatedLostCount();
      case 4:
        return node.getSensorDataAggregator().getAverageValue(SensorData.HOPS);
      case 5:
        return node.getSensorDataAggregator().getNextHopChangeCount();
      case 6:
        return node.getSensorDataAggregator().getAveragePeriod();
      case 7: {
        long time = node.getSensorDataAggregator().getShortestPeriod();
        return time < Long.MAX_VALUE ? time : 0;
      }
      case 8:
        return node.getSensorDataAggregator().getLongestPeriod();
      default:
        return null;
      }
    }

    public Class<?> getColumnClass(int c) {
      return COLUMN_TYPES[c];
    }

    public String getColumnName(int col) {
      return COLUMN_NAMES[col * 2];
    }

    public String getColumnToolTip(int col) {
      return COLUMN_NAMES[col * 2 + 1];
    }

    public int getColumnCount() {
      return COLUMN_NAMES.length / 2;
    }

    public int getRowCount() {
      return nodes == null ? 0 : nodes.length;
    }

  }
}

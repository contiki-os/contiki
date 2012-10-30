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
 *
 * -----------------------------------------------------------------
 *
 * NodeInfoPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 6 sep 2010
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.Comparator;
import java.util.Properties;

import javax.swing.AbstractAction;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.SwingUtilities;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableRowSorter;

import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Configurable;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.SensorInfo;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class NodeInfoPanel extends JPanel implements Visualizer, Configurable {

  private static final long serialVersionUID = -1060893468047793431L;

  private static Comparator<Number> NUMBER_COMPARATOR = new Comparator<Number>() {

    public int compare(Number o1, Number o2) {
      double v1 = o1.doubleValue();
      double v2 = o2.doubleValue();
      return (v1 < v2 ? -1 : (v1 == v2 ? 0 : 1));
    }

  };

  private final CollectServer server;
  private final String category;
  private final JTable table;
  private final NodeModel nodeModel;

  @SuppressWarnings("serial")
  public NodeInfoPanel(CollectServer server, String category) {
    super(new BorderLayout());
    this.server = server;
    this.category = category;

    TableData[] columns = new TableData[] {
        new TableData("Node", Node.class) {
          public Object getValue(Node node) {
            return node;
          }
        },
        new TableData("Received", "Packets Received", Number.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getPacketCount();
          }
        },
        new TableData("Dups", "Duplicate Packets Received", Number.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getDuplicateCount();
          }
        },
        new TableData("Lost", "Estimated Lost Packets", Number.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getEstimatedLostCount();
          }
        },
        new TableData("Hops", "Average Hops to Sink", Double.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getAverageValue(SensorData.HOPS);
          }
        },
        new TableData("Rtmetric", "Average Routing Metric", Double.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getAverageRtmetric();
          }
        },
        new TableData("ETX", "Average ETX to Next Hop", Double.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getAverageBestNeighborETX();
          }
        },
        new TableData("Churn", "Next Hop Change Count", Number.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getNextHopChangeCount();
          }
        },
        new TableData("Beacon Interval", "Average Beacon Interval", Long.class) {
          public Object getValue(Node node) {
            return (long)(node.getSensorDataAggregator().getAverageValue(SensorData.BEACON_INTERVAL) * 1000);
          }
        },

        new TableData("Reboots", "Estimated Node Restart Count", Number.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getEstimatedRestarts();
          }
        },

        // Power
        new TableData("CPU Power", "Average CPU Power Consumption", Double.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getCPUPower();
          }
        },
        new TableData("LPM Power", "Average LPM Power Consumption", Double.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getLPMPower();
          }
        },
        new TableData("Listen Power", "Average Radio Listen Power Consumption", Double.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getListenPower();
          }
        },
        new TableData("Transmit Power", "Average Radio Transmit Power Consumption", Double.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getTransmitPower();
          }
        },
        new TableData("Power", "Average Power Consumption", Double.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getAveragePower();
          }
        },
        new TableData("On-time", "Power Measure Time", Long.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getPowerMeasureTime();
          }
        },

        new TableData("Listen Duty Cycle", "Average Radio Listen Duty Cycle (%)", Double.class) {
          public Object getValue(Node node) {
            return 100 * node.getSensorDataAggregator().getAverageDutyCycle(SensorInfo.TIME_LISTEN);
          }
        },
        new TableData("Transmit Duty Cycle", "Average Radio Transmit Duty Cycle (%)", Double.class) {
          public Object getValue(Node node) {
            return 100 * node.getSensorDataAggregator().getAverageDutyCycle(SensorInfo.TIME_TRANSMIT);
          }
        },

        // Inter-packet times
        new TableData("Avg Inter-packet Time", Long.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getAveragePeriod();
          }
        },
        new TableData("Min Inter-packet Time", Long.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getShortestPeriod();
          }
        },
        new TableData("Max Inter-packet Time", Long.class) {
          public Object getValue(Node node) {
            return node.getSensorDataAggregator().getLongestPeriod();
          }
        }
    };
    nodeModel = new NodeModel(columns);
    table = new JTable(nodeModel) {
      private static final long serialVersionUID = 1L;
      private Font fontForAverage;

      protected JTableHeader createDefaultTableHeader() {
          return new JTableHeader(columnModel) {
            private static final long serialVersionUID = 1L;

            public String getToolTipText(MouseEvent e) {
              int index = columnModel.getColumnIndexAtX(e.getX());
              int modelIndex = index < 0 ? index : columnModel.getColumn(index).getModelIndex();
              return modelIndex < 0 ? null : nodeModel.getColumnToolTip(modelIndex);
            }
          };
      }

      public Component prepareRenderer(TableCellRenderer renderer, int rowIndex, int vColIndex) {
       Component c = super.prepareRenderer(renderer, rowIndex, vColIndex);
       int row = convertRowIndexToModel(rowIndex);
       if (row == nodeModel.getRowCount() - 1) {
         if (fontForAverage == null) {
           fontForAverage = c.getFont().deriveFont(Font.BOLD);
         }
         // Last line is average
         c.setFont(fontForAverage);
       }
       return c;
      }

    };

    // Do not sort column when clicking between the columns (resizing)
    TableRowSorter<NodeModel> sorter = new TableRowSorter<NodeModel>(nodeModel) {
      public void toggleSortOrder(int column) {
        if(table.getTableHeader().getCursor().getType() != Cursor.E_RESIZE_CURSOR) {
          super.toggleSortOrder(column);
        }
      }
    };
    for(int c = 0; c < columns.length; c++) {
      if (columns[c].dataClass == Number.class) {
        sorter.setComparator(c, NUMBER_COMPARATOR);
      }
    }
    table.setRowSorter(sorter);
    // Pack the column when double clicking between columns (resizing)
    table.getTableHeader().addMouseListener(new MouseAdapter() {
      public void mouseClicked(MouseEvent e) {
        if(e.getClickCount() == 2 && SwingUtilities.isLeftMouseButton(e) &&
            table.getTableHeader().getCursor().getType() == Cursor.E_RESIZE_CURSOR) {
          int index = table.getColumnModel().getColumnIndexAtX(e.getX() - 3);
          if (index >= 0) {
            packColumn(table, index);
          }
        }
      }
    });

    // Add right aligned renderer for node name
    DefaultTableCellRenderer renderer = new DefaultTableCellRenderer();
    renderer.setHorizontalAlignment(JLabel.RIGHT);
    table.setDefaultRenderer(Node.class, renderer);

    // Add renderer for time
    renderer = new DefaultTableCellRenderer() {
      private static final long serialVersionUID = 1L;

      public void setValue(Object value) {
        long time = (Long) value;
        setText(time > 0 ? getTimeAsString(time) : null);
      }
    };
    renderer.setHorizontalAlignment(JLabel.RIGHT);
    table.setDefaultRenderer(Long.class, renderer);

    // Add renderer for double
    renderer = new DefaultTableCellRenderer() {
      private static final long serialVersionUID = 1L;

      public void setValue(Object value) {
        if (value == null) {
          setText(null);
        } else {
          double v = ((Number) value).doubleValue() + 0.0005;
          int dec = ((int)(v * 1000)) % 1000;
          setText((long)v + "." + (dec > 99 ? "" : "0") + (dec > 9 ? "" : "0") + dec);
        }
      }
    };
    renderer.setHorizontalAlignment(JLabel.RIGHT);
    table.setDefaultRenderer(Double.class, renderer);

    // Add renderer for mixed integers and doubles
    renderer = new DefaultTableCellRenderer() {
      private static final long serialVersionUID = 1L;

      public void setValue(Object value) {
        if (value == null) {
          setText(null);
        } else if (value instanceof Integer) {
          setText(value.toString());
        } else {
          double v = ((Number) value).doubleValue() + 0.0005;
          int dec = ((int)(v * 1000)) % 1000;
          setText((long)v + "." + (dec > 99 ? "" : "0") + (dec > 9 ? "" : "0") + dec);
        }
      }
    };
    renderer.setHorizontalAlignment(JLabel.RIGHT);
    table.setDefaultRenderer(Number.class, renderer);

    table.setFillsViewportHeight(true);
    table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
    for (int i = 0, n = table.getColumnCount(); i < n; i++) {
      packColumn(table, i);
    }

    String savedColumnData = server.getConfig("collect.nodeinfo.table");
    if (savedColumnData != null) {
      String[] columnList = savedColumnData.split("[ ,]");
      for(int i = 1; i < columns.length; i++) {
        columns[i].setVisible(false);
      }
      for(int i = 0; i < columnList.length; i++) {
        int c = Integer.parseInt(columnList[i]);
        int index = table.convertColumnIndexToView(c);
        if (index >= 0) {
          table.getColumnModel().moveColumn(index, i);
        }
        columns[c].setVisible(true);
      }
    }
    JPopupMenu popupMenu = new JPopupMenu();
    // The first column (the node name) should always be visible.
    for(int i = 1; i < columns.length; i++) {
      popupMenu.add(new JCheckBoxMenuItem(columns[i].init(table, i)));
    }
    table.setComponentPopupMenu(popupMenu);
    add(new JScrollPane(table), BorderLayout.CENTER);
  }

  public void updateConfig(Properties config) {
    StringBuilder sb = new StringBuilder();
    for(int i = 0, n = table.getColumnCount(); i < n; i++) {
      int index = table.convertColumnIndexToModel(i);
      if (index >= 0) {
        if (sb.length() > 0) {
          sb.append(',');
        }
        sb.append(index);
      }
    }
    config.setProperty("collect.nodeinfo.table", sb.toString());
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
    nodeModel.setNodes(server.getNodes());
  }

  @Override
  public void nodeDataReceived(SensorData sensorData) {
    nodeModel.updateNode(sensorData.getNode());
  }

  @Override
  public void clearNodeData() {
    nodeModel.setNodes(null);
  }

  @Override
  public void nodesSelected(Node[] nodes) {
    // Ignore
  }

  @Override
  public void setVisible(boolean visible) {
    nodeModel.setNodes(visible ? server.getNodes() : null);
    super.setVisible(visible);
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
    long sec = time % 60;
    sb.append(time / 60).append(" min, ");
    if (sec < 10) {
      sb.append('0');
    }
    sb.append(sec).append(" sec");
    return sb.toString();
  }

  private static void packColumn(JTable table, int columnIndex) {
    TableColumn tableColumn = table.getColumnModel().getColumn(columnIndex);
    Object value = tableColumn.getHeaderValue();
    TableCellRenderer columnRenderer = tableColumn.getHeaderRenderer();
    if (columnRenderer == null) {
      columnRenderer = table.getTableHeader().getDefaultRenderer();
    }
    Component c = columnRenderer.getTableCellRendererComponent(table, value, false, false, -1, columnIndex);
    int width = c.getPreferredSize().width + 6;
    int intercellSpacing = table.getIntercellSpacing().width;
    for(int i = 0, n = table.getRowCount(); i < n; i++) {
      TableCellRenderer cellRenderer = table.getCellRenderer(i, columnIndex);
      value = table.getValueAt(i, columnIndex);
      c = cellRenderer.getTableCellRendererComponent(table, value, false, false, i, columnIndex);
      int w = c.getPreferredSize().width + intercellSpacing + 2;
      if (w > width) {
        width = w;
      }
    }
    table.getTableHeader().setResizingColumn(tableColumn);
    tableColumn.setWidth(width);
    tableColumn.setPreferredWidth(width);
  }

  private static class NodeModel extends AbstractTableModel {

    private static final long serialVersionUID = 1692207305977527004L;

    private final TableData[] columns;
    private Node[] nodes;

    public NodeModel(TableData[] columns) {
      this.columns = columns;
    }

    public void recalculateAverage() {
      for(TableData td : columns) {
        td.clearAverageCache();
      }
      int row = getRowCount() - 1;
      fireTableRowsUpdated(row, row);
    }

    public Object getValueAt(int row, int col) {
      int count = nodes == null ? 0 : nodes.length;
      if (row == count) {
        return columns[col].getAverageValue(nodes);
      }
      return columns[col].getValue(nodes[row]);
    }

    public Class<?> getColumnClass(int col) {
      return columns[col].dataClass;
    }

    public String getColumnName(int col) {
      return columns[col].name;
    }

    public String getColumnToolTip(int col) {
      Object v = columns[col].getValue(TableData.SHORT_DESCRIPTION);
      return v == null ? null : v.toString();
    }

    public int getColumnCount() {
      return columns.length;
    }

    public int getRowCount() {
      return (nodes == null ? 0 : nodes.length) + 1;
    }

    public void setNodes(Node[] nodes) {
      if (this.nodes != null && this.nodes.length > 0) {
        fireTableRowsDeleted(0, this.nodes.length - 1);
      }
      this.nodes = nodes;
      if (this.nodes != null && this.nodes.length > 0) {
        fireTableRowsInserted(0, this.nodes.length - 1);
      }
      recalculateAverage();
    }

    public void updateNode(Node node) {
      if (this.nodes != null) {
        for(int row = 0; row < this.nodes.length; row++) {
          if (this.nodes[row] == node) {
            fireTableRowsUpdated(row, row);
            recalculateAverage();
            break;
          }
        }
      }
    }

  }

  public static abstract class TableData extends AbstractAction {
    private static final long serialVersionUID = -3045755073722516926L;

    private final static Node AVERAGE_NODE = new Node("99999999.9", "Avg");

    public final String name;
    public final Class<?> dataClass;

    private JTable table;
    private TableColumn tableColumn;
    private int modelIndex = -1;

    private Object averageCache;

    protected TableData(String name, Class<?> dataClass) {
      this(name, name, dataClass);
    }

    protected TableData(String name, String description, Class<?> dataClass) {
      super(name);
      this.name = name;
      this.dataClass = dataClass;
      putValue(SHORT_DESCRIPTION, description);
      setVisible(true);
    }

    TableData init(JTable table, int modelIndex) {
      this.table = table;
      this.modelIndex = modelIndex;
      if (!isVisible()) {
        // The column should initially be hidden
        setColumnVisible(false);
      }
      return this;
    }

    public boolean isVisible() {
      return Boolean.TRUE.equals(getValue(SELECTED_KEY));
    }

    public TableData setVisible(boolean isVisible) {
      putValue(SELECTED_KEY, isVisible);
      return this;
    }

    public void actionPerformed(ActionEvent event) {
      if (modelIndex >= 0) {
        setColumnVisible(isVisible());
      }
    }

    private void setColumnVisible(boolean isVisible) {
      if (isVisible) {
        if (tableColumn != null) {
          int count = table.getColumnCount();
          table.addColumn(tableColumn);
          tableColumn = null;
          packColumn(table, count);

          int newIndex = 0;
          for(int i = 0; i < modelIndex; i++) {
            if (table.convertColumnIndexToView(i) >= 0) {
              // The new column should be after this visible column
              newIndex++;
            }
          }
          if (newIndex < count) {
            table.getColumnModel().moveColumn(count, newIndex);
          }
        }
      } else {
        int columnIndex = table.convertColumnIndexToView(modelIndex);
        if (columnIndex >= 0 ) {
          tableColumn = table.getColumnModel().getColumn(columnIndex);
          table.removeColumn(tableColumn);
        }
      }
    }

    public final Object getAverageValue(Node[] nodes) {
      Object tmp = averageCache;
      if (tmp != null) {
        return tmp;
      }

      if (dataClass == Long.class || dataClass == Double.class
          || dataClass == Number.class) {
        double average = 0.0;
        if (nodes != null && nodes.length > 0) {
          int count = 0;
          for(Node node : nodes) {
            if (node.getSensorDataAggregator().getDataCount() > 0) {
              average += ((Number) getValue(node)).doubleValue();
              count++;
            }
          }
          if (count > 0) {
            average = average / count;
          }
        }
        if (dataClass == Long.class) {
          tmp = (long) (average + 0.5);
        } else {
          tmp = average;
        }
      } else if (dataClass == Node.class) {
        tmp = AVERAGE_NODE;
      }
      averageCache = tmp;
      return tmp;
    }

    public final void clearAverageCache() {
      averageCache = null;
    }

    public abstract Object getValue(Node node);

  }

}

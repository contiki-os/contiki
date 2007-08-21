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
 * $Id: RadioLogger.java,v 1.10 2007/08/21 13:28:52 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseEvent;
import java.util.*;
import javax.swing.*;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.PacketRadio;
import se.sics.cooja.interfaces.Radio;

/**
 * Radio logger listens to the simulation radio medium and lists all transmitted
 * data in a table. By overriding the transformDataToString method, protocol
 * specific data can be interpreted.
 *
 * @see #transformDataToString(byte[])
 * @author Fredrik Osterlind
 */
@ClassDescription("Radio Logger")
@PluginType(PluginType.SIM_PLUGIN)
public class RadioLogger extends VisPlugin {
  private static final long serialVersionUID = 1L;

  private final int DATAPOS_TIME = 0;
  private final int DATAPOS_CONNECTION = 1;
  private final int DATAPOS_DATA = 2;

  private final int COLUMN_TIME = 0;
  private final int COLUMN_FROM = 1;
  private final int COLUMN_TO = 2;
  private final int COLUMN_DATA = 3;

  private static Logger logger = Logger.getLogger(RadioLogger.class);

  private Simulation simulation;

  private Vector<String> columnNames = new Vector<String>();

  private Vector<Object[]> rowData = new Vector<Object[]>();

  private JTable dataTable = null;

  private RadioMedium radioMedium;

  private Observer radioMediumObserver;

  public RadioLogger(final Simulation simulationToControl, final GUI gui) {
    super("Radio Logger", gui);
    simulation = simulationToControl;
    radioMedium = simulation.getRadioMedium();

    columnNames.add("Time");
    columnNames.add("From");
    columnNames.add("To");
    columnNames.add("Data");

    final AbstractTableModel model = new AbstractTableModel() {
      public String getColumnName(int col) {
        return columnNames.get(col).toString();
      }

      public int getRowCount() {
        return rowData.size();
      }

      public int getColumnCount() {
        return columnNames.size();
      }

      public Object getValueAt(int row, int col) {
        if (col == COLUMN_TIME) {
          return rowData.get(row)[DATAPOS_TIME];
        } else if (col == COLUMN_FROM) {
          return ((RadioConnection)rowData.get(row)[DATAPOS_CONNECTION]).getSource().getMote();
        } else if (col == COLUMN_TO) {
          return "[" + ((RadioConnection)rowData.get(row)[DATAPOS_CONNECTION]).getDestinations().length + " motes]";
        } else if (col == COLUMN_DATA) {
          return transformDataToString((byte[]) rowData.get(row)[DATAPOS_DATA]);
        } else {
          logger.fatal("Bad row/col: " + row + "/" + col);
        }
        return null;
      }

      public boolean isCellEditable(int row, int col) {
        if (col == COLUMN_FROM) {
          // Try to highligt selected mote
          gui.signalMoteHighlight(
              ((RadioConnection)rowData.get(row)[DATAPOS_CONNECTION]).getSource().getMote()
          );
        }

        if (col == COLUMN_TO) {
          return true;
        }
        return false;
      }

      public Class getColumnClass(int c) {
        return getValueAt(0, c).getClass();
      }
    };

    final JComboBox comboBox = new JComboBox();

    comboBox.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED) {
          if (e.getItem() instanceof Mote) {
            gui.signalMoteHighlight((Mote) e.getItem());
          }
        }
      }
    });

    dataTable = new JTable(model) {
      public TableCellEditor getCellEditor(int row, int column) {
        // Populate combo box
        comboBox.removeAllItems();
        if (row < 0 || row >= rowData.size()) {
          return super.getCellEditor(row, column);
        }

        RadioConnection conn = (RadioConnection) rowData.get(row)[DATAPOS_CONNECTION];
        if (conn == null) {
          return super.getCellEditor(row, column);
        }

        for (Radio destRadio: conn.getDestinations()) {
          if (destRadio.getMote() != null) {
            comboBox.addItem(destRadio.getMote());
          } else {
            comboBox.addItem("[standalone radio]");
          }
        }

        return super.getCellEditor(row, column);
      }

      public String getToolTipText(MouseEvent e) {
        java.awt.Point p = e.getPoint();
        int rowIndex = rowAtPoint(p);
        int colIndex = columnAtPoint(p);
        int realColumnIndex = convertColumnIndexToModel(colIndex);

        String tip = "";
        if (realColumnIndex == COLUMN_DATA) {
          byte[] data = (byte[]) rowData.get(rowIndex)[DATAPOS_DATA];

          Packet packet = analyzePacket(data);
          if (packet != null) {
            tip = packet.getToolTip();
          }
        } else {
          tip = super.getToolTipText(e);
        }
        return tip;
      }
    };

    // Set data column width greedy
    dataTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
    dataTable.getColumnModel().getColumn(COLUMN_TIME).setPreferredWidth(55);
    dataTable.getColumnModel().getColumn(COLUMN_FROM).setPreferredWidth(130);
    dataTable.getColumnModel().getColumn(COLUMN_TO).setPreferredWidth(100);

    dataTable.getSelectionModel().setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    dataTable.getSelectionModel().addListSelectionListener(
        new ListSelectionListener() {
          public void valueChanged(ListSelectionEvent e) {
            int selectedRowIndex = dataTable.getSelectedRow();
            if (selectedRowIndex < 0) {
              return;
            }

            Object[] row = rowData.get(selectedRowIndex);
            if (row == null) {
              return;
            }

            RadioConnection conn = (RadioConnection) row[DATAPOS_CONNECTION];
            if (conn == null) {
              return;
            }

            Mote mote = conn.getSource().getMote();
            if (mote == null) {
              return;
            }

            gui.signalMoteHighlight(mote);
          }
        });

    TableColumn destColumn = dataTable.getColumnModel().getColumn(COLUMN_TO);
    destColumn.setCellEditor(new DefaultCellEditor(comboBox));

    final JScrollPane scrollPane = new JScrollPane(dataTable);
//    dataTable.setFillsViewportHeight(true);

    add(scrollPane);

    simulation.getRadioMedium().addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        RadioConnection[] newConnections = radioMedium.getLastTickConnections();
        if (newConnections == null) {
          return;
        }

        for (RadioConnection newConnection: newConnections) {
          Object[] data = new Object[3];
          data[DATAPOS_TIME] = new Integer(simulation.getSimulationTime());
          data[DATAPOS_CONNECTION] = newConnection;

          if (newConnection.getSource() instanceof PacketRadio) {
            data[DATAPOS_DATA] = ((PacketRadio) newConnection.getSource()).getLastPacketTransmitted();
          } else {
            data[DATAPOS_DATA] = null;
          }

          rowData.add(data);
        }
        model.fireTableRowsInserted(rowData.size() - newConnections.length + 1, rowData.size());
      }
    });

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  /**
   * Transform transmitted data to representable object, such as a string.
   *
   * @param data Transmitted data
   * @return Representable object
   */
  public Object transformDataToString(byte[] data) {
    if (data == null) {
      return "[unknown data]";
    }

    Packet packet = analyzePacket(data);
    if (packet == null) {
      return "Unknown packet, size " + data.length;
    } else {
      return packet.getShortDescription();
    }
  }

  static abstract class Packet {
    public abstract String getShortDescription();
    public abstract String getToolTip();
  }

  static class PacketAODV_RREQ extends Packet {
    public final static int SIZE = 52;
    public final static int HEADER_SIZE = 24;
    public final static int TYPE = 1;

    private byte[] data;
    public PacketAODV_RREQ(byte[] packetData) {
      this.data = new byte[HEADER_SIZE];
      System.arraycopy(packetData, packetData.length - HEADER_SIZE, data, 0, HEADER_SIZE);
    }

    public int getType() {
      return data[0];
    }

    public int getFlags() {
      return data[1];
    }

    public int getReserved() {
      return data[2];
    }

    public int getHopCount() {
      return data[3];
    }

    public int getID() {
      int id =
        ((data[4] & 0xFF) << 24) +
        ((data[5] & 0xFF) << 16) +
        ((data[6] & 0xFF) << 8) +
        ((data[7] & 0xFF) << 0);
      return id;
    }

    public String getDestAddr() {
      return (0xff&data[8]) + "." + (0xff&data[9]) + "." + (0xff&data[10]) + "." + (0xff&data[11]);
    }

    public int getDestSeqNo() {
      int seqNo =
        ((data[12] & 0xFF) << 24) +
        ((data[13] & 0xFF) << 16) +
        ((data[14] & 0xFF) << 8) +
        ((data[15] & 0xFF) << 0);
      return seqNo;
    }

    public String getOrigAddr() {
      return (0xff&data[16]) + "." + (0xff&data[17]) + "." + (0xff&data[18]) + "." + (0xff&data[19]);
    }

    public int getOrigSeqNo() {
      int seqNo =
        ((data[20] & 0xFF) << 24) +
        ((data[21] & 0xFF) << 16) +
        ((data[22] & 0xFF) << 8) +
        ((data[23] & 0xFF) << 0);
      return seqNo;
    }

    public String getShortDescription() {
      return "AODV RREQ to " + getDestAddr() + " from " + getOrigAddr();
    }

    public String getToolTip() {
      return "<html>" +
      "AODV RREQ type: " + getType() + "<br>" +
      "AODV RREQ flags: " + getFlags() + "<br>" +
      "AODV RREQ reserved: " + getReserved() + "<br>" +
      "AODV RREQ hop_count: " + getHopCount() + "<br>" +
      "AODV RREQ id: " + getID() + "<br>" +
      "AODV RREQ dest_addr: " + getDestAddr() + "<br>" +
      "AODV RREQ dest_seqno: " + getDestSeqNo() + "<br>" +
      "AODV RREQ orig_addr: " + getOrigAddr() + "<br>" +
      "AODV RREQ orig_seqno: " + getOrigSeqNo() +
      "</html>";
    }

    public static boolean dataFits(byte[] packetData) {
      if (packetData.length != SIZE) {
        return false;
      }

      byte[] dataNoHeader = new byte[HEADER_SIZE];
      System.arraycopy(packetData, packetData.length - HEADER_SIZE, dataNoHeader, 0, HEADER_SIZE);

      if (dataNoHeader[0] != TYPE) {
        return false;
      }

      return true;
    }
  };

  static class PacketAODV_RREP extends Packet {
    public final static int SIZE = 56;
    public final static int HEADER_SIZE = 20;
    public final static int TYPE = 2;

    private byte[] data;

    public PacketAODV_RREP(byte[] packetData) {
      this.data = new byte[HEADER_SIZE];
      System.arraycopy(packetData, packetData.length - HEADER_SIZE, data, 0, HEADER_SIZE);
    }

    public int getType() {
      return data[0];
    }

    public int getFlags() {
      return data[1];
    }

    public int getPrefix() {
      return data[2];
    }

    public int getHopCount() {
      return data[3];
    }

    public String getDestAddr() {
      return (0xff&data[4]) + "." + (0xff&data[5]) + "." + (0xff&data[6]) + "." + (0xff&data[7]);
    }

    public int getDestSeqNo() {
      int seqNo =
        ((data[8] & 0xFF) << 24) +
        ((data[9] & 0xFF) << 16) +
        ((data[10] & 0xFF) << 8) +
        ((data[11] & 0xFF) << 0);
      return seqNo;
    }

    public String getOrigAddr() {
      return (0xff&data[12]) + "." + (0xff&data[13]) + "." + (0xff&data[14]) + "." + (0xff&data[15]);
    }

    public int getLifetime() {
      int seqNo =
        ((data[16] & 0xFF) << 24) +
        ((data[17] & 0xFF) << 16) +
        ((data[18] & 0xFF) << 8) +
        ((data[19] & 0xFF) << 0);
      return seqNo;
    }

    public String getShortDescription() {
      return "AODV RREP to " + getDestAddr() + " from " + getOrigAddr();
    }

    public String getToolTip() {
      return "<html>" +
      "AODV RREP type: " + getType() + "<br>" +
      "AODV RREP flags: " + getFlags() + "<br>" +
      "AODV RREP prefix: " + getPrefix() + "<br>" +
      "AODV RREP hop_count: " + getHopCount() + "<br>" +
      "AODV RREP dest_addr: " + getDestAddr() + "<br>" +
      "AODV RREP dest_seqno: " + getDestSeqNo() + "<br>" +
      "AODV RREP orig_addr: " + getOrigAddr() + "<br>" +
      "AODV RREP lifetime: " + getLifetime() + "<br>" +
      "</html>";
    }

    public static boolean dataFits(byte[] packetData) {
      if (packetData.length != SIZE) {
        return false;
      }

      byte[] dataNoHeader = new byte[HEADER_SIZE];
      System.arraycopy(packetData, packetData.length - HEADER_SIZE, dataNoHeader, 0, HEADER_SIZE);

      if (dataNoHeader[0] != TYPE) {
        return false;
      }

      return true;
    }
  };

  static class PacketAODV_RERR extends Packet {
    public final static int SIZE = 40;
    public final static int HEADER_SIZE = 12;
    public final static int TYPE = 3;

    private byte[] data;
    public PacketAODV_RERR(byte[] packetData) {
      this.data = new byte[HEADER_SIZE];
      System.arraycopy(packetData, packetData.length - HEADER_SIZE, data, 0, HEADER_SIZE);
    }

    public int getType() {
      return data[0];
    }

    public int getFlags() {
      return data[1];
    }

    public int getReserved() {
      return data[2];
    }

    public int getDestCount() {
      return data[3];
    }

    public String getUnreachAddr() {
      return (0xff&data[4]) + "." + (0xff&data[5]) + "." + (0xff&data[6]) + "." + (0xff&data[7]);
    }

    public int getUnreachSeqNo() {
      int seqNo =
        ((data[8] & 0xFF) << 24) +
        ((data[9] & 0xFF) << 16) +
        ((data[10] & 0xFF) << 8) +
        ((data[11] & 0xFF) << 0);
      return seqNo;
    }

    public String getShortDescription() {
      return "AODV RERR for " + getUnreachAddr();
    }

    public String getToolTip() {
      return "<html>" +
      "AODV RERR type: " + getType() + "<br>" +
      "AODV RERR flags: " + getFlags() + "<br>" +
      "AODV RERR reserved: " + getReserved() + "<br>" +
      "AODV RERR dest_count: " + getDestCount() + "<br>" +
      "AODV RERR unreach_addr: " + getUnreachAddr() + "<br>" +
      "AODV RERR unreach_seqno: " + getUnreachSeqNo() + "<br>" +
      "</html>";
    }

    public static boolean dataFits(byte[] packetData) {
      if (packetData.length != SIZE) {
        return false;
      }

      byte[] dataNoHeader = new byte[HEADER_SIZE];
      System.arraycopy(packetData, packetData.length - HEADER_SIZE, dataNoHeader, 0, HEADER_SIZE);

      if (dataNoHeader[0] != TYPE) {
        return false;
      }

      return true;
    }
  };

  static class AckPacket extends Packet {
    public final static int SIZE = 5;

    private byte[] data;
    public AckPacket(byte[] packetData) {
      this.data = packetData;
    }

    public int getChecksum() {
      int checksum =
        ((data[3] & 0xFF) << 8) +
        ((data[4] & 0xFF) << 0);
      return checksum;
    }

    public String getShortDescription() {
      return "ACK";
    }

    public String getToolTip() {
      return "<html>" +
      "ACK checksum: " + getChecksum() + "<br>" +
      "</html>";
    }

    public static boolean dataFits(byte[] packetData) {
      if (packetData.length != SIZE) {
        return false;
      }

      if (packetData[0] != (byte) 'a') {
        return false;
      }

      if (packetData[1] != (byte) 'C') {
        return false;
      }

      if (packetData[2] != (byte) 'k') {
        return false;
      }

      return true;
    }
  };

  static class ForwardedPacketUnknown extends PacketUnknown {
    public final static int MINIMUM_SIZE = 4;

    public ForwardedPacketUnknown(byte[] data) {
      super(data);
    }

    public String getShortDescription() {
      return "(FWD) " + super.getShortDescription();
    }

    public static boolean dataFits(byte[] packetData) {
      if (packetData.length < ForwardedPacketUnknown.MINIMUM_SIZE) {
        return false;
      }

      if (packetData[0] != (byte) 'f') {
        return false;
      }

      if (packetData[1] != (byte) 'W') {
        return false;
      }

      if (packetData[2] != (byte) 'd') {
        return false;
      }

      if (packetData[3] != (byte) ':') {
        return false;
      }

      return true;
    }
  }

  static class PacketUnknown extends Packet {
    private byte[] data = null;

    public PacketUnknown(byte[] data) {
      this.data = data;
    }

    public String getShortDescription() {
      return "Data packet, size " + data.length;
    }

    public String getToolTip() {
      String toolTip = "<html><b>Data packet</b><br>";

      int byteCounter = 0;
      for (byte b: data) {
        String hexB = "0" + Integer.toHexString(b);
        hexB = hexB.substring(hexB.length() - 2);
        toolTip += "0x" + hexB + " ";
        if (byteCounter++ > 2) {
          toolTip += "<br>";
          byteCounter = 0;
        }
      }
      toolTip += "<br><br>";
      for (byte b: data) {
        toolTip += (char) b;
      }
      toolTip += "</html>";
      return toolTip;
    }

  }

  private Packet analyzePacket(byte[] data) {

    if (PacketAODV_RREQ.dataFits(data)) {
      return new PacketAODV_RREQ(data);
    }

    if (PacketAODV_RREP.dataFits(data)) {
      return new PacketAODV_RREP(data);
    }

    if (PacketAODV_RERR.dataFits(data)) {
      return new PacketAODV_RERR(data);
    }

    if (ForwardedPacketUnknown.dataFits(data)) {
      return new ForwardedPacketUnknown(data);
    }

    if (AckPacket.dataFits(data)) {
      return new AckPacket(data);
    }

    return new PacketUnknown(data);
  }

  public void closePlugin() {
    if (radioMediumObserver != null) {
      radioMedium.deleteRadioMediumObserver(radioMediumObserver);
    }
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable) {
    return true;
  }

}

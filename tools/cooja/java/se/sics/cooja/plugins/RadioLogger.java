/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: RadioLogger.java,v 1.18 2009/05/20 12:02:06 fros4943 Exp $
 */

package se.sics.cooja.plugins;
import java.awt.Font;
import java.awt.event.MouseEvent;
import java.util.*;
import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.MoteID;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.util.StringUtils;

/**
 * Radio logger listens to the simulation radio medium and lists all transmitted
 * data in a table.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Radio Logger")
@PluginType(PluginType.SIM_PLUGIN)
public class RadioLogger extends VisPlugin {
  private static Logger logger = Logger.getLogger(RadioLogger.class);

  private final static int COLUMN_TIME = 0;
  private final static int COLUMN_FROM = 1;
  private final static int COLUMN_TO = 2;
  private final static int COLUMN_DATA = 3;

  private final static String[] COLUMN_NAMES = {
    "Time",
    "From",
    "To",
    "Data"
  };

  private Simulation simulation;
  private ArrayList<RadioConnectionLog> connections = new ArrayList<RadioConnectionLog>();
  private JTable dataTable = null;
  private RadioMedium radioMedium;
  private Observer radioMediumObserver;

  public RadioLogger(final Simulation simulationToControl, final GUI gui) {
    super("Radio Logger", gui);
    simulation = simulationToControl;
    radioMedium = simulation.getRadioMedium();

    final AbstractTableModel model = new AbstractTableModel() {
      public String getColumnName(int col) {
        return COLUMN_NAMES[col];
      }

      public int getRowCount() {
        return connections.size();
      }

      public int getColumnCount() {
        return COLUMN_NAMES.length;
      }

      public Object getValueAt(int row, int col) {
        RadioConnectionLog conn = connections.get(row);
        if (col == COLUMN_TIME) {
          return conn.startTime;
        } else if (col == COLUMN_FROM) {
          return getMoteID(conn.connection.getSource().getMote());
        } else if (col == COLUMN_TO) {
          Radio[] dests = conn.connection.getDestinations();
          if (dests.length == 1) {
            return getMoteID(dests[0].getMote());
          }
          return "[" + dests.length + " motes]";
        } else if (col == COLUMN_DATA) {
          if (conn.data == null) {
            prepareDataString(connections.get(row));
          }
          return conn.data;
        }
        return null;
      }

      public boolean isCellEditable(int row, int col) {
        if (col == COLUMN_FROM) {
          /* Highlight source */
          gui.signalMoteHighlight(connections.get(row).connection.getSource().getMote());
          return false;
        }

        if (col == COLUMN_TO) {
          /* Highlight all destinations */
          Radio dests[] = connections.get(row).connection.getDestinations();
          for (Radio dest: dests) {
            gui.signalMoteHighlight(dest.getMote());
          }
          return false;
        }
        return false;
      }

      public Class getColumnClass(int c) {
        return getValueAt(0, c).getClass();
      }
    };

    dataTable = new JTable(model) {

      public String getToolTipText(MouseEvent e) {
        java.awt.Point p = e.getPoint();
        int rowIndex = rowAtPoint(p);
        int colIndex = columnAtPoint(p);
        int realColumnIndex = convertColumnIndexToModel(colIndex);

        RadioConnectionLog conn = connections.get(rowIndex);
        if (realColumnIndex == COLUMN_TIME) {
          return
            "<html>" +
            "Start time: " + conn.startTime +
            "<br>" +
            "End time: " + conn.endTime +
            "<br><br>" +
            "Duration: " + (conn.endTime - conn.startTime) +
            "</html>";
        } else if (realColumnIndex == COLUMN_FROM) {
          return conn.connection.getSource().getMote().toString();
        } else if (realColumnIndex == COLUMN_TO) {
          StringBuilder tip = new StringBuilder();
          tip.append("<html>");
          Radio[] dests = conn.connection.getDestinations();
          for (Radio radio: dests) {
            tip.append(radio.getMote()).append("<br>");
          }
          tip.append("</html>");
          return tip.toString();
        } else if (realColumnIndex == COLUMN_DATA) {
          if (conn.tooltip == null) {
            prepareTooltipString(conn);
          }
          return conn.tooltip;
        }
        return super.getToolTipText(e);
      }
    };

    // Set data column width greedy
    dataTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
    dataTable.getColumnModel().getColumn(COLUMN_TIME).setPreferredWidth(130);
//    dataTable.getColumnModel().getColumn(COLUMN_TIME).setResizable(false);
    dataTable.getColumnModel().getColumn(COLUMN_FROM).setPreferredWidth(90);
    dataTable.getColumnModel().getColumn(COLUMN_TO).setPreferredWidth(150);
    dataTable.getColumnModel().getColumn(COLUMN_DATA).setPreferredWidth(1500);

    dataTable.getSelectionModel().setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

    dataTable.setFont(new Font("Monospaced", Font.PLAIN, 12));

    simulation.getRadioMedium().addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        RadioConnection[] conns = radioMedium.getLastTickConnections();
        if (conns == null) {
          return;
        }

        for (RadioConnection conn : conns) {
          RadioConnectionLog loggedConn = new RadioConnectionLog();
          loggedConn.startTime = conn.getStartTime();
          loggedConn.endTime = simulation.getSimulationTime();
          loggedConn.connection = conn;
          loggedConn.packet = conn.getSource().getLastPacketTransmitted();
          connections.add(loggedConn);
        }
        model.fireTableRowsInserted(connections.size() - conns.length + 1, connections.size());
        setTitle("Radio Logger: " + connections.size() + " packets");
      }
    });

    add(new JScrollPane(dataTable));

    setSize(500, 300);
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  private String getMoteID(Mote mote) {
    MoteID moteID = mote.getInterfaces().getMoteID();
    if (moteID != null) {
      return Integer.toString(moteID.getMoteID());
    }
    return mote.toString();
  }

  private void prepareDataString(RadioConnectionLog conn) {
    byte[] data;
    if (conn.packet == null) {
      data = null;
    } else if (conn.packet instanceof ConvertedRadioPacket) {
      data = ((ConvertedRadioPacket)conn.packet).getOriginalPacketData();
    } else {
      data = conn.packet.getPacketData();
    }
    if (data == null) {
      conn.data = "[unknown data]";
      return;
    }
    conn.data = data.length + ": 0x" + StringUtils.toHex(data, 4);
  }

  private void prepareTooltipString(RadioConnectionLog conn) {
    RadioPacket packet = conn.packet;
    if (packet == null) {
      conn.tooltip = "";
      return;
    }

    if (packet instanceof ConvertedRadioPacket && packet.getPacketData().length > 0) {
      byte[] original = ((ConvertedRadioPacket)packet).getOriginalPacketData();
      byte[] converted = ((ConvertedRadioPacket)packet).getPacketData();
      conn.tooltip = "<html><font face=\"Monospaced\">" +
      "<b>Packet data (" + original.length + " bytes)</b><br>" +
      "<pre>" + StringUtils.hexDump(original) + "</pre>" +
      "</font><font face=\"Monospaced\">" +
      "<b>Cross-level packet data (" + converted.length + " bytes)</b><br>" +
      "<pre>" + StringUtils.hexDump(converted) + "</pre>" +
      "</font></html>";
    } else if (packet instanceof ConvertedRadioPacket) {
      byte[] original = ((ConvertedRadioPacket)packet).getOriginalPacketData();
      conn.tooltip = "<html><font face=\"Monospaced\">" +
      "<b>Packet data (" + original.length + " bytes)</b><br>" +
      "<pre>" + StringUtils.hexDump(original) + "</pre>" +
      "</font><font face=\"Monospaced\">" +
      "<b>Cross-level packet data conversion failed</b><br>" +
      "</font></html>";
    } else {
      byte[] data = packet.getPacketData();
      conn.tooltip = "<html><font face=\"Monospaced\">" +
      "<b>Packet data (" + data.length + " bytes)</b><br>" +
      "<pre>" + StringUtils.hexDump(data) + "</pre>" +
      "</font></html>";
    }

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

  private static class RadioConnectionLog {
    long startTime;
    long endTime;
    RadioConnection connection;
    RadioPacket packet;

    String data = null;
    String tooltip = null;
  }
}

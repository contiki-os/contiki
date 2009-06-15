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
 * $Id: RadioLogger.java,v 1.22 2009/06/15 10:53:24 fros4943 Exp $
 */

package se.sics.cooja.plugins;
import java.awt.Font;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;

import org.apache.log4j.Logger;
import org.jdom.Element;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.ConvertedRadioPacket;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.PluginType;
import se.sics.cooja.RadioConnection;
import se.sics.cooja.RadioMedium;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.dialogs.TableColumnAdjuster;
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
  private static final long serialVersionUID = -6927091711697081353L;

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

  private final Simulation simulation;
  private final JTable dataTable;
  private ArrayList<RadioConnectionLog> connections = new ArrayList<RadioConnectionLog>();
  private RadioMedium radioMedium;
  private Observer radioMediumObserver;
  private AbstractTableModel model;
  
  public RadioLogger(final Simulation simulationToControl, final GUI gui) {
    super("Radio Logger", gui);
    simulation = simulationToControl;
    radioMedium = simulation.getRadioMedium();

    model = new AbstractTableModel() {

      private static final long serialVersionUID = 1692207305977527004L;

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
          return Long.toString(conn.startTime / Simulation.MILLISECOND);
        } else if (col == COLUMN_FROM) {
          return getMoteID(conn.connection.getSource().getMote());
        } else if (col == COLUMN_TO) {
          Radio[] dests = conn.connection.getDestinations();
          if (dests.length == 0) {
            return "-";
          }
          if (dests.length == 1) {
            return getMoteID(dests[0].getMote());
          }
          if (dests.length == 2) {
            return getMoteID(dests[0].getMote()) + ',' + getMoteID(dests[1].getMote());
          }
          return "[" + dests.length + " d]";
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

      public Class<?> getColumnClass(int c) {
        return getValueAt(0, c).getClass();
      }
    };

    dataTable = new JTable(model) {

      private static final long serialVersionUID = -2199726885069809686L;

      public String getToolTipText(MouseEvent e) {
        java.awt.Point p = e.getPoint();
        int rowIndex = rowAtPoint(p);
        int colIndex = columnAtPoint(p);
        int realColumnIndex = convertColumnIndexToModel(colIndex);
        if (rowIndex < 0 || realColumnIndex < 0) {
          return super.getToolTipText(e);
        }

        RadioConnectionLog conn = connections.get(rowIndex);
        if (realColumnIndex == COLUMN_TIME) {
          return
            "<html>" +
            "Start time (us): " + conn.startTime +
            "<br>" +
            "End time (us): " + conn.endTime +
            "<br><br>" +
            "Duration (us): " + (conn.endTime - conn.startTime) +
            "</html>";
        } else if (realColumnIndex == COLUMN_FROM) {
          return conn.connection.getSource().getMote().toString();
        } else if (realColumnIndex == COLUMN_TO) {
          Radio[] dests = conn.connection.getDestinations();
          if (dests.length == 0) {
            return "No destinations";
          }
          StringBuilder tip = new StringBuilder();
          tip.append("<html>");
          if (dests.length == 1) {
            tip.append("One destination:<br>");
          } else {
            tip.append(dests.length).append(" destinations:<br>");
          }
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

    dataTable.setFont(new Font("Monospaced", Font.PLAIN, 12));

    JPopupMenu popupMenu = new JPopupMenu();
    popupMenu.add(new JMenuItem(copyAction));
    popupMenu.add(new JMenuItem(copyAllAction));
    popupMenu.add(new JMenuItem(clearAction));
    popupMenu.addSeparator();
    popupMenu.add(new JMenuItem(saveAction));
    dataTable.setComponentPopupMenu(popupMenu);

    add(new JScrollPane(dataTable));

    TableColumnAdjuster adjuster = new TableColumnAdjuster(dataTable);
    adjuster.setDynamicAdjustment(true);
    adjuster.packColumns();

    radioMedium.addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        RadioConnection[] conns = radioMedium.getLastTickConnections();
        if (conns == null || conns.length == 0) {
          return;
        }
        final RadioConnectionLog[] logged = new RadioConnectionLog[conns.length];
        for (int i = 0, n = logged.length; i < n; i++) {
          RadioConnectionLog loggedConn = new RadioConnectionLog();
          loggedConn.startTime = conns[i].getStartTime();
          loggedConn.endTime = simulation.getSimulationTime();
          loggedConn.connection = conns[i];
          loggedConn.packet = conns[i].getSource().getLastPacketTransmitted();
          logged[i] = loggedConn;
        }
        java.awt.EventQueue.invokeLater(new Runnable() {
          public void run() {
            int lastSize = connections.size();
            // Check if the last row is visible
            boolean isVisible = false;
            int rowCount = dataTable.getRowCount();
            if (rowCount > 0) {
              Rectangle lastRow = dataTable.getCellRect(rowCount - 1, 0, true);
              Rectangle visible = dataTable.getVisibleRect();
              isVisible = visible.y <= lastRow.y && visible.y + visible.height >= lastRow.y + lastRow.height;
            }
            for(RadioConnectionLog log: logged) {
              connections.add(log);
            }
            if (connections.size() > lastSize) {
              model.fireTableRowsInserted(lastSize, connections.size() - 1);
            }
            if (isVisible) {
              dataTable.scrollRectToVisible(dataTable.getCellRect(dataTable.getRowCount() - 1, 0, true));
            }
            setTitle("Radio Logger: " + dataTable.getRowCount() + " packets");
          }
        });
      }
    });

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
      "<b>No cross-level conversion available</b><br>" +
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

  private String getDestString(RadioConnectionLog c) {
    Radio[] dests = c.connection.getDestinations();
    if (dests.length == 0) {
      return "-";
    }
    if (dests.length == 1) {
      return getMoteID(dests[0].getMote());
    }
    StringBuilder sb = new StringBuilder();
    for (Radio dest: dests) {
      sb.append(getMoteID(dest.getMote()) + ',');
    }
    sb.setLength(sb.length()-1);
    return sb.toString();
  }

  private Action clearAction = new AbstractAction("Clear") {
    public void actionPerformed(ActionEvent e) {
      int size = connections.size();
      if (size > 0) {
        connections.clear();
        model.fireTableRowsDeleted(0, size - 1);
        setTitle("Radio Logger: " + dataTable.getRowCount() + " packets");
      }
    }
  };
  
  private Action copyAction = new AbstractAction("Copy selected") {
    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      int[] selectedRows = dataTable.getSelectedRows();

      StringBuilder sb = new StringBuilder();
      for (int i: selectedRows) {
        sb.append("" + dataTable.getValueAt(i, COLUMN_TIME) + '\t');
        sb.append("" + dataTable.getValueAt(i, COLUMN_FROM) + '\t');
        sb.append("" + getDestString(connections.get(i)) + '\t');
        sb.append("" + dataTable.getValueAt(i, COLUMN_DATA) + '\n');
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };
  
  private Action copyAllAction = new AbstractAction("Copy all") {
    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      StringBuilder sb = new StringBuilder();
      for(int i=0; i < connections.size(); i++) {
        sb.append("" + dataTable.getValueAt(i, COLUMN_TIME) + '\t');
        sb.append("" + dataTable.getValueAt(i, COLUMN_FROM) + '\t');
        sb.append("" + getDestString(connections.get(i)) + '\t');
        sb.append("" + dataTable.getValueAt(i, COLUMN_DATA) + '\n');
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };
  
  private Action saveAction = new AbstractAction("Save to file") {
    public void actionPerformed(ActionEvent e) {
      JFileChooser fc = new JFileChooser();
      int returnVal = fc.showSaveDialog(GUI.getTopParentContainer());
      if (returnVal != JFileChooser.APPROVE_OPTION) {
        return;
      }

      File saveFile = fc.getSelectedFile();
      if (saveFile.exists()) {
        String s1 = "Overwrite";
        String s2 = "Cancel";
        Object[] options = { s1, s2 };
        int n = JOptionPane.showOptionDialog(
            GUI.getTopParentContainer(),
            "A file with the same name already exists.\nDo you want to remove it?",
            "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, options, s1);
        if (n != JOptionPane.YES_OPTION) {
          return;
        }
      }

      if (saveFile.exists() && !saveFile.canWrite()) {
        logger.fatal("No write access to file: " + saveFile);
        return;
      }

      try {
        PrintWriter outStream = new PrintWriter(new FileWriter(saveFile));
        for(int i=0; i < connections.size(); i++) {
          outStream.print("" + dataTable.getValueAt(i, COLUMN_TIME) + '\t');
          outStream.print("" + dataTable.getValueAt(i, COLUMN_FROM) + '\t');
          outStream.print("" + getDestString(connections.get(i)) + '\t');
          outStream.print("" + dataTable.getValueAt(i, COLUMN_DATA) + '\n');
        }
        outStream.close();
      } catch (Exception ex) {
        logger.fatal("Could not write to file: " + saveFile);
        return;
      }

    }
  };
  
}

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
 * $Id: RadioLogger.java,v 1.1 2007/05/30 11:13:31 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseEvent;
import java.util.*;
import javax.swing.*;
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
        
        if (col == COLUMN_TO)
          return true;
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
          if (e.getItem() instanceof Mote)
            gui.signalMoteHighlight((Mote) e.getItem());
        }
      }
    });

    dataTable = new JTable(model) {
      public TableCellEditor getCellEditor(int row, int column) {
        // Populate combo box
        comboBox.removeAllItems();
        if (row < 0 || row >= rowData.size())
          return super.getCellEditor(row, column);

        RadioConnection conn = (RadioConnection) rowData.get(row)[DATAPOS_CONNECTION];
        if (conn == null)
          return super.getCellEditor(row, column);

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
          for (byte b: data) {
            String hexB = "0" + Integer.toHexString(b);
            hexB = hexB.substring(hexB.length() - 2);
            tip += "0x" + hexB + " ";
          }
        } else {
          tip = super.getToolTipText(e);
        }
        return tip;
    }
    };

    TableColumn destColumn = dataTable.getColumnModel().getColumn(COLUMN_TO);
    destColumn.setCellEditor(new DefaultCellEditor(comboBox));

    final JScrollPane scrollPane = new JScrollPane(dataTable);
    dataTable.setFillsViewportHeight(true);

    add(scrollPane);

    simulation.getRadioMedium().addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        RadioConnection[] newConnections = radioMedium.getLastTickConnections();
        if (newConnections == null)
          return;
        
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
    if (data == null)
      return "[unknown data]";
    return data.length + " bytes";
  }

  public void closePlugin() {
    if (radioMediumObserver != null)
      radioMedium.deleteRadioMediumObserver(radioMediumObserver);
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable) {
    return true;
  }

}

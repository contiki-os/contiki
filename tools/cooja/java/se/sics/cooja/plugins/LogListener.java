/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: LogListener.java,v 1.16 2009/06/23 12:57:19 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.EventQueue;
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
import java.util.Vector;
import java.util.regex.PatternSyntaxException;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.RowFilter;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;
import javax.swing.table.TableRowSorter;
import org.apache.log4j.Logger;
import org.jdom.Element;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.dialogs.TableColumnAdjuster;
import se.sics.cooja.dialogs.MessageList.MessageContainer;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.interfaces.MoteID;

/**
 * A simple mote log listener.
 * Listens to all motes' log interfaces.
 *
 * @author Fredrik Osterlind, Niclas Finne
 */
@ClassDescription("Log Listener")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class LogListener extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(LogListener.class);

  private final static int COLUMN_TIME = 0;
  private final static int COLUMN_FROM = 1;
  private final static int COLUMN_DATA = 2;

  private final static String[] COLUMN_NAMES = {
    "Time",
    "Mote",
    "Data"
  };
  private final JTable logTable;
  private TableRowSorter<TableModel> logFilter;
  private ArrayList<LogData> logs = new ArrayList<LogData>();

  private Observer logObserver;
  private Simulation simulation;

  private String filterText = "";
  private JTextField filterTextField = null;
  private Color filterTextFieldBackground;

  private AbstractTableModel model;
  
  /**
   * Create a new simulation control panel.
   *
   * @param simulationToControl Simulation to control
   */
  public LogListener(final Simulation simulationToControl, final GUI gui) {
    super("Log Listener - Listening on ?? mote logs", gui);
    simulation = simulationToControl;
    int nrLogs = 0;

    model = new AbstractTableModel() {

      private static final long serialVersionUID = 3065150390849332924L;

      public String getColumnName(int col) {
        return COLUMN_NAMES[col];
      }

      public int getRowCount() {
        return logs.size();
      }

      public int getColumnCount() {
        return COLUMN_NAMES.length;
      }

      public Object getValueAt(int row, int col) {
        LogData log = logs.get(row);
        if (col == COLUMN_TIME) {
          return Long.toString(log.time);
        } else if (col == COLUMN_FROM) {
          return log.moteID;
        } else if (col == COLUMN_DATA) {
          return log.data;
        }
        return null;
      }
    };

    logTable = new JTable(model) {

      private static final long serialVersionUID = -930616018336483196L;

      public String getToolTipText(MouseEvent e) {
        java.awt.Point p = e.getPoint();
        int rowIndex = rowAtPoint(p);
        int colIndex = columnAtPoint(p);
        int columnIndex = convertColumnIndexToModel(colIndex);
        if (rowIndex < 0 || columnIndex < 0) {
          return super.getToolTipText(e);
        }
        Object v = getValueAt(rowIndex, columnIndex);
        if (v != null) {
          String t = v.toString();
          if (t.length() > 60) {
            StringBuilder sb = new StringBuilder();
            sb.append("<html>");
            do {
              sb.append(t.substring(0, 60)).append("<br>");
              t = t.substring(60);
            } while (t.length() > 60);
            return sb.append(t).append("</html>").toString();
          }
        }
        return super.getToolTipText(e);
      }

    };
    logTable.setFillsViewportHeight(true);
    logTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
    logTable.setFont(new Font("Monospaced", Font.PLAIN, 12));
    logFilter = new TableRowSorter<TableModel>(model);
    for (int i = 0, n = model.getColumnCount(); i < n; i++) {
      logFilter.setSortable(i, false);
    }
    logTable.setRowSorter(logFilter);

    /* Popup menu */
    JPopupMenu popupMenu = new JPopupMenu();
    popupMenu.add(new JMenuItem(copyAction));
    popupMenu.add(new JMenuItem(copyAllAction));
    popupMenu.add(new JMenuItem(clearAction));
    popupMenu.addSeparator();
    popupMenu.add(new JMenuItem(saveAction));
    logTable.setComponentPopupMenu(popupMenu);

    TableColumnAdjuster adjuster = new TableColumnAdjuster(logTable);
    adjuster.setDynamicAdjustment(true);
    adjuster.packColumns();

    /* Observe simulation motes for log data */
    logObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        Mote mote = (Mote) obj;
        Log moteLogInterface = (Log) obs;
        String lastMessage = moteLogInterface.getLastLogMessage();
        if (lastMessage.length() > 0 && lastMessage.charAt(lastMessage.length() - 1) == '\n') {
          lastMessage = lastMessage.substring(0, lastMessage.length() - 1);
        }
        final LogData data =
            new LogData("ID:" + getMoteID(mote),
                simulation.getSimulationTimeMillis(), lastMessage);
        java.awt.EventQueue.invokeLater(new Runnable() {
          public void run() {
            // Check if the last row is visible
            boolean isVisible = false;
            int rowCount = logTable.getRowCount();
            if (rowCount > 0) {
              Rectangle visible = logTable.getVisibleRect();
              isVisible = visible.y + visible.height >= logTable.getHeight();
            }
            int index = logs.size();
            logs.add(data);
            model.fireTableRowsInserted(index, index);
            if (isVisible) {
              logTable.scrollRectToVisible(new Rectangle(0, logTable.getHeight() - 2, 1, logTable.getHeight()));
            }
          }
        });
      }
    };
    for (Mote mote: simulation.getMotes()) {
      if (mote.getInterfaces().getLog() != null) {
        mote.getInterfaces().getLog().addObserver(logObserver);
        nrLogs++;
      }
    }

    simulation.addObserver(new Observer() {
      public void update(Observable obs, Object obj) {
        /* Reregister as log listener */
        int nrLogs = 0;
        for (Mote mote: simulation.getMotes()) {
          if (mote.getInterfaces().getLog() != null) {
            mote.getInterfaces().getLog().deleteObserver(logObserver);
            mote.getInterfaces().getLog().addObserver(logObserver);
            nrLogs++;
          }
        }
        setTitle("Log Listener - Listening on " + nrLogs + " mote logs");
      }
    });

    // Main panel
    JPanel filterPanel = new JPanel();
    filterPanel.setLayout(new BoxLayout(filterPanel, BoxLayout.X_AXIS));
    filterTextField = new JTextField("");
    filterTextFieldBackground = filterTextField.getBackground();
    filterPanel.add(Box.createHorizontalStrut(2));
    filterPanel.add(new JLabel("Filter on string: "));
    filterPanel.add(filterTextField);
    filterTextField.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        filterText = filterTextField.getText();
        setFilter(filterText);
        // Ensure last row is visible
        logTable.scrollRectToVisible(new Rectangle(0, logTable.getHeight() - 2, 1, logTable.getHeight()));
      }
    });
    filterPanel.add(Box.createHorizontalStrut(2));

    getContentPane().add(BorderLayout.CENTER, new JScrollPane(logTable));
    getContentPane().add(BorderLayout.SOUTH, filterPanel);

    setTitle("Log Listener - Listening on " + nrLogs + " mote logs");
    pack();
    setSize(gui.getDesktopPane().getWidth(), 150);
    setLocation(0, gui.getDesktopPane().getHeight() - 300);
  }

  private String getMoteID(Mote mote) {
    MoteID moteID = mote.getInterfaces().getMoteID();
    if (moteID != null) {
      return Integer.toString(moteID.getMoteID());
    }
    return mote.toString();
  }

  public void closePlugin() {
    /* Stop observing mote logs */
    for (Mote mote: simulation.getMotes()) {
      if (mote.getInterfaces().getLog() != null) {
        mote.getInterfaces().getLog().deleteObserver(logObserver);
      }
    }
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    element = new Element("filter");
    element.setText(filterText);
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();
      if ("filter".equals(name)) {
        filterText = element.getText();
        EventQueue.invokeLater(new Runnable() {
          public void run() {
            filterTextField.setText(filterText);
            setFilter(filterText);
          }
        });
      }
    }

    return true;
  }

  private void setFilter(String text) {
    try {
      if (text != null && text.length() > 0) {
        logFilter.setRowFilter(RowFilter.regexFilter(text, 1, 2));
      } else {
        logFilter.setRowFilter(null);
      }
      filterTextField.setBackground(filterTextFieldBackground);
      filterTextField.setToolTipText(null);
    } catch (PatternSyntaxException e) {
      filterTextField.setBackground(Color.red);
      filterTextField.setToolTipText("Syntax error in regular expression: " + e.getMessage());
    }
  }

  private static class LogData {
    public final String moteID;
    public final long time;
    public final String data;

    public LogData(String moteID, long time, String data) {
      this.moteID = moteID;
      this.time = time;
      this.data = data;
    }
  }

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
        for(LogData data : logs) {
          outStream.println(data.time + "  " + data.moteID + "  " + data.data);
        }
        outStream.close();
      } catch (Exception ex) {
        logger.fatal("Could not write to file: " + saveFile);
        return;
      }

    }
  };
  
  private Action clearAction = new AbstractAction("Clear") {
    public void actionPerformed(ActionEvent e) {
      int size = logs.size();
      if (size > 0) {
        logs.clear();
        model.fireTableRowsDeleted(0, size - 1);
      }
    }
  };
  
  private Action copyAction = new AbstractAction("Copy selected") {
    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      int[] selectedRows = logTable.getSelectedRows();

      StringBuilder sb = new StringBuilder();
      for (int i: selectedRows) {
        sb.append(logTable.getValueAt(i, 0));
        sb.append("\t");
        sb.append(logTable.getValueAt(i, 1));
        sb.append("\t");
        sb.append(logTable.getValueAt(i, 2));
        sb.append("\n");
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };
  
  private Action copyAllAction = new AbstractAction("Copy all") {
    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      StringBuilder sb = new StringBuilder();
      for(LogData data : logs) {
        sb.append(data.time + "  " + data.moteID + "  " + data.data + "\n");
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };
  
}

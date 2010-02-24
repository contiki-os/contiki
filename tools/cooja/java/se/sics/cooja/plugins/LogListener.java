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
 * $Id: LogListener.java,v 1.26 2010/02/24 10:45:44 fros4943 Exp $
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
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Vector;
import java.util.regex.PatternSyntaxException;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.Box;
import javax.swing.BoxLayout;
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
import se.sics.cooja.SimEventCentral.LogOutputEvent;
import se.sics.cooja.SimEventCentral.LogOutputListener;
import se.sics.cooja.dialogs.TableColumnAdjuster;

/**
 * A simple mote log listener.
 * Listens to all motes' log interfaces.
 *
 * @author Fredrik Osterlind, Niclas Finne
 */
@ClassDescription("Log Listener")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class LogListener extends VisPlugin {
  private static final long serialVersionUID = 3294595371354857261L;
  private static Logger logger = Logger.getLogger(LogListener.class);

  private final static int COLUMN_TIME = 0;
  private final static int COLUMN_FROM = 1;
  private final static int COLUMN_DATA = 2;
  private final static int COLUMN_CONCAT = 3;
  private final static String[] COLUMN_NAMES = {
    "Time",
    "Mote",
    "Message",
    "#"
  };

  private final JTable logTable;
  private TableRowSorter<TableModel> logFilter;
  private ArrayList<LogData> logs = new ArrayList<LogData>();

  private Simulation simulation;

  private JTextField filterTextField = null;
  private Color filterTextFieldBackground;

  private AbstractTableModel model;

  private LogOutputListener logOutputListener;

  /**
   * @param simulation Simulation
   * @param gui GUI
   */
  public LogListener(final Simulation simulation, final GUI gui) {
    super("Log Listener - Listening on ?? mote logs", gui);
    this.simulation = simulation;

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
          return log.strTime;
        } else if (col == COLUMN_FROM) {
          return log.strID;
        } else if (col == COLUMN_DATA) {
          return log.ev.getMessage();
        } else if (col == COLUMN_CONCAT) {
          return log.strID + ' ' + log.ev.getMessage();
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
    logTable.getColumnModel().removeColumn(logTable.getColumnModel().getColumn(COLUMN_CONCAT));
    logTable.setFillsViewportHeight(true);
    logTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
    logTable.setFont(new Font("Monospaced", Font.PLAIN, 12));
    logTable.addKeyListener(new KeyAdapter() {
      public void keyPressed(KeyEvent e) {
        if (e.getKeyCode() == KeyEvent.VK_SPACE) {
          timeLineAction.actionPerformed(null);
          radioLoggerAction.actionPerformed(null);
        }
      }
    });
    logFilter = new TableRowSorter<TableModel>(model);
    for (int i = 0, n = model.getColumnCount(); i < n; i++) {
      logFilter.setSortable(i, false);
    }
    logTable.setRowSorter(logFilter);

    /* Automatically update column widths */
    TableColumnAdjuster adjuster = new TableColumnAdjuster(logTable);
    adjuster.setDynamicAdjustment(true);
    adjuster.packColumns();

    /* Popup menu */
    JPopupMenu popupMenu = new JPopupMenu();
    popupMenu.add(new JMenuItem(copyAction));
    popupMenu.add(new JMenuItem(copyAllAction));
    popupMenu.add(new JMenuItem(clearAction));
    popupMenu.addSeparator();
    popupMenu.add(new JMenuItem(saveAction));
    popupMenu.addSeparator();
    popupMenu.add(new JMenuItem(radioLoggerAction));
    popupMenu.add(new JMenuItem(timeLineAction));
    
    logTable.setComponentPopupMenu(popupMenu);

    /* Fetch log output history */
    LogOutputEvent[] history = simulation.getEventCentral().getLogOutputHistory();
    if (history.length > 0) {
      for (LogOutputEvent historyEv: history) {
        LogData data = new LogData(historyEv);
        logs.add(data);
      }
      final int index = logs.size()-1;
      java.awt.EventQueue.invokeLater(new Runnable() {
        public void run() {
          model.fireTableRowsInserted(0, index);
          logTable.scrollRectToVisible(
              new Rectangle(0, logTable.getHeight() - 2, 1, logTable.getHeight()));
        }
      });
    }

    /* Start observing motes for new log output */
    simulation.getEventCentral().addLogOutputListener(logOutputListener = new LogOutputListener() {
      public void moteWasAdded(Mote mote) {
        /* Update title */
        updateTitle();
      }
      public void moteWasRemoved(Mote mote) {
        /* Update title */
        updateTitle();
      }
      public void newLogOutput(LogOutputEvent ev) {
        /* Display new log output */
        final LogData data = new LogData(ev);
        java.awt.EventQueue.invokeLater(new Runnable() {
          public void run() {
            /* Autoscroll */
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
              logTable.scrollRectToVisible(
                  new Rectangle(0, logTable.getHeight() - 2, 1, logTable.getHeight()));
            }
          }
        });
      }
      public void removedLogOutput(final LogOutputEvent ev) {
        java.awt.EventQueue.invokeLater(new Runnable() {
          public void run() {
            for (int i = 0, n = logs.size(); i < n; i++) {
              if (logs.get(i).ev == ev) {
                logs.remove(i);
                model.fireTableRowsDeleted(i, i);
                break;
              }
            }
          }
        });
      }
    });

    /* UI components */
    JPanel filterPanel = new JPanel();
    filterPanel.setLayout(new BoxLayout(filterPanel, BoxLayout.X_AXIS));
    filterTextField = new JTextField("");
    filterTextFieldBackground = filterTextField.getBackground();
    filterPanel.add(Box.createHorizontalStrut(2));
    filterPanel.add(new JLabel("Filter: "));
    filterPanel.add(filterTextField);
    filterTextField.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        String str = filterTextField.getText();
        setFilter(str);
        /* Autoscroll */
        logTable.scrollRectToVisible(new Rectangle(0, logTable.getHeight() - 2, 1, logTable.getHeight()));
      }
    });
    filterPanel.add(Box.createHorizontalStrut(2));

    getContentPane().add(BorderLayout.CENTER, new JScrollPane(logTable));
    getContentPane().add(BorderLayout.SOUTH, filterPanel);

    updateTitle();
    pack();
    setSize(gui.getDesktopPane().getWidth(), 150);
    setLocation(0, gui.getDesktopPane().getHeight() - 300);
  }

  private void updateTitle() {
    setTitle("Log Listener (listening on " 
        + simulation.getEventCentral().getLogOutputObservationsCount() + " log interfaces)");
  }

  public void closePlugin() {
    /* Stop observing motes */
    simulation.getEventCentral().removeLogOutputListener(logOutputListener);
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    element = new Element("filter");
    element.setText(filterTextField.getText());
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();
      if ("filter".equals(name)) {
        final String str = element.getText();
        EventQueue.invokeLater(new Runnable() {
          public void run() {
            setFilter(str);
          }
        });
      }
    }

    return true;
  }

  public String getFilter() {
    return filterTextField.getText();
  }

  public void setFilter(String str) {
    filterTextField.setText(str);

    try {
      if (str != null && str.length() > 0) {
        logFilter.setRowFilter(RowFilter.regexFilter(str, COLUMN_FROM, COLUMN_DATA, COLUMN_CONCAT));
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

  public void trySelectTime(final long time) {
    java.awt.EventQueue.invokeLater(new Runnable() {
      public void run() {
        for (int i=0; i < logs.size(); i++) {
          if (logs.get(i).ev.getTime() < time) {
            continue;
          }

          int view = logTable.convertRowIndexToView(i);
          if (view < 0) {
            continue;
          }
          logTable.scrollRectToVisible(logTable.getCellRect(view, 0, true));
          logTable.setRowSelectionInterval(view, view);
          return;
        }
      }
    });  
  }

  private static class LogData {
    public final LogOutputEvent ev;
    public final String strID; /* cached value */
    public final String strTime; /* cached value */

    public LogData(LogOutputEvent ev) {
      this.ev = ev;
      this.strID = "ID:" + ev.getMote().getID();
      this.strTime = "" + ev.getTime()/Simulation.MILLISECOND;
    }
  }

  private Action saveAction = new AbstractAction("Save to file") {
    private static final long serialVersionUID = -4140706275748686944L;

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
          outStream.println(
              data.strTime + "\t" + 
              data.strID + "\t" + 
              data.ev.getMessage());
        }
        outStream.close();
      } catch (Exception ex) {
        logger.fatal("Could not write to file: " + saveFile);
        return;
      }
    }
  };

  private Action timeLineAction = new AbstractAction("to Timeline") {
    private static final long serialVersionUID = -6358463434933029699L;

    public void actionPerformed(ActionEvent e) {
      TimeLine plugin = (TimeLine) simulation.getGUI().getStartedPlugin(TimeLine.class.getName());
      if (plugin == null) {
        logger.fatal("No Timeline plugin");
        return;
      }

      int view = logTable.getSelectedRow();
      if (view < 0) {
        return;
      }
      int model = logTable.convertRowIndexToModel(view);
      
      /* Select simulation time */
      plugin.trySelectTime(logs.get(model).ev.getTime());
    }
  };

  private Action radioLoggerAction = new AbstractAction("to Radio Logger") {
    private static final long serialVersionUID = -3041714249257346688L;

    public void actionPerformed(ActionEvent e) {
      RadioLogger plugin = (RadioLogger) simulation.getGUI().getStartedPlugin(RadioLogger.class.getName());
      if (plugin == null) {
        logger.fatal("No Radio Logger plugin");
        return;
      }

      int view = logTable.getSelectedRow();
      if (view < 0) {
        return;
      }
      int model = logTable.convertRowIndexToModel(view);
      
      /* Select simulation time */
      plugin.trySelectTime(logs.get(model).ev.getTime());
    }
  };

  
  private Action clearAction = new AbstractAction("Clear") {
    private static final long serialVersionUID = -2115620313183440224L;

    public void actionPerformed(ActionEvent e) {
      int size = logs.size();
      if (size > 0) {
        logs.clear();
        model.fireTableRowsDeleted(0, size - 1);
      }
    }
  };

  private Action copyAction = new AbstractAction("Copy selected") {
    private static final long serialVersionUID = -8433490108577001803L;

    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      int[] selectedRows = logTable.getSelectedRows();

      StringBuilder sb = new StringBuilder();
      for (int i: selectedRows) {
        sb.append(logTable.getValueAt(i, COLUMN_TIME));
        sb.append("\t");
        sb.append(logTable.getValueAt(i, COLUMN_FROM));
        sb.append("\t");
        sb.append(logTable.getValueAt(i, COLUMN_DATA));
        sb.append("\n");
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };

  private Action copyAllAction = new AbstractAction("Copy all") {
    private static final long serialVersionUID = -5038884975254178373L;

    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      StringBuilder sb = new StringBuilder();
      for(LogData data : logs) {
        sb.append(data.strTime);
        sb.append("\t");
        sb.append(data.strID);
        sb.append("\t");
        sb.append(data.ev.getMessage());
        sb.append("\n");
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };

}

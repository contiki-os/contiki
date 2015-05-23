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
 */

package org.contikios.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
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
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.regex.PatternSyntaxException;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.RowFilter;
import javax.swing.SwingUtilities;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableModel;
import javax.swing.table.TableRowSorter;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.HasQuickHelp;
import org.contikios.cooja.Mote;
import org.contikios.cooja.Plugin;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.SimEventCentral.LogOutputEvent;
import org.contikios.cooja.SimEventCentral.LogOutputListener;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.VisPlugin;
import org.contikios.cooja.dialogs.TableColumnAdjuster;
import org.contikios.cooja.dialogs.UpdateAggregator;
import org.contikios.cooja.util.ArrayQueue;

/**
 * A simple mote log listener.
 * Listens to all motes' log interfaces.
 *
 * @author Fredrik Osterlind, Niclas Finne
 */
@ClassDescription("Mote output")
@PluginType(PluginType.SIM_STANDARD_PLUGIN)
public class LogListener extends VisPlugin implements HasQuickHelp {
  private static final long serialVersionUID = 3294595371354857261L;
  private static Logger logger = Logger.getLogger(LogListener.class);

  private final Color[] BG_COLORS = new Color[] {
      new Color(200, 200, 200),
      new Color(200, 200, 255),
      new Color(200, 255, 200),
      new Color(200, 255, 255),
      new Color(255, 200, 200),
      new Color(255, 255, 200),
      new Color(255, 255, 255),
      new Color(255, 220, 200),
      new Color(220, 255, 220),
      new Color(255, 200, 255),
  };

  private final static int COLUMN_TIME = 0;
  private final static int COLUMN_FROM = 1;
  private final static int COLUMN_DATA = 2;
  private final static int COLUMN_CONCAT = 3;
  private final static String[] COLUMN_NAMES = {
    "Time ms",
    "Mote",
    "Message",
    "#"
  };

  public static final long TIME_SECOND = 1000*Simulation.MILLISECOND;
  public static final long TIME_MINUTE = 60*TIME_SECOND;
  public static final long TIME_HOUR = 60*TIME_MINUTE;

  private boolean formatTimeString = true;
  private boolean hasHours = false;

  private final JTable logTable;
  private TableRowSorter<TableModel> logFilter;
  private ArrayQueue<LogData> logs = new ArrayQueue<LogData>();

  private Simulation simulation;

  private JTextField filterTextField = null;
  private JLabel filterLabel = new JLabel("Filter: ");
  private Color filterTextFieldBackground;

  private AbstractTableModel model;

  private LogOutputListener logOutputListener;

  private boolean backgroundColors = true;
  private JCheckBoxMenuItem colorCheckbox;

  private boolean inverseFilter = false;
  private JCheckBoxMenuItem inverseFilterCheckbox;

  private boolean hideDebug = false;
  private JCheckBoxMenuItem hideDebugCheckbox;

  private JCheckBoxMenuItem appendCheckBox;

  private static final int UPDATE_INTERVAL = 250;
  private UpdateAggregator<LogData> logUpdateAggregator = new UpdateAggregator<LogData>(UPDATE_INTERVAL) {
    private Runnable scroll = new Runnable() {
      public void run() {
        logTable.scrollRectToVisible(
            new Rectangle(0, logTable.getHeight() - 2, 1, logTable.getHeight()));
      }
    };
    protected void handle(List<LogData> ls) {
      boolean isVisible = true;
      if (logTable.getRowCount() > 0) {
        Rectangle visible = logTable.getVisibleRect();
        if (visible.y + visible.height < logTable.getHeight()) {
          isVisible = false;
        }
      }

      /* Add */
      int index = logs.size();
      logs.addAll(ls);
      model.fireTableRowsInserted(index, logs.size()-1);

      /* Remove old */
      int removed = 0;
      while (logs.size() > simulation.getEventCentral().getLogOutputBufferSize()) {
        logs.remove(0);
        removed++;
      }
      if (removed > 0) {
        model.fireTableRowsDeleted(0, removed-1);
      }

      if (isVisible) {
        SwingUtilities.invokeLater(scroll);
      }
    }
  };

  /**
   * @param simulation Simulation
   * @param gui GUI
   */
  public LogListener(final Simulation simulation, final Cooja gui) {
    super("Mote output", gui);
    this.simulation = simulation;

    /* Menus */
    JMenuBar menuBar = new JMenuBar();
    JMenu fileMenu = new JMenu("File");
    JMenu editMenu = new JMenu("Edit");
    JMenu showMenu = new JMenu("View");

    menuBar.add(fileMenu);
    menuBar.add(editMenu);
    menuBar.add(showMenu);
    this.setJMenuBar(menuBar);

    editMenu.add(new JMenuItem(copyAllAction));
    editMenu.add(new JMenuItem(copyAllMessagesAction));
    editMenu.add(new JMenuItem(copyAction));
    editMenu.addSeparator();
    editMenu.add(new JMenuItem(clearAction));


    fileMenu.add(new JMenuItem(saveAction));
    appendCheckBox = new JCheckBoxMenuItem(appendAction);
    fileMenu.add(appendCheckBox);

    colorCheckbox = new JCheckBoxMenuItem("Mote-specific coloring", backgroundColors);
    showMenu.add(colorCheckbox);
    colorCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        backgroundColors = colorCheckbox.isSelected();
        repaint();
      }
    });
    hideDebugCheckbox = new JCheckBoxMenuItem("Hide \"DEBUG: \" messages");
    showMenu.add(hideDebugCheckbox);
    hideDebugCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        hideDebug = hideDebugCheckbox.isSelected();
        setFilter(getFilter());
        repaint();
      }
    });
    inverseFilterCheckbox = new JCheckBoxMenuItem("Inverse filter");
    showMenu.add(inverseFilterCheckbox);
    inverseFilterCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        inverseFilter = inverseFilterCheckbox.isSelected();
        if (inverseFilter) {
          filterLabel.setText("Exclude:");
        } else {
          filterLabel.setText("Filter:");
        }
        setFilter(getFilter());
        repaint();
      }
    });


    model = new AbstractTableModel() {
      private static final long serialVersionUID = 3065150390849332924L;
      public String getColumnName(int col) {
      	if (col == COLUMN_TIME && formatTimeString) {
    			return "Time";
      	}
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
          return log.getTime();
        } else if (col == COLUMN_FROM) {
          return log.getID();
        } else if (col == COLUMN_DATA) {
          return log.ev.getMessage();
        } else if (col == COLUMN_CONCAT) {
          return log.getID() + ' ' + log.ev.getMessage();
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
    DefaultTableCellRenderer cellRenderer = new DefaultTableCellRenderer() {
      private static final long serialVersionUID = -340743275865216182L;
      public Component getTableCellRendererComponent(JTable table,
          Object value, boolean isSelected, boolean hasFocus, int row,
          int column) {
      	if (row >= logTable.getRowCount()) {
          return super.getTableCellRendererComponent(
              table, value, isSelected, hasFocus, row, column);
      	}

      	if (backgroundColors) {
          LogData d = logs.get(logTable.getRowSorter().convertRowIndexToModel(row));
          int color = (10+d.ev.getMote().getID())%10;
          setBackground(BG_COLORS[color]);
        } else {
          setBackground(null);
        }

        return super.getTableCellRendererComponent(
            table, value, isSelected, hasFocus, row, column);
      }
    };
    logTable.getColumnModel().getColumn(COLUMN_TIME).setCellRenderer(cellRenderer);
    logTable.getColumnModel().getColumn(COLUMN_FROM).setCellRenderer(cellRenderer);
    logTable.getColumnModel().getColumn(COLUMN_DATA).setCellRenderer(cellRenderer);
    logTable.getColumnModel().removeColumn(logTable.getColumnModel().getColumn(COLUMN_CONCAT));
    logTable.setFillsViewportHeight(true);
    logTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
    logTable.setFont(new Font("Monospaced", Font.PLAIN, 12));
    logTable.addKeyListener(new KeyAdapter() {
      public void keyPressed(KeyEvent e) {
        if (e.getKeyCode() == KeyEvent.VK_SPACE) {
          showInAllAction.actionPerformed(null);
        }
      }
    });
    logFilter = new TableRowSorter<TableModel>(model);
    for (int i = 0, n = model.getColumnCount(); i < n; i++) {
      logFilter.setSortable(i, false);
    }
    logTable.setRowSorter(logFilter);

    /* Toggle time format */
    logTable.getTableHeader().addMouseListener(new MouseAdapter() {
    	public void mouseClicked(MouseEvent e) {
        int colIndex = logTable.columnAtPoint(e.getPoint());
        int columnIndex = logTable.convertColumnIndexToModel(colIndex);
        if (columnIndex != COLUMN_TIME) {
        	return;
        }
    		formatTimeString = !formatTimeString;
    		repaintTimeColumn();
    	}
		});
    logTable.addMouseListener(new MouseAdapter() {
    	public void mouseClicked(MouseEvent e) {
        int colIndex = logTable.columnAtPoint(e.getPoint());
        int columnIndex = logTable.convertColumnIndexToModel(colIndex);
        if (columnIndex != COLUMN_FROM) {
        	return;
        }

        int rowIndex = logTable.rowAtPoint(e.getPoint());
        if (rowIndex == -1) {
          return;
        }
        LogData d = logs.get(logTable.getRowSorter().convertRowIndexToModel(rowIndex));
        if (d == null) {
        	return;
        }
        simulation.getCooja().signalMoteHighlight(d.ev.getMote());
    	}
		});

    /* Automatically update column widths */
    final TableColumnAdjuster adjuster = new TableColumnAdjuster(logTable);
    adjuster.packColumns();

    /* Popup menu */

    JPopupMenu popupMenu = new JPopupMenu();
    /*
    JMenu copyClipboard = new JMenu("Copy to clipboard");
    copyClipboard.add(new JMenuItem(copyAllAction));
    copyClipboard.add(new JMenuItem(copyAllMessagesAction));
    copyClipboard.add(new JMenuItem(copyAction));
    popupMenu.add(copyClipboard);
    popupMenu.add(new JMenuItem(clearAction));
    popupMenu.addSeparator();
    popupMenu.add(new JMenuItem(saveAction));
    appendCheckBox = new JCheckBoxMenuItem(appendAction);
    popupMenu.add(appendCheckBox);
    popupMenu.addSeparator();
    */
    JMenu focusMenu = new JMenu("Show in");
    focusMenu.add(new JMenuItem(showInAllAction));
    focusMenu.addSeparator();
    focusMenu.add(new JMenuItem(timeLineAction));
    focusMenu.add(new JMenuItem(radioLoggerAction));
    popupMenu.add(focusMenu);
    /*
    popupMenu.addSeparator();
    colorCheckbox = new JCheckBoxMenuItem("Mote-specific coloring");
    popupMenu.add(colorCheckbox);
    colorCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        backgroundColors = colorCheckbox.isSelected();
        repaint();
      }
    });
    hideDebugCheckbox = new JCheckBoxMenuItem("Hide \"DEBUG: \" messages");
    popupMenu.add(hideDebugCheckbox);
    hideDebugCheckbox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        hideDebug = hideDebugCheckbox.isSelected();
        setFilter(getFilter());
        repaint();
      }
    });
    inverseFilterCheckbox = new JCheckBoxMenuItem("Inverse filter");
    popupMenu.add(inverseFilterCheckbox);
    inverseFilterCheckbox.addActionListener(new ActionListener() {
    	public void actionPerformed(ActionEvent e) {
    		inverseFilter = inverseFilterCheckbox.isSelected();
    		if (inverseFilter) {
    			filterLabel.setText("Exclude:");
    		} else {
    			filterLabel.setText("Filter:");
    		}
        setFilter(getFilter());
    		repaint();
    	}
    });


    logTable.setComponentPopupMenu(popupMenu);
*/
    /* Fetch log output history */
    LogOutputEvent[] history = simulation.getEventCentral().getLogOutputHistory();
    if (history.length > 0) {
      for (LogOutputEvent historyEv: history) {
      	if (!hasHours && historyEv.getTime() > TIME_HOUR) {
      		hasHours = true;
      		repaintTimeColumn();
      	}
        LogData data = new LogData(historyEv);
        logs.add(data);
      }
      java.awt.EventQueue.invokeLater(new Runnable() {
        public void run() {
          model.fireTableDataChanged();
          logTable.scrollRectToVisible(
              new Rectangle(0, logTable.getHeight() - 2, 1, logTable.getHeight()));
        }
      });
    }

    /* Column width adjustment */
    java.awt.EventQueue.invokeLater(new Runnable() {
      public void run() {
        /* Make sure this happens *after* adding history */
        adjuster.setDynamicAdjustment(true);
      }
    });

    /* Start observing motes for new log output */
    logUpdateAggregator.start();
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
        registerNewLogOutput(ev);
      }
      public void removedLogOutput(LogOutputEvent ev) {
      }
    });

    /* UI components */
    JPanel filterPanel = new JPanel();
    filterPanel.setLayout(new BoxLayout(filterPanel, BoxLayout.X_AXIS));
    filterTextField = new JTextField("");
    filterTextFieldBackground = filterTextField.getBackground();
    filterPanel.add(Box.createHorizontalStrut(2));
    filterPanel.add(filterLabel);
    filterPanel.add(filterTextField);
    filterTextField.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        String str = filterTextField.getText();
        setFilter(str);

        /* Autoscroll */
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            int s = logTable.getSelectedRow();
            if (s < 0) {
              return;
            }

            s = logTable.getRowSorter().convertRowIndexToView(s);
            if (s < 0) {
              return;
            }

            int v = logTable.getRowHeight()*s;
            logTable.scrollRectToVisible(new Rectangle(0, v-5, 1, v+5));
          }
        });
      }
    });
    filterPanel.add(Box.createHorizontalStrut(2));

    getContentPane().add(BorderLayout.CENTER, new JScrollPane(logTable));
    getContentPane().add(BorderLayout.SOUTH, filterPanel);

    updateTitle();
    pack();

    /* XXX HACK: here we set the position and size of the window when it appears on a blank simulation screen. */
    this.setLocation(400, 160);
    this.setSize(gui.getDesktopPane().getWidth() - 400, 240);
  }

  public void registerNewLogOutput(Mote mote, long time, String msg) {
    LogOutputEvent ev = new LogOutputEvent(mote, time, msg);
    registerNewLogOutput(ev);
  }

  private void registerNewLogOutput(LogOutputEvent ev) {
    /* Display new log output */
    if (!hasHours && ev.getTime() > TIME_HOUR) {
            hasHours = true;
            repaintTimeColumn();
    }
    LogData data = new LogData(ev);
    logUpdateAggregator.add(data);
    if (appendToFile) {
      appendToFile(appendStreamFile,
          data.getTime() + "\t" +
          data.getID() + "\t" +
          data.ev.getMessage() + "\n"
      );
    }
  }

  private void repaintTimeColumn() {
  	logTable.getColumnModel().getColumn(COLUMN_TIME).setHeaderValue(
  			logTable.getModel().getColumnName(COLUMN_TIME));
  	repaint();
	}

	private void updateTitle() {
 /*   setTitle("Log Listener listening on "
        + simulation.getEventCentral().getLogOutputObservationsCount() + " log interfaces");*/
  }

  public void closePlugin() {
    /* Stop observing motes */
    appendToFile(null, null);
    logUpdateAggregator.stop();
    simulation.getEventCentral().removeLogOutputListener(logOutputListener);
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    element = new Element("filter");
    element.setText(filterTextField.getText());
    config.add(element);

    if (formatTimeString) {
    	element = new Element("formatted_time");
    	config.add(element);
    }
    if (backgroundColors) {
      element = new Element("coloring");
      config.add(element);
    }
    if (hideDebug) {
      element = new Element("hidedebug");
      config.add(element);
    }
    if (inverseFilter) {
    	element = new Element("inversefilter");
    	config.add(element);
    }
    if (appendToFile) {
      element = new Element("append");
      element.setText(simulation.getCooja().createPortablePath(appendStreamFile).getPath());
      config.add(element);
    }
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
      } else if ("coloring".equals(name)) {
        backgroundColors = true;
        colorCheckbox.setSelected(true);
      } else if ("hidedebug".equals(name)) {
        hideDebug = true;
        hideDebugCheckbox.setSelected(true);
      } else if ("inversefilter".equals(name)) {
      	inverseFilter = true;
      	inverseFilterCheckbox.setSelected(true);
      } else if ("formatted_time".equals(name)) {
      	formatTimeString = true;
      	repaintTimeColumn();
      } else if ("append".equals(name)) {
        appendToFile = true;
        appendStreamFile = simulation.getCooja().restorePortablePath(new File(element.getText()));
        appendCheckBox.setSelected(true);
        if (!appendStreamFile.exists()) {
          try {
            appendStreamFile.createNewFile();
          } catch (IOException e) {
          }
        }
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
    	final RowFilter<Object,Object> regexp;
      if (str != null && str.length() > 0) {
      	regexp = RowFilter.regexFilter(str, COLUMN_FROM, COLUMN_DATA, COLUMN_CONCAT);
      } else {
      	regexp = null;
      }
    	RowFilter<Object, Object> wrapped = new RowFilter<Object, Object>() {
    		public boolean include(RowFilter.Entry<? extends Object, ? extends Object> entry) {
    		  if (regexp != null) {
    				boolean pass = regexp.include(entry);
    				if (inverseFilter && pass) {
    					return false;
    				} else if (!inverseFilter && !pass) {
    					return false;
    				}
    			}
    			if (hideDebug) {
    				if (entry.getStringValue(COLUMN_DATA).startsWith("DEBUG: ")) {
    					return false;
    				}
    			}
    			return true;
    		}
    	};
      logFilter.setRowFilter(wrapped);
      filterTextField.setBackground(filterTextFieldBackground);
      filterTextField.setToolTipText(null);
    } catch (PatternSyntaxException e) {
      logFilter.setRowFilter(null);
      filterTextField.setBackground(Color.red);
      filterTextField.setToolTipText("Syntax error in regular expression: " + e.getMessage());
    }
    simulation.getCooja().getDesktopPane().repaint();
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

  private class LogData {
    public final LogOutputEvent ev;
    public LogData(LogOutputEvent ev) {
      this.ev = ev;
    }

    public String getID() {
      return "ID:" + ev.getMote().getID();
    }

    public String getTime() {
      if (formatTimeString) {
        return getFormattedTime(ev.getTime());
      } else {
        return "" + ev.getTime() / Simulation.MILLISECOND;
      }
    }
  }

  private Action saveAction = new AbstractAction("Save to file") {
    private static final long serialVersionUID = -4140706275748686944L;

    public void actionPerformed(ActionEvent e) {
      JFileChooser fc = new JFileChooser();
      File suggest = new File(Cooja.getExternalToolsSetting("LOG_LISTENER_SAVEFILE", "loglistener.txt"));
      fc.setSelectedFile(suggest);
      int returnVal = fc.showSaveDialog(Cooja.getTopParentContainer());
      if (returnVal != JFileChooser.APPROVE_OPTION) {
        return;
      }

      File saveFile = fc.getSelectedFile();
      if (saveFile.exists()) {
        String s1 = "Overwrite";
        String s2 = "Cancel";
        Object[] options = { s1, s2 };
        int n = JOptionPane.showOptionDialog(
            Cooja.getTopParentContainer(),
            "A file with the same name already exists.\nDo you want to remove it?",
            "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, options, s1);
        if (n != JOptionPane.YES_OPTION) {
          return;
        }
      }

      Cooja.setExternalToolsSetting("LOG_LISTENER_SAVEFILE", saveFile.getPath());
      if (saveFile.exists() && !saveFile.canWrite()) {
        logger.fatal("No write access to file: " + saveFile);
        return;
      }

      try {
        PrintWriter outStream = new PrintWriter(new FileWriter(saveFile));
        for(LogData data : logs) {
          outStream.println(
              data.getTime() + "\t" +
              data.getID() + "\t" +
              data.ev.getMessage());
        }
        outStream.close();
      } catch (Exception ex) {
        logger.fatal("Could not write to file: " + saveFile);
        return;
      }
    }
  };

  private boolean appendToFile = false;
  private File appendStreamFile = null;
  private boolean appendToFileWroteHeader = false;
  private PrintWriter appendStream = null;
  public boolean appendToFile(File file, String text) {
    /* Close stream */
    if (file == null) {
      if (appendStream != null) {
        appendStream.close();
        appendStream = null;
      }
      return false;
    }

    /* Open stream */
    if (appendStream == null || file != appendStreamFile) {
      try {
        if (appendStream != null) {
          appendStream.close();
          appendStream = null;
        }
        appendStream = new PrintWriter(new FileWriter(file,true));
        appendStreamFile = file;
        appendToFileWroteHeader = false;
      } catch (Exception ex) {
        logger.fatal("Append file failed: " + ex.getMessage(), ex);
        return false;
      }
    }

    /* Append to file */
    if (!appendToFileWroteHeader) {
      appendStream.println("-- Log Listener [" + simulation.getTitle() + "]: Started at " + (new Date()).toString());
      appendToFileWroteHeader = true;
    }
    appendStream.print(text);
    appendStream.flush();
    return true;
  }

  private Action appendAction = new AbstractAction("Append to file") {
    private static final long serialVersionUID = -3041714249257346688L;
    public void actionPerformed(ActionEvent e) {
      JCheckBoxMenuItem cb = (JCheckBoxMenuItem) e.getSource();
      appendToFile = cb.isSelected();
      if (!appendToFile) {
        appendToFile(null, null);
        appendStreamFile = null;
        return;
      }

      JFileChooser fc = new JFileChooser();
      File suggest = new File(Cooja.getExternalToolsSetting("LOG_LISTENER_APPENDFILE", "loglistener_append.txt"));
      fc.setSelectedFile(suggest);
      int returnVal = fc.showSaveDialog(Cooja.getTopParentContainer());
      if (returnVal != JFileChooser.APPROVE_OPTION) {
        appendToFile = false;
        cb.setSelected(appendToFile);
        return;
      }

      File saveFile = fc.getSelectedFile();
      Cooja.setExternalToolsSetting("LOG_LISTENER_APPENDFILE", saveFile.getPath());
      if (saveFile.exists() && !saveFile.canWrite()) {
        logger.fatal("No write access to file: " + saveFile);
        appendToFile = false;
        cb.setSelected(appendToFile);
        return;
      }
      appendToFile = true;
      appendStreamFile = saveFile;
      if (!appendStreamFile.exists()) {
        try {
          appendStreamFile.createNewFile();
        } catch (IOException ex) {
        }
      }
    }
  };

  private Action timeLineAction = new AbstractAction("Timeline") {
    private static final long serialVersionUID = -6358463434933029699L;
    public void actionPerformed(ActionEvent e) {
      int view = logTable.getSelectedRow();
      if (view < 0) {
        return;
      }
      int model = logTable.convertRowIndexToModel(view);
      long time = logs.get(model).ev.getTime();

      Plugin[] plugins = simulation.getCooja().getStartedPlugins();
      for (Plugin p: plugins) {
      	if (!(p instanceof TimeLine)) {
      		continue;
      	}

        /* Select simulation time */
      	TimeLine plugin = (TimeLine) p;
        plugin.trySelectTime(time);
      }
    }
  };

  private Action radioLoggerAction = new AbstractAction("Radio Logger") {
    private static final long serialVersionUID = -3041714249257346688L;
    public void actionPerformed(ActionEvent e) {
      int view = logTable.getSelectedRow();
      if (view < 0) {
        return;
      }
      int model = logTable.convertRowIndexToModel(view);
      long time = logs.get(model).ev.getTime();

      Plugin[] plugins = simulation.getCooja().getStartedPlugins();
      for (Plugin p: plugins) {
      	if (!(p instanceof RadioLogger)) {
      		continue;
      	}

        /* Select simulation time */
      	RadioLogger plugin = (RadioLogger) p;
        plugin.trySelectTime(time);
      }
    }
  };

  private Action showInAllAction = new AbstractAction("All") {
    private static final long serialVersionUID = -8433490108577001803L;
    {
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_SPACE, 0, true));
    }

    public void actionPerformed(ActionEvent e) {
      timeLineAction.actionPerformed(null);
      radioLoggerAction.actionPerformed(null);
    }
  };

  private Action clearAction = new AbstractAction("Clear all messages") {
    private static final long serialVersionUID = -2115620313183440224L;

    public void actionPerformed(ActionEvent e) {
      clear();
    }
  };

  public void clear() {
    int size = logs.size();
    if (size > 0) {
      logs.clear();
      model.fireTableRowsDeleted(0, size - 1);
    }
  }

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

  private Action copyAllAction = new AbstractAction("Copy all data") {
    private static final long serialVersionUID = -5038884975254178373L;

    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      StringBuilder sb = new StringBuilder();
      for(LogData data : logs) {
        sb.append(data.getTime());
        sb.append("\t");
        sb.append(data.getID());
        sb.append("\t");
        sb.append(data.ev.getMessage());
        sb.append("\n");
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };

  private Action copyAllMessagesAction = new AbstractAction("Copy all messages") {
    private static final long serialVersionUID = -5038884975254178373L;

    public void actionPerformed(ActionEvent e) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();

      StringBuilder sb = new StringBuilder();
      for(LogData data : logs) {
        sb.append(data.ev.getMessage());
        sb.append("\n");
      }

      StringSelection stringSelection = new StringSelection(sb.toString());
      clipboard.setContents(stringSelection, null);
    }
  };

  public String getQuickHelp() {
    return
        "<b>Log Listener</b>" +
        "<p>Listens to log output from all simulated motes. " +
        "Right-click the main area for a popup menu with more options. " +
        "<p>You may filter shown logs by entering regular expressions in the bottom text field. " +
        "Filtering is performed on both the Mote and the Data columns." +
        "<p><b>Filter examples:</b> " +
        "<br><br>Hello<br><i>logs containing the string 'Hello'</i>" +
        "<br><br>^Contiki<br><i>logs starting with 'Contiki'</i>" +
        "<br><br>^[CR]<br><i>logs starting either a C or an R</i>" +
        "<br><br>Hello$<br><i>logs ending with 'Hello'</i>" +
        "<br><br>^ID:[2-5]$<br><i>logs from motes 2 to 5</i>" +
        "<br><br>^ID:[2-5] Contiki<br><i>logs from motes 2 to 5 starting with 'Contiki'</i>";
  }

  /* Experimental feature: let other plugins learn if a log output would be filtered or not */
  public boolean filterWouldAccept(LogOutputEvent ev) {
    RowFilter<? super TableModel, ? super Integer> rowFilter = logFilter.getRowFilter();
    if (rowFilter == null) {
      /* No filter */
      return true;
    }

    final LogData ld = new LogData(ev);
    RowFilter.Entry<? extends TableModel, ? extends Integer> entry = new RowFilter.Entry<TableModel, Integer>() {
      public TableModel getModel() {
        return model;
      }
      public int getValueCount() {
        return model.getColumnCount();
      }
      public Object getValue(int index) {
        if (index == COLUMN_TIME) {
          return ld.getTime();
        } else if (index == COLUMN_FROM) {
          return ld.getID();
        } else if (index == COLUMN_DATA) {
          return ld.ev.getMessage();
        } else if (index == COLUMN_CONCAT) {
          return ld.getID() + ' ' + ld.ev.getMessage();
        }
        return null;
      }
      public Integer getIdentifier() {
        return null;
      }
    };
    boolean show;
    show = rowFilter.include(entry);
    return show;
  }
  public Color getColorOfEntry(LogOutputEvent logEvent) {
    int color = (10+logEvent.getMote().getID())%10;
    return BG_COLORS[color];
  }

  public static String getFormattedTime(long t) {
    long h = (t / LogListener.TIME_HOUR);
    t -= (t / TIME_HOUR)*TIME_HOUR;
    long m = (t / TIME_MINUTE);
    t -= (t / TIME_MINUTE)*TIME_MINUTE;
    long s = (t / TIME_SECOND);
    t -= (t / TIME_SECOND)*TIME_SECOND;
    long ms = t / Simulation.MILLISECOND;
    if (h > 0) {
      return String.format("%d:%02d:%02d.%03d", h,m,s,ms);
    } else {
      return String.format("%02d:%02d.%03d", m,s,ms);
    }
  }

}

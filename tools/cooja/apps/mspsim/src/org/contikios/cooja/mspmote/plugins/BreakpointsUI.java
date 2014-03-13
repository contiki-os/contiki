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

package org.contikios.cooja.mspmote.plugins;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JColorChooser;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JSeparator;
import javax.swing.JTable;
import javax.swing.SwingUtilities;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;

import org.apache.log4j.Logger;

import org.contikios.cooja.Cooja;
import org.contikios.cooja.Watchpoint;
import org.contikios.cooja.WatchpointMote;

/**
 * Displays a set of breakpoints.
 * 
 * @author Fredrik Osterlind
 */
public class BreakpointsUI extends JPanel {
  private static Logger logger = Logger.getLogger(BreakpointsUI.class);

  private static final int COLUMN_ADDRESS = 0;
  private static final int COLUMN_FILELINE = 1;
  private static final int COLUMN_INFO = 2;
  private static final int COLUMN_STOP = 3;

  private static final String[] COLUMN_NAMES = {
    "Address",
    "Source",
    "Info",
    "Stops simulation"
  };

  private WatchpointMote mote;
  private MspCodeWatcher codeWatcher;
  private JTable table = null;

  private Watchpoint selectedWatchpoint = null;

  public BreakpointsUI(WatchpointMote mote, final MspCodeWatcher codeWatcher) {
    this.mote = mote;
    this.codeWatcher = codeWatcher;

    /* Breakpoints table */
    table = new JTable(tableModel) {
      public String getToolTipText(MouseEvent e) {

        /* Tooltips: show full file paths */
        java.awt.Point p = e.getPoint();
        int rowIndex = table.rowAtPoint(p);
        int colIndex = table.columnAtPoint(p);
        int realColumnIndex = table.convertColumnIndexToModel(colIndex);

        if (realColumnIndex == COLUMN_FILELINE) {
          Watchpoint[] allBreakpoints = BreakpointsUI.this.mote.getBreakpoints();
          if (rowIndex < 0 || rowIndex >= allBreakpoints.length) {
            return null;
          }
          Watchpoint watchpoint = allBreakpoints[rowIndex];
          File file = watchpoint.getCodeFile();
          if (file == null) {
            return String.format("[unknown @ 0x%04x]", watchpoint.getExecutableAddress());
          }
          Integer line = watchpoint.getLineNumber();
          if (line == null) {
            return file.getPath() + ":?";
          }
          return file.getPath() + ":" + line;
        }

        if (realColumnIndex == COLUMN_INFO) {
          return "Optional watchpoint info: description and color"; 
        }

        if (realColumnIndex == COLUMN_STOP) {
          return "Indicates whether the watchpoint will stop the simulation when triggered"; 
        }
        return null;
      }
    };
    table.getColumnModel().getColumn(COLUMN_INFO).setCellRenderer(new DefaultTableCellRenderer() {
      public Component getTableCellRendererComponent(JTable table, Object value, 
          boolean isSelected, boolean hasFocus, int row, int column) {
        Component c = super.getTableCellRendererComponent(
            table, value, isSelected, hasFocus, row, column);
        if (column != COLUMN_INFO) {
          return c;
        }

        Watchpoint[] allBreakpoints = BreakpointsUI.this.mote.getBreakpoints();
        if (row < 0 || row >= allBreakpoints.length) {
          return c;
        }
        Watchpoint breakpoint = allBreakpoints[row];
        if (breakpoint.getColor() == null) {
          return c;
        }

        /* Use watchpoint color */
        c.setBackground(Color.WHITE);
        c.setForeground(breakpoint.getColor());
        return c;
      }
    });

    /* Popup menu: register on all motes */
    final JPopupMenu popupMenu = new JPopupMenu();
    popupMenu.add(new JMenuItem(gotoCodeAction));
    popupMenu.add(new JSeparator());
    popupMenu.add(new JMenuItem(removeWatchpointAction));
    popupMenu.add(new JMenuItem(configureWatchpointAction));

    table.addMouseListener(new MouseAdapter() {
      public void mouseClicked(MouseEvent e) {
        java.awt.Point p = e.getPoint();
        int rowIndex = table.rowAtPoint(p);
        int colIndex = table.columnAtPoint(p);
        int realColumnIndex = table.convertColumnIndexToModel(colIndex);

        if (realColumnIndex != COLUMN_ADDRESS 
            && realColumnIndex != COLUMN_FILELINE
            && realColumnIndex != COLUMN_INFO) {
          return;
        }

        Watchpoint[] allBreakpoints = BreakpointsUI.this.mote.getBreakpoints();
        if (rowIndex < 0 || rowIndex >= allBreakpoints.length) {
          return;
        }
        Watchpoint breakpoint = allBreakpoints[rowIndex];

        if (e.isPopupTrigger() || SwingUtilities.isRightMouseButton(e)) {
          selectedWatchpoint = breakpoint;
          popupMenu.show(table, e.getX(), e.getY());
          return;
        }

        if (realColumnIndex == COLUMN_INFO) {
          configureWatchpointInfo(breakpoint);
          return;
        }

        /*File file = allBreakpoints[rowIndex].getCodeFile();
        int line = allBreakpoints[rowIndex].getLineNumber();
        if (file == null) {
          return;
        }*/
      }
    });

    setLayout(new BorderLayout());
    add(BorderLayout.NORTH, table.getTableHeader());
    add(BorderLayout.CENTER, table);
  }

  private void configureWatchpointInfo(Watchpoint breakpoint) {
    String msg = (String) JOptionPane.showInputDialog(
        Cooja.getTopParentContainer(),
        "Enter description;",
        "Watchpoint description",
        JOptionPane.QUESTION_MESSAGE, null, null, breakpoint.getUserMessage());
    if (msg == null) {
      return;
    }
    breakpoint.setUserMessage(msg);
    Color newColor = JColorChooser.showDialog(
        Cooja.getTopParentContainer(),
        "Watchpoint color", 
        breakpoint.getColor());
    if (newColor == null) {
      return;
    }
    breakpoint.setColor(newColor);
  }

  public void selectBreakpoint(final Watchpoint breakpoint) {
    if (breakpoint == null) {
      return;
    }
    /* Locate breakpoints table index */
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        Watchpoint[] watchpoints = mote.getBreakpoints();
        for (int i=0; i < watchpoints.length; i++) {
          if (breakpoint == watchpoints[i]) {
            /* Select */
            table.setRowSelectionInterval(i, i);
            return;
          }
        }
      }
    });
  }

  private AbstractTableModel tableModel = new AbstractTableModel() {
    public String getColumnName(int col) {
      return COLUMN_NAMES[col].toString();
    }
    public int getRowCount() {
      return mote.getBreakpoints().length;
    }
    public int getColumnCount() {
      return COLUMN_NAMES.length;
    }
    public Object getValueAt(int row, int col) {
      Watchpoint breakpoint = mote.getBreakpoints()[row];

      /* Executable address in hexadecimal */
      if (col == COLUMN_ADDRESS) {
        Integer address = breakpoint.getExecutableAddress();
        return String.format("0x%04x", address.intValue());
      }

      /* Source file + line number */
      if (col == COLUMN_FILELINE) {
        File file = breakpoint.getCodeFile();
        if (file == null) {
          return "";
        }
        return file.getName() + ":" + breakpoint.getLineNumber();
      }

      if (col == COLUMN_INFO) {
        if (breakpoint.getUserMessage() != null) {
          return breakpoint.getUserMessage();
        }
        return "";
      }

      /* Stops simulation */
      if (col == COLUMN_STOP) {
        return breakpoint.stopsSimulation();
      }

      return new Boolean(false);
    }
    public boolean isCellEditable(int row, int col){
      return getColumnClass(col) == Boolean.class;
    }
    public void setValueAt(Object value, int row, int col) {
      Watchpoint breakpoint = mote.getBreakpoints()[row];

      if (col == COLUMN_STOP) {
        /* Toggle stop state */
        breakpoint.setStopsSimulation(!breakpoint.stopsSimulation());
        fireTableCellUpdated(row, col);
        return;
      }
    }
    public Class<?> getColumnClass(int c) {
      return getValueAt(0, c).getClass();
    }
  };

  private Action gotoCodeAction = new AbstractAction("Show in source code") {
    public void actionPerformed(ActionEvent e) {
      if (selectedWatchpoint == null) {
        return;
      }
      codeWatcher.displaySourceFile(selectedWatchpoint.getCodeFile(), selectedWatchpoint.getLineNumber(), false);
    }
  };
  private Action removeWatchpointAction = new AbstractAction("Remove watchpoint") {
    public void actionPerformed(ActionEvent e) {
      if (selectedWatchpoint == null) {
        return;
      }
      mote.removeBreakpoint(selectedWatchpoint);
      table.invalidate();
      table.repaint();
    }
  };
  private Action configureWatchpointAction = new AbstractAction("Configure watchpoint information") {
    public void actionPerformed(ActionEvent e) {
      if (selectedWatchpoint == null) {
        return;
      }
      configureWatchpointInfo(selectedWatchpoint);
    }
  };
}

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
 * $Id: BreakpointsUI.java,v 1.3 2009/06/11 10:06:47 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import org.apache.log4j.Logger;

/**
 * Displays a set of breakpoints.
 * 
 * @author Fredrik Osterlind
 */
public class BreakpointsUI extends JPanel {
  private static Logger logger = Logger.getLogger(BreakpointsUI.class);

  private MspBreakpointContainer breakpoints = null;
  private JTable table = null;

  public BreakpointsUI(MspBreakpointContainer breakpoints, final MspCodeWatcher codeWatcher) {
    this.breakpoints = breakpoints;

    /* Breakpoints table */
    table = new JTable(tableModel) {
      public String getToolTipText(MouseEvent e) {

        /* Tooltips: show full file paths */
        java.awt.Point p = e.getPoint();
        int rowIndex = table.rowAtPoint(p);
        int colIndex = table.columnAtPoint(p);
        int realColumnIndex = table.convertColumnIndexToModel(colIndex);

        if (realColumnIndex != 1) {
          return null;
        }

        MspBreakpoint[] allBreakpoints = BreakpointsUI.this.breakpoints.getBreakpoints();
        if (rowIndex < 0 || rowIndex >= allBreakpoints.length) {
          return null;
        }
        File file = allBreakpoints[rowIndex].getCodeFile();
        if (file == null) {
          return null;
        }
        return file.getPath();
      }
    };
    table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
    table.getColumnModel().getColumn(0).setPreferredWidth(60); /* XXX */
    table.getColumnModel().getColumn(0).setMaxWidth(60);
    table.getColumnModel().getColumn(2).setPreferredWidth(60);
    table.getColumnModel().getColumn(2).setMaxWidth(60);
    table.getColumnModel().getColumn(3).setPreferredWidth(60);
    table.getColumnModel().getColumn(3).setMaxWidth(60);

    /* Show source file on breakpoint mouse click */
    table.addMouseListener(new MouseAdapter() {
      public void mouseClicked(MouseEvent e) {

        java.awt.Point p = e.getPoint();
        int rowIndex = table.rowAtPoint(p);
        int colIndex = table.columnAtPoint(p);
        int realColumnIndex = table.convertColumnIndexToModel(colIndex);

        if (realColumnIndex != 1 && realColumnIndex != 2) {
          return;
        }

        MspBreakpoint[] allBreakpoints = BreakpointsUI.this.breakpoints.getBreakpoints();
        if (rowIndex < 0 || rowIndex >= allBreakpoints.length) {
          return;
        }
        File file = allBreakpoints[rowIndex].getCodeFile();
        int line = allBreakpoints[rowIndex].getLineNumber();
        if (file == null) {
          return;
        }

        /* Display source code */
        codeWatcher.displaySourceFile(file, line);
      }
    });
    
    /* Update when breakpoints are triggered/added/removed */
    breakpoints.addWatchpointListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        MspBreakpoint triggered = BreakpointsUI.this.breakpoints.getLastWatchpoint();
        if (triggered == null) {
          table.repaint();
          return;
        }
          
        flashBreakpoint(triggered);
      }
    });

    setLayout(new BorderLayout());
    add(BorderLayout.NORTH, table.getTableHeader());
    add(BorderLayout.CENTER, table);
  }

  private void flashBreakpoint(MspBreakpoint breakpoint) {
    /* Locate breakpoints table index */
    int index = -1;
    MspBreakpoint[] all = breakpoints.getBreakpoints();
    for (int i=0; i < breakpoints.getBreakpointsCount(); i++) {
      if (breakpoint == all[i]) {
        index = i;
        break;
      }
    }
    if (index < 0) {
      return;
    }

    final int breakpointIndex = index;
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        table.setRowSelectionInterval(breakpointIndex, breakpointIndex);
      }
    });
  }

  private AbstractTableModel tableModel = new AbstractTableModel() {
    private final String[] tableColumnNames = {
        "Address",
        "File:Line",
        "Breaks",
        "Remove"
    };
    public String getColumnName(int col) {
      return tableColumnNames[col].toString();
    }
    public int getRowCount() {
      return breakpoints.getBreakpointsCount();
    }
    public int getColumnCount() {
      return tableColumnNames.length;
    }
    public Object getValueAt(int row, int col) {
      MspBreakpoint breakpoint = breakpoints.getBreakpoints()[row];

      /* Executable address in hexadecimal */
      if (col == 0) {
        Integer address = breakpoint.getExecutableAddress();
        return "0x" + Integer.toHexString(address.intValue());
      }

      /* Source file + line number */
      if (col == 1) {
        File file = breakpoint.getCodeFile();
        if (file == null) {
          return "";
        }
        return file.getName() + ":" + breakpoint.getLineNumber();
      }

      /* Stops simulation */
      if (col == 2) {
        return breakpoint.stopsSimulation();
      }

      return new Boolean(false);
    }
    public boolean isCellEditable(int row, int col){
      return getColumnClass(col) == Boolean.class;
    }
    public void setValueAt(Object value, int row, int col) {
      MspBreakpoint breakpoint = breakpoints.getBreakpoints()[row];

      if (col == 2) {
        /* Toggle stop state */
        breakpoint.setStopsSimulation(!breakpoint.stopsSimulation());
        fireTableCellUpdated(row, col);
        return;
      }
      
      /* Remove breakpoint */
      Integer address = breakpoint.getExecutableAddress();
      breakpoints.removeBreakpoint(address);

      fireTableCellUpdated(row, col);
    }
    public Class<?> getColumnClass(int c) {
      return getValueAt(0, c).getClass();
    }
  };
}

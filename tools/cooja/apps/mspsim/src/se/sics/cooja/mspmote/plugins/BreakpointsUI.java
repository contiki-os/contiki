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
 * $Id: BreakpointsUI.java,v 1.1 2008/02/07 14:55:18 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.io.File;
import java.util.Vector;
import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import org.apache.log4j.Logger;

import se.sics.cooja.mspmote.plugins.MspCodeWatcher.Breakpoints;
import se.sics.cooja.mspmote.plugins.MspCodeWatcher.Breakpoints.Breakpoint;

public class BreakpointsUI extends JPanel {
  private static Logger logger = Logger.getLogger(BreakpointsUI.class);

  private JTable breakpointsTable = null;
  private Breakpoints breakpoints = null;

  private AbstractTableModel tableModel = new AbstractTableModel() {
    private String[] tableColumnNames = {
        "Address",
        "File",
        "Line",
        "Remove"
    };

    public String getColumnName(int col) { return tableColumnNames[col].toString(); }
    public int getRowCount() { return breakpoints.getBreakpoints().size(); }
    public int getColumnCount() { return tableColumnNames.length; }
    public Object getValueAt(int row, int col) {
      // Display executable address in hexadecimal
      if (col == 0) {
        Integer address = breakpoints.getBreakpoints().get(row).getExecutableAddress();
        return "0x" + Integer.toHexString(address.intValue());
      }

      // Display only name of file
      if (col == 1) {
        File file = breakpoints.getBreakpoints().get(row).getCodeFile();
        if (file == null) {
          return "";
        }
        return file.getName();
      }

      // Display line number
      if (col == 2) {
        Integer line = breakpoints.getBreakpoints().get(row).getLineNumber();
        if (line == null) {
          return "";
        }
        return line;
      }

      return new Boolean(false);
    }
    public boolean isCellEditable(int row, int col){
      return getColumnClass(col) == Boolean.class;
    }
    public void setValueAt(Object value, int row, int col) {
      fireTableCellUpdated(row, col);
      Integer address = breakpoints.getBreakpoints().get(row).getExecutableAddress();
      breakpoints.removeBreakpoint(address);
    }
    public Class getColumnClass(int c) {
      return getValueAt(0, c).getClass();
    }

  };

  public BreakpointsUI(Breakpoints breakpoints) {
    this.breakpoints = breakpoints;
    breakpoints.addBreakpointListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        Breakpoint triggered = BreakpointsUI.this.breakpoints.getLastTriggered();
        if (triggered != null) {
          flashBreakpoint(triggered);
        }

        breakpointsTable.repaint();
      }
    });

    breakpointsTable = new JTable(tableModel) {
      public String getToolTipText(MouseEvent e) {
        String tip = null;
        java.awt.Point p = e.getPoint();
        int rowIndex = breakpointsTable.rowAtPoint(p);
        int colIndex = breakpointsTable.columnAtPoint(p);
        int realColumnIndex = breakpointsTable.convertColumnIndexToModel(colIndex);

        if (realColumnIndex == 1) {
          Vector<Breakpoint> allBreakpoints = BreakpointsUI.this.breakpoints.getBreakpoints();
          if (rowIndex < 0 || rowIndex >= allBreakpoints.size()) {
            return "";
          }
          File file = allBreakpoints.get(rowIndex).getCodeFile();
          if (file == null) {
            return "";
          } else {
            tip = file.getPath();
          }
        }
        return tip;
      }
    };

    setLayout(new BorderLayout());
    add(breakpointsTable.getTableHeader(), BorderLayout.PAGE_START);
    add(breakpointsTable, BorderLayout.CENTER);
  }

  private int flashCounter = 0;
  private void flashBreakpoint(Breakpoint breakpoint) {
    int index = breakpoints.getBreakpoints().indexOf(breakpoint);
    breakpointsTable.setRowSelectionInterval(index, index);
    breakpointsTable.setSelectionBackground(Color.RED);

    flashCounter = 8;

    final Timer timer = new Timer(100, null);
    timer.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (flashCounter-- <= 0) {
          timer.stop();
          SwingUtilities.invokeLater(new Runnable() {
            public void run() {
              breakpointsTable.setSelectionBackground(Color.WHITE);
            }
          });
          return;
        }
        // Toggle background color
        if (breakpointsTable.getSelectionBackground() != Color.RED) {
          SwingUtilities.invokeLater(new Runnable() {
            public void run() {
              breakpointsTable.setSelectionBackground(Color.RED);
            }
          });
        } else {
          SwingUtilities.invokeLater(new Runnable() {
            public void run() {
              breakpointsTable.setSelectionBackground(Color.WHITE);
            }
          });
        }
      }
    });

    timer.start();

  }

}

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
 * $Id: BreakpointsUI.java,v 1.4 2009/06/15 09:45:46 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.util.ArrayList;

import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.Watchpoint;
import se.sics.cooja.WatchpointMote;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteType;

/**
 * Displays a set of breakpoints.
 * 
 * @author Fredrik Osterlind
 */
public class BreakpointsUI extends JPanel {
  private static Logger logger = Logger.getLogger(BreakpointsUI.class);

  private MspBreakpointContainer breakpoints = null;
  private JTable table = null;

  private MspBreakpoint popupBreakpoint = null;
  
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

    /* Popup menu: register on all motes */
    final JPopupMenu popupMenu = new JPopupMenu();
    popupMenu.add(new JMenuItem(addToMoteTypeAction));
    popupMenu.add(new JMenuItem(delFromMoteTypeAction));

    /* Show source file on breakpoint mouse click */
    table.addMouseListener(new MouseAdapter() {
      public void mouseClicked(MouseEvent e) {
        java.awt.Point p = e.getPoint();
        int rowIndex = table.rowAtPoint(p);
        int colIndex = table.columnAtPoint(p);
        int realColumnIndex = table.convertColumnIndexToModel(colIndex);

        if (realColumnIndex != 0 && realColumnIndex != 1) {
          return;
        }

        MspBreakpoint[] allBreakpoints = BreakpointsUI.this.breakpoints.getBreakpoints();
        if (rowIndex < 0 || rowIndex >= allBreakpoints.length) {
          return;
        }

        if (e.isPopupTrigger() || SwingUtilities.isRightMouseButton(e)) {
          popupBreakpoint = allBreakpoints[rowIndex];
          popupMenu.show(table, e.getX(), e.getY());
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

  private Action addToMoteTypeAction = new AbstractAction("Register on all motes (mote type)") {
    public void actionPerformed(ActionEvent e) {
      if (popupBreakpoint == null) {
        logger.fatal("No breakpoint to add");
      }

      /* Extract all motes of the same mote type */
      Simulation sim = popupBreakpoint.getMote().getSimulation();
      MoteType type = popupBreakpoint.getMote().getType();
      ArrayList<MspMote> motes = new ArrayList<MspMote>();
      for (Mote m: sim.getMotes()) {
        if (m.getType() == type) {
          if (!(m instanceof MspMote)) {
            logger.fatal("At least one mote was not a MSP mote: " + m);
            return;
          }
          
          motes.add((MspMote)m);
        }
      }

      /* Register breakpoints */
      int reregistered = 0;
      for (MspMote m: motes) {
        /* Avoid duplicates (match executable addresses) */
        MspBreakpointContainer container = m.getBreakpointsContainer();
        MspBreakpoint[] breakpoints = container.getBreakpoints();
        for (MspBreakpoint w: breakpoints) {
          if (popupBreakpoint.getExecutableAddress().intValue() ==
            w.getExecutableAddress().intValue()) {
            logger.info("Reregistering breakpoint at mote: " + m);
            container.removeBreakpoint(w.getExecutableAddress());
            reregistered++;
          }
        }

        MspBreakpoint newBreakpoint = container.addBreakpoint(
            popupBreakpoint.getCodeFile(),
            popupBreakpoint.getLineNumber(),
            popupBreakpoint.getExecutableAddress());
        newBreakpoint.setStopsSimulation(popupBreakpoint.stopsSimulation());
      }
      
      JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
          "Registered " + motes.size() + " breakpoints (" + reregistered + " re-registered)",
          "Breakpoints added", JOptionPane.INFORMATION_MESSAGE);
    }
  };
  private Action delFromMoteTypeAction = new AbstractAction("Delete from all motes (mote type)") {
    public void actionPerformed(ActionEvent e) {
      if (popupBreakpoint == null) {
        logger.fatal("No breakpoint to delete");
      }

      /* Extract all motes of the same mote type */
      Simulation sim = popupBreakpoint.getMote().getSimulation();
      MoteType type = popupBreakpoint.getMote().getType();
      ArrayList<MspMote> motes = new ArrayList<MspMote>();
      for (Mote m: sim.getMotes()) {
        if (m.getType() == type) {
          if (!(m instanceof MspMote)) {
            logger.fatal("At least one mote was not a MSP mote: " + m);
            return;
          }
          
          motes.add((MspMote)m);
        }
      }

      /* Delete breakpoints */
      int deleted = 0;
      for (MspMote m: motes) {
        /* Avoid duplicates (match executable addresses) */
        MspBreakpointContainer container = m.getBreakpointsContainer();
        MspBreakpoint[] breakpoints = container.getBreakpoints();
        for (MspBreakpoint w: breakpoints) {
          if (popupBreakpoint.getExecutableAddress().intValue() ==
            w.getExecutableAddress().intValue()) {
            container.removeBreakpoint(w.getExecutableAddress());
            deleted++;
          }
        }
      }
      
      JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
          "Deleted " + deleted + " breakpoints",
          "Breakpoints deleted", JOptionPane.INFORMATION_MESSAGE);  
    }
  };
}

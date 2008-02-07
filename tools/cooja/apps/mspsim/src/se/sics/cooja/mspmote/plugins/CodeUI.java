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
 * $Id: CodeUI.java,v 1.1 2008/02/07 14:55:18 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.File;
import java.util.Vector;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.mspmote.plugins.MspCodeWatcher.Breakpoints;

/**
 * Displays source code and allows a user to add and remove breakpoints.
 *
 * @author Fredrik Österlind
 */
public class CodeUI extends JPanel {
  private static Logger logger = Logger.getLogger(CodeUI.class);

  private JPanel panel = null;
  private JList codeList = null;
  private File currentFile = null;

  private Breakpoints breakpoints = null;

  /**
   * @param breakpoints Breakpoints
   */
  public CodeUI(Breakpoints breakpoints) {
    this.breakpoints = breakpoints;

    setLayout(new BorderLayout());
    panel = new JPanel(new BorderLayout());
    add(panel, BorderLayout.CENTER);
    displayNoCode();

    breakpoints.addBreakpointListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (codeList != null) {
          codeList.updateUI();
        }
      }
    });
  }

  /**
   * Remove any shown source code.
   */
  public void displayNoCode() {
    // Display "no code" message
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        panel.removeAll();
        panel.repaint();
      }
    });
    currentFile = null;
    return;
  }


  /**
   * Display given source code and mark given line.
   *
   * @param codeFile Source code file
   * @param codeData Source code
   * @param lineNr Line numer
   */
  public void displayNewCode(final File codeFile, final Vector<String> codeData, final int lineNr) {
    currentFile = codeFile;

    if (codeData == null || codeData.size() == 0) {
      displayNoCode();
      return;
    }

    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        // Display code
        codeList = new JList(new CodeListModel(codeData));
        codeList.setFont(new Font("courier", 0, 12));
        codeList.setCellRenderer(new CodeCellRenderer(lineNr));
        codeList.addMouseListener(new MouseListener() {
          public void mousePressed(MouseEvent e) {
            handleMouseEvent(e);
          }
          public void mouseReleased(MouseEvent e) {
            handleMouseEvent(e);
          }
          public void mouseEntered(MouseEvent e) {
            handleMouseEvent(e);
          }
          public void mouseExited(MouseEvent e) {
            handleMouseEvent(e);
          }
          public void mouseClicked(MouseEvent e) {
            handleMouseEvent(e);
          }
        });
        panel.removeAll();
        panel.add(codeList);
        displayLine(lineNr);
      }
    });
    }

  /**
   * Mark given line number in shown source code.
   *
   * @param lineNumber Line number
   */
  public void displayLine(final int lineNumber) {
    if (codeList == null) {
      return;
    }

    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        if (lineNumber > 0) {
          ((CodeCellRenderer) codeList.getCellRenderer()).changeCurrentLine(lineNumber);
          int index = lineNumber - 1;
          codeList.setSelectedIndex(index);

          codeList.ensureIndexIsVisible(Math.max(0, index-3));
          codeList.ensureIndexIsVisible(Math.min(index+3, codeList.getModel().getSize()));
          codeList.ensureIndexIsVisible(index);
        }
        codeList.updateUI();
      }
    });
  }

  private void handleMouseEvent(MouseEvent event) {
    if (event.isPopupTrigger()) {
      Point menuLocation = codeList.getPopupLocation(event);
      if (menuLocation == null) {
        menuLocation = new Point(
            codeList.getLocationOnScreen().x + event.getX(),
            codeList.getLocationOnScreen().y + event.getY());
      }

      final int currentLine = codeList.locationToIndex(new Point(event.getX(), event.getY())) + 1;
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          codeList.setSelectedIndex(currentLine - 1);
        }
      });
      JPopupMenu popupMenu = createPopupMenu(currentFile, currentLine);

      popupMenu.setLocation(menuLocation);
      popupMenu.setInvoker(codeList);
      popupMenu.setVisible(true);
    }
  }

  private JPopupMenu createPopupMenu(final File codeFile, final int lineNr) {
    final Integer executableAddress = breakpoints.getExecutableAddressOf(codeFile, lineNr);
    boolean breakpointExists = false;
    if (executableAddress != null) {
      breakpointExists = breakpoints.breakpointExists(executableAddress);
    }

    JPopupMenu menuMotePlugins = new JPopupMenu();
    JMenuItem headerMenuItem = new JMenuItem("Breakpoints:");
    headerMenuItem.setEnabled(false);
    menuMotePlugins.add(headerMenuItem);
    menuMotePlugins.add(new JSeparator());

    JMenuItem addBreakpointMenuItem = new JMenuItem("Add breakpoint on line " + lineNr);
    if (executableAddress == null || breakpointExists) {
      addBreakpointMenuItem.setEnabled(false);
    } else {
      addBreakpointMenuItem.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          breakpoints.addBreakpoint(codeFile, lineNr, executableAddress);
        }
      });
    }
    menuMotePlugins.add(addBreakpointMenuItem);

    JMenuItem delBreakpointMenuItem = new JMenuItem("Delete breakpoint on line " + lineNr);
    if (executableAddress == null || !breakpointExists) {
      delBreakpointMenuItem.setEnabled(false);
    } else {
      delBreakpointMenuItem.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          breakpoints.removeBreakpoint(executableAddress);
        }
      });
    }
    menuMotePlugins.add(delBreakpointMenuItem);

    return menuMotePlugins;
  }

  private class CodeListModel extends AbstractListModel {
    private Vector<String> codeData;

    public CodeListModel(Vector<String> codeData) {
      super();
      this.codeData = codeData;
    }

    public int getSize() {
      if (codeData == null || codeData.isEmpty()) {
        return 0;
      }

      return codeData.size();
    }

    public Object getElementAt(int index) {
      if (codeData == null || codeData.isEmpty()) {
        return "No code to display";
      }

      return codeData.get(index);
    }
  }

  private class CodeCellRenderer extends JLabel implements ListCellRenderer {
    private int currentIndex;

    public CodeCellRenderer(int currentLineNr) {
      this.currentIndex = currentLineNr - 1;
    }

    public void changeCurrentLine(int currentLineNr) {
      this.currentIndex = currentLineNr - 1;
    }

    public Component getListCellRendererComponent(
       JList list,
       Object value,
       int index,
       boolean isSelected,
       boolean cellHasFocus)
     {
      int lineNr = index + 1;

      setText(lineNr + ":  " + value);
      if (index == currentIndex) {
        setBackground(Color.green);
      } else if (isSelected) {
        setBackground(list.getSelectionBackground());
        setForeground(list.getSelectionForeground());
      } else {
        setBackground(list.getBackground());
        setForeground(list.getForeground());
      }
      setEnabled(list.isEnabled());

      Integer executableAddress = breakpoints.getExecutableAddressOf(currentFile, lineNr);
      if (breakpoints.breakpointExists(executableAddress)) {
        setFont(list.getFont().deriveFont(Font.BOLD));
      } else {
        setFont(list.getFont());
      }
      setOpaque(true);

      return this;
     }
  }

}

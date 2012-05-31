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
 * $Id: CodeUI.java,v 1.8 2009/09/23 08:16:06 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Point;
import java.awt.Rectangle;
import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import javax.swing.JEditorPane;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.Highlighter;
import javax.swing.text.Highlighter.HighlightPainter;

import jsyntaxpane.DefaultSyntaxKit;
import jsyntaxpane.components.Markers.SimpleMarker;

import org.apache.log4j.Logger;

import se.sics.cooja.Watchpoint;
import se.sics.cooja.WatchpointMote;
import se.sics.cooja.util.JSyntaxAddBreakpoint;
import se.sics.cooja.util.JSyntaxRemoveBreakpoint;
import se.sics.cooja.util.StringUtils;

/**
 * Displays source code and allows a user to add and remove breakpoints.
 *
 * @author Fredrik Osterlind
 */
public class CodeUI extends JPanel {
  private static Logger logger = Logger.getLogger(CodeUI.class);

  {
    DefaultSyntaxKit.initKit();
  }

  private JEditorPane codeEditor = null;
  private HashMap<Integer, Integer> codeEditorLines = null;
  protected File displayedFile = null;

  private static final HighlightPainter CURRENT_LINE_MARKER = new SimpleMarker(Color.ORANGE);
  private static final HighlightPainter SELECTED_LINE_MARKER = new SimpleMarker(Color.GREEN);
  private static final HighlightPainter BREAKPOINTS_MARKER = new SimpleMarker(Color.LIGHT_GRAY);
  private final Object currentLineTag;
  private final Object selectedLineTag;
  private final ArrayList<Object> breakpointsLineTags = new ArrayList<Object>();

  private JSyntaxAddBreakpoint actionAddBreakpoint = null;
  private JSyntaxRemoveBreakpoint actionRemoveBreakpoint = null;

  private WatchpointMote mote;

  public CodeUI(WatchpointMote mote) {
    this.mote = mote;

    {
      /* Workaround to configure jsyntaxpane */
      JEditorPane e = new JEditorPane();
      new JScrollPane(e);
      e.setContentType("text/c");
      DefaultSyntaxKit kit = (DefaultSyntaxKit) e.getEditorKit();
      kit.setProperty("Action.addbreakpoint", JSyntaxAddBreakpoint.class.getName());
      kit.setProperty("Action.removebreakpoint", JSyntaxRemoveBreakpoint.class.getName());
      kit.setProperty("PopupMenu", "copy-to-clipboard,-,find,find-next,goto-line,-,addbreakpoint,removebreakpoint");
    }

    setLayout(new BorderLayout());
    codeEditor = new JEditorPane();
    add(new JScrollPane(codeEditor), BorderLayout.CENTER);
    doLayout();

    codeEditorLines = new HashMap<Integer, Integer>();
    codeEditor.setContentType("text/c");
    DefaultSyntaxKit kit = (DefaultSyntaxKit) codeEditor.getEditorKit();
    kit.setProperty("Action.addbreakpoint", JSyntaxAddBreakpoint.class.getName());
    kit.setProperty("Action.removebreakpoint", JSyntaxRemoveBreakpoint.class.getName());
    kit.setProperty("PopupMenu", "copy-to-clipboard,-,find,find-next,goto-line,-,addbreakpoint,removebreakpoint");

    JPopupMenu p = codeEditor.getComponentPopupMenu();
    for (Component c: p.getComponents()) {
      if (c instanceof JMenuItem) {
        if (((JMenuItem) c).getAction() != null &&
            ((JMenuItem) c).getAction() instanceof JSyntaxAddBreakpoint) {
          actionAddBreakpoint = (JSyntaxAddBreakpoint)(((JMenuItem) c).getAction());
          actionAddBreakpoint.setMenuText("Add breakpoint");
        }
        if (((JMenuItem) c).getAction() != null &&
            ((JMenuItem) c).getAction() instanceof JSyntaxRemoveBreakpoint) {
          actionRemoveBreakpoint = (JSyntaxRemoveBreakpoint)(((JMenuItem) c).getAction());
          actionRemoveBreakpoint.setMenuText("Remove breakpoint");
        }
      }
    }

    codeEditor.setText("");
    codeEditorLines.clear();
    codeEditor.setEditable(false);

    Highlighter hl = codeEditor.getHighlighter();
    Object o = null;
    try {
      o = hl.addHighlight(0, 0, CURRENT_LINE_MARKER);
    } catch (BadLocationException e1) {
    }
    currentLineTag = o;

    o = null;
    try {
      o = hl.addHighlight(0, 0, SELECTED_LINE_MARKER);
    } catch (BadLocationException e1) {
    }
    selectedLineTag = o;

    codeEditor.getComponentPopupMenu().addPopupMenuListener(new PopupMenuListener() {
      public void popupMenuWillBecomeVisible(PopupMenuEvent e) {
        /* Disable breakpoint actions */
        actionAddBreakpoint.setEnabled(false);
        actionRemoveBreakpoint.setEnabled(false);

        int line = getCodeEditorMouseLine();
        if (line < 1) {
          return;
        }

        /* Configure breakpoint menu options */
        /* XXX TODO We should ask for the file specified in the firmware, not
         * the actual file on disk. */
        int address =
          CodeUI.this.mote.getExecutableAddressOf(displayedFile, line);
        if (address < 0) {
          return;
        }
        final int start = codeEditorLines.get(line);
        int end = codeEditorLines.get(line+1);
        Highlighter hl = codeEditor.getHighlighter();
        try {
          hl.changeHighlight(selectedLineTag, start, end);
        } catch (BadLocationException e1) {
        }
        boolean hasBreakpoint =
          CodeUI.this.mote.breakpointExists(address);
        if (!hasBreakpoint) {
          actionAddBreakpoint.setEnabled(true);
          actionAddBreakpoint.putValue("WatchpointMote", CodeUI.this.mote);
          actionAddBreakpoint.putValue("WatchpointFile", displayedFile);
          actionAddBreakpoint.putValue("WatchpointLine", new Integer(line));
          actionAddBreakpoint.putValue("WatchpointAddress", new Integer(address));
        } else {
          actionRemoveBreakpoint.setEnabled(true);
          actionRemoveBreakpoint.putValue("WatchpointMote", CodeUI.this.mote);
          actionRemoveBreakpoint.putValue("WatchpointFile", displayedFile);
          actionRemoveBreakpoint.putValue("WatchpointLine", new Integer(line));
          actionRemoveBreakpoint.putValue("WatchpointAddress", new Integer(address));
        }
      }
      public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {
        Highlighter hl = codeEditor.getHighlighter();
        try {
          hl.changeHighlight(selectedLineTag, 0, 0);
        } catch (BadLocationException e1) {
        }
      }
      public void popupMenuCanceled(PopupMenuEvent e) {
      }
    });

    displayNoCode(true);
  }

  public void updateBreakpoints() {
    Highlighter hl = codeEditor.getHighlighter();

    for (Object breakpointsLineTag: breakpointsLineTags) {
      hl.removeHighlight(breakpointsLineTag);
    }
    breakpointsLineTags.clear();

    for (Watchpoint w: mote.getBreakpoints()) {
      if (!w.getCodeFile().equals(displayedFile)) {
        continue;
      }

      final int start = codeEditorLines.get(w.getLineNumber());
      int end = codeEditorLines.get(w.getLineNumber()+1);
      try {
        breakpointsLineTags.add(hl.addHighlight(start, end, BREAKPOINTS_MARKER));
      } catch (BadLocationException e1) {
      }
    }
  }

  private int getCodeEditorMouseLine() {
    if (codeEditorLines == null) {
      return -1;
    }
    Point mousePos = codeEditor.getMousePosition();
    if (mousePos == null) {
      return -1;
    }
    int modelPos = codeEditor.viewToModel(mousePos);
    int line = 1;
    while (codeEditorLines.containsKey(line+1)) {
      int next = codeEditorLines.get(line+1);
      if (modelPos < next) {
        return line;
      }
      line++;
    }
    return -1;
  }

  /**
   * Remove any shown source code.
   */
  public void displayNoCode(final boolean markCurrent) {
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        displayedFile = null;
        codeEditor.setText("[no source displayed]");
        codeEditor.setEnabled(false);
        codeEditorLines.clear();
        displayLine(-1, markCurrent);
      }
    });
  }

  /**
   * Display given source code and mark given line.
   *
   * @param codeFile Source code file
   * @param lineNr Line numer
   */
  public void displayNewCode(final File codeFile, final int lineNr, final boolean markCurrent) {
    if (!codeFile.equals(displayedFile)) {
      /* Read from disk */
      final String data = StringUtils.loadFromFile(codeFile);
      if (data == null || data.length() == 0) {
        displayNoCode(markCurrent);
        return;
      }
      codeEditor.setEnabled(true);

      String[] lines = data.split("\n");
      logger.info("Opening " + codeFile + " (" + lines.length + " lines)");
      int length = 0;
      codeEditorLines.clear();
      for (int line=1; line-1 < lines.length; line++) {
        codeEditorLines.put(line, length);
        length += lines[line-1].length()+1;
      }
      codeEditor.setText(data.toString());
      displayedFile = codeFile;
      updateBreakpoints();
    }

    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        displayLine(lineNr, markCurrent);
      }
    });

  }

  /**
   * Mark given line number in shown source code.
   * Should be called from AWT thread.
   *
   * @param lineNumber Line number
   */
  private void displayLine(int lineNumber, boolean markCurrent) {
    try {
      if (markCurrent) {
        /* remove previous highlight */
        Highlighter hl = codeEditor.getHighlighter();
        hl.changeHighlight(currentLineTag, 0, 0);
      }

      if (lineNumber >= 0) {
        final int start = codeEditorLines.get(lineNumber);
        int end = codeEditorLines.get(lineNumber+1);
        if (markCurrent) {
          /* highlight code */
          Highlighter hl = codeEditor.getHighlighter();
          hl.changeHighlight(currentLineTag, start, end);
        }

        /* ensure visible */
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            try {
              Rectangle r = codeEditor.modelToView(start);
              if (r != null) {
                codeEditor.scrollRectToVisible(codeEditor.modelToView(start));
              }
            } catch (BadLocationException e) {
            }
          }
        });
      }
    } catch (Exception e) {
      logger.warn("Error when highlighting current line: " + e.getMessage(), e);
    }
  }
}

/**
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * $Id: ChartPanel.java 740 2010-07-19 12:53:27Z nifi $
 *
 * -----------------------------------------------------------------
 *
 * ConsoleUI
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : March 20 2010
 * Updated : $Date$
 *           $Revision$
 */
package se.sics.mspsim.ui;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.StringSelection;
import java.awt.datatransfer.Transferable;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayDeque;

import javax.swing.JComponent;
import javax.swing.Timer;

import se.sics.mspsim.cli.CommandHandler;

/* Console UI for command line interfaces
 *
 * TODO
 * - add setting selection
 * - scrollbar for view-history.
 *
 */
public class ConsoleUI extends JComponent {

  private static final long serialVersionUID = -4398393961025971500L;

  private Timer timer;

  private String[] history = new String[200];
  private int pos;

  /* this is the "edit" line */
  private char[] chars = new char[1024];

  /* up to 200 visible lines */
  private String[] lines = new String[200];

  private int lineCount = 0;
  private int lastVisible = 0;

  /* the lines that are on the screen */
  /*
   * will be rendered line-by-line from line 0 to line "bottomLine"
   */

  private String[] screenLines = new String[50];
  /* size of lines */
  int lineWidth = 40;

  private ArrayDeque<String> commands = new ArrayDeque<String>();

  private int len = 0;
  private int back = 0;

  private int charWidth = 7;
  private int charHeight = 11;
  private int bottomLine = 30;

  private CommandHandler commandHandler;

  /* editor variables */
  private boolean insert = true;
  private boolean editing = false;
  private boolean cursor;

  private int editPos = 0;
  private int cursorX = 0;
  private int minCursorX = 0;
  private int cursorY = 0;

  private boolean selectActive = false;
  private int selectStartX = 0;
  private int selectStartY = 0;
  private int selectEndX = 0;
  private int selectEndY = 0;


  private static int MIN_X = 8;
  
  public ConsoleUI() {
    setFont(Font.decode("Courier-12"));
    setOpaque(true);
    setFocusable(true);
    setFocusTraversalKeysEnabled(false);

    MouseAdapter mouseHandler = new MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        if (e.getButton() == MouseEvent.BUTTON1) {
          selectActive = false;
          selectEndX = (e.getX() - 10) / charWidth;
          selectEndY = (e.getY() - 4) / charHeight;
          repaint();
        }
      }

      public void mousePressed(MouseEvent e) {
        if (e.getButton() == MouseEvent.BUTTON1) {
          selectActive = true;
          selectStartX = selectEndX = (e.getX() - 10) / charWidth;
          selectStartY = selectEndY = (e.getY() - 4) / charHeight;
        }
      }

      public void mouseDragged(MouseEvent e) {
        if (selectActive) {
          selectEndX = (e.getX() - 10) / charWidth;
          selectEndY = (e.getY() - 4) / charHeight;
          repaint();
        }
      }
    };
    addMouseListener(mouseHandler);
    addMouseMotionListener(mouseHandler);
    addKeyListener(new KeyListener() {
      public void keyTyped(KeyEvent e) {
        switch (e.getKeyChar()) {
        case 1: /* Ctrl-A */
          cursorX = minCursorX;
          editPos = 0;
          break;
        case 2: /* Ctrl-B */
          if (editPos > 0) {
            editPos--;
            cursorX--;
          }
          break;
        case 4: /* Ctrl-D */
          removeChar(false);
          break;
        case 5: /* Ctrl-E */
          cursorX = len + minCursorX;
          editPos = len;
          break;
        case 6: /* Ctrl-F */
          if (editPos < len) {
            cursorX++;
            editPos++;
          }
          break;
        case 11: /* Ctrl-K */
          if (editPos < len) {
            // TODO copy to clipboard
            len = editPos;
            layoutRows();
            repaint();
          }
          break;
        case 12: /* Ctrl-L */
          cursorY = 0;
          lastVisible = 0;
          lines[0] = null;
          layoutRows();
          repaint();
          break;
        case 14:
          setHistory(-1);
          break;
        case 16:
          setHistory(1);
          break;
        case 3: /* Ctrl-C */
          copySelection();
          break;
        case 22: /* Ctrl-V */
        case 25: /* Ctrl-Y */
          paste(getClipboardContents());
          break;
        case KeyEvent.VK_ENTER:
          handleChar('\n');
          break;
        case KeyEvent.VK_BACK_SPACE:
        case KeyEvent.VK_DELETE:
          removeChar(e.getKeyChar() == KeyEvent.VK_BACK_SPACE);
          break;
        default:
          handleChar(e.getKeyChar());
        }
      }

      public void keyReleased(KeyEvent e) {
      }

      public void keyPressed(KeyEvent e) {
        switch (e.getKeyCode()) {
        case KeyEvent.VK_RIGHT:
          if (editPos < len) {
            cursorX++;
            editPos++;
          }
          break;
        case KeyEvent.VK_LEFT:
          if (editPos > 0) {
            editPos--;
            cursorX--;
          }
          break;
        case KeyEvent.VK_HOME:
          cursorX = minCursorX;
          editPos = 0;
          break;
        case KeyEvent.VK_END:
          cursorX = len + minCursorX;
          editPos = len;
          break;
        case KeyEvent.VK_UP:
          setHistory(1);
          break;
        case KeyEvent.VK_DOWN:
          setHistory(-1);
          break;
        }
        layoutRows();
        repaint();
      }
    });
    output('>');
    timer = new Timer(500, new ActionListener() {
      public void actionPerformed(ActionEvent arg0) {
        flashCursor();
      }
    });
    timer.start();
  }

  @Override
  public void setVisible(boolean visible) {
    super.setVisible(visible);
    if (timer == null) {
      // Ignore
    } else if (visible) {
      timer.start();
    } else {
      timer.stop();
    }
  }

  private void removeChar(boolean back) {
    if ((back && editPos > 0) || (!back && editPos < len)) {
      int ep = editPos - (back ? 1 : 0);
      System.arraycopy(chars, ep + 1, chars, ep, len - ep);
      if (back) {
        cursorX--;
        editPos--;
      }
      len--;
      layoutRows();
      repaint();
    }
  }

  private void paste(String s) {
    for (int i = 0; s != null && i < s.length(); i++) {
      handleChar(s.charAt(i));
    }
  }

  private void handleChar(char c) {
    switch (c) {
    case '\n':
      String line = new String(chars, 0, len);
      if (line.trim().length() > 0)
        history[pos++] = line;
      addLine(currentOutput.toString() + line);
      if (pos >= history.length)
        pos = 0;
      back = 0;
      executeCommand(line);
      break;
    default:
      if (insert && editPos < len) {
        System.arraycopy(chars, editPos, chars, editPos + 1, len - editPos);
      }
      chars[editPos++] = c;
      cursorX++;
      len++;
      editing = true;
    }
    /*
     * complete re-layout - needs to be optimized later - layout is only needed
     * for last "row" that is under editing.
     */
    layoutRows();
    repaint();
  }

  private void executeCommand(String line) {
    synchronized (commands) {
      commands.add(line);
      commands.notify();
    }
  }

  private void copySelection() {
    if (selectEndX != selectStartX || selectEndY != selectStartY) {
      int x = selectStartX;
      int xe = selectEndX;
      int y = selectStartY;
      int ye = selectEndY;
      if (selectEndY < selectStartY || selectStartY == selectEndY
          && selectEndX < selectStartX) {
        x = selectEndX;
        xe = selectStartX;
        y = selectEndY;
        ye = selectStartY;
      }
      StringBuilder selection = new StringBuilder();
      for (int i = y; i < ye; i++) {
        String data = screenLines[i];
        if (data.length() > x) {
          selection.append(data.substring(x, data.length())).append('\n');
          x = 0;
        }
      }
      if (screenLines[ye].length() > xe) {
        selection.append(screenLines[ye].substring(x, xe));
      }
      StringSelection stringSelection = new StringSelection(
          selection.toString());
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
      clipboard.setContents(stringSelection, null);
    }
  }

  /**
   * Get the String residing on the clipboard.
   */
  public String getClipboardContents() {
    Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
    Transferable contents = clipboard.getContents(null);
    if ((contents != null)
        && contents.isDataFlavorSupported(DataFlavor.stringFlavor)) {
      try {
        return (String) contents.getTransferData(DataFlavor.stringFlavor);
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
    return null;
  }

  /* make the "screen" lines the correct ones. */
  /*
   * This will NOT handle the current EDIT line(s) This is used for "refreshing"
   * the screen completely. It is possible to just scroll upwards if just adding
   * another "last" line.
   */
  int scrCursorX = 0;
  int scrCursorY = 0;

  private int layoutRows() {
      
    scrCursorX = cursorX;
    scrCursorY = cursorY;

    /* bottom line is the last visible line */
    int scrLine = bottomLine;

    int fetchLine = lastVisible - 1;
    /* wrap fetch-line if needed - or possibly move back to just 0 */
    if (fetchLine == -1) {
        fetchLine += lineCount == lines.length ? lines.length : 1;
    }
    /* actual last visible line */
    String ll = currentOutput.toString() + new String(chars, 0, len);
    while (scrLine >= 0 && fetchLine >= 0 && ll != null) {
      int neededRows = 1 + (ll.length() / lineWidth);
      int lastStartPos = (neededRows - 1) * lineWidth;
      int lastEndPos = ll.length();

      while (neededRows > 0 && scrLine >= 0) {
        screenLines[scrLine] = ll.substring(lastStartPos, lastEndPos);
        scrLine--;
        neededRows--;
        lastEndPos = lastStartPos;
        lastStartPos -= lineWidth;
      }
      ll = lines[fetchLine--];
      /* wrap if needed */
      if (fetchLine == -1 && lineCount == lines.length) {
          fetchLine = lines.length - 1;
      }
    }
    return bottomLine - scrLine;
  }

  private void setHistory(int diff) {
    back += diff;
    if (back < 0)
      back = 0;
    String historyLine = history[((pos - back) + history.length)
        % history.length];
    if (historyLine == null) {
      back = 0;
      return;
    }
    char[] arr = historyLine.toCharArray();
    System.arraycopy(arr, 0, chars, 0, arr.length);
    len = arr.length;
    cursorX = minCursorX + len;
    editPos = len;
    editing = true;
    /* layout and repaint */
    layoutRows();
    repaint();
  }

  private void flashCursor() {
    cursor = !cursor;
    repaint(MIN_X + (cursorX % lineWidth) * charWidth, 10 + cursorY * charHeight,
        charWidth, 2);
  }

  StringBuffer currentOutput = new StringBuffer();

  int oldBottomLine = 0;
  protected void paintComponent(Graphics g) {
    int pos = 0;
    int w = getWidth() - 8;
    int h = getHeight() - 15;
        
    Rectangle clip = g.getClipBounds();
    if (w / charWidth != lineWidth || h / charHeight != bottomLine) {
      lineWidth = w / charWidth;
      bottomLine = h / charHeight;

      if (oldBottomLine != bottomLine) {
          if (cursorY > bottomLine) cursorY = bottomLine;
          if (screenLines.length < bottomLine) {
              screenLines = new String[(h / charHeight) + 10];
          }
          int rowCount = layoutRows();
          if (cursorY < rowCount) cursorY = rowCount;
          oldBottomLine = bottomLine;
      } else {
          /* no Y-size change */
          layoutRows();
      }
    }
    g.setColor(getBackground());
    g.fillRect(clip.x, clip.y, clip.width, clip.height);
    g.setColor(getForeground());

    if (selectEndX != selectStartX || selectEndY != selectStartY) {
      g.setColor(Color.LIGHT_GRAY);
      int x = selectStartX;
      int xe = selectEndX;
      int y = selectStartY;
      int ye = selectEndY;
      if (selectEndY < selectStartY || selectStartY == selectEndY
          && selectEndX < selectStartX) {
        x = selectEndX;
        xe = selectStartX;
        y = selectEndY;
        ye = selectStartY;
      }
      for (; y < ye; y++) {
        g.fillRect(MIN_X + x * charWidth, y * charHeight, w, charHeight + 2);
        x = 0;
      }
      g.fillRect(MIN_X + x * charWidth, ye * charHeight, (xe - x) * charWidth,
          charHeight + 2);
      g.setColor(getForeground());
    }

    /* reserved... => scr */
    int i = bottomLine;
    pos = cursorY * charHeight;
    while (i >= 0 && screenLines[i] != null) {
      g.drawString(screenLines[i], MIN_X, 10 + pos);
      pos -= charHeight;
      i--;
    }

    if (cursor) {
      g.fillRect(MIN_X + (cursorX % lineWidth) * charWidth, 10 + cursorY
          * charHeight, charWidth, 2);
    }
  }

  StringBuffer buffer = new StringBuffer();

  /* add a line to the "visible" screen */
  private void addLine(String line) {
    lines[lastVisible] = line;
    currentOutput.setLength(0);
    if (cursorY < bottomLine) {
      cursorY++;
    }
    lastVisible++;

    if (lineCount < lines.length) lineCount++;
    
    if (lastVisible >= lines.length) {
        lastVisible = 0;
    }
    /* editor is reset too */
    editPos = minCursorX = cursorX = len = 0;

    if (editing != false && buffer.length() > 0) {
      /*
       * set edit to false so we do not get here again before end of "printout".
       */
      editing = false;
      String s = buffer.toString();
      for (int i = 0; i < s.length(); i++) {
        addChar(s.charAt(i));
      }
      buffer.setLength(0);
    }
    editing = false;
    layoutRows();
    repaint();
  }

  /* just print space instead of tabs */
  String tabs = "           ";

  /* print char to the shell */
  public void output(int c) {
    if (!editing) {
      addChar(c);
    } else {
      /* ignore or buffer when in edit mode (add buffer later) */
      buffer.append((char) c);
    }
  }

  private void addChar(int c) {
    if (c == '\t') {
      int p = 8 - (cursorX & 7);
      currentOutput.append(tabs.substring(0, p));
      cursorX += p;
      minCursorX += p;
    } else if (c == '\n') {
      addLine(currentOutput.toString());
      currentOutput.setLength(0);
    } else {
      cursorX++;
      minCursorX++;
      currentOutput.append((char) c);
    }
  }

  public OutputStream getOutputStream() {
    return new OutputStream() {
      public void write(int c) throws IOException {
        output(c);
      }
    };
  }

  public void setCommandHandler(CommandHandler commandHandler) {
    this.commandHandler = commandHandler;
    new Thread(new Runnable() {
      public void run() {
        while (true) {
          String command = null;
          try {
            synchronized (commands) {
              while (commands.isEmpty()) {
                commands.wait();
              }
              command = commands.removeFirst();
            }
          } catch (InterruptedException e) {
            e.printStackTrace();
          }
          if (command != null) {
            ConsoleUI.this.commandHandler.lineRead(command);
            output('>');
            layoutRows();
            repaint();
          }
        }
      }
    }).start();
  }
}

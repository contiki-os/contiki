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
 * $Id: CodeUI.java,v 1.2 2008/03/19 14:52:04 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.File;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.mspmote.plugins.MspCodeWatcher.Breakpoints;
import se.sics.mspsim.extutil.highlight.CScanner;
import se.sics.mspsim.extutil.highlight.Token;
import se.sics.mspsim.extutil.highlight.TokenTypes;

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

  private Token tokensArray[][] = null;
  private int tokensStartPos[] = null;

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

  private void createTokens(Vector<String> codeData) {

    /* Merge code lines */
    String code = "";
    for (String line: codeData) {
      code += line + "\n";
    }

    /* Scan code */
    CScanner cScanner = new CScanner();
    cScanner.change(0, 0, code.length());
    int nrTokens;
    nrTokens = cScanner.scan(code.toCharArray(), 0, code.length());

    /* Extract tokens */
    Vector<Token> codeTokensVector = new Vector<Token>();
    for (int i=0; i < nrTokens; i++) {
      Token token = cScanner.getToken(i);
      codeTokensVector.add(token);
    }

    /* Create new line token array */
    Token newTokensArray[][] = new Token[codeData.size()][];
    int[] newTokensStartPos = new int[codeData.size()];
    int lineStart=0, lineEnd=-1;
    Enumeration<Token> tokensEnum = codeTokensVector.elements();
    Token currentToken = tokensEnum.nextElement();
    for (int i=0; i < newTokensArray.length; i++) {
      lineStart = lineEnd + 1;
      lineEnd = lineStart + codeData.get(i).length();

      newTokensStartPos[i] = lineStart;;

      /* Advance tokens until correct line */
      while (currentToken.position + currentToken.symbol.name.length() < lineStart) {
        if (!tokensEnum.hasMoreElements()) {
          break;
        }
        currentToken = tokensEnum.nextElement();
      }

      /* Advance tokens until last token on line */
      Vector<Token> lineTokens = new Vector<Token>();
      while (currentToken.position < lineEnd) {
        lineTokens.add(currentToken);

        if (!tokensEnum.hasMoreElements()) {
          break;
        }
        currentToken = tokensEnum.nextElement();
      }

      if (currentToken == null) {
        break;
      }

      /* Store line tokens */
      Token[] lineTokensArray = new Token[lineTokens.size()];
      for (int j=0; j < lineTokens.size(); j++) {
        lineTokensArray[j] = lineTokens.get(j);
      }
      newTokensArray[i] = lineTokensArray;
    }

    /* Start using tokens array */
    tokensArray = newTokensArray;
    tokensStartPos = newTokensStartPos;
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

        createTokens(codeData);
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

  /* FROM: http://www.rgagnon.com/javadetails/java-0306.html, 03/19/2008 */
  private static String stringToHTMLString(String string) {
    StringBuffer sb = new StringBuffer(string.length());
    boolean lastWasBlankChar = false;
    int len = string.length();
    char c;

    for (int i = 0; i < len; i++)
    {
      c = string.charAt(i);
      if (c == ' ') {
        if (lastWasBlankChar) {
          lastWasBlankChar = false;
          sb.append("&nbsp;");
        }
        else {
          lastWasBlankChar = true;
          sb.append(' ');
        }
      }
      else {
        lastWasBlankChar = false;
        //
        // HTML Special Chars
        if (c == '"') {
          sb.append("&quot;");
        } else if (c == '&') {
          sb.append("&amp;");
        } else if (c == '<') {
          sb.append("&lt;");
        } else if (c == '>') {
          sb.append("&gt;");
        } else if (c == '\n') {
          // Handle Newline
          sb.append("&lt;br/&gt;");
        } else {
          int ci = 0xffff & c;
          if (ci < 160 ) {
            // nothing special only 7 Bit
            sb.append(c);
          } else {
            // Not 7 Bit use the unicode system
            sb.append("&#");
            sb.append(new Integer(ci).toString());
            sb.append(';');
          }
        }
      }
    }
    return sb.toString();
  }

  private class CodeCellRenderer extends JLabel implements ListCellRenderer {
    private int currentIndex;

    public CodeCellRenderer(int currentLineNr) {
      this.currentIndex = currentLineNr - 1;
    }

    public void changeCurrentLine(int currentLineNr) {
      this.currentIndex = currentLineNr - 1;
    }

    private String getColoredLabelText(int lineNr, int lineStartPos, Token[] tokens, String code) {
      String html = "<html>";

      /* Add line number */
      html += "<font color=\"333333\">" + lineNr + ":  </font>";

      /* Add code */
      if (tokens == null || tokens.length == 0 || lineStartPos < 0) {
        html += "<font color=\"000000\">" + code + "</font>";
      } else {
        for (int i=tokens.length-1; i >= 0; i--) {
          Token subToken = tokens[i];

          String colorString = "000000";

          /* Determine code color */
          final int type = subToken.symbol.type;
          switch (type) {
          case TokenTypes.COMMENT:
          case TokenTypes.START_COMMENT:
          case TokenTypes.MID_COMMENT:
          case TokenTypes.END_COMMENT:
            colorString = "00AA00";
            break;
          case TokenTypes.STRING:
            colorString = "0000AA";
            break;
          case TokenTypes.KEYWORD:
          case TokenTypes.KEYWORD2:
            colorString = "AA0000";
            break;
          }

          logger.debug(subToken.symbol.name + " is type " + subToken.symbol.type + " and gets color: " + colorString);

          /* Extract part of token residing in current line */
          int tokenLinePos;
          String subCode;
          if (subToken.position < lineStartPos) {
            subCode = subToken.symbol.name.substring(lineStartPos - subToken.position);
            tokenLinePos = 0;
          } else if (subToken.position + subToken.symbol.name.length() > lineStartPos + code.length()) {
            subCode = subToken.symbol.name.substring(0, code.length() + lineStartPos - subToken.position);
            tokenLinePos = subToken.position - lineStartPos;
          } else {
            subCode = subToken.symbol.name;
            tokenLinePos = subToken.position - lineStartPos;
          }

          subCode = stringToHTMLString(subCode);
          String firstPart = code.substring(0, tokenLinePos);
          String coloredSubCode = "<font color=\"" + colorString + "\">" + subCode + "</font>";
          String lastPart =
            tokenLinePos + subToken.symbol.name.length() >= code.length()?
                "":code.substring(tokenLinePos + subToken.symbol.name.length());

            logger.debug(" IN>> '" + code + "'");
            code = firstPart + coloredSubCode + lastPart;
            logger.debug("OUT>> '" + code + "'");
        }

        code = code.replace("  ", " &nbsp;");
        html += code;
      }

      html += "</html>";
      return html;
    }


    public Component getListCellRendererComponent(
       JList list,
       Object value,
       int index,
       boolean isSelected,
       boolean cellHasFocus)
     {
      int lineNr = index + 1;

      if (tokensArray != null && index < tokensArray.length && tokensArray[index] != null) {
        setText(getColoredLabelText(lineNr, tokensStartPos[index], tokensArray[index], (String) value));
      } else {
        setText(getColoredLabelText(lineNr, 0, null, (String) value));
      }

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

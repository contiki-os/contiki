package se.sics.mspsim.extutil.highlight;
import java.awt.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.event.*;

import java.io.*;

// Public domain, no restrictions, Ian Holyer, University of Bristol.

/**
 * Display text with syntax highlighting. Highlighting is done with full
 * accuracy, using a given language scanner. Large amounts of re-highlighting
 * are done in small bursts to make sure the user interface doesn't freeze.
 */
public class SyntaxHighlighter extends JTextPane implements DocumentListener, TokenTypes {


  private static final long serialVersionUID = -1801145479677890566L;

  private StyledDocument doc;
  private Scanner scanner;
  private int rows, columns;

  private int currentY, currentHeight = -1;

  /**
   * Create a graphics component which displays text with syntax highlighting.
   * Provide a rows and columns, in characters, and a language scanner.
   */
  public SyntaxHighlighter(int rows, int columns, Scanner scanner) {
    super(new DefaultStyledDocument());
    doc = (StyledDocument) getDocument();
    this.rows = rows;
    this.columns = columns;
    this.scanner = scanner;
    doc.addDocumentListener(this);
    Font font = new Font("Monospaced", Font.PLAIN, getFont().getSize());
    changeFont(font);
    initStyles();

    // Quick fix to highlight selected line
    addCaretListener(new CaretListener() {

      public void caretUpdate(CaretEvent e) {
        int caret = getCaretPosition();
        if (caret >= 0) {
          try {
            Rectangle r = getUI().modelToView(SyntaxHighlighter.this, caret);
            if (currentHeight > 0) {
              repaint(0, currentY, getWidth(), currentHeight);
            }
            if (r != null && r.height > 0) {
              currentY = r.y;
              currentHeight = r.height;
              repaint(0, r.y, getWidth(), r.height);
            } else {
              currentHeight = -1;
            }

          } catch (BadLocationException e1) {
            // Ignore
          }
        }
      }

    });

    setOpaque(false);

  }

  public int getColumns() {
    return columns;
  }

  public int getRows() {
    return rows;
  }

  /**
   * Change the component's font, and change the size of the component to match.
   */
  public void changeFont(Font font) {
    int borderOfJTextPane = 3;
    setFont(font);
    FontMetrics metrics = getFontMetrics(font);
    int paneWidth = columns * metrics.charWidth('m') + 2 * borderOfJTextPane;
    int paneHeight = rows * metrics.getHeight() + 2 * borderOfJTextPane;
    Dimension size = new Dimension(paneWidth, paneHeight);
    setMinimumSize(size);
    setPreferredSize(size);
    invalidate();
  }

  /**
   * Read new text into the component from a <code>Reader</code>. Overrides
   * <code>read</code> in <code>JTextComponent</code> in order to highlight
   * the new text.
   */
  public void read(Reader in, Object desc) throws IOException {
    int oldLength = getDocument().getLength();
    doc.removeDocumentListener(this);
    super.read(in, desc);
    doc = (StyledDocument) getDocument();
    doc.addDocumentListener(this);
    int newLength = getDocument().getLength();
    firstRehighlightToken = scanner.change(0, oldLength, newLength);
    repaint();
  }

  // An array of styles, indexed by token type. Default styles are set up,
  // which can be used for any languages.

  private Style[] styles;

  private void initStyles() {
    styles = new Style[typeNames.length];
    changeStyle(UNRECOGNIZED, Color.black);
    changeStyle(WHITESPACE, Color.black);
    changeStyle(WORD, Color.black);
    changeStyle(NUMBER, Color.black);
    changeStyle(PUNCTUATION, Color.blue);
    changeStyle(COMMENT, new Color(178,34,34), Font.ITALIC);
    changeStyle(START_COMMENT, new Color(178,34,34), Font.ITALIC);
    changeStyle(MID_COMMENT, new Color(178,34,34), Font.ITALIC);
    changeStyle(END_COMMENT, new Color(178,34,34), Font.ITALIC);
    changeStyle(TAG, Color.blue, Font.BOLD);
    changeStyle(END_TAG, Color.blue, Font.BOLD);
    changeStyle(KEYWORD, new Color(160,32,240));
    changeStyle(KEYWORD2, new Color(160,32,240));
    changeStyle(IDENTIFIER, Color.black);
    changeStyle(LITERAL, Color.green.darker());
    changeStyle(STRING, new Color(188,143,143));
    changeStyle(CHARACTER, new Color(188,143,143));
    changeStyle(OPERATOR, Color.black, Font.BOLD);
    changeStyle(BRACKET, Color.black);
    changeStyle(SEPARATOR, Color.black);
    changeStyle(URL, Color.blue.darker());

    for (int i = 0; i < styles.length; i++) {
      if (styles[i] == null) {
        styles[i] = styles[WHITESPACE];
      }
    }
  }

  /**
   * Change the style of a particular type of token.
   */
  public void changeStyle(int type, Color color) {
    Style style = addStyle(typeNames[type], null);
    StyleConstants.setForeground(style, color);
    styles[type] = style;
  }

  /**
   * Change the style of a particular type of token, including adding bold or
   * italic using a third argument of <code>Font.BOLD</code> or
   * <code>Font.ITALIC</code> or the bitwise union
   * <code>Font.BOLD|Font.ITALIC</code>.
   */
  public void changeStyle(int type, Color color, int fontStyle) {
    Style style = addStyle(typeNames[type], null);
    StyleConstants.setForeground(style, color);
    if ((fontStyle & Font.BOLD) != 0)
      StyleConstants.setBold(style, true);
    if ((fontStyle & Font.ITALIC) != 0)
      StyleConstants.setItalic(style, true);
    styles[type] = style;
  }

  public int getLineCount() {
    return getDocument().getDefaultRootElement().getElementCount();
  }

  public int getLineStartOffset(int line) {
    Element root = getDocument().getDefaultRootElement();
    if (line < 0 || line >= root.getElementCount()) {
      throw new IndexOutOfBoundsException("illegal line");
    }
    return root.getElement(line).getStartOffset();
  }

  public int getLineEndOffset(int line) {
    Element root = getDocument().getDefaultRootElement();
    if (line < 0 || line >= root.getElementCount()) {
      throw new IndexOutOfBoundsException("illegal line");
    }
    return root.getElement(line).getEndOffset();
  }


  /**
   * <font style='color:gray;'>Ignore this method. Responds to the underlying
   * document changes by re-highlighting.</font>
   */
  public void insertUpdate(DocumentEvent e) {
    int offset = e.getOffset();
    int length = e.getLength();
    firstRehighlightToken = scanner.change(offset, 0, length);
    repaint();
  }

  /**
   * <font style='color:gray;'>Ignore this method. Responds to the underlying
   * document changes by re-highlighting.</font>
   */
  public void removeUpdate(DocumentEvent e) {
    int offset = e.getOffset();
    int length = e.getLength();
    firstRehighlightToken = scanner.change(offset, length, 0);
    repaint();
  }

  /**
   * <font style='color:gray;'>Ignore this method. Responds to the underlying
   * document changes by re-highlighting.</font>
   */
  public void changedUpdate(DocumentEvent e) {
    // Do nothing.
  }

  // Scan a small portion of the document. If more is needed, call repaint()
  // so the GUI gets a go and doesn't freeze, but calls this again later.

  private Segment text = new Segment();

  private int firstRehighlightToken;

  private int smallAmount = 100;

  private Color highlightColor = new Color(0, 240, 0, 255);

  /**
   * <font style='color:gray;'>Ignore this method. Carries out a small amount of
   * re-highlighting for each call to <code>repaint</code>.</font>
   */
  protected void paintComponent(Graphics g) {
    if (currentHeight > 0) {
      g.setColor(highlightColor);
      g.fillRect(0, currentY, getWidth(), currentHeight);
    }

    super.paintComponent(g);


    int offset = scanner.position();
    if (offset < 0)
      return;

    int tokensToRedo = 0;
    int amount = smallAmount;
    while (tokensToRedo == 0 && offset >= 0) {
      int length = doc.getLength() - offset;
      if (length > amount) {
        length = amount;
      }
      try {
        doc.getText(offset, length, text);
      } catch (BadLocationException e) {
        return;
      }
      tokensToRedo = scanner.scan(text.array, text.offset, text.count);
      offset = scanner.position();
      amount = 2 * amount;
    }
    for (int i = 0; i < tokensToRedo; i++) {
      Token t = scanner.getToken(firstRehighlightToken + i);
      int length = t.symbol.name.length();
      int type = t.symbol.type;
      if (type < 0) {
        type = UNRECOGNIZED;
      }
      doc.setCharacterAttributes(t.position, length, styles[type], false);
    }
    firstRehighlightToken += tokensToRedo;
    if (offset >= 0) {
      repaint(2);
    }
  }

  public void viewLine(int line) {
    if (line >= 0 && line < getLineCount()) {
      try {
        int pos = getLineStartOffset(line);

        // Quick fix to position the line somewhere in the center
        Rectangle r = getUI().modelToView(this, pos);
        if (r != null && r.height > 0) {
          Rectangle vr = getVisibleRect();
          vr.y = r.y - vr.height / 2;
          if (vr.y < 0) {
            vr.y = 0;
          }
          scrollRectToVisible(vr);
        }

        setCaretPosition(pos);
      } catch (BadLocationException e1) {
        // Ignore
      }
    }
  }
}

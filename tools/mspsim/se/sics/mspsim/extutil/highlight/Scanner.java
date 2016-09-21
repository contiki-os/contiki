package se.sics.mspsim.extutil.highlight;
import java.util.HashMap;

// Public domain, no restrictions, Ian Holyer, University of Bristol.

/**
 * <p>
 * A Scanner object provides a lexical analyser and a resulting token array.
 * Incremental rescanning is supported, e.g. for use in a token colouring
 * editor. This is a base class dealing with plain text, which can be extended
 * to support other languages.
 * 
 * <p>
 * The actual text is assumed to be held elsewhere, e.g. in a document. The
 * <code>change()</code> method is called to report the position and length of
 * a change in the text, and the <code>scan()</code> method is called to
 * perform scanning or rescanning. For example, to scan an entire document held
 * in a character array <code>text</code> in one go:
 * 
 * <blockquote>
 * 
 * <pre>
 * scanner.change(0, 0, text.length);
 * scanner.scan(text, 0, text.length);
 * </pre>
 * 
 * </blockquote>
 * 
 * <p>
 * For incremental scanning, the <code>position()</code> method is used to
 * find the text position at which rescanning should start. For example, a
 * syntax highlighter might contain this code:
 * 
 * <blockquote>
 * 
 * <pre>
 * // Where to start rehighlighting, and a segment object
 * int firstRehighlightToken;
 * Segment segment;
 * ...
 * // Whenever the text changes, e.g. on an insert or remove or read.
 * firstRehighlightToken = scanner.change(offset, oldLength, newLength);
 * repaint();
 * ...
 * // in repaintComponent
 * int offset = scanner.position();
 * if (offset &lt; 0) return;
 * int tokensToRedo = 0;
 * int amount = 100;
 * while (tokensToRedo == 0 &amp;&amp; offset &gt;= 0)
 * {
 *    int length = doc.getLength() - offset;
 *    if (length &gt; amount) length = amount;
 *    try { doc.getText(offset, length, text); }
 *    catch (BadLocationException e) { return; }
 *    tokensToRedo = scanner.scan(text.array, text.offset, text.count);
 *    offset = scanner.position();
 *    amount = 2*amount;
 * }
 * for (int i = 0; i &lt; tokensToRedo; i++)
 * {
 *    Token t = scanner.getToken(firstRehighlightToken + i);
 *    int length = t.symbol.name.length();
 *    int type = t.symbol.type;
 *    doc.setCharacterAttributes (t.position, length, styles[type], false);
 * }
 * firstRehighlightToken += tokensToRedo;
 * if (offset &gt;= 0) repaint(2);
 * </pre>
 * 
 * </blockquote>
 * 
 * <p>
 * Note that <code>change</code> can be called at any time, even between calls
 * to <code>scan</code>. Only small number of characters are passed to
 * <code>scan</code> so that only a small burst of scanning is done, to
 * prevent the program's user interface from freezing.
 */
public class Scanner implements TokenTypes {
  /**
   * <p>
   * Read one token from the start of the current text buffer, given the start
   * offset, end offset, and current scanner state. The method moves the start
   * offset past the token, updates the scanner state, and returns the type of
   * the token just scanned.
   * 
   * <p>
   * The scanner state is a representative token type. It is either the state
   * left after the last call to read, or the type of the old token at the same
   * position if rescanning, or WHITESPACE if at the start of a document. The
   * method succeeds in all cases, returning whitespace or comment or error
   * tokens where necessary. Each line of a multi-line comment is treated as a
   * separate token, to improve incremental rescanning. If the buffer does not
   * extend to the end of the document, the last token returned for the buffer
   * may be incomplete and the caller must rescan it. The read method can be
   * overridden to implement different languages. The default version splits
   * plain text into words, numbers and punctuation.
   */
  protected int read() {
    char c = buffer[start];
    int type;
    // Ignore the state, since there is only one.
    if (Character.isWhitespace(c)) {
      type = WHITESPACE;
      while (++start < end) {
        if (!Character.isWhitespace(buffer[start]))
          break;
      }
    } else if (Character.isLetter(c)) {
      type = WORD;
      while (++start < end) {
        c = buffer[start];
        if (Character.isLetter(c) || Character.isDigit(c))
          continue;
        if (c == '-' || c == '\'' || c == '_')
          continue;
        break;
      }
    } else if (Character.isDigit(c)) {
      type = NUMBER;
      while (++start < end) {
        c = buffer[start];
        if (!Character.isDigit(c) && c != '.')
          break;
      }
    } else if (c >= '!' || c <= '~') {
      type = PUNCTUATION;
      start++;
    } else {
      type = UNRECOGNIZED;
      start++;
    }

    // state = WHITESPACE;
    return type;
  }

  /**
   * The current buffer of text being scanned.
   */
  protected char[] buffer;

  /**
   * The current offset within the buffer, at which to scan the next token.
   */
  protected int start;

  /**
   * The end offset in the buffer.
   */
  protected int end;

  /**
   * The current scanner state, as a representative token type.
   */
  protected int state = WHITESPACE;

  // The array of tokens forms a gap buffer. The total length of the text is
  // tracked, and tokens after the gap have (negative) positions relative to
  // the end of the text. While scanning, the gap represents the area to be
  // scanned, no tokens after the gap can be taken as valid, and in particular
  // the end-of-text sentinel token is after the gap.

  private Token[] tokens;

  private int gap, endgap, textLength;

  private boolean scanning;

  private int position;

  /**
   * The symbol table can be accessed by <code>initSymbolTable</code> or
   * <code>lookup</code>, if they are overridden. Symbols are inserted with
   * <code>symbolTable.put(sym,sym)</code> and extracted with
   * <code>symbolTable.get(sym)</code>.
   */
  protected HashMap<Symbol,Symbol> symbolTable;

  /**
   * Create a new Scanner representing an empty text document. For
   * non-incremental scanning, use change() to report the document size, then
   * pass the entire text to the scan() method in one go, or if coming from an
   * input stream, a bufferful at a time.
   */
  public Scanner() {
    tokens = new Token[1];
    gap = 0;
    endgap = 0;
    textLength = 0;
    symbolTable = new HashMap<Symbol,Symbol>();
    initSymbolTable();
    Symbol endOfText = new Symbol(WHITESPACE, "");
    tokens[0] = new Token(endOfText, 0);
    scanning = false;
    position = 0;
  }

  // Move the gap to a new index within the tokens array. When preparing to
  // pass a token back to a caller, this is used to ensure that the token's
  // position is relative to the start of the text and not the end.

  private void moveGap(int newgap) {
    if (scanning)
      throw new Error("moveGap called while scanning");
    if (newgap < 0 || newgap > gap + tokens.length - endgap) {
      throw new Error("bad argument to moveGap");
    }
    if (gap < newgap) {
      while (gap < newgap) {
        tokens[endgap].position += textLength;
        tokens[gap++] = tokens[endgap++];
      }
    } else if (gap > newgap) {
      while (gap > newgap) {
        tokens[--endgap] = tokens[--gap];
        tokens[endgap].position -= textLength;
      }
    }
  }

  /**
   * Find the number of available valid tokens, not counting tokens in or after
   * any area yet to be rescanned.
   */
  public int size() {
    if (scanning) {
      return gap;
    }
    return gap + tokens.length - endgap;
  }

  /**
   * Find the n'th token, or null if it is not currently valid.
   */
  public Token getToken(int n) {
    if (n < 0 || n >= gap && scanning)
      return null;
    if (n >= gap)
      moveGap(n + 1);
    return tokens[n];
  }

  /**
   * Find the index of the valid token starting before, but nearest to, text
   * position p. This uses an O(log(n)) binary chop search.
   */
  public int find(int p) {
    int start = 0, end, mid, midpos;
    if (!scanning)
      moveGap(gap + tokens.length - endgap);
    end = gap - 1;
    if (p > tokens[end].position)
      return end;
    while (end > start + 1) {
      mid = (start + end) / 2;
      midpos = tokens[mid].position;
      if (p > midpos) {
        start = mid;
      } else {
        end = mid;
      }
    }
    return start;
  }

  /**
   * Report the position of an edit, the length of the text being replaced, and
   * the length of the replacement text, to prepare for rescanning. The call
   * returns the index of the token at which rescanning will start.
   */
  public int change(int start, int len, int newLen) {
    if (start < 0 || len < 0 || newLen < 0 || start + len > textLength) {
      throw new Error("change(" + start + "," + len + "," + newLen + ")");
    }
    textLength += newLen - len;
    int end = start + newLen;
    if (scanning) {
      while (gap > 0 && tokens[gap - 1].position > start)
        gap--;
      if (gap > 0)
        gap--;
      if (gap > 0) {
        gap--;
        position = tokens[gap].position;
        state = tokens[gap].symbol.type;
      } else {
        position = 0;
        state = WHITESPACE;
      }
      while (tokens[endgap].position + textLength < end)
        endgap++;
      return gap;
    }
    if (endgap == tokens.length)
      moveGap(gap - 1);
    scanning = true;
    while (tokens[endgap].position + textLength < start) {
      tokens[endgap].position += textLength;
      tokens[gap++] = tokens[endgap++];
    }
    while (gap > 0 && tokens[gap - 1].position > start) {
      tokens[--endgap] = tokens[--gap];
      tokens[endgap].position -= textLength;
    }
    if (gap > 0)
      gap--;
    if (gap > 0) {
      gap--;
      position = tokens[gap].position;
      state = tokens[gap].symbol.type;
    } else {
      position = 0;
      state = WHITESPACE;
    }
    while (tokens[endgap].position + textLength < end)
      endgap++;
    return gap;
  }

  /**
   * Find out at what text position any remaining scanning work should start, or
   * -1 if scanning is complete.
   */
  public int position() {
    if (!scanning) {
      return -1;
    }
    return position;
  }

  /**
   * Create the initial symbol table. This can be overridden to enter keywords,
   * for example. The default implementation does nothing.
   */
  protected void initSymbolTable() {
    // Nothing as default
  }

  // Reuse this symbol object to create each new symbol, then look it up in
  // the symbol table, to replace it by a shared version to minimize space.

  private Symbol symbol = new Symbol(0, null);

  /**
   * Lookup a symbol in the symbol table. This can be overridden to implement
   * keyword detection, for example. The default implementation just uses the
   * table to ensure that there is only one shared occurrence of each symbol.
   */
  protected Symbol lookup(int type, String name) {
    symbol.type = type;
    symbol.name = name;
    Symbol sym = symbolTable.get(symbol);
    if (sym != null) {
      return sym;
    }
    sym = new Symbol(type, name);
    symbolTable.put(sym, sym);
    return sym;
  }

  /**
   * Scan or rescan a given read-only segment of text. The segment is assumed to
   * represent a portion of the document starting at <code>position()</code>.
   * Return the number of tokens successfully scanned, excluding any partial
   * token at the end of the text segment but not at the end of the document. If
   * the result is 0, the call should be retried with a longer segment.
   */
  public int scan(char[] array, int offset, int length) {
    if (!scanning)
      throw new Error("scan called when not scanning");
    if (position + length > textLength)
      throw new Error("scan too much");
    boolean all = position + length == textLength;
    end = start + length;
    int startGap = gap;

    buffer = array;
    start = offset;
    end = start + length;
    while (start < end) {
      int tokenStart = start;
      int type = read();
      if (start == end && !all)
        break;

      if (type != WHITESPACE) {
        String name = new String(buffer, tokenStart, start - tokenStart);
        Symbol sym = lookup(type, name);
        Token t = new Token(sym, position);
        if (gap >= endgap)
          checkCapacity(gap + tokens.length - endgap + 1);
        tokens[gap++] = t;
      }

      // Try to synchronise

      while (tokens[endgap].position + textLength < position)
        endgap++;
      if (position + start - tokenStart == textLength)
        scanning = false;
      else if (gap > 0 && tokens[endgap].position + textLength == position
          && tokens[endgap].symbol.type == type) {
        endgap++;
        scanning = false;
        break;
      }
      position += start - tokenStart;
    }
    checkCapacity(gap + tokens.length - endgap);
    return gap - startGap;
  }

  // Change the size of the gap buffer, doubling it if it fills up, and
  // halving if it becomes less than a quarter full.

  private void checkCapacity(int capacity) {
    int oldCapacity = tokens.length;
    if (capacity <= oldCapacity && 4 * capacity >= oldCapacity)
      return;
    Token[] oldTokens = tokens;
    int newCapacity;
    if (capacity > oldCapacity) {
      newCapacity = oldCapacity * 2;
      if (newCapacity < capacity)
        newCapacity = capacity;
    } else
      newCapacity = capacity * 2;

    tokens = new Token[newCapacity];
    System.arraycopy(oldTokens, 0, tokens, 0, gap);
    int n = oldCapacity - endgap;
    System.arraycopy(oldTokens, endgap, tokens, newCapacity - n, n);
    endgap = newCapacity - n;
  }

  void print() {
    for (int i = 0; i < tokens.length; i++) {
      if (i >= gap && i < endgap)
        continue;
      if (i == endgap)
        System.out.print("... ");
      System.out.print("" + i + ":" + tokens[i].position);
      System.out.print("-"
          + (tokens[i].position + tokens[i].symbol.name.length()));
      System.out.print(" ");
    }
    System.out.println();
  }
}

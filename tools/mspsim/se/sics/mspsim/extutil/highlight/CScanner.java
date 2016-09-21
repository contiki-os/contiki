package se.sics.mspsim.extutil.highlight;
// Public domain, no restrictions, Ian Holyer, University of Bristol.

/**
 * <p>
 * Provide a hand-written scanner for the Java language.
 */

public class CScanner extends Scanner {

  private boolean debug = false;

  /** Create a Java scanner, for Java version 1.5 by default. */
  public CScanner() {
    super();
    initKind();
    initUniKind();
  }

  /** Create a Java scanner, for a given version between "1.1" and "1.5". */
  public CScanner(String version) {
    super();
    initKind();
    initUniKind();
  }

  /** Override the read method from the Scanner class. */
  protected int read() {
    int type, saveStart = 0;
    if (debug)
      saveStart = start;

    if (start >= end)
      return WHITESPACE;

    switch (state) {
    case MID_COMMENT:
    case END_COMMENT:
      type = readComment(MID_COMMENT);
      if (type == END_COMMENT)
        state = WHITESPACE;
      else
        state = MID_COMMENT;
      return type;
    default:
      char c = buffer[start];
      if (c == '\\')
        c = next();
      if (c < 128)
        type = kind[c];
      else
        type = unikind[Character.getType(c)];
      switch (type) {
      case WHITESPACE:
        start = start + charlength;
        charlength = 1;
        while (start < end) {
          c = buffer[start];
          if (c == '\\')
            c = next();
          int k;
          if (c < 128)
            k = kind[c];
          else
            k = unikind[Character.getType(c)];
          if (k != WHITESPACE)
            break;
          start = start + charlength;
          charlength = 1;
        }
        break;
      case UNRECOGNIZED:
      case BRACKET:
      case SEPARATOR:
        start = start + charlength;
        charlength = 1;
        break;
      case OPERATOR:
        start = start + charlength;
        charlength = 1;
        type = readOperator(c);
        break;
      case CHARACTER:
        start = start + charlength;
        charlength = 1;
        type = readCharLiteral();
        break;
      case STRING:
        start = start + charlength;
        charlength = 1;
        type = readStringLiteral();
        break;
      case IDENTIFIER:
        start = start + charlength;
        charlength = 1;
        while (start < end) {
          c = buffer[start];
          if (c == '\\')
            c = next();
          int k;
          if (c < 128)
            k = kind[c];
          else
            k = unikind[Character.getType(c)];
          if (k != IDENTIFIER && k != NUMBER)
            break;
          start = start + charlength;
          charlength = 1;
        }
        break;
      case NUMBER:
        start = start + charlength;
        charlength = 1;
        type = readNumber(c);
        break;
      case PUNCTUATION:
        start = start + charlength;
        charlength = 1;
        type = readDot();
        break;
      case COMMENT:
        start = start + charlength;
        charlength = 1;
        type = readSlash();
        if (type == START_COMMENT)
          state = MID_COMMENT;
        break;
      }
    }
    if (debug) {
      System.out.print(TokenTypes.typeNames[type]);
      System.out.println(" " + saveStart + "," + end + "("
          + (start - saveStart) + ")");
    }
    return type;
  }

  private int readOperator(char c) {
    if (start >= end)
      return OPERATOR;
    char c2;

    switch (c) {
    case '~':
    case '?':
    case ':':
      break;
    case '+':
    case '-':
    case '&':
    case '|':
      c2 = buffer[start];
      if (c2 == '\\')
        c2 = next();
      if (c2 != c && c2 != '=')
        break;
      start = start + charlength;
      charlength = 1;
      break;
    case '=':
    case '*':
    case '!':
    case '^':
    case '%':
    case '/':
      c2 = buffer[start];
      if (c2 == '\\')
        c2 = next();
      if (c2 != '=')
        break;
      start = start + charlength;
      charlength = 1;
      break;
    case '<':
    case '>':
      c2 = buffer[start];
      if (c2 == '\\')
        c2 = next();
      if (c2 == '=') {
        start = start + charlength;
        charlength = 1;
      } else if (c2 == c) {
        start = start + charlength;
        charlength = 1;
        if (start >= end)
          break;
        char c3 = buffer[start];
        if (c3 == '\\')
          c3 = next();
        if (c3 == '=') {
          start = start + charlength;
          charlength = 1;
        } else if (c == '>' && c3 == '>') // >>>
        {
          start = start + charlength;
          charlength = 1;
          if (start >= end)
            break;
          char c4 = buffer[start];
          if (c4 == '\\')
            c4 = next();
          if (c4 != '=')
            break;
          start = start + charlength;
          charlength = 1;
        }
      }
      break;
    }
    return OPERATOR;
  }

  private int readCharLiteral() {
    if (start >= end)
      return bad(CHARACTER);
    char c2 = buffer[start];
    if (c2 == '\\')
      c2 = next();

    switch (c2) {
    case '\\':
      start = start + charlength;
      charlength = 1;
      boolean ok = readEscapeSequence();
      if (!ok)
        return bad(CHARACTER);
      break;
    case '\'':
    case '\n':
      return bad(CHARACTER);
    default:
      start = start + charlength;
      charlength = 1;
      break;
    }
    if (start >= end)
      return bad(CHARACTER);
    char c3 = buffer[start];
    if (c3 == '\\')
      c3 = next();
    if (c3 != '\'')
      return bad(CHARACTER);
    start = start + charlength;
    charlength = 1;
    return CHARACTER;
  }

  private int readStringLiteral() {
    if (start >= end)
      return bad(STRING);
    char c = buffer[start];
    if (c == '\\')
      c = next();

    while (c != '"') {
      switch (c) {
      case '\\':
        start = start + charlength;
        charlength = 1;
        boolean ok = readEscapeSequence();
        if (!ok)
          return bad(STRING);
        break;
      case '\n':
        return bad(STRING);
      default:
        start = start + charlength;
        charlength = 1;
        if (start >= end)
          return bad(STRING);
        break;
      }
      c = buffer[start];
      if (c == '\\')
        c = next();
    }
    if (c != '"')
      return bad(STRING);
    start = start + charlength;
    charlength = 1;
    return STRING;
  }

  private int readSlash() {
    if (start >= end)
      return OPERATOR;
    char c = buffer[start];
    if (c == '\\')
      c = next();
    if (c == '/') {
      while (c != '\n') {
        start = start + charlength;
        charlength = 1;
        if (start >= end)
          return COMMENT;
        c = buffer[start];
        if (c == '\\')
          c = next();
      }
      start = start + charlength;
      charlength = 1;
      return COMMENT;
    } else if (c == '*') {
      start = start + charlength;
      charlength = 1;
      return readComment(START_COMMENT);
    }
    return readOperator('/');
  }

  // Read one line of a /*...*/ comment, given the expected type
  int readComment(int type) {
    if (start >= end)
      return type;
    char c = buffer[start];
    if (c == '\\')
      c = next();

    while (true) {
      while (c != '*' && c != '\n') {
        start = start + charlength;
        charlength = 1;
        if (start >= end)
          return type;
        c = buffer[start];
        if (c == '\\')
          c = next();
      }
      start = start + charlength;
      charlength = 1;
      if (c == '\n')
        return type;
      if (start >= end)
        return type;
      c = buffer[start];
      if (c == '\\')
        c = next();
      if (c == '/') {
        start = start + charlength;
        charlength = 1;
        if (type == START_COMMENT) {
          return COMMENT;
        }
        return END_COMMENT;
      }
    }
  }

  // Read a number, without checking whether it is out of range
  // Doesn't deal with e.g. 0777.9 or 07779f
  private int readNumber(char c) {
    if (c == '0') {
      int saveStart = start, saveLength = charlength;
      start = start + charlength;
      charlength = 1;
      if (start >= end)
        return NUMBER;
      char c2 = buffer[start];
      if (c2 == '\\')
        c2 = next();
      switch (c2) {
      case 'x':
      case 'X':
        start = start + charlength;
        charlength = 1;
        boolean ok = readDigits(16);
        if (!ok)
          return bad(NUMBER);
        readSuffix();
        return NUMBER;
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        readDigits(8);
        readSuffix();
        return NUMBER;
      case '.':
      case 'e':
      case 'E':
        start = saveStart;
        charlength = saveLength;
        break;
      case 'f':
      case 'F':
      case 'd':
      case 'D':
        start = start + charlength;
        charlength = 1;
        return NUMBER;
      case 'l':
      case 'L':
        start = start + charlength;
        charlength = 1;
        return NUMBER;
      }
    }
    boolean hasDigits = false;
    if ('0' <= c && c <= '9') {
      hasDigits = true;
      readDigits(10);
      if (start >= end)
        return NUMBER;
      c = buffer[start];
      if (c == '\\')
        c = next();
      if (c == 'l' || c == 'L') {
        start = start + charlength;
        charlength = 1;
        return NUMBER;
      }
    }
    if (c == '.') {
      start = start + charlength;
      charlength = 1;
      if (start >= end)
        return NUMBER;
      c = buffer[start];
      if (c == '\\')
        c = next();
      if ('0' <= c && c <= '9') {
        hasDigits = true;
        readDigits(10);
        if (start >= end)
          return NUMBER;
        c = buffer[start];
        if (c == '\\')
          c = next();
      }
    }
    if (!hasDigits)
      return bad(NUMBER);
    switch (c) {
    case 'e':
    case 'E':
      start = start + charlength;
      charlength = 1;
      if (start >= end)
        return bad(NUMBER);
      c = buffer[start];
      if (c == '\\')
        c = next();
      if (c == '+' || c == '-') {
        start = start + charlength;
        charlength = 1;
        if (start >= end)
          return bad(NUMBER);
        c = buffer[start];
        if (c == '\\')
          c = next();
      }
      readDigits(10);
      break;
    case 'f':
    case 'F':
    case 'd':
    case 'D':
      start = start + charlength;
      charlength = 1;
      return NUMBER;
    }
    return NUMBER;
  }

  boolean readDigits(int radix) {
    if (start >= end)
      return false;
    char c = buffer[start];
    if (c == '\\')
      c = next();
    if (Character.digit(c, radix) == -1)
      return false;
    while (Character.digit(c, radix) != -1) {
      start = start + charlength;
      charlength = 1;
      if (start >= end)
        return true;
      c = buffer[start];
      if (c == '\\')
        c = next();
    }
    return true;
  }

  void readSuffix() {
    if (start >= end)
      return;
    char c = buffer[start];
    if (c == '\\')
      c = next();
    switch (c) {
    case 'f':
    case 'F':
    case 'd':
    case 'D':
    case 'l':
    case 'L':
      start = start + charlength;
      charlength = 1;
    }
  }

  private int readDot() {
    if (start >= end)
      return SEPARATOR;
    char c2 = buffer[start];
    if (c2 == '\\')
      c2 = next();
    if (Character.isDigit(c2)) {
      return readNumber('.');
    }
    if (start + 1 >= end) //  || version < 15)
      return SEPARATOR;
    if (c2 != '.' || buffer[start + 1] != '.')
      return SEPARATOR;
    start = start + 2;
    return SEPARATOR;
  }

  private boolean readEscapeSequence() {
    if (start >= end)
      return false;
    char c2 = buffer[start];
    if (c2 == '\\')
      c2 = next();

    switch (c2) {
    case 'b':
    case 't':
    case 'n':
    case 'f':
    case 'r':
    case '\"':
    case '\'':
    case '\\':
      start = start + charlength;
      charlength = 1;
      return true;
    case '0':
    case '1':
    case '2':
    case '3':
      return readOctal(3);
    case '4':
    case '5':
    case '6':
    case '7':
      return readOctal(2);
    default:
      return false;
    }
  }

  boolean readOctal(int maxlength) {
    if (start >= end)
      return false;
    char c = buffer[start];
    if (c == '\\')
      c = next();

    int i, val = 0;
    for (i = 0; i < maxlength; i++) {
      if (Character.digit(c, 8) != -1) {
        val = 8 * val + Character.digit(c, 8);
        start = start + charlength;
        charlength = 1;
        if (start >= end)
          break;
        c = buffer[start];
        if (c == '\\')
          c = next();
      } else
        break;
    }
    if ((i == 0) || (val > 0xFF))
      return false;
    return true;
  }

  // A malformed or incomplete token has a negative type
  private int bad(int type) {
    return -type;
  }

  // Look ahead at the next character or unicode escape.
  // For efficiency, replace c = next(); with
  // c = buffer[start]; if (c == '\\') c = next();
  // To accept the character after looking at it, use:
  // start = start + charlength; charlength = 1;

  // Record the number of source code characters used up. To deal with an odd
  // or even number of backslashes preceding a unicode escape, whenever a
  // second backslash is coming up, mark its position as a pair.

  private int charlength = 1;

  private int pair = 0;

  private char next() {
    if (start >= end)
      return 26; // EOF
    char c = buffer[start];
    if (c != '\\')
      return c;
    if (start == pair) {
      pair = 0;
      return '\\';
    }
    if (start + 1 >= end)
      return '\\';

    c = buffer[start + 1];
    if (c == '\\')
      pair = start + 1;
    if (c != 'u')
      return '\\';

    int pos = start + 2;
    while (pos < end && buffer[pos] == 'u')
      pos++;
    if (pos + 4 > end) {
      charlength = end - start;
      return '\0';
    }

    c = 0;
    for (int j = 0; j < 4; j++) {
      int d = Character.digit(buffer[pos + j], 16);
      if (d < 0) {
        charlength = pos + j - start;
        return '\0';
      }
      c = (char) (c * 16 + d);
    }
    charlength = pos + 4 - start;
    return c;
  }

  // Override initSymbolTable

  protected void initSymbolTable() {
    lookup(KEYWORD, "auto");
    lookup(KEYWORD, "asm");
    lookup(KEYWORD, "break");
    lookup(KEYWORD, "case");
    lookup(KEYWORD, "const");
    lookup(KEYWORD, "continue");
    lookup(KEYWORD, "default");
    lookup(KEYWORD, "define");
    lookup(KEYWORD, "do");
    lookup(KEYWORD, "double");
    lookup(KEYWORD, "else");
    lookup(KEYWORD, "endif");
    lookup(KEYWORD, "enum");
    lookup(KEYWORD, "extern");
    lookup(KEYWORD, "for");
    lookup(KEYWORD, "goto");
    lookup(KEYWORD, "if");
    lookup(KEYWORD, "ifdef");
    lookup(KEYWORD, "ifndef");
    lookup(KEYWORD, "inline");
    lookup(KEYWORD, "include");
    lookup(KEYWORD, "private");
    lookup(KEYWORD, "protected");
    lookup(KEYWORD, "public");
    lookup(KEYWORD, "register");
    lookup(KEYWORD, "return");
    lookup(KEYWORD, "sizeof");
    lookup(KEYWORD, "static");
    lookup(KEYWORD, "struct");
    lookup(KEYWORD, "super");
    lookup(KEYWORD, "switch");
    lookup(KEYWORD, "typedef");
    lookup(KEYWORD, "union");
    lookup(KEYWORD, "volatile");
    lookup(KEYWORD, "while");

    lookup(LITERAL, "TRUE");
    lookup(LITERAL, "FALSE");
    lookup(LITERAL, "NULL");
    lookup(LITERAL, "int8_t");
    lookup(LITERAL, "int16_t");
    lookup(LITERAL, "int32_t");
    lookup(LITERAL, "uint8_t");
    lookup(LITERAL, "uint16_t");
    lookup(LITERAL, "uint32_t");
    lookup(LITERAL, "u8_t");
    lookup(LITERAL, "u16_t");
    lookup(LITERAL, "u32_t");
    lookup(LITERAL, "int");
    lookup(LITERAL, "long");
    lookup(LITERAL, "float");
    lookup(LITERAL, "double");
    lookup(LITERAL, "void");
    lookup(LITERAL, "unsigned");
    lookup(LITERAL, "signed");
    lookup(LITERAL, "char");
    lookup(LITERAL, "short");
  }

  // *** Override lookup, but what about unicode escape translation?

  private Symbol temp = new Symbol(0, null);

  protected Symbol lookup(int type, String name) {
    if (type != IDENTIFIER)
      return super.lookup(type, name);
    temp.type = KEYWORD;
    temp.name = name;
    Symbol sym = symbolTable.get(temp);
    if (sym != null)
      return sym;
    temp.type = LITERAL;
    sym = symbolTable.get(temp);
    if (sym != null)
      return sym;
    return super.lookup(type, name);
  }

  // Classify the ascii characters using an array of kinds, and classify all
  // other unicode characters using an array indexed by unicode category.
  // See the source file java/lang/Character.java for the categories.
  // To find the classification of a character, use:
  // if (c < 128) k = kind[c]; else k = unikind[Character.getType(c)];

  private static final byte[] kind = new byte[128];

  private static final byte[] unikind = new byte[31];

  // Initialise the two classification arrays using static initializer code.
  // Token types from the TokenTypes class are used to classify characters.

  private void initKind() {
    for (char c = 0; c < 128; c++)
      kind[c] = -1;
    for (char c = 0; c < 128; c++)
      switch (c) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 11:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 27:
      case 28:
      case 29:
      case 30:
      case 31:
      case 127:
      case '#':
      case '@':
      case '`':
      case '\\':
        kind[c] = UNRECOGNIZED;
        break;
      case '\t':
      case '\n':
      case ' ':
      case '\f':
      case 26:
        kind[c] = WHITESPACE;
        break;
      case '!':
      case '%':
      case '&':
      case '*':
      case '+':
      case '-':
      case ':':
      case '<':
      case '=':
      case '>':
      case '?':
      case '^':
      case '|':
      case '~':
        kind[c] = OPERATOR;
        break;
      case '"':
        kind[c] = STRING;
        break;
      case '\'':
        kind[c] = CHARACTER;
        break;
      case '.':
        kind[c] = PUNCTUATION;
        break;
      case '/':
        kind[c] = COMMENT;
        break;
      case '$':
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'G':
      case 'H':
      case 'I':
      case 'J':
      case 'K':
      case 'L':
      case 'M':
      case 'N':
      case 'O':
      case 'P':
      case 'Q':
      case 'R':
      case 'S':
      case 'T':
      case 'U':
      case 'V':
      case 'W':
      case 'X':
      case 'Y':
      case 'Z':
      case '_':
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
      case 'i':
      case 'j':
      case 'k':
      case 'l':
      case 'm':
      case 'n':
      case 'o':
      case 'p':
      case 'q':
      case 'r':
      case 's':
      case 't':
      case 'u':
      case 'v':
      case 'w':
      case 'x':
      case 'y':
      case 'z':
        kind[c] = IDENTIFIER;
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        kind[c] = NUMBER;
        break;
      case '(':
      case ')':
      case '[':
      case ']':
      case '{':
      case '}':
        kind[c] = BRACKET;
        break;
      case ',':
      case ';':
        kind[c] = SEPARATOR;
        break;
      }
    for (char c = 0; c < 128; c++)
      if (kind[c] == -1)
        System.out.println("Char " + ((int) c) + " hasn't been classified");
  }

  private void initUniKind() {
    for (byte b = 0; b < 31; b++)
      unikind[b] = -1;
    for (byte b = 0; b < 31; b++)
      switch (b) {
      case Character.UNASSIGNED:
      case Character.ENCLOSING_MARK:
      case Character.OTHER_NUMBER:
      case Character.SPACE_SEPARATOR:
      case Character.LINE_SEPARATOR:
      case Character.PARAGRAPH_SEPARATOR:
      case Character.CONTROL:
      case 17: // category 17 is unused
      case Character.PRIVATE_USE:
      case Character.SURROGATE:
      case Character.DASH_PUNCTUATION:
      case Character.START_PUNCTUATION:
      case Character.END_PUNCTUATION:
      case Character.OTHER_PUNCTUATION:
      case Character.MATH_SYMBOL:
      case Character.MODIFIER_SYMBOL:
      case Character.OTHER_SYMBOL:
      case Character.INITIAL_QUOTE_PUNCTUATION:
      case Character.FINAL_QUOTE_PUNCTUATION:
        unikind[b] = UNRECOGNIZED;
        break;
      case Character.UPPERCASE_LETTER:
      case Character.LOWERCASE_LETTER:
      case Character.TITLECASE_LETTER:
      case Character.MODIFIER_LETTER:
      case Character.OTHER_LETTER:
      case Character.LETTER_NUMBER:
      case Character.CONNECTOR_PUNCTUATION: // maybe NUMBER
      case Character.CURRENCY_SYMBOL:
        // Characters where Other_ID_Start is true
        unikind[b] = IDENTIFIER;
        break;
      case Character.NON_SPACING_MARK:
      case Character.COMBINING_SPACING_MARK:
      case Character.DECIMAL_DIGIT_NUMBER:
      case Character.FORMAT:
        unikind[b] = NUMBER;
        break;
      }
    for (byte b = 0; b < 31; b++)
      if (unikind[b] == -1)
        System.out.println("Unicode cat " + b + " hasn't been classified");
  }

}

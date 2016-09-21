package se.sics.mspsim.extutil.highlight;
// Public domain, no restrictions, Ian Holyer, University of Bristol.

/**
 * The TokenTypes interface defines the integer constants representing different
 * types of tokens, for use with any languages. The constants are used in
 * symbols to represent the types of similar tokens, and in scanners as scanner
 * states, and in highlighters to determine the colour or style of tokens. There
 * is also an array typeNames of textual names, indexed by type, for descriptive
 * purposes.
 *
 * <p>
 * The UNRECOGNIZED constant (zero) is for tokens which are completely
 * unrecognized, usually consisting of a single illegal character. Other error
 * tokens are represented by negative types, where -t represents an incomplete
 * or malformed token of type t. An error token usually consists of the maximal
 * legal substring of the source text.
 *
 * <p>
 * The WHITESPACE constant is used to classify tokens which are to be discarded,
 * it acts as a suitable scanner state at the beginning of a document, and it is
 * used for the usual end-of-text sentinel token which marks the end of the
 * document. Comments can optionally be classified as WHITESPACE and discarded,
 * if they are not needed for highlighting. No other types besides UNRECOGNIZED
 * and WHITESPACE are treated specially.
 *
 * <p>
 * The constants are presented as an interface so that any class can implement
 * the interface and use the names of the constants directly, without prefixing
 * them with a class name.
 *
 */
public interface TokenTypes {
  public static final int UNRECOGNIZED = 0, WHITESPACE = 1, WORD = 2,
      NUMBER = 3, PUNCTUATION = 4, COMMENT = 5, START_COMMENT = 6,
      MID_COMMENT = 7, END_COMMENT = 8, TAG = 9, END_TAG = 10, KEYWORD = 11,
      KEYWORD2 = 12, IDENTIFIER = 13, LITERAL = 14, STRING = 15,
      CHARACTER = 16, OPERATOR = 17, BRACKET = 18, SEPARATOR = 19, URL = 20, DEFINE = 21;


  /**
   * The names of the token types, indexed by type, are provided for descriptive
   * purposes.
   */
  public static final String[] typeNames = {
    "bad token", "whitespace", "word", "number", "punctuation", "comment",
    "start of comment", "middle of comment", "end of comment", "tag",
    "end tag", "keyword", "keyword 2", "identifier", "literal", "string",
    "character", "operator", "bracket", "separator", "url", "define"
  };
}

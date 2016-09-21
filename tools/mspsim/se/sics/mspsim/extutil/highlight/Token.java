package se.sics.mspsim.extutil.highlight;
// Public domain, no restrictions, Ian Holyer, University of Bristol.

/**
 * A token represents a smallest meaningful fragment of text, such as a word,
 * recognised by a scanner.
 */
public class Token {
  /**
   * The symbol contains all the properties shared with similar tokens.
   */
  public Symbol symbol;

  /**
   * The token's position is given by an index into the document text.
   */
  public int position;

  /**
   * Create a token with a given symbol and position.
   */
  public Token(Symbol symbol, int position) {
    this.symbol = symbol;
    this.position = position;
  }
}

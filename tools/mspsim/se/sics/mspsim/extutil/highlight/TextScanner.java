package se.sics.mspsim.extutil.highlight;
// This does exactly the same job as the Scanner class, but uses a table driven
// scanner.  This illustrates how to extend the Scanner class, and how a
// generator might produce a table-driven scanner (though this one was written
// by hand).  Public domain, no restrictions, Ian Holyer, University of Bristol.

public class TextScanner extends Scanner {
  // Classify the 128 ASCII characters. Define a compact 'readable' form,
  // then expand into an array, using static initializer code. The 128 are:
  // .........tn..................... !"#$%&'()*+,-./0123456789:;<=>?
  // @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~.

  static final String cs =
      "1111111112211111111111111111111127777774777774675555555555777777"
          + "7333333333333333333333333337777473333333333333333333333333377771";

  static final byte[] kinds = new byte[cs.length()];
  {
    for (int i = 0; i < cs.length(); i++) {
      kinds[i] = (byte) (cs.charAt(i) - '0');
    }
  }

  // This state table has a row per scanner state, and a column per class of
  // character. Each entry is '-s' to mean accept the current character and
  // goto state s, or '+t' to mean end the token giving it type t. There is
  // no stack of states, or extra table to determine the next state after a
  // token is recognized, as there could be with more sophisticated scanners.
  // The raw numbers +t must use or match the constants in TokenTypes.

  int[][] table = {
  // $ \0 \n z '-_ 9 . ( // $ = end of text, must be column 0
      { +1, -1, -2, -3, -4, -5, -4, -4 }, // s=0: start token
      { +1, +1, +1, +1, +1, +1, +1, +1 }, // s=1: illegal character (t=1)
      { +0, +0, -2, +0, +0, +0, +0, +0 }, // s=2: whitespace (t=0)
      { +2, +2, +2, -3, -3, -3, +2, +2 }, // s=3: word (t=2)
      { +4, +4, +4, +4, +4, +4, +4, +4 }, // s=4: punctuation (t=4)
      { +3, +3, +3, +3, +3, -5, -5, +3 } // s=5: number (t=3)
      };

  // Now all we have to do is to override read(), using the parent variables
  // buffer, start, end, state.

  protected int read() {
    state = 0;
    int kind = kinds[buffer[start]];
    int type;
    while (true) {
      type = table[state][kind];
      if (type >= 0)
        break;
      state = -type;
      start++;
      if (start >= end)
        kind = 0;
      else
        kind = kinds[buffer[start]];
    }
    state = WHITESPACE;
    return type;
  }
}

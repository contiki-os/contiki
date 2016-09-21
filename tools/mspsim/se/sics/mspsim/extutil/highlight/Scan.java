package se.sics.mspsim.extutil.highlight;
// Illustrate the use of the scanner by reading in a file and displaying its
// tokens. Public domain, no restrictions, Ian Holyer, University of Bristol.

import java.io.*;

public class Scan {
  // Get the filename from the command line
  public static void main(String[] args) throws IOException {
    Scan program = new Scan();
    if (args.length != 1) {
      System.out.println("Usage: java Scan filename");
    } else {
      program.scan(args[0]);
    }
  }

  // Scan each line in turn
  public void scan(String filename) throws IOException {
    File file = new File(filename);
    int len = (int) file.length();
    char[] buffer = new char[len];
    Reader in = new FileReader(file);
    in.read(buffer);
    in.close();

    Scanner scanner = new Scanner();
    scanner.change(0, 0, len);
    scanner.scan(buffer, 0, len);

    for (int i = 0; i < scanner.size(); i++) {
      Token t = scanner.getToken(i);
      System.out.print("" + t.position);
      System.out.print(": " + t.symbol.name);
      System.out.println(" " + TokenTypes.typeNames[t.symbol.type]);
    }
  }
}

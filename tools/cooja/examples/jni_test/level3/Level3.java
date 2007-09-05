/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: Level3.java,v 1.3 2007/09/05 18:39:42 fros4943 Exp $
 */

import java.io.*;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Level3 {

  final static private String bssSectionAddrRegExp =
    "^.bss[ \t]*0x([0-9A-Fa-f]*)[ \t]*0x[0-9A-Fa-f]*[ \t]*$";
  final static private String bssSectionSizeRegExp =
    "^.bss[ \t]*0x[0-9A-Fa-f]*[ \t]*0x([0-9A-Fa-f]*)[ \t]*$";
  final static private String dataSectionAddrRegExp =
    "^.data[ \t]*0x([0-9A-Fa-f]*)[ \t]*0x[0-9A-Fa-f]*[ \t]*$";
  final static private String dataSectionSizeRegExp =
    "^.data[ \t]*0x[0-9A-Fa-f]*[ \t]*0x([0-9A-Fa-f]*)[ \t]*$";

  static {
    System.load(new File("level3.library").getAbsolutePath());
  }

  private native void test();

  public Level3() {
    File mapFile = new File("level3.map");

    // Check that map file exists
    if (!mapFile.exists()) {
      System.err.println("No map file could be loaded");
      System.exit(1);
    }

    System.err.println("Loading map file");
    Vector<String> mapContents = loadMapFile(mapFile);

    System.err.println("Parsing map file");
    int relDataSectionAddr = loadRelDataSectionAddr(mapContents);
    int dataSectionSize = loadDataSectionSize(mapContents);
    int relBssSectionAddr = loadRelBssSectionAddr(mapContents);
    int bssSectionSize = loadBssSectionSize(mapContents);

    System.err.println("Found relative data section address: 0x" + Integer.toHexString(relDataSectionAddr));
    System.err.println("Found data section size: 0x" + Integer.toHexString(dataSectionSize));
    System.err.println("Found relative bss section address: 0x" + Integer.toHexString(relBssSectionAddr));
    System.err.println("Found bss section size: 0x" + Integer.toHexString(bssSectionSize));

    if (dataSectionSize < 4) {
      System.err.println("Data section size is " + Integer.toHexString(dataSectionSize) + " but should be 0x4!");
      System.exit(1);
    }
    if (bssSectionSize < 4) {
      System.err.println("BSS section size is " + Integer.toHexString(bssSectionSize) + " but should be 0x4!");
      System.exit(1);
    }
    test();
  }

  private static Vector<String> loadMapFile(File mapFile) {
    Vector<String> mapContents = new Vector<String>();

    try {
      BufferedReader in =
        new BufferedReader(
            new InputStreamReader(
                new FileInputStream(mapFile)));

      while (in.ready())
      {
        mapContents.add(in.readLine());
      }
    } catch (FileNotFoundException e) {
      System.err.println("File not found: " + e);
      return null;
    } catch (IOException e) {
      System.err.println("IO error: " + e);
      return null;
    }

    return mapContents;
  }

  private static int loadRelDataSectionAddr(Vector<String> mapFile) {
    String retString = getFirstMatchGroup(mapFile, dataSectionAddrRegExp, 1);

    if (retString != null) {
      return Integer.parseInt(retString.trim(), 16);
    } else {
      return 0;
    }
  }

  private static int loadDataSectionSize(Vector<String> mapFile) {
    String retString = getFirstMatchGroup(mapFile, dataSectionSizeRegExp, 1);

    if (retString != null) {
      return Integer.parseInt(retString.trim(), 16);
    } else {
      return 0;
    }
  }

  private static int loadRelBssSectionAddr(Vector<String> mapFile) {
    String retString = getFirstMatchGroup(mapFile, bssSectionAddrRegExp, 1);

    if (retString != null) {
      return Integer.parseInt(retString.trim(), 16);
    } else {
      return 0;
    }
  }

  private static int loadBssSectionSize(Vector<String> mapFile) {
    String retString = getFirstMatchGroup(mapFile, bssSectionSizeRegExp, 1);

    if (retString != null) {
      return Integer.parseInt(retString.trim(), 16);
    } else {
      return 0;
    }
  }

  private static String getFirstMatchGroup(Vector<String> lines, String regexp, int groupNr) {
    Pattern pattern = Pattern.compile(regexp);
    for (int i=0; i < lines.size(); i++) {
      Matcher matcher = pattern.matcher(lines.elementAt(i));
      if (matcher.find()) {
        return matcher.group(groupNr);
      }
    }
    return null;
  }


  public static void main(String[] args) {
    new Level3();
  }

}

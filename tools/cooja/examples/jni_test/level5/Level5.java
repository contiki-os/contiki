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
 * $Id: Level5.java,v 1.2 2007/01/11 14:28:26 fros4943 Exp $
 */

import java.io.*;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Level5 {

  static {
    System.load(new File("level5.library").getAbsolutePath());
  }

  final static private String bssSectionAddrRegExp =
    "^.bss[ \t]*0x([0-9A-Fa-f]*)[ \t]*0x[0-9A-Fa-f]*[ \t]*$";
  final static private String bssSectionSizeRegExp =
    "^.bss[ \t]*0x[0-9A-Fa-f]*[ \t]*0x([0-9A-Fa-f]*)[ \t]*$";
  final static private String dataSectionAddrRegExp =
    "^.data[ \t]*0x([0-9A-Fa-f]*)[ \t]*0x[0-9A-Fa-f]*[ \t]*$";
  final static private String dataSectionSizeRegExp =
    "^.data[ \t]*0x[0-9A-Fa-f]*[ \t]*0x([0-9A-Fa-f]*)[ \t]*$";
  final static private String varAddressRegExpPrefix =
    "^[ \t]*0x([0-9A-Fa-f]*)[ \t]*";
  final static private String varAddressRegExpSuffix =
    "[ \t]*$";
  final static private String varNameRegExp =
    "^[ \t]*(0x[0-9A-Fa-f]*)[ \t]*([^ ]*)[ \t]*$";
  final static private String varSizeRegExpPrefix =
    "^";
  final static private String varSizeRegExpSuffix =
    "[ \t]*(0x[0-9A-Fa-f]*)[ \t]*[^ ]*[ \t]*$";
  
  private native void doCount();
  private native int getRefAddress();
  private native byte[] getMemory(int start, int length);
  private native void setMemory(int start, int length, byte[] mem);

  private int javaDataCounter = 1;
  private int javaBssCounter = 0;
  
  public Level5() {
    File mapFile = new File("level5.map");

    // Check that map file exists
    if (!mapFile.exists()) {
      System.err.println("No map file could be loaded");
      System.exit(1);
    }

    Vector<String> mapContents = loadMapFile(mapFile);

    int relDataSectionAddr = loadRelDataSectionAddr(mapContents);
    int dataSectionSize = (int) loadDataSectionSize(mapContents);
    int relBssSectionAddr = loadRelBssSectionAddr(mapContents);
    int bssSectionSize = (int) loadBssSectionSize(mapContents);

    int referenceAddress = getRefAddress();

    int offsetRelToAbs = referenceAddress - getRelVarAddr(mapContents, "ref_var");

    System.err.println("\n\n--- RUNNING DO_COUNT 5 TIMES ---");
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;

    System.err.println("\n\n--- CHECKPOINT #1: JAVA COUNTERS SHOULD EQUAL C ---");
    System.err.println(">> JavaDATA_counter=\t" + javaDataCounter + "\tJavaBSS_counter=\t" + javaBssCounter);
    
    System.err.println("\n\n--- FETCHING AND SAVING MEMORY ---");
    byte[] savedDataSection = getMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize);
    byte[] savedBssSection = getMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize);
    System.err.println("data section size:\t" + savedDataSection.length + " = " + "0x" + Integer.toHexString(savedDataSection.length));
    System.err.println("bss section size:\t" + savedBssSection.length + " = " + "0x" + Integer.toHexString(savedBssSection.length));
    
    System.err.println("\n\n--- RUNNING DO_COUNT 3 TIMES ---");
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;

    System.err.println("\n\n--- CHECKPOINT #2: JAVA COUNTERS SHOULD EQUAL C ---");
    System.err.println(">> JavaDATA_counter=\t" + javaDataCounter + "\tJavaBSS_counter=\t" + javaBssCounter);
    
    System.err.println("\n\n--- RESTORING MEMORY: DATA ---");
    setMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, savedDataSection); javaDataCounter -= 3;

    System.err.println("\n\n--- RUNNING DO_COUNT 3 TIMES ---");
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;

    System.err.println("\n\n--- CHECKPOINT #3: JAVA COUNTERS SHOULD EQUAL C ---");
    System.err.println(">> JavaDATA_counter=\t" + javaDataCounter + "\tJavaBSS_counter=\t" + javaBssCounter);
    
    System.err.println("\n\n--- RESTORING MEMORY: BSS ---");
    setMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, savedBssSection); javaBssCounter -= 6;
    
    System.err.println("\n\n--- RUNNING DO_COUNT 3 TIMES ---");
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;

    System.err.println("\n\n--- CHECKPOINT #4: JAVA COUNTERS SHOULD EQUAL C ---");
    System.err.println(">> JavaDATA_counter=\t" + javaDataCounter + "\tJavaBSS_counter=\t" + javaBssCounter);
    
    System.err.println("Level 5 OK!");
  }

  private static int getRelVarAddr(Vector<String> mapContents, String varName) {
    String regExp = varAddressRegExpPrefix + varName + varAddressRegExpSuffix;
    String retString = getFirstMatchGroup(mapContents, regExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else return 0;
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

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else return 0;
  }

  private static int loadDataSectionSize(Vector<String> mapFile) {
    String retString = getFirstMatchGroup(mapFile, dataSectionSizeRegExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else return 0;
  }

  private static int loadRelBssSectionAddr(Vector<String> mapFile) {
    String retString = getFirstMatchGroup(mapFile, bssSectionAddrRegExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else return 0;
  }

  private static int loadBssSectionSize(Vector<String> mapFile) {
    String retString = getFirstMatchGroup(mapFile, bssSectionSizeRegExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else return 0;
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
    new Level5();
  }

}

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
 * $Id: Level3a.java,v 1.1 2007/09/10 14:06:03 fros4943 Exp $
 */

import java.io.*;
import java.util.*;
import org.apache.log4j.xml.DOMConfigurator;

import se.sics.cooja.GUI;
import se.sics.cooja.contikimote.ContikiMoteType;

public class Level3a {
  private final File externalToolsSettingsFile = new File("../exttools.cfg");

  public Level3a() {
    // Configure logger
    DOMConfigurator.configure(GUI.class.getResource("/" + GUI.LOG_CONFIG_FILE));

    // Load configuration
    System.out.println("Loading COOJA configuration");
    GUI.externalToolsUserSettingsFile = externalToolsSettingsFile;
    GUI.loadExternalToolsDefaultSettings();
    GUI.loadExternalToolsUserSettings();

    System.out.println("Locating map file");
    File mapFile = new File("level3a.map");
    if (!mapFile.exists()) {
      System.err.println("No map file could be loaded");
      System.exit(1);
    }

    System.out.println("Loading map data");
    Vector<String> mapData = ContikiMoteType.loadMapFile(mapFile);
    if (mapData == null) {
      System.err.println("No map data could be loaded");
      System.exit(1);
    }

    System.out.println("Parsing map data");
    Properties addresses = new Properties();
    boolean parseOK = ContikiMoteType.parseMapFileData(mapData, addresses);
    if (!parseOK) {
      System.err.println("Map data parsing failed");
      System.exit(1);
    }
    int relDataSectionAddr = ContikiMoteType.loadRelDataSectionAddr(mapData);
    int dataSectionSize = ContikiMoteType.loadDataSectionSize(mapData);
    int relBssSectionAddr = ContikiMoteType.loadRelBssSectionAddr(mapData);
    int bssSectionSize = ContikiMoteType.loadBssSectionSize(mapData);

    System.out.println("Found relative data section address: 0x" + Integer.toHexString(relDataSectionAddr));
    System.out.println("Found data section size: 0x" + Integer.toHexString(dataSectionSize));
    System.out.println("Found relative bss section address: 0x" + Integer.toHexString(relBssSectionAddr));
    System.out.println("Found bss section size: 0x" + Integer.toHexString(bssSectionSize));

    System.out.println("Checking validity of parsed addresses");

    String varName;
    varName = "var1";
    if (!addresses.containsKey(varName)) {
      System.err.println("Could not find address of: " + varName);
      System.exit(1);
    }
    varName = "var2";
    if (!addresses.containsKey(varName)) {
      System.err.println("Could not find address of: " + varName);
      System.exit(1);
    }
    varName = "arr1";
    if (!addresses.containsKey(varName)) {
      System.err.println("Could not find address of: " + varName);
      System.exit(1);
    }
    varName = "arr2";
    if (!addresses.containsKey(varName)) {
      System.err.println("Could not find address of: " + varName);
      System.exit(1);
    }
    varName = "uvar1";
    if (!addresses.containsKey(varName)) {
      System.err.println("Could not find address of: " + varName);
      System.exit(1);
    }
    varName = "uvar2";
    if (!addresses.containsKey(varName)) {
      System.err.println("Could not find address of: " + varName);
      System.exit(1);
    }
    if (relDataSectionAddr < 0) {
      System.err.println("Data segment address < 0: 0x" + Integer.toHexString(relDataSectionAddr));
      System.exit(1);
    }
    if (relBssSectionAddr < 0) {
      System.err.println("BSS segment address < 0: 0x" + Integer.toHexString(relBssSectionAddr));
      System.exit(1);
    }
    if (dataSectionSize <= 0) {
      System.err.println("Data segment size <= 0: 0x" + Integer.toHexString(dataSectionSize));
      System.exit(1);
    }
    if (bssSectionSize <= 0) {
      System.err.println("BSS segment size <= 0: 0x" + Integer.toHexString(bssSectionSize));
      System.exit(1);
    }

    System.out.println("Level 3a OK!");
  }

  public static void main(String[] args) {
    new Level3a();
  }

}

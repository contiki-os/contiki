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
 * $Id: Level5.java,v 1.3 2007/09/10 14:07:12 fros4943 Exp $
 */

import java.io.*;
import java.util.Properties;
import java.util.Vector;
import org.apache.log4j.xml.DOMConfigurator;

import se.sics.cooja.GUI;
import se.sics.cooja.SectionMoteMemory;
import se.sics.cooja.contikimote.ContikiMoteType;

public class Level5 {
  private final File externalToolsSettingsFile = new File("../exttools.cfg");

  static {
    System.load(new File("level5.library").getAbsolutePath());
  }

  private native void doCount();
  private native int getRefAddress();
  public native void getMemory(int start, int length, byte[] mem);
  public native void setMemory(int start, int length, byte[] mem);

  private int javaDataCounter = 1;
  private int javaBssCounter = 0;

  public Level5() {

    // Configure logger
    DOMConfigurator.configure(GUI.class.getResource("/" + GUI.LOG_CONFIG_FILE));

    // Load configuration
    System.out.println("Loading COOJA configuration");
    GUI.externalToolsUserSettingsFile = externalToolsSettingsFile;
    GUI.loadExternalToolsDefaultSettings();
    GUI.loadExternalToolsUserSettings();

    // Should we parse addresses using map file or nm?
    boolean useNm = Boolean.parseBoolean(GUI.getExternalToolsSetting("PARSE_WITH_NM", "false"));

    Properties addresses = new Properties();
    int relDataSectionAddr = -1;
    int dataSectionSize = -1;
    int relBssSectionAddr = -1;
    int bssSectionSize = -1;

    if (useNm) {
      // Parse nm output
      System.out.println("Parsing using nm");

      File libFile = new File("level5.library");
      if (!libFile.exists()) {
        System.err.println("Library file " + libFile.getAbsolutePath() + " could not be found!");
        System.exit(1);
      }

      Vector<String> nmData = ContikiMoteType.loadNmData(libFile);
      if (nmData == null) {
        System.err.println("No nm data could be loaded");
        System.exit(1);
      }

      boolean parseOK = ContikiMoteType.parseNmData(nmData, addresses);
      if (!parseOK) {
        System.err.println("Nm data parsing failed");
        System.exit(1);
      }

      relDataSectionAddr = ContikiMoteType.loadNmRelDataSectionAddr(nmData);
      dataSectionSize = ContikiMoteType.loadNmDataSectionSize(nmData);
      relBssSectionAddr = ContikiMoteType.loadNmRelBssSectionAddr(nmData);
      bssSectionSize = ContikiMoteType.loadNmBssSectionSize(nmData);
    } else {
      // Parse map file
      System.out.println("Parsing using map file");
      File mapFile = new File("level5.map");
      if (!mapFile.exists()) {
        System.err.println("No map file could be loaded");
        System.exit(1);
      }

      Vector<String> mapData = ContikiMoteType.loadMapFile(mapFile);
      if (mapData == null) {
        System.err.println("No map data could be loaded");
        System.exit(1);
      }

      boolean parseOK = ContikiMoteType.parseMapFileData(mapData, addresses);
      if (!parseOK) {
        System.err.println("Map data parsing failed");
        System.exit(1);
      }

      relDataSectionAddr = ContikiMoteType.loadRelDataSectionAddr(mapData);
      dataSectionSize = ContikiMoteType.loadDataSectionSize(mapData);
      relBssSectionAddr = ContikiMoteType.loadRelBssSectionAddr(mapData);
      bssSectionSize = ContikiMoteType.loadBssSectionSize(mapData);
    }

    int absRefAddress = getRefAddress();
    int relRefAddress = (Integer) addresses.get("ref_var");
    int offsetRelToAbs = absRefAddress - relRefAddress;

    System.out.println("Creating section memory");
    byte[] initialDataSection = new byte[dataSectionSize];
    byte[] initialBssSection = new byte[bssSectionSize];

    getMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, initialDataSection);
    getMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, initialBssSection);
    SectionMoteMemory memory = new SectionMoteMemory(addresses);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);

    int dataCounter, bssCounter;

    System.out.print("Checking initial values: ");
    dataCounter = memory.getIntValueOf("initialized_counter");
    bssCounter = memory.getIntValueOf("uninitialized_counter");
    if (dataCounter != javaDataCounter || bssCounter != javaBssCounter) {
      System.out.println("FAILED!");
    } else {
      System.out.println("OK!");
    }

    System.out.println("Increasing counters 5 times");
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;

    System.out.print("Checking increased values: ");
    getMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, initialDataSection);
    getMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, initialBssSection);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);
    dataCounter = memory.getIntValueOf("initialized_counter");
    bssCounter = memory.getIntValueOf("uninitialized_counter");
    if (dataCounter != javaDataCounter || bssCounter != javaBssCounter) {
      System.out.println("FAILED!");
      System.exit(1);
    } else {
      System.out.println("OK!");
    }

    System.out.println("Storing both memory segments now");
    byte[] savedDataSection = new byte[dataSectionSize];
    byte[] savedBssSection = new byte[bssSectionSize];
    int savedDataCounter = dataCounter;
    int savedBssCounter = bssCounter;
    getMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, savedDataSection);
    getMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, savedBssSection);

    System.out.println("Increasing counters 3 times");
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;

    System.out.print("Checking increased values: ");
    getMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, initialDataSection);
    getMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, initialBssSection);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);
    dataCounter = memory.getIntValueOf("initialized_counter");
    bssCounter = memory.getIntValueOf("uninitialized_counter");
    if (dataCounter != javaDataCounter || bssCounter != javaBssCounter) {
      System.out.println("FAILED!");
      System.exit(1);
    } else {
      System.out.println("OK!");
    }

    System.out.println("Restoring data segment");
    setMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, savedDataSection);
    javaDataCounter = savedDataCounter;

    System.out.println("Increasing counters 3 times");
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;
    doCount(); javaDataCounter++; javaBssCounter++;

    System.out.print("Checking reset data counter: ");
    getMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, initialDataSection);
    getMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, initialBssSection);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);
    dataCounter = memory.getIntValueOf("initialized_counter");
    bssCounter = memory.getIntValueOf("uninitialized_counter");
    if (dataCounter != javaDataCounter || bssCounter != javaBssCounter) {
      System.out.println("FAILED!");
      System.exit(1);
    } else {
      System.out.println("OK!");
    }

    System.out.println("Restoring bss segment");
    setMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, savedBssSection);
    javaBssCounter = savedBssCounter;

    System.out.print("Checking reset bss counter: ");
    getMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, initialDataSection);
    getMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, initialBssSection);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);
    dataCounter = memory.getIntValueOf("initialized_counter");
    bssCounter = memory.getIntValueOf("uninitialized_counter");
    if (dataCounter != javaDataCounter || bssCounter != javaBssCounter) {
      System.out.println("FAILED!");
      System.exit(1);
    } else {
      System.out.println("OK!");
    }

    System.out.println("\n");
    System.out.println("Reading and writing memory segments via JNI successfully");
    System.out.println("Level 5 OK!");
  }

  public static void main(String[] args) {
    new Level5();
  }

}

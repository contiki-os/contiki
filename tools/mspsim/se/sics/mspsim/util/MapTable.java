/**
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * -----------------------------------------------------------------
 *
 * MapTable
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.util;
import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.regex.Pattern;


/**
 * The map reader reads the map file with memory map and
 * other information about the binary/firmware to load into the
 * node.
 *
 * Format of the map file must be:
 * .text WS Adress WS Size WS file
 * WS Adress WS function_name
 * WS Adress WS function_name
 * ...
 */
public class MapTable {

  private final static boolean DEBUG = false;

  private enum Mode {NONE,CODE,DATA,BSS};
  private Mode mode;

  public int heapStartAddress = -1;
  public int stackStartAddress = -1;
  private int bssFill = 0;
  private int dataFill = 0;

  private ArrayList<MapEntry> modules = new ArrayList<MapEntry>();
  private ArrayList<MapEntry> entries = new ArrayList<MapEntry>();
  private HashMap<Integer, MapEntry> addressMap = new HashMap<Integer, MapEntry>();
  
  public MapTable() {
  }

  public MapTable(String file) throws IOException {
    loadMap(file);
  }

  private MapEntry addModuleEntry(HashMap<String,MapEntry> moduleTable,
                                  int addr, int size, String name) {
    MapEntry entry = moduleTable.get(name);
    if (entry == null) {
      entry = new MapEntry(MapEntry.TYPE.module, addr, size, name, null, false);
      moduleTable.put(name, entry);
      modules.add(entry);
    } else if (size > 0) {
      entry.setSize(entry.getSize() + size);
    }
    return entry;
  }

  /**
   *  <code>parseMapLine</code>
   * parses a line of a map file!
   * @param line a <code>String</code> value
   */
  private void parseMapLine(HashMap<String,MapEntry> moduleTable, String line) {
    String parts[] = line.split("\\s+");
    if (line.startsWith(".text")) {
      mode = Mode.CODE;
      if (DEBUG) {
        System.out.println("CODE Mode");
      }
    } else if (line.startsWith(".bss")) {
      mode = Mode.BSS;
      if (DEBUG) {
        System.out.println("BSS Mode!");
      }
    } else if (line.startsWith(".data")) {
      mode = Mode.DATA;
      if (DEBUG) {
        System.out.println("Data Mode!");
      }
    } else if (line.startsWith(" .text") || line.startsWith(" .init")
               || line.startsWith(" .vectors")) {
      if (parts.length > 3) {
        int addr = Integer.parseInt(parts[2].substring(2), 16);
        int size = Integer.parseInt(parts[3].substring(2), 16);
        addModuleEntry(moduleTable, addr, size, parts[4]);
        if (DEBUG) {
          System.out.println("Module add: " + addr + " Size:" + size
                             + " file:" + parts[4]);
        }
      }
    } else if (line.startsWith(" .data")) {
      if (parts.length > 3) {
        int addr = Integer.parseInt(parts[2].substring(2), 16);
        int size = Integer.parseInt(parts[3].substring(2), 16);
        MapEntry entry = addModuleEntry(moduleTable, addr, 0, parts[4]);
        if (DEBUG) {
          System.out.println("Module add data: " + addr + " Size:" + size
                             + " file:" + parts[4]);
        }
        entry.setData(addr, size);
      }
    } else if (line.startsWith(" .bss") || line.startsWith(" COMMON")) {
      if (parts.length > 3) {
        int addr = Integer.parseInt(parts[2].substring(2), 16);
        int size = Integer.parseInt(parts[3].substring(2), 16);
        MapEntry entry = addModuleEntry(moduleTable, addr, 0, parts[4]);
        if (DEBUG) {
          System.out.println("Module add bss: " + addr + " Size:" + size
                             + " file: " + parts[4]);
        }
        entry.setBSS(addr, entry.getBSSSize() + size);
      }
    } else if (line.startsWith(" *fill*")) {
      if(parts.length > 3) {
        int size = Integer.parseInt(parts[3].substring(2), 16);
        if (mode == Mode.BSS) {
          bssFill += size;
        } else if (mode == Mode.DATA) {
          dataFill += size;
        }
      }

    } else if (mode == Mode.CODE && line.startsWith("    ")) {
      if (parts.length > 2) {
	// Scrap 0x and parse as hex!
	int val = Integer.parseInt(parts[1].substring(2), 16);
        if (DEBUG) {
          System.out.println("Function: " + parts[2] + " at " +
                             Utils.hex16(val));
        }
	// Add the file part later some time...
	// After the demo...
	setEntry(new MapEntry(MapEntry.TYPE.function, val, 0, parts[2], null, false));
      }

    } else if (line.contains(" _end = .") && parts.length > 2) {
      heapStartAddress = Integer.parseInt(parts[1].substring(2), 16);

    } else if (line.contains("PROVIDE (__stack") && parts.length > 2) {
      stackStartAddress = Integer.parseInt(parts[1].substring(2), 16);

//     } else if ((line.startsWith("text ")
//                 || line.startsWith("data ")
//                 || line.startsWith("vectors ")
//                 || line.startsWith("bootloader ")
//                 || line.startsWith("infomem ")
//                 || line.startsWith("infomemnobits ")) && parts.length == 4) {
      // Memory configuration

    }
  }

  public void loadMap(String file) throws IOException {
    FileInputStream fInput = new FileInputStream(file);
    BufferedReader bInput = new BufferedReader(new InputStreamReader(fInput));
    HashMap<String,MapEntry> moduleTable = new HashMap<String,MapEntry>();
    String line;
    while ((line = bInput.readLine()) != null) {
      parseMapLine(moduleTable, line);
    }
    bInput.close();
    fInput.close();
  }

  public String getFunctionName(int address) {
      MapEntry entry = getEntry(address);
      if (entry != null) {
          return entry.getName();
      } else {
          return null;
      }
  }

  public MapEntry getEntry(int address) {
      return addressMap.get(address);
  }

  public MapEntry[] getAllEntries() {
    return entries.toArray(new MapEntry[entries.size()]);
  }

  public MapEntry[] getEntries(String regexp) {
    Pattern pattern = Pattern.compile(regexp);
    ArrayList<MapEntry> allEntries = new ArrayList<MapEntry>();
    for (MapEntry entry : entries) {
        if (pattern.matcher(entry.getName()).find()) {
            allEntries.add(entry);
        }
    }
    return allEntries.toArray(new MapEntry[allEntries.size()]);
  }

  // Should be any symbol... not just function...
  public void setFunctionName(int address, String name) {
    setEntry(new MapEntry(MapEntry.TYPE.function, address, 0, name, null, false));
  }

  public void setEntry(MapEntry entry) {
    entries.add(entry);
    addressMap.put(entry.getAddress(), entry);
  }

  // Really slow way to find a specific function address!!!!
  // Either reimplement this or cache in hashtable...
  public int getFunctionAddress(String function) {
      for (MapEntry entry : entries) {
        if (function.equals(entry.getName())) {
          return entry.getAddress();
        }
      }
    return -1;
  }

  public int getStackStart() {
      return stackStartAddress;
  }

  public void setStackStart(int start) {
    stackStartAddress = start;
  }

  public int getHeapStart() {
      return heapStartAddress;
  }

  public void setHeapStart(int start) {
    heapStartAddress = start;
  }

  public static void main(String[] args) throws IOException {
    MapTable map = new MapTable(args[0]);
    int totsize = 0;
    int totdata = map.dataFill, totbss = map.bssFill;
//    int totmemory = totdata + totbss;
    System.out.printf("%7s %7s %7s  %4s %s\n",
                      "text", "data", "bss", "addr", "name");
    for (int i = 0; i < map.modules.size(); i++) {
      MapEntry module = map.modules.get(i);
      totsize += module.getSize();
      totdata += module.getDataSize();
      totbss += module.getBSSSize();
//      totmemory += module.getDataSize() + module.getBSSSize();
      System.out.printf("%7d %7d %7d $%04x %s\n", module.getSize(),
                        module.getDataSize(), module.getBSSSize(),
                        module.getAddress(), module.getName());
    }
    System.out.printf("%7d %7d %7d       Total Size\n",
                      totsize, totdata, totbss);
//     System.out.println("Total data/bss size: " + totmemory + " (0x" + Integer.toHexString(totmemory) + ") data: " + totdata + " bss: " + totbss);
  }
}

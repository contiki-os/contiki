package org.markn.contiki.z80.linker;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @author markn
 * 
 */
public class Objfile {
  private static final Pattern REFLINE = Pattern.compile("^S\\s(\\w+)\\sRef([\\dA-F]{4})");
  private static final Pattern DEFLINE = Pattern.compile("^S\\s(\\w+)\\sDef([\\dA-F]{4})");
  private static final Pattern AREALINE = Pattern.compile("^A\\s(\\w+)\\ssize\\s([\\dA-F]+)");

  /**
   * Parent object.
   */
  private Linker _linker;

  /**
   * A file that this object indicates.
   */
  private File _file;

  /**
   * References to be imported.
   */
  private List<String> _refs;

  /**
   * Area name and its size.
   */
  private Map<String, Area> _areas;

  /**
   * @param linker
   * @param file
   */
  public Objfile(Linker linker, File file) {
    _linker = linker;
    _file = file;
    _refs = new ArrayList<String>();
    _areas = new HashMap<String, Area>();
  }

  public File getFile() {
    return _file;
  }

  public Set<File> getRequiredFiles() {
    Set<File> files = new HashSet<File>();
    for (String ref : _refs) {
      if (_linker.getSymbol(ref).isAbsolute()) {
        // no need to link
        continue;
      }
      Symbol symbol = _linker.getSymbol(ref);
      if (symbol != null) {
        files.add(symbol.getFile());
      } else {
        System.out.println("undefined symbol:" + ref);
      }
    }
    return files;
  }

  public Linker getLinker() {
    return _linker;
  }
  
  public Area getArea(String name) {
    return _areas.get(name);
  }
  
  public int getAreaSize(String name) {
    Area area = _areas.get(name);
    if (area != null) {
      return area.getSize();
    } else {
      return 0;
    }
  }

  public void setAreaOffset(String name, short offset) {
    Area area = _areas.get(name);
    if (area != null) {
      area.setOffset(offset);
    }
  }

  public void analyze() throws IOException {
    System.out.println("analyzing:" + _file);
    int areaindex = 0;
    BufferedReader isr = new BufferedReader(new FileReader(_file));
    String tline = null;
    Area area = null;
    while (true) {
      String line = isr.readLine();
      if (line == null) {
        break;
      }
      Matcher m = AREALINE.matcher(line);
      if (m.find()) {
        String areaname = m.group(1);
        area = new Area(areaindex, areaname, Integer.parseInt(m.group(2), 16));
        _areas.put(areaname, area);
        areaindex++;
        continue;
      }
      m = REFLINE.matcher(line);
      if (m.find()) {
        _refs.add(m.group(1));
        continue;
      }
      m = DEFLINE.matcher(line);
      if (m.find()) {
        String symbolname = m.group(1);
        int address = Integer.parseInt(m.group(2), 16);
        Symbol symbol = new Symbol(_file, area, (short) address);
        _linker.addSymbol(symbolname, symbol);
        continue;
      }
      if (line.startsWith("T")) {
        // process T line
        tline = line;
      }
      if (line.startsWith("R")) {
        // process R line
        if (tline == null) {
          System.out.println("wrong format as object file:" + _file);
          continue;
        }
        new Line(this, tline, line);
        tline = null;
      }
    }
    isr.close();
  }

  public Area getArea(int index) {
    for (Area area : _areas.values()) {
      if (area.getIndex() == index) {
        return area;
      }
    }
    return null;
  }
  
  public Symbol getSymbol(int index) {
    String name = _refs.get(index);
    return _linker.getSymbol(name);
  }
  
  public void relocate(String areaname, byte[] image) {
    Area area = _areas.get(areaname);
    if (area != null) {
      area.relocate(this, image);
      return;
    }
    System.out.println("no such area:" + areaname + " on " + _file);
  }



}

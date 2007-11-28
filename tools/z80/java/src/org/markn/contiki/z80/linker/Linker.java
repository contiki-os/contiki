package org.markn.contiki.z80.linker;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Linker {
  private static final String _DATA = "_DATA";

  private static final String _GSINIT = "_GSINIT";

  private static final String _CODE = "_CODE";

  private static final Pattern SYMLINE = Pattern.compile("^00:([\\dA-F]{4})\\s(\\w+)");

  public static void main(String[] arg) throws IOException {
    File target = new File(arg[0]); 
    if (!target.exists()) {
      System.out.println(arg[0] + " is no exist.");
      return;
    }
    Linker linker = new Linker();
    linker.prepare();
    linker.make(target);
  }

  private Map<String, Symbol> _symbols;

  private Map<File, Objfile> _objfiles;

  private List<RelocationInformation> _relocations;

  public Linker() {
    _symbols = new HashMap<String, Symbol>();
    _objfiles = new HashMap<File, Objfile>();
    _relocations = new ArrayList<RelocationInformation>();
  }

  private void prepare() throws IOException {
    loadSymfile("contiki.sym");
    loadLibfile("contiki-pc-6001.lib");
    loadLibfile("c:/dev/sdcc/lib/z80/z80.lib");
  }

  private void make(File file) throws IOException {
    List<Objfile> required = new ArrayList<Objfile>();
    Objfile object = _objfiles.get(file);
    make(required, object);
    short codeSize = 0;
    for (Objfile obj : required) {
      obj.setAreaOffset(_CODE, codeSize);
      codeSize += obj.getAreaSize(_CODE);
    }
    short gsinitSize = 0;
    short gsinitOffset = codeSize;
    for (Objfile obj : required) {
      obj.setAreaOffset(_GSINIT, gsinitOffset);
      gsinitSize += obj.getAreaSize(_GSINIT);
      gsinitOffset += obj.getAreaSize(_GSINIT);
    }
    // add space for C9 (ret)
    gsinitSize++;
    gsinitOffset++;
    short dataSize = 0;
    short dataOffset = gsinitOffset;
    for (Objfile obj : required) {
      obj.setAreaOffset(_DATA, dataOffset);
      dataSize += obj.getAreaSize(_DATA);
      dataOffset += obj.getAreaSize(_DATA);
    }
    byte[] image = new byte[gsinitOffset];
    for (Objfile obj : required) {
      System.out.printf("Relocating: %s %s=%04X %s=%04X %s=%04X\n", obj.getFile(),
          _CODE, obj.getArea(_CODE).getOffset(),
          _GSINIT, obj.getArea(_GSINIT).getOffset(),
          _DATA, obj.getArea(_DATA).getOffset());
      obj.relocate(_CODE, image);
      obj.relocate(_GSINIT, image);
    }
    // the end of GSINIT
    image[image.length - 1] = (byte) 0xc9;
    
    System.out.println("_CODE:" + Integer.toHexString(codeSize));
    System.out.println("_GSINIT:" + Integer.toHexString(gsinitSize));
    System.out.println("_DATA:" + Integer.toHexString(dataSize));
    dump(image, dataOffset);
    out(new File("tmp.out"), image, dataOffset);
  }
  
  private void dump(byte[] image, int size) {
    int address = 0;
    System.out.printf("size:%04X", size);
    while (address < image.length) {
      if (address % 16 == 0) {
        System.out.printf("\n%04X:", address);
      }
      System.out.printf("%02x ", image[address++]);
    }
    System.out.println();
    System.out.println("Relocations:" + _relocations.size());
    for (RelocationInformation reloc : _relocations) {
      System.out.println(reloc);
    }
  }
  
  private void out(File file, byte[] image, int size) throws IOException {
    FileOutputStream stream = new FileOutputStream(file);
    stream.write(size & 0xff);
    stream.write(size >> 8);
    stream.write(image);
    stream.write(_relocations.size() & 0xff);
    stream.write(_relocations.size() >> 8);
    for (RelocationInformation reloc : _relocations) {
      reloc.write(stream);
    }
    stream.close();
  }

  private void make(List<Objfile> objects, Objfile obj) {
    if (objects.contains(obj)) {
      return;
    }
    objects.add(obj);
    Set<File> required = obj.getRequiredFiles();
    for (File require : required) {
      make(objects, _objfiles.get(require));
    }
  }

  private void loadLibfile(String filename) throws IOException {
    File file = new File(filename);
    File dir = file.getParentFile();
    BufferedReader isr = new BufferedReader(new FileReader(file));
    while (true) {
      String line = isr.readLine();
      if (line == null) {
        break;
      }
      File objfile = new File(dir, line);
      Objfile object = new Objfile(this, objfile);
      _objfiles.put(objfile, object);
      object.analyze();
    }
  }

  public void addSymbol(String name, Symbol symbol) {
    if (!_symbols.containsKey(name)) {
      _symbols.put(name, symbol);
    } else if (_symbols.get(name).getArea() != null) {
      System.out.println("Warning: duplicate symbol:" + name);
    }
  }

  public void addRelocation(RelocationInformation info) {
    _relocations.add(info);
  }

  public Symbol getSymbol(String name) {
    return _symbols.get(name);
  }

  private void loadSymfile(String filename) throws IOException {
    File file = new File(filename);
    BufferedReader isr = new BufferedReader(new FileReader(file));
    while (true) {
      String line = isr.readLine();
      if (line == null) {
        break;
      }
      if (line.startsWith(";")) {
        // comment
        continue;
      }
      Matcher m = SYMLINE.matcher(line);
      if (!m.find()) {
        continue;
      }
      Symbol symbol = new Symbol(file, Integer.parseInt(m.group(1), 16));
      _symbols.put(m.group(2), symbol);
    }
    isr.close();
  }
}

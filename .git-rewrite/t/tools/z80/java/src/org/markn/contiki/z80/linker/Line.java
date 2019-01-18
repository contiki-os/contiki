package org.markn.contiki.z80.linker;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Line {
  private static final Pattern WORD = Pattern.compile("([\\dA-F]{2})\\s([\\dA-F]{2})");
  private static final Pattern BYTE = Pattern.compile("([\\dA-F]{2})");

  private class Relocation {
    private int _mode;
    private int _offset;
    // area index(!S) or symbol index(S)
    private int _symbol;
  }

  private Area _area;
  private int _address;
  private List<Short> _bytes;
  private List<Relocation> _relocs;

  public Line(Objfile object, String tline, String rline) {
    _relocs = new ArrayList<Relocation>(16);
    _bytes = new ArrayList<Short>(16);
    rline = rline.substring(8);
    int areaindex = getWord(rline);
    _area = object.getArea(areaindex);
    if (_area == null) {
      throw new IllegalArgumentException("no such area:" + areaindex);
    }
    _area.addLine(this);
    tline = tline.substring(2);
    _address = getWord(tline);
    tline = tline.substring(3);
    while (true) {
      if (tline.length() < 3) {
        break;
      }
      tline = tline.substring(3);
      _bytes.add(getByte(tline));
    }
    // relocation line
    while (true) {
      if (rline.length() < 6) {
        break;
      }
      Relocation reloc = new Relocation();
      _relocs.add(reloc);
      rline = rline.substring(6);
      reloc._mode = getByte(rline);
      rline = rline.substring(3);
      reloc._offset = getByte(rline) - 2;
      rline = rline.substring(3);
      reloc._symbol = getWord(rline);
    }
  }
  private int getWord(String line) {
    Matcher m = WORD.matcher(line);
    if (!m.find()) {
      return -1;
    }
    String hexstr = m.group(2) + m.group(1);
    return Integer.parseInt(hexstr, 16);
  }

  private short getByte(String line) {
    Matcher m = BYTE.matcher(line);
    if (!m.find()) {
      return -1;
    }
    String hexstr = m.group(1);
    return Short.parseShort(hexstr, 16);
  }
  public void fill(Objfile object, byte[] image) {
    int address = _address + _area.getOffset();
    for (Relocation reloc : _relocs) {
      int target = 0;
      byte mode = 0;  // Ext/Int MSB/LSB Byte/Word 
      RelocationInformation info = new RelocationInformation();
      if ((reloc._mode & 2) > 0) {
        // external
        Symbol symbol = object.getSymbol(reloc._symbol);
        target = symbol.calcOffset();
        System.out.printf("%s %04X=>%04X\n", symbol, symbol.getOffset(), target);
        if (symbol.isAbsolute()) {
          mode |= 0x80;
        }
      } else {
        // internal
        Area area = object.getArea(reloc._symbol);
        int offset = area.getOffset();
        short source = (short) ((_bytes.get(reloc._offset + 1) << 8) + _bytes.get(reloc._offset)); 
        target = (short) (source + offset);
        // TODO: save relocation information
        System.out.printf("%s:%04X=>%04X\n", area, source,  target);
      }
      info.setAddress(address);
      if ((reloc._mode & 1) > 0) {
        // byte mode
        if ((reloc._mode & 128) > 0) {
          // MSB
          mode |= 0x60;
          _bytes.set(reloc._offset, (short) (target >> 8));
          _bytes.set(reloc._offset + 1, (short) -1);
        } else {
          // LSB
          mode |= 0x20;
          _bytes.set(reloc._offset, (short) -1);
          _bytes.set(reloc._offset, (short) (target & 0xff));
        }
        address++;
      } else {
        // word mode
        _bytes.set(reloc._offset, (short) (target & 0xff));
        _bytes.set(reloc._offset + 1, (short) (target >> 8));
        address += 2;
      }
      info.setMode(mode);
      info.setData(target);
      object.getLinker().addRelocation(info);
    }
    address = _address + _area.getOffset();
    for (int data : _bytes) {
      if (data >= 0) {
        image[address++] = (byte) data;
      }
    }
  }

}

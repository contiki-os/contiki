package org.markn.contiki.z80.linker;

import java.io.File;

public class Symbol {
  private File _file;
  private Area _area;
  private int _offset;
  public Symbol(File file, int offset) {
    this(file, null, offset);
  }
  public Symbol(File file, Area area, int offset) {
    this._file = file;
    this._area = area;
    this._offset = offset;
  }
  public File getFile() {
    return _file;
  }
  public Area getArea() {
    return _area;
  }
  public boolean isAbsolute() {
    return _area == null;
  }
  public int getOffset() {
    return _offset;
  }
  public int calcOffset() {
    if (isAbsolute()) {
      return _offset;
    } else {
      return _offset + _area.getOffset();
    }
  }
  
  public String toString() {
    StringBuffer buf = new StringBuffer(120);
    buf.append(_file.toString());
    buf.append(':');
    if (_area != null) {
      buf.append(_area);
    } else {
      buf.append("Absolute");
    }
    buf.append(':');
    buf.append(Integer.toHexString(_offset));
    return buf.toString();
    
  }
  
}

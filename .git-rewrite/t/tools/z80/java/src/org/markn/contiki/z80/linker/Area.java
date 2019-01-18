/**
 * 
 */
package org.markn.contiki.z80.linker;

import java.util.ArrayList;
import java.util.List;

class Area {
  private short _offset;
  private int _index;
  private String _name;
  private int _size;
  /**
   * Line data.
   */
  private List<Line> _lines;

  public Area(int index, String name, int size) {
    _index = index;
    _name = name;
    _size = size;
    _lines = new ArrayList<Line>();
  }
  public short getOffset() {
    return _offset;
  }
  public int getIndex() {
    return _index;
  }
  public String getName() {
    return _name;
  }
  public int getSize() {
    return _size;
  }
  public void setOffset(short offset) {
    _offset = offset;
  }
  public void addLine(Line line) {
    _lines.add(line);
  }
  public void relocate(Objfile object, byte[] image) {
    for (Line line: _lines) {
      line.fill(object, image);
    }
  }
  public String toString() {
    StringBuffer buf = new StringBuffer(120);
    buf.append(_name);
    return buf.toString();
  }
}
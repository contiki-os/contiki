package org.markn.contiki.z80.linker;

import java.io.IOException;
import java.io.OutputStream;

public class RelocationInformation {
  private byte _mode;
  private int _address;
  private int _data;
  public byte getMode() {
    return _mode;
  }
  public void setMode(byte mode) {
    this._mode = mode;
  }
  public int getAddress() {
    return _address;
  }
  public void setAddress(int address) {
    _address = address;
  }
  public int getData() {
    return _data;
  }
  public void setData(int data) {
    _data = data;
  }
  public void write(OutputStream stream) throws IOException {
    stream.write(_mode);
    stream.write(_address & 0xff);
    stream.write(_address >> 8);
    stream.write(_data & 0xff);
    stream.write(_data >> 8);
  }
  public String toString() {
    StringBuffer buf = new StringBuffer();
    buf.append((_mode & 0x80) > 0 ? 'E' : 'I');
    if ((_mode & 0x60) == 0x60) {
      buf.append("MB");
    } else if ((_mode & 0x20) > 0) {
      buf.append("LB");
    } else {
      buf.append("_W");
    }
//    buf.append((_mode & 0x60) > 0 ? 'M' : 'L');
//    buf.append((_mode & 0x20) > 0 ? 'B' : 'W');
    buf.append(String.format(":%04X:%04X", _address, _data));
    return buf.toString();
  }
  
}

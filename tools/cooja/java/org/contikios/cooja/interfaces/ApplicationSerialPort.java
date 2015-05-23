package org.contikios.cooja.interfaces;

import org.contikios.cooja.Mote;
import org.contikios.cooja.dialogs.SerialUI;
import org.contikios.cooja.motes.AbstractApplicationMote;

public class ApplicationSerialPort extends SerialUI {
  private Mote mote;

  public ApplicationSerialPort(Mote mote) {
    this.mote = mote;
  }

  /**
   * @param log Trigger log event from application
   */
  public void triggerLog(String log) {
    byte[] bytes = log.getBytes();
    for (byte b: bytes) {
      dataReceived(b);
    }
    dataReceived('\n');
  }

  public Mote getMote() {
    return mote;
  }

  public void writeArray(byte[] s) {
    ((AbstractApplicationMote) getMote()).writeArray(s);
  }
  public void writeByte(byte b) {
    ((AbstractApplicationMote)getMote()).writeByte(b);
  }
  public void writeString(String s) {
    ((AbstractApplicationMote)getMote()).writeString(s);
  }
}

package se.sics.cooja.interfaces;

import se.sics.cooja.Mote;
import se.sics.cooja.dialogs.SerialUI;
import se.sics.cooja.motes.AbstractApplicationMote;

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

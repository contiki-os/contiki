package se.sics.cooja.interfaces;

import se.sics.cooja.Mote;
import se.sics.cooja.dialogs.SerialUI;

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
    /* Not implemented */
  }
  public void writeByte(byte b) {
    /* Not implemented */
  }
  public void writeString(String s) {
    /* Not implemented */
  }
}

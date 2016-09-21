package se.sics.mspsim.platform.sky;
import se.sics.mspsim.chip.CC2420;
import se.sics.mspsim.chip.PacketListener;
import se.sics.mspsim.chip.RFListener;

public class RadioWrapper implements RFListener {

  private final CC2420 radio;
  private PacketListener packetListener;
  int len = 0;
  int pos = 0;
  byte[] buffer = new byte[128];
  
  public RadioWrapper(CC2420 radio) {
    this.radio = radio;
    radio.addRFListener(this);
  }
  
  public synchronized void addPacketListener(PacketListener listener) {
    packetListener = PacketListener.Proxy.INSTANCE.add(packetListener, listener);
  }

  public synchronized void removePacketListener(PacketListener listener) {
    packetListener = PacketListener.Proxy.INSTANCE.remove(packetListener, listener);
  }

  public void packetReceived(byte[] receivedData) {
    // four zero bytes, 7a and then length...
    radio.receivedByte((byte)0);
    radio.receivedByte((byte)0);
    radio.receivedByte((byte)0);
    radio.receivedByte((byte)0);
    radio.receivedByte((byte)0x7a);
   // radio.receivedByte((byte) receivedData.length);
    
    for (int i = 0; i < receivedData.length; i++) {
//      int data = receivedData[i];
//      System.out.println("*** RF (external) Data :" + data + " = $" + Utils.hex8(data) + " => " +
//          (char) data);
     
      radio.receivedByte(receivedData[i]);
    }
  }

  // NOTE: len is not in the packet for now...
  public void receivedByte(byte data) {
    PacketListener listener = this.packetListener;
//    System.out.println("*** RF Data :" + data + " = $" + Utils.hex8(data) + " => " +
//        (char) data);
    if (pos == 5) {
      len = data;
    }
    if (pos == 0) {
        if (listener != null) {
            listener.transmissionStarted();
        }
    }
    buffer[pos++] = data;
    // len + 1 = pos + 5 (preambles)
    if (len > 0 && len + 1 == pos - 5) {
//      System.out.println("***** SENDING DATA from CC2420 len = " + len);
      byte[] packet = new byte[len + 1];
      System.arraycopy(buffer, 5, packet, 0, len + 1);
      if (listener != null) {
          listener.transmissionEnded(packet);
      }
      pos = 0;
      len = 0;
    }
  }
}

package org.contikios.cooja.plugins.analyzers;

public abstract class PacketAnalyzer {

  public static final int ANALYSIS_FAILED = -1;
  public static final int ANALYSIS_OK_CONTINUE = 1;
  public static final int ANALYSIS_OK_FINAL = 2;

  public static final int RADIO_LEVEL = 0;
  public static final int MAC_LEVEL = 1;
  public static final int NETWORK_LEVEL = 2;
  public static final int APPLICATION_LEVEL = 3;

  public static class Packet {

    byte[] data;
    int pos;
    int level;
    /* size = length - consumed bytes at tail */
    int size;

    /* L2 addresseses */
    byte[] llsender;
    byte[] llreceiver;

    byte lastDispatch = 0;

    public Packet(byte[] data, int level) {
      this.level = level;
      this.data = data;
      this.size = data.length;
    }

    public void consumeBytesStart(int bytes) {
      pos += bytes;
    }

    public void consumeBytesEnd(int bytes) {
      size -= bytes;
    }

    public boolean hasMoreData() {
      return size > pos;
    }

    public int size() {
      return size - pos;
    }

    public byte get(int index) {
      if (index >= size) return 0;
      return data[pos + index];
    }

    public int getInt(int index, int size) {
      int value = 0;
      for (int i = 0; i < size; i++) {
        value = (value << 8) + (get(index + i) & 0xFF);
      }
      return value;
    }

    public byte[] getPayload() {
      byte[] pload = new byte[size - pos];
      System.arraycopy(data, pos, pload, 0, pload.length);
      return pload;
    }

    public void copy(int srcpos, byte[] arr, int pos, int len) {
      for (int i = 0; i < len; i++) {
        arr[pos + i] = get(srcpos + i);
      }
    }

    public byte[] getLLSender() {
      return llsender;
    }

    public byte[] getLLReceiver() {
      return llreceiver;
    }
  };

  public abstract boolean matchPacket(Packet packet);

  public abstract int analyzePacket(Packet packet, StringBuilder brief, StringBuilder verbose);
}

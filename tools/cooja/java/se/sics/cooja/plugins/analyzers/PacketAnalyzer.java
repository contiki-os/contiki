package se.sics.cooja.plugins.analyzers;

public abstract class PacketAnalyzer {

    public static final int RADIO_LEVEL = 0;
    public static final int MAC_LEVEL = 1;
    public static final int NETWORK_LEVEL = 2;
    
    public static class Packet {
        byte[] data;
        int pos;
        int level;

        /* L2 addresseses */
        byte[] llsender;
        byte[] llreceiver;

        public Packet(byte[] data, int level) {
            this.level = level;
            this.data = data;
        }

        public boolean hasMoreData() {
            return data.length > pos;
        }
        
        public byte get(int index) {
            return data[pos + index];
        }

        public int getInt(int index, int size) {
            int value = 0;
            for (int i = 0; i < size; i++) {
                value = (value << 8) + get(index + i);
            }
            return value;
        }

        
        public byte[] getPayload() {
            byte[] pload = new byte[data.length - pos];
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
    
    public abstract void analyzePacket(Packet packet, StringBuffer brief, StringBuffer verbose);
}
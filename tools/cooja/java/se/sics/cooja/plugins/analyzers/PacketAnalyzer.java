package se.sics.cooja.plugins.analyzers;

public abstract class PacketAnalyzer {

    public static final int RADIO_LEVEL = 0;
    public static final int MAC_LEVEL = 1;
    public static final int NETWORK_LEVEL = 2;
    
    public static class Packet {
        byte[] data;
        int pos;
        int level;

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
        
        public byte[] getPayload() {
            byte[] pload = new byte[data.length - pos];
            System.arraycopy(data, pos, pload, 0, pload.length);
            return pload;
        }
    };
    
    public abstract boolean matchPacket(Packet packet);
    
    public abstract void analyzePacket(Packet packet, StringBuffer brief, StringBuffer verbose);
}
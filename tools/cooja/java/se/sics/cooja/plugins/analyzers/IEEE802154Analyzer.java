package se.sics.cooja.plugins.analyzers;

import java.io.IOException;

import se.sics.cooja.util.StringUtils;

public class IEEE802154Analyzer extends PacketAnalyzer {

    /* TODO: fix this to be correct */
    public static final int NO_ADDRESS = 0;
    public static final int RSV_ADDRESS = 1;
    public static final int SHORT_ADDRESS = 2;
    public static final int LONG_ADDRESS = 3;

    public static final int BEACONFRAME = 0x00;
    public static final int DATAFRAME = 0x01;
    public static final int ACKFRAME = 0x02;
    public static final int CMDFRAME = 0x03;

//    private static final byte[] BROADCAST_ADDR = {(byte)0xff, (byte)0xff};

    private static final String[] typeS = {"-", "D", "A"};
    private static final String[] typeVerbose = {"BEACON", "DATA", "ACK"};
    private PcapExporter pcapExporter;

//    private int defaultAddressMode = LONG_ADDRESS;
//    private byte seqNo = 0;

//    private int myPanID = 0xabcd;

    public IEEE802154Analyzer(boolean pcap) {
        if (pcap) try {
            pcapExporter = new PcapExporter();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public boolean matchPacket(Packet packet) {
        return packet.level == MAC_LEVEL;
    }

    /* this protocol always have network level packets as payload */
    public int nextLevel(byte[] packet, int level) {
        return NETWORK_LEVEL;
    }
    /* create a 802.15.4 packet of the bytes and "dispatch" to the
     * next handler
     */
    public int analyzePacket(Packet packet, StringBuffer brief, StringBuffer verbose) {
        
        if (pcapExporter != null) {
            try {
                pcapExporter.exportPacketData(packet.getPayload());
            } catch (IOException e) {
                System.err.println("Could not export PCap data");
                e.printStackTrace();
            }
        }

        int pos = packet.pos;
        int type = packet.data[pos + 0] & 7;
//        int security = (packet.data[pos + 0] >> 3) & 1;
//        int pending = (packet.data[pos + 0] >> 4) & 1;
//        int ackRequired = (packet.data[pos + 0] >> 5) & 1;
        int panCompression  = (packet.data[pos + 0]>> 6) & 1;
        int destAddrMode = (packet.data[pos + 1] >> 2) & 3;
//        int frameVersion = (packet.data[pos + 1] >> 4) & 3;
        int srcAddrMode = (packet.data[pos + 1] >> 6) & 3;
        int seqNumber = packet.data[pos + 2] & 0xff;
        int destPanID = 0;
        int srcPanID = 0;
        byte[] sourceAddress = null;
        byte[] destAddress = null;

        pos += 3;

        if (destAddrMode > 0) {
            destPanID = (packet.data[pos] & 0xff) + ((packet.data[pos + 1] & 0xff) << 8);
            pos += 2;
            if (destAddrMode == SHORT_ADDRESS) {
                destAddress = new byte[2];
                destAddress[1] = packet.data[pos];
                destAddress[0] = packet.data[pos + 1];
                pos += 2;
            } else if (destAddrMode == LONG_ADDRESS) {
                destAddress = new byte[8];
                for (int i = 0; i < 8; i++) {
                    destAddress[i] = packet.data[pos + 7 - i];
                }
                pos += 8;
            }
        }

        if (srcAddrMode > 0) {
            if (panCompression == 0){
                srcPanID = (packet.data[pos] & 0xff) + ((packet.data[pos + 1] & 0xff) << 8);
                pos += 2;
            } else {
                srcPanID = destPanID;
            }
            if (srcAddrMode == SHORT_ADDRESS) {
                sourceAddress = new byte[2];
                sourceAddress[1] = packet.data[pos];
                sourceAddress[0] = packet.data[pos + 1];
                pos += 2;
            } else if (srcAddrMode == LONG_ADDRESS) {
                sourceAddress = new byte[8];
                for (int i = 0; i < 8; i++) {
                    sourceAddress[i] = packet.data[pos + 7 - i];
                }
                pos += 8;
            }
        }

//        int payloadLen = packet.data.length - pos;

        brief.append("15.4 ");
        brief.append(type < typeS.length ? typeS[type] : "?").append(' ');

        verbose.append("<html><b>IEEE 802.15.4 ")
        .append(type < typeVerbose.length ? typeVerbose[type] : "?")
        .append(' ').append(seqNumber);
        if (type != ACKFRAME) {
            printAddress(brief, srcAddrMode, sourceAddress);
            brief.append(' ');
            printAddress(brief, destAddrMode, destAddress);

            verbose.append("</b><br>From ");
            if (srcPanID != 0) {
                verbose.append(StringUtils.toHex((byte)(srcPanID >> 8)))
                .append(StringUtils.toHex((byte)(srcPanID & 0xff)))
                .append('/');
            }
            printAddress(verbose, srcAddrMode, sourceAddress);
            verbose.append(" to ");
            if (destPanID != 0) {
                verbose.append(StringUtils.toHex((byte)(destPanID >> 8)))
                .append(StringUtils.toHex((byte)(destPanID & 0xff)))
                .append('/');
            }
            printAddress(verbose, destAddrMode, destAddress);
        } else {
            /* got ack - no more to do ... */
            return ANALYSIS_OK_FINAL;
        }

        /* update packet */
        packet.pos = pos;
        packet.level = NETWORK_LEVEL;
        /* remove CRC from the packet */
        packet.consumeBytesEnd(2);
        
        packet.llsender = sourceAddress;
        packet.llreceiver = destAddress;
        return ANALYSIS_OK_CONTINUE;
    }

    private void printAddress(StringBuffer sb, int type, byte[] addr) {
        if (type == SHORT_ADDRESS) {
            sb.append(StringUtils.toHex(addr));
        } else if (type == LONG_ADDRESS) {
            sb.append(StringUtils.toHex(addr[0]) + StringUtils.toHex(addr[1]) + ":" + 
                    StringUtils.toHex(addr[2]) + StringUtils.toHex(addr[3]) + ":" +
                    StringUtils.toHex(addr[4]) + StringUtils.toHex(addr[5]) + ":" +
                    StringUtils.toHex(addr[6]) + StringUtils.toHex(addr[7]));
        }
    }
}

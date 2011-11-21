package se.sics.cooja.plugins.analyzers;

import se.sics.cooja.util.StringUtils;

public class IPv6PacketAnalyzer extends PacketAnalyzer {

    
    public final static int PROTO_UDP = 17;
    public final static int PROTO_TCP = 6;
    public final static int PROTO_ICMP = 58;
    
    
    public final static byte[] UNSPECIFIED_ADDRESS = 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    private static final int IPV6_DISPATCH = 0x41;

    public boolean matchPacket(Packet packet) {
        return packet.level == NETWORK_LEVEL && packet.get(0) == IPV6_DISPATCH;
    }

    public int analyzePacket(Packet packet, StringBuffer brief,
            StringBuffer verbose) {

        /* if packet has less than 40 bytes it is not interesting ... */
        if (packet.size() < 40) return ANALYSIS_FAILED;
        

        brief.append("IPv6");

        /* need to decompress while analyzing - add that later... */

        verbose.append("<b>IPv6</b><br>");

        int pos = 1;
        
        int version = 6;
        int trafficClass = 0;
        int flowLabel = 0;
        int len = packet.getInt(pos + 4, 2);
        int proto = packet.getInt(pos + 6, 1);
        int ttl = packet.getInt(pos + 7, 1);
        byte[] srcAddress = new byte[16];
        byte[] destAddress = new byte[16];
        
        packet.copy(pos + 8, srcAddress, 0, 16);
        packet.copy(pos + 24, destAddress, 0, 16);
        
        String protoStr = "" + proto;
        if (proto == PROTO_ICMP) {
            protoStr = "ICMPv6";
        } else if (proto == PROTO_UDP) protoStr = "UDP";
        else if (proto == PROTO_TCP) protoStr = "TCP";
        
        /* consume dispatch + IP header */
        packet.pos += 41;
        
        verbose.append("<br><b>IPv6 ").append(protoStr).append("</b> TC = " + trafficClass +
                " FL: " + flowLabel + "<br>");
        verbose.append("From ");
        printAddress(verbose, srcAddress);
        verbose.append("  to ");
        printAddress(verbose, destAddress);
        
        packet.lastDispatch = (byte) (proto & 0xff);
        packet.level = APPLICATION_LEVEL;
        return ANALYSIS_OK_CONTINUE;
    }

    public static void printAddress(StringBuffer out, byte[] address) {
        for (int i = 0; i < 16; i+=2) {
          out.append(StringUtils.toHex((byte) (address[i] & 0xff)) + 
                  StringUtils.toHex((byte) (address[i + 1] & 0xff)));
          if (i < 14) {
              out.append(":");
          }
        }
      }

    
}

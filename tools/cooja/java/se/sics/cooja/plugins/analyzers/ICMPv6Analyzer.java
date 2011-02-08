package se.sics.cooja.plugins.analyzers;

public class ICMPv6Analyzer extends PacketAnalyzer {

    public static final byte ICMPv6_DISPATCH = 58;
    
    public static final int ECHO_REQUEST = 128;
    public static final int ECHO_REPLY = 129;
    public static final int GROUP_QUERY = 130;
    public static final int GROUP_REPORT = 131;
    public static final int GROUP_REDUCTION = 132;
    public static final int ROUTER_SOLICITATION = 133;
    public static final int ROUTER_ADVERTISEMENT = 134;
    public static final int NEIGHBOR_SOLICITATION = 135;
    public static final int NEIGHBOR_ADVERTISEMENT = 136;

    public static final int RPL_CODE_DIS = 0; /* DIS message */
    public static final int RPL_CODE_DIO = 1; /* DIO message */
    public static final int RPL_CODE_DAO = 2;/* DAO message */
    public static final int RPL_CODE_DAO_ACK = 3;/* DAO ACK message */
    
    public static final int FLAG_ROUTER = 0x80;
    public static final int FLAG_SOLICITED = 0x40;
    public static final int FLAG_OVERRIDE = 0x20;

    public static final int ON_LINK = 0x80;
    public static final int AUTOCONFIG = 0x40;

    public static final int SOURCE_LINKADDR = 1;
    public static final int TARGET_LINKADDR = 2;
    public static final int PREFIX_INFO = 3;
    public static final int MTU_INFO = 5;
    
    public static final String[] TYPE_NAME = new String[] {
      "ECHO_REQUEST", "ECHO_REPLY",
      "GROUP_QUERY", "GROUP_REPORT", "GROUP_REDUCTION",
      "ROUTER_SOLICITATION", "ROUTER_ADVERTISEMENT",
      "NEIGHBOR_SOLICITATION", "NEIGHBOR_ADVERTISEMENT", "REDIRECT",
      "ROUTER RENUMBER", "NODE INFORMATION QUERY", "NODE INFORMATION RESPONSE"};

    
    public int analyzePacket(Packet packet, StringBuffer brief,
            StringBuffer verbose) {
        int type = packet.get(0) & 0xff;
        int code = packet.get(1) & 0xff;
//        int checksum = ((packet.get(2) & 0xff) << 8) | packet.get(3) & 0xff;

        brief.append("ICMPv6 ");
        if (type >= 128 && (type - 128) < TYPE_NAME.length) {
            brief.append(TYPE_NAME[type - 128]).append(' ').append(code);
            verbose.append("Type: ").append(TYPE_NAME[type - 128]);
            verbose.append(" Code:").append(code);
        } else if (type == 155) {
            /* RPL */
            brief.append("RPL ");
            verbose.append("Type: RPL Code: ");
            switch(code) {
            case RPL_CODE_DIS:
                brief.append("DIS");
                verbose.append("DIS");
                break;
            case RPL_CODE_DIO:
                brief.append("DIO");
                verbose.append("DIO<br>");
                
                int instanceID = packet.get(4) & 0xff;
                int version = packet.get(5) & 0xff;
                int rank = ((packet.get(6) & 0xff) << 8) + (packet.get(7) & 0xff);
                int mop = (packet.get(8) >> 3) & 0x07;
                int dtsn = packet.get(9);
                
                verbose.append(" InstanceID: " + instanceID + " Version: " + version +
                        " Rank:" + rank + " MOP: " + mop + " DTSN: " + dtsn);
                packet.consumeBytesStart(8);

                break;
            case RPL_CODE_DAO:
                brief.append("DAO");
                verbose.append("DAO");
                break;
            case RPL_CODE_DAO_ACK:
                brief.append("DAO ACK");
                verbose.append("DAO ACK");
                break;
            default:
                brief.append(code);
                verbose.append(code);
            }
        }

        /* remove type, code, crc */
        packet.consumeBytesStart(4);
        return ANALYSIS_OK_FINAL;
    }

    @Override
    public boolean matchPacket(Packet packet) {
        return packet.level == NETWORK_LEVEL && packet.lastDispatch == ICMPv6_DISPATCH;
    }
}

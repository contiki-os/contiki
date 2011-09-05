package se.sics.cooja.plugins.analyzers;

import se.sics.cooja.util.StringUtils;

public class IPHCPacketAnalyzer extends PacketAnalyzer {

    public final static int SICSLOWPAN_UDP_PORT_MIN                     = 0xF0B0;
    public final static int SICSLOWPAN_UDP_PORT_MAX                     = 0xF0BF;   /* F0B0 + 15 */

    public final static int SICSLOWPAN_DISPATCH_IPV6                    = 0x41; /* 01000001 = 65 */
    public final static int SICSLOWPAN_DISPATCH_HC1                     = 0x42; /* 01000010 = 66 */
    public final static int SICSLOWPAN_DISPATCH_IPHC                    = 0x60; /* 011xxxxx = ... */
    public final static int SICSLOWPAN_DISPATCH_FRAG1                   = 0xc0; /* 1100= 0xxx */
    public final static int SICSLOWPAN_DISPATCH_FRAGN                   = 0xe0; /* 1110= 0xxx */

    /*
     * Values of fields within the IPHC encoding first byte
     * (C stands for compressed and I for inline)
     */
    public final static int SICSLOWPAN_IPHC_FL_C                        = 0x10;
    public final static int SICSLOWPAN_IPHC_TC_C                        = 0x08;
    public final static int SICSLOWPAN_IPHC_NH_C                        = 0x04;
    public final static int SICSLOWPAN_IPHC_TTL_1                       = 0x01;
    public final static int SICSLOWPAN_IPHC_TTL_64                      = 0x02;
    public final static int SICSLOWPAN_IPHC_TTL_255                     = 0x03;
    public final static int SICSLOWPAN_IPHC_TTL_I                       = 0x00;


    /* Values of fields within the IPHC encoding second byte */
    public final static int SICSLOWPAN_IPHC_CID                         = 0x80;

    public final static int SICSLOWPAN_IPHC_SAC                         = 0x40;
    public final static int SICSLOWPAN_IPHC_SAM_00                      = 0x00;
    public final static int SICSLOWPAN_IPHC_SAM_01                      = 0x10;
    public final static int SICSLOWPAN_IPHC_SAM_10                      = 0x20;
    public final static int SICSLOWPAN_IPHC_SAM_11                      = 0x30;

    public final static int SICSLOWPAN_IPHC_M                           = 0x08;
    public final static int SICSLOWPAN_IPHC_DAC                         = 0x04;
    public final static int SICSLOWPAN_IPHC_DAM_00                      = 0x00;
    public final static int SICSLOWPAN_IPHC_DAM_01                      = 0x01;
    public final static int SICSLOWPAN_IPHC_DAM_10                      = 0x02;
    public final static int SICSLOWPAN_IPHC_DAM_11                      = 0x03;

    private static final int SICSLOWPAN_NDC_UDP_MASK                     = 0xf8;
    private static final int SICSLOWPAN_NHC_UDP_ID =                       0xf0;
    private static final int SICSLOWPAN_NHC_UDP_C  =                       0xf3;
    private static final int SICSLOWPAN_NHC_UDP_I =                        0xf0;
    
    public final static int PROTO_UDP = 17;
    public final static int PROTO_TCP = 6;
    public final static int PROTO_ICMP = 58;
    
    
    public final static byte[] UNSPECIFIED_ADDRESS = 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    private static byte[][] addrContexts = new byte[][] {
        {(byte)0xaa, (byte)0xaa, 0, 0,  0, 0, 0, 0}
    };

    private static final int IPHC_DISPATCH = 0x60;

    /* packet must be on network level && have a IPHC dispatch */
    public boolean matchPacket(Packet packet) {
        return packet.level == NETWORK_LEVEL && (packet.get(0) & 0xe0) == IPHC_DISPATCH;
    }

    public int analyzePacket(Packet packet, StringBuffer brief,
            StringBuffer verbose) {

        /* if packet has less than 3 bytes it is not interesting ... */
        if (packet.size() < 3) return ANALYSIS_FAILED;
        
        int tf = (packet.get(0) >> 3) & 0x03;
        boolean nhc = (packet.get(0) & SICSLOWPAN_IPHC_NH_C) > 0;
        int hlim = (packet.get(0) & 0x03);
        int cid = (packet.get(1) >> 7) & 0x01;
        int sac = (packet.get(1) >> 6) & 0x01;
        int sam = (packet.get(1) >> 4) & 0x03;
        int m = (packet.get(1) >> 3) & 0x01;
        int dac = (packet.get(1) >> 2) & 0x01;
        int dam = packet.get(1) & 0x03;
        int sci = 0;
        int dci = 0;

        String error = null;
        
        brief.append("IPHC");

        /* need to decompress while analyzing - add that later... */

        verbose.append("<b>IPHC HC-06</b><br>");
        verbose.append("tf = " + tf + " nhc = " + nhc + " hlim = " + hlim
                + " cid = " + cid + " sac = " + sac + " sam = " + sam
                + " MCast = " + m + " dac = " + dac + " dam = " + dam);
        if (cid == 1) {
            verbose.append("<br>Contexts: sci=" + (packet.get(2) >> 4) + " dci="
                    + (packet.get(2) & 0x0f));
            sci = packet.get(2) >> 4;
            dci = packet.get(2) & 0x0f;
        }

        int hc06_ptr = 2 + cid;

        int version = 6;
        int trafficClass = 0;
        int flowLabel = 0;
        int len = 0;
        int proto = 0;
        int ttl = 0;
        byte[] srcAddress = new byte[16];
        byte[] destAddress = new byte[16];
        
        int srcPort = 0;
        int destPort = 0;

        
        try {
        /* Traffic class and flow label */
        if((packet.get(0) & SICSLOWPAN_IPHC_FL_C) == 0) {
            /* Flow label are carried inline */
            if((packet.get(0) & SICSLOWPAN_IPHC_TC_C) == 0) {
                /* Traffic class is carried inline */
                flowLabel = packet.getInt(hc06_ptr + 1, 3);
                int tmp = packet.get(hc06_ptr);
                hc06_ptr += 4;
                /* hc06 format of tc is ECN | DSCP , original is DSCP | ECN */
                trafficClass = ((tmp >> 2) & 0x3f) | (tmp << 6) & (0x80 + 0x40);
                /* ECN rolled down two steps + lowest DSCP bits at top two bits */
            } else {
                /* highest flow label bits + ECN bits */
                int tmp = packet.get(hc06_ptr);
                trafficClass = (tmp >> 6) & 0x0f;
                flowLabel = packet.getInt(hc06_ptr + 1, 2);
                hc06_ptr += 3;
            }
        } else {
            /* Version is always 6! */
            /* Version and flow label are compressed */
            if((packet.get(0) & SICSLOWPAN_IPHC_TC_C) == 0) {
                /* Traffic class is inline */
                trafficClass =((packet.get(hc06_ptr) >> 6) & 0x03);
                trafficClass |= (packet.get(hc06_ptr) << 2);
                hc06_ptr += 1;
            }
        }

        /* Next Header */
        if((packet.get(0) & SICSLOWPAN_IPHC_NH_C) == 0) {
            /* Next header is carried inline */
            proto = packet.get(hc06_ptr);
            hc06_ptr += 1;
        }

        /* Hop limit */
        switch(packet.get(0) & 0x03) {
        case SICSLOWPAN_IPHC_TTL_1:
            ttl = 1;
            break;
        case SICSLOWPAN_IPHC_TTL_64:
            ttl = 2;
            break;
        case SICSLOWPAN_IPHC_TTL_255:
            ttl = 255;
            break;
        case SICSLOWPAN_IPHC_TTL_I:
            ttl = packet.get(hc06_ptr);
            hc06_ptr += 1;
            break;
        }

        /* context based compression */
        if((packet.get(1) & SICSLOWPAN_IPHC_SAC) > 0) {
            /* Source address */
            byte[] context = null;
            if((packet.get(1) & SICSLOWPAN_IPHC_SAM_11) != SICSLOWPAN_IPHC_SAM_00) {
                context = addrContexts[sci];
            }

            switch(packet.get(1) & SICSLOWPAN_IPHC_SAM_11) {
            case SICSLOWPAN_IPHC_SAM_00:
                /* copy the unspecificed address */
                srcAddress = UNSPECIFIED_ADDRESS;
                break;
            case SICSLOWPAN_IPHC_SAM_01: /* 64 bits */
                /* copy prefix from context */
                System.arraycopy(context, 0, srcAddress, 0, 8);
                /* copy IID from packet */
                packet.copy(hc06_ptr, srcAddress, 8, 8);
                hc06_ptr += 8;
                break;
            case SICSLOWPAN_IPHC_SAM_10: /* 16 bits */
                /* unicast address */
                System.arraycopy(context, 0, srcAddress, 0, 8);
                /* copy 6 NULL bytes then 2 last bytes of IID */
                packet.copy(hc06_ptr, srcAddress, 14, 2);
                hc06_ptr += 2;
                break;
            case SICSLOWPAN_IPHC_SAM_11: /* 0-bits */
                /* copy prefix from context */
                System.arraycopy(context, 0, srcAddress, 0, 8);
                /* infer IID from L2 address */
                System.arraycopy(packet.llsender, 0, srcAddress, 
                        16 - packet.llsender.length, packet.llsender.length);
                break;
            }
            /* end context based compression */
        } else {
            /* no compression and link local */
            switch(packet.get(1) & SICSLOWPAN_IPHC_SAM_11) {
            case SICSLOWPAN_IPHC_SAM_00: /* 128 bits */
                /* copy whole address from packet */
                packet.copy(hc06_ptr, srcAddress, 0, 16);
                hc06_ptr += 16;
                break;
            case SICSLOWPAN_IPHC_SAM_01: /* 64 bits */
                srcAddress[0] = (byte) 0xfe;
                srcAddress[1] = (byte) 0x80;
                /* copy IID from packet */
                packet.copy(hc06_ptr, srcAddress, 8, 8);
                hc06_ptr += 8;
                break;
            case SICSLOWPAN_IPHC_SAM_10: /* 16 bits */
                srcAddress[0] = (byte) 0xfe;
                srcAddress[1] = (byte) 0x80;
                packet.copy(hc06_ptr, srcAddress, 14, 2);
                hc06_ptr += 2;
                break;
            case SICSLOWPAN_IPHC_SAM_11: /* 0 bits */
                /* setup link-local address */
                srcAddress[0] = (byte) 0xfe;
                srcAddress[1] = (byte) 0x80;
                /* infer IID from L2 address */
                System.arraycopy(packet.llsender, 0, srcAddress, 
                        16 - packet.llsender.length, packet.llsender.length);
                break;
            }
        }

        /* Destination address */

        /* multicast compression */
        if((packet.get(1) & SICSLOWPAN_IPHC_M) != 0) {
            /* context based multicast compression */
            if((packet.get(1) & SICSLOWPAN_IPHC_DAC) != 0) {
                /* TODO: implement this */
            } else {
                /* non-context based multicast compression */
                switch (packet.get(1) & SICSLOWPAN_IPHC_DAM_11) {
                case SICSLOWPAN_IPHC_DAM_00: /* 128 bits */
                    /* copy whole address from packet */
                    packet.copy(hc06_ptr, destAddress, 0, 16);
                    hc06_ptr += 16;
                    break;
                case SICSLOWPAN_IPHC_DAM_01: /* 48 bits FFXX::00XX:XXXX:XXXX */
                    destAddress[0] = (byte) 0xff;
                    destAddress[1] = packet.get(hc06_ptr);
                    packet.copy(hc06_ptr + 1, destAddress, 11, 5);
                    hc06_ptr += 6;
                    break;
                case SICSLOWPAN_IPHC_DAM_10: /* 32 bits FFXX::00XX:XXXX */
                    destAddress[0] = (byte) 0xff;
                    destAddress[1] = packet.get(hc06_ptr);
                    packet.copy(hc06_ptr + 1, destAddress, 13, 3);
                    hc06_ptr += 4;
                    break;
                case SICSLOWPAN_IPHC_DAM_11: /* 8 bits FF02::00XX */
                    destAddress[0] = (byte) 0xff;
                    destAddress[1] = (byte) 0x02;
                    destAddress[15] = packet.get(hc06_ptr);
                    hc06_ptr++;
                    break;
                }
            }
        } else {
            /* no multicast */
            /* Context based */
            if((packet.get(1) & SICSLOWPAN_IPHC_DAC) != 0) {
                byte[] context = addrContexts[dci];

                switch (packet.get(1) & SICSLOWPAN_IPHC_DAM_11) {
                case SICSLOWPAN_IPHC_DAM_01: /* 64 bits */
                    System.arraycopy(context, 0, destAddress, 0, 8);
                    /* copy IID from packet */
                    packet.copy(hc06_ptr, destAddress, 8, 8);
                    hc06_ptr += 8;
                    break;
                case SICSLOWPAN_IPHC_DAM_10: /* 16 bits */
                    /* unicast address */
                    System.arraycopy(context, 0, destAddress, 0, 8);
                    /* copy IID from packet */
                    packet.copy(hc06_ptr, destAddress, 14, 2);
                    hc06_ptr += 2;
                    break;
                case SICSLOWPAN_IPHC_DAM_11: /* 0 bits */
                    /* unicast address */
                    System.arraycopy(context, 0, destAddress, 0, 8);
                    /* infer IID from L2 address */
                    System.arraycopy(packet.llreceiver, 0, destAddress, 
                            16 - packet.llreceiver.length, packet.llreceiver.length);
                    break;
                }      
            } else {
                /* not context based => link local M = 0, DAC = 0 - same as SAC */
                switch (packet.get(1) & SICSLOWPAN_IPHC_DAM_11) {
                case SICSLOWPAN_IPHC_DAM_00: /* 128 bits */
                    packet.copy(hc06_ptr, destAddress, 0, 16);
                    hc06_ptr += 16;
                    break;
                case SICSLOWPAN_IPHC_DAM_01: /* 64 bits */
                    destAddress[0] = (byte) 0xfe;
                    destAddress[1] = (byte) 0x80;
                    packet.copy(hc06_ptr, destAddress, 8, 8);
                    hc06_ptr += 8;
                    break;
                case SICSLOWPAN_IPHC_DAM_10: /* 16 bits */
                    destAddress[0] = (byte) 0xfe;
                    destAddress[1] = (byte) 0x80;
                    packet.copy(hc06_ptr, destAddress, 14, 2);
                    hc06_ptr += 2;
                    break;
                case SICSLOWPAN_IPHC_DAM_11: /* 0 bits */
                    destAddress[0] = (byte) 0xfe;
                    destAddress[1] = (byte) 0x80;
                    System.arraycopy(packet.llreceiver, 0, destAddress, 
                            16 - packet.llreceiver.length, packet.llreceiver.length);
                    break;
                }
            }
        }

        /* Next header processing - continued */
        if(nhc) {
            /* TODO: check if this is correct in hc-06 */
            /* The next header is compressed, NHC is following */
            if((packet.get(hc06_ptr) & SICSLOWPAN_NDC_UDP_MASK) == SICSLOWPAN_NHC_UDP_ID) {
                proto = PROTO_UDP;
                switch(packet.get(hc06_ptr)) {
                case (byte) SICSLOWPAN_NHC_UDP_C:
                    /* 1 byte for NHC, 1 byte for ports, 2 bytes chksum */
                    srcPort = SICSLOWPAN_UDP_PORT_MIN + (packet.get(hc06_ptr + 1) >> 4);
                    destPort = SICSLOWPAN_UDP_PORT_MIN + (packet.get(hc06_ptr + 1) & 0x0F);
//                    memcpy(&SICSLOWPAN_UDP_BUF->udpchksum, hc06_ptr + 2, 2);
//                    PRINTF("IPHC: Uncompressed UDP ports (4): %x, %x\n",
//                            SICSLOWPAN_UDP_BUF->srcport, SICSLOWPAN_UDP_BUF->destport);
                    hc06_ptr += 4;
                    break;
                case (byte) SICSLOWPAN_NHC_UDP_I:
                    /* 1 byte for NHC, 4 byte for ports, 2 bytes chksum */
                    srcPort = packet.getInt(hc06_ptr + 1, 2);
                    destPort = packet.getInt(hc06_ptr + 3, 2);
//                    memcpy(&SICSLOWPAN_UDP_BUF->udpchksum, hc06_ptr + 5, 2);
//                    PRINTF("IPHC: Uncompressed UDP ports (7): %x, %x\n",
//                            SICSLOWPAN_UDP_BUF->srcport, SICSLOWPAN_UDP_BUF->destport);

                    hc06_ptr += 7;
                    break;
                default:
//                    PRINTF("sicslowpan uncompress_hdr: error unsupported UDP compression\n");
                    return ANALYSIS_FAILED;
                }
            }
        }


//        /* IP length field. */
//        if(ip_len == 0) {
//            /* This is not a fragmented packet */
//            SICSLOWPAN_IP_BUF->len[0] = 0;
//            SICSLOWPAN_IP_BUF->len[1] = packetbuf_datalen() - rime_hdr_len + uncomp_hdr_len - UIP_IPH_LEN;
//        } else {
//            /* This is a 1st fragment */
//            SICSLOWPAN_IP_BUF->len[0] = (ip_len - UIP_IPH_LEN) >> 8;
//            SICSLOWPAN_IP_BUF->len[1] = (ip_len - UIP_IPH_LEN) & 0x00FF;
//        }

//        /* length field in UDP header */
//        if(SICSLOWPAN_IP_BUF->proto == UIP_PROTO_UDP) {
//            memcpy(&SICSLOWPAN_UDP_BUF->udplen, ipBuf + len[0], 2);
//        }

        /*--------------------------------------------- */        

        } catch (Exception e) {
            // some kind of unexpected error...
            error = " error during IPHC parsing: " + e.getMessage();
        }
        packet.pos += hc06_ptr;
        
        String protoStr = "" + proto;
        if (proto == PROTO_ICMP) {
            protoStr = "ICMPv6";
        } else if (proto == PROTO_UDP) protoStr = "UDP";
        else if (proto == PROTO_TCP) protoStr = "TCP";
        
        verbose.append("<br><b>IPv6 ").append(protoStr).append("</b> TC = " + trafficClass +
                " FL: " + flowLabel + "<br>");
        verbose.append("From ");
        printAddress(verbose, srcAddress);
        verbose.append("  to ");
        printAddress(verbose, destAddress);
        if (error != null) verbose.append(" " + error);
        
        packet.lastDispatch = (byte) (proto & 0xff);
        if (proto == PROTO_UDP || proto == PROTO_ICMP ||
            proto == PROTO_TCP) {
            packet.level = APPLICATION_LEVEL;
            return ANALYSIS_OK_CONTINUE;
        } else {
            packet.level = NETWORK_LEVEL;
            return ANALYSIS_OK_CONTINUE;
        }
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

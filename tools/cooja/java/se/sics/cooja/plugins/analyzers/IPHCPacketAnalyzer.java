package se.sics.cooja.plugins.analyzers;

import se.sics.cooja.util.StringUtils;

public class IPHCPacketAnalyzer extends PacketAnalyzer {

    private static final int IPHC_DISPATCH = 0x60;
    
    public boolean matchPacket(Packet packet) {
        return (packet.get(0) & 0xe0) == IPHC_DISPATCH;
    }

    public void analyzePacket(Packet packet, StringBuffer brief,
            StringBuffer verbose) {

        int tf = (packet.get(0) >> 3) & 0x03;
        int nh = (packet.get(0) >> 2) & 0x01;
        int hlim = (packet.get(0) & 0x03);
        int cid = (packet.get(1) >> 7) & 0x01;
        int sac = (packet.get(1) >> 6) & 0x01;
        int sam = (packet.get(1) >> 4) & 0x03;
        int m = (packet.get(1) >> 3) & 0x01;
        int dac = (packet.get(1) >> 2) & 0x01;
        int dam = packet.get(1) & 0x03;

        brief.append("iphc tf=" + tf + (nh == 1 ? " nh" : "") + " hl=" + hlim + (cid == 1 ? " cid " : ""));
        brief.append((sac == 1 ? " sac" : "") + " sam=" + sam + (m == 1 ? " M" : "") +
                (dac == 1 ? " dac" : " -") + " dam=" + dam);

        if (cid == 1) {
            brief.append(" sci=" + (packet.get(2) >> 4) + " dci=" + (packet.get(2) & 0x0f));
        }
        /* need to decompress while analyzing - add that later... */
        
        verbose.append("<br><b>IPHC HC-06</b><br>");
        verbose.append("tf = " + tf + " nhc = " + nh + " hlim = " + hlim + " cid = " + cid);
        verbose.append("sac = " + sac + " sam = " + sam + " MCast = " + m + " dac = " + dac +
        " dam = " + dam + "<br>");
        if (cid == 1) {
            verbose.append("Contexts: sci=" + (packet.get(2) >> 4) + " dci=" + (packet.get(2) & 0x0f));
        }

        packet.pos += cid == 1 ? 3 : 2;
        brief.append("|").append(StringUtils.toHex(packet.getPayload(), 4));
        verbose.append("Payload: ").append(StringUtils.toHex(packet.getPayload(), 4));
        
        packet.pos = packet.data.length;
        packet.level = NETWORK_LEVEL;
    }

}

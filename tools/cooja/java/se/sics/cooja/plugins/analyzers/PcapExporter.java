package se.sics.cooja.plugins.analyzers;

import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;

public class PcapExporter {

    private static final byte[] ETH_DATA = {(byte)0xaf, (byte)0xab, (byte)0xac, (byte)0xad,
        (byte)0xae, (byte)0xaf, 0x42, (byte)0xfb, (byte)0x9f, (byte)0x81, 0x5a,
        (byte)0x81, (byte)0x80, (byte)0x9a};

    DataOutputStream out;
    
    public PcapExporter() throws IOException {
    }
    
    public void openPcap() throws IOException {
        out = new DataOutputStream(new FileOutputStream("radiolog-" + System.currentTimeMillis() + ".pcap"));
        /* pcap header */
        out.writeInt(0xa1b2c3d4);
        out.writeShort(0x0002);
        out.writeShort(0x0004);
        out.writeInt(0);
        out.writeInt(0);
        out.writeInt(4096);
        out.writeInt(1); /* 1 for ethernet ? */
        out.flush();
        System.out.println("Opened pcap file!");
    }
    public void closePcap() throws IOException {
        out.close();
    }

    public void exportPacketData(byte[] data) throws IOException {
        if (out == null) {
            openPcap();
        }
        try {
            /* pcap packet header */
            out.writeInt((int) System.currentTimeMillis() / 1000);
            out.writeInt((int) ((System.currentTimeMillis() % 1000) * 1000));
            out.writeInt(data.length + 14);
            out.writeInt(data.length + 14);
            out.write(ETH_DATA);
            /* and the data */
            out.write(data);
            out.flush();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    
    
}

package org.contikios.cooja.plugins.analyzers;

import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.File;

public class PcapExporter {

  DataOutputStream out;

  public PcapExporter() throws IOException {
  }

  public void openPcap(File pcapFile) throws IOException {
    if (out != null) {
      closePcap();
    }
    if (pcapFile == null) {
      /* pcap file not specified, use default file name */
      pcapFile = new File("radiolog-" + System.currentTimeMillis() + ".pcap");
    }
    out = new DataOutputStream(new FileOutputStream(pcapFile));
    /* pcap header */
    out.writeInt(0xa1b2c3d4);
    out.writeShort(0x0002);
    out.writeShort(0x0004);
    out.writeInt(0);
    out.writeInt(0);
    out.writeInt(4096);
    out.writeInt(195); /* 195 for LINKTYPE_IEEE802_15_4 */

    out.flush();
    System.out.println("Opened pcap file " + pcapFile);
  }

  public void closePcap() throws IOException {
    out.close();
    out = null;
  }

  public void exportPacketData(byte[] data) throws IOException {
    if (out == null) {
      /* pcap file never set, open default */
      openPcap(null);
    }
    try {
      /* pcap packet header */
      out.writeInt((int) (System.currentTimeMillis() / 1000));
      out.writeInt((int) ((System.currentTimeMillis() % 1000) * 1000));
      out.writeInt(data.length);
      out.writeInt(data.length);
      /* and the data */
      out.write(data);
      out.flush();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

}

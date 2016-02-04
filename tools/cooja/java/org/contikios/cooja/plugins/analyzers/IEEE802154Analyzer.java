package org.contikios.cooja.plugins.analyzers;

import java.io.IOException;
import java.io.File;
import org.apache.log4j.Logger;

import org.contikios.cooja.util.StringUtils;

public class IEEE802154Analyzer extends PacketAnalyzer {

  private static final Logger logger = Logger.getLogger(IEEE802154Analyzer.class);

  // Addressing modes
  public static final int NO_ADDRESS = 0;
  public static final int RSV_ADDRESS = 1;
  public static final int SHORT_ADDRESS = 2;
  public static final int LONG_ADDRESS = 3;

  // Frame types
  public static final int BEACONFRAME = 0x00;
  public static final int DATAFRAME = 0x01;
  public static final int ACKFRAME = 0x02;
  public static final int CMDFRAME = 0x03;

//    private static final byte[] BROADCAST_ADDR = {(byte)0xff, (byte)0xff};
  private static final String[] typeS = {"-", "D", "A", "C"};
  private static final String[] typeVerbose = {"BEACON", "DATA", "ACK", "CMD"};
  private static final String[] addrModeNames = {"None", "Reserved", "Short", "Long"};
  private PcapExporter pcapExporter;

//    private int defaultAddressMode = LONG_ADDRESS;
//    private byte seqNo = 0;
//    private int myPanID = 0xabcd;
  public IEEE802154Analyzer(boolean pcap) {
    if (pcap) {
      try {
        pcapExporter = new PcapExporter();
      } catch (IOException e) {
        logger.error(e);
      }
    }
  }

  public void setPcapFile(File pcapFile) {
    if (pcapExporter != null) {
      try {
        pcapExporter.openPcap(pcapFile);
      } catch (IOException e) {
        logger.error("Could not open pcap file", e);
      }
    }
  }

  @Override
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

  @Override
  public int analyzePacket(Packet packet, StringBuilder brief, StringBuilder verbose) {

    if (pcapExporter != null) {
      try {
        pcapExporter.exportPacketData(packet.getPayload(), packet.getTimestamp());
      } catch (IOException e) {
        logger.error("Could not export PCap data", e);
      }
    }

    int pos = packet.pos;
    // FCF field
    int fcfType = packet.data[pos + 0] & 0x07;
    boolean fcfSecurity = ((packet.data[pos + 0] >> 3) & 0x01) != 0;
    boolean fcfPending = ((packet.data[pos + 0] >> 4) & 0x01) != 0;
    boolean fcfAckRequested = ((packet.data[pos + 0] >> 5) & 0x01) != 0;
    boolean fcfIntraPAN = ((packet.data[pos + 0] >> 6) & 0x01) != 0;
    int fcfDestAddrMode = (packet.data[pos + 1] >> 2) & 0x03;
    int fcfFrameVersion = (packet.data[pos + 1] >> 4) & 0x03;
    int fcfSrcAddrMode = (packet.data[pos + 1] >> 6) & 0x03;
    // Sequence number
    int seqNumber = packet.data[pos + 2] & 0xff;
    // Addressing Fields
    int destPanID = 0;
    int srcPanID = 0;
    byte[] sourceAddress = null;
    byte[] destAddress = null;

    pos += 3;

    if (fcfDestAddrMode > 0) {
      destPanID = (packet.data[pos] & 0xff) + ((packet.data[pos + 1] & 0xff) << 8);
      pos += 2;
      if (fcfDestAddrMode == SHORT_ADDRESS) {
        destAddress = new byte[2];
        destAddress[1] = packet.data[pos];
        destAddress[0] = packet.data[pos + 1];
        pos += 2;
      } else if (fcfDestAddrMode == LONG_ADDRESS) {
        destAddress = new byte[8];
        for (int i = 0; i < 8; i++) {
          destAddress[i] = packet.data[pos + 7 - i];
        }
        pos += 8;
      }
    }

    if (fcfSrcAddrMode > 0) {
      if (fcfIntraPAN) {
        srcPanID = destPanID;
      } else {
        srcPanID = (packet.data[pos] & 0xff) + ((packet.data[pos + 1] & 0xff) << 8);
        pos += 2;
      }
      if (fcfSrcAddrMode == SHORT_ADDRESS) {
        sourceAddress = new byte[2];
        sourceAddress[1] = packet.data[pos];
        sourceAddress[0] = packet.data[pos + 1];
        pos += 2;
      } else if (fcfSrcAddrMode == LONG_ADDRESS) {
        sourceAddress = new byte[8];
        for (int i = 0; i < 8; i++) {
          sourceAddress[i] = packet.data[pos + 7 - i];
        }
        pos += 8;
      }
    }

//        int payloadLen = packet.data.length - pos;
    brief.append("15.4 ");
    brief.append(fcfType < typeS.length ? typeS[fcfType] : "?").append(' ');

    verbose.append("<html><b>IEEE 802.15.4 ")
            .append(fcfType < typeVerbose.length ? typeVerbose[fcfType] : "?")
            .append("</b> #").append(seqNumber);

    if (fcfType != ACKFRAME) {
      printAddress(brief, fcfSrcAddrMode, sourceAddress);
      brief.append(' ');
      printAddress(brief, fcfDestAddrMode, destAddress);

      verbose.append("<br>From ");
      if (srcPanID != 0) {
        verbose.append("0x")
                .append(StringUtils.toHex((byte) (srcPanID >> 8)))
                .append(StringUtils.toHex((byte) (srcPanID & 0xff)))
                .append('/');
      }
      printAddress(verbose, fcfSrcAddrMode, sourceAddress);
      verbose.append(" to ");
      if (destPanID != 0) {
        verbose.append("0x")
                .append(StringUtils.toHex((byte) (destPanID >> 8)))
                .append(StringUtils.toHex((byte) (destPanID & 0xff)))
                .append('/');
      }
      printAddress(verbose, fcfDestAddrMode, destAddress);
    }

    verbose.append("<br/>Sec = ").append(fcfSecurity)
            .append(", Pend = ").append(fcfPending)
            .append(", ACK = ").append(fcfAckRequested)
            .append(", iPAN = ").append(fcfIntraPAN)
            .append(", DestAddr = ").append(addrModeNames[fcfDestAddrMode])
            .append(", Vers. = ").append(fcfFrameVersion)
            .append(", SrcAddr = ").append(addrModeNames[fcfSrcAddrMode]);

    /* update packet */
    packet.pos = pos;
    /* remove CRC from the packet */
    packet.consumeBytesEnd(2);

    if (fcfType == ACKFRAME) {
      /* got ack - no more to do ... */
      return ANALYSIS_OK_FINAL;
    }

    packet.level = NETWORK_LEVEL;
    packet.llsender = sourceAddress;
    packet.llreceiver = destAddress;
    return ANALYSIS_OK_CONTINUE;
  }

  private void printAddress(StringBuilder sb, int type, byte[] addr) {
    if (type == SHORT_ADDRESS) {
      sb.append("0x").append(StringUtils.toHex(addr));
    } else if (type == LONG_ADDRESS) {
      sb.append(StringUtils.toHex(addr[0]))
              .append(':')
              .append(StringUtils.toHex(addr[1]))
              .append(':')
              .append(StringUtils.toHex(addr[2]))
              .append(':')
              .append(StringUtils.toHex(addr[3]))
              .append(':')
              .append(StringUtils.toHex(addr[4]))
              .append(':')
              .append(StringUtils.toHex(addr[5]))
              .append(':')
              .append(StringUtils.toHex(addr[6]))
              .append(':')
              .append(StringUtils.toHex(addr[7]));

    }
  }
}

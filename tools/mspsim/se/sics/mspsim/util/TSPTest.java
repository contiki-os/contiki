package se.sics.mspsim.util;

import se.sics.jipv6.core.AbstractPacketHandler;
import se.sics.jipv6.core.ICMP6Packet;
import se.sics.jipv6.core.IPStack;
import se.sics.jipv6.core.IPv6Packet;
import se.sics.jipv6.core.Packet;
import se.sics.jipv6.tunnel.TSPClient;

public class TSPTest extends AbstractPacketHandler implements Runnable {

  public byte[] GOOGLE = new byte[] {
      0x20, 0x01,  0x48, 0x60,  (byte) 0xb0, 0x02,  0x00, 0x00,
      0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x68
  };
  static IPStack ipStack;
  
  public static void main(String[] args) {
    ipStack = new IPStack();
    TSPTest testLink = new TSPTest();
    ipStack.setLinkLayerHandler(testLink);
    TSPClient tunnel = TSPClient.startTSPTunnel(ipStack, "anon.freenet6.net", null, null);
    ipStack.setTunnel(tunnel);
    
    new Thread(testLink).start();
   
  }

  public void run() {
    IPv6Packet ping = new IPv6Packet();
    ping.setDestinationAddress(GOOGLE);
    ping.setSourceAddress(ipStack.getIPAddress());
    ICMP6Packet icmpER = new ICMP6Packet();
    icmpER.setType(ICMP6Packet.ECHO_REQUEST);
    ping.setIPPayload(icmpER);
    while(true) {
      try {
        Thread.sleep(10000);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
      System.out.println("Sending ping to google...");
      ipStack.sendPacket(ping, null);
    }
  }
  
  public void packetReceived(Packet container) {
    System.out.println("Packet received from IP Stack...: " + container);
  }

  public void sendPacket(Packet packet) {  }
}

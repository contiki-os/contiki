package se.sics.cooja.interfaces;

/**
 * A packet radio is able to transmit and receive radio data on a packet level.
 * 
 * The packet radio is the highest abstraction level of radios, and must
 * therefore be implemented by all lower abstraction levels.
 * 
 * @author Fredrik Osterlind
 */
public interface PacketRadio {

  /**
   * Sets the packet data that is being received during a connection. Different
   * radio may handle the data differently, but as a general rule this data
   * should be supplied as soon as possible.
   * 
   * @param p
   *          Packet dat
   */
  public void setReceivedPacket(byte[] p);

  /**
   * @return Last packet transmitted by radio
   */
  public byte[] getLastPacketTransmitted();

  /**
   * @return Last packet received by radio
   */
  public byte[] getLastPacketReceived();

}

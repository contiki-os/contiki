package se.sics.cooja.interfaces;

/**
 * A byte radio is able to transmit and receive radio data on a byte level.
 * 
 * The byte radio is a lower abstraction level than the packet radio and should,
 * according to the bottom-up abstraction policy, implement the packet
 * abstraction level.
 * 
 * @author Fredrik Osterlind
 */
public interface ByteRadio extends PacketRadio {

  /**
   * Radio receives given byte.
   * 
   * @param b
   *          Byte
   * @param timestamp
   *          Timestamp information
   */
  public void receiveByte(byte b, long timestamp);

  /**
   * @return Last byte transmitted by radio
   */
  public byte getLastByteTransmitted();

  /**
   * Returns timestamp information of byte transmitted. This may for example be
   * the number of cycles since transmission started.
   * 
   * @return Timestamp info
   */
  public long getLastByteTransmittedTimestamp();

  /**
   * @return Last byte received by radio
   */
  public byte getLastByteReceived();
}

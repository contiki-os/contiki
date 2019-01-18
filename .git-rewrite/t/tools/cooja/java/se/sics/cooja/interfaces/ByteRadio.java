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
   * @param delay
   *          Delay cycle information
   */
  public void receiveByte(byte b, long delay);

  /**
   * @return Last byte transmitted by radio
   */
  public byte getLastByteTransmitted();

  /**
   * Returns number of cycles since last byte was transmitted.
   * 
   * @return Timestamp info
   */
  public long getLastByteTransmittedDelay();

  /**
   * @return Last byte received by radio
   */
  public byte getLastByteReceived();
}

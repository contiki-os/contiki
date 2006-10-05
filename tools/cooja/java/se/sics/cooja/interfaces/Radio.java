/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: Radio.java,v 1.4 2006/10/05 14:47:48 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import se.sics.cooja.*;

/**
 * A Radio represents a mote radio transceiver. An implementation should notify
 * all observers both when packets are received and transmitted.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Packet Radio")
public abstract class Radio extends MoteInterface {

  /**
   * Events that radios should notify observers about.
   */
  public enum RadioEvent {
    UNKNOWN, 
    HW_OFF, HW_ON, 
    RECEPTION_STARTED, RECEPTION_INTERFERED, RECEPTION_FINISHED, 
    TRANSMISSION_STARTED, TRANSMISSION_FINISHED
  }

  /**
   * Returns last significant event at this radio. Method may for example be
   * used to learn the reason when a radio notifies a change to observers.
   * 
   * @return Last radio event
   */
  public abstract RadioEvent getLastEvent();

  /**
   * @return Last packet data transmitted, or currently being transmitted, from
   *         this radio.
   */
  public abstract byte[] getLastPacketTransmitted();

  /**
   * @return Last packet data received, or currently being received, by this
   *         radio.
   */
  public abstract byte[] getLastPacketReceived();

  /**
   * Receive given packet. If reception is not interfered during
   * this time, the packet will be delivered ok.
   * 
   * @param data
   *          Packet data
   * @param endTime
   *          Time (ms) when reception is finished
   */
  public abstract void receivePacket(byte[] data, int endTime);

  /**
   * Returns true if this radio is transmitting, or just finished transmitting,
   * data.
   * 
   * @see #getLastPacketTransmitted()
   * @return True if radio is transmitting data
   */
  public abstract boolean isTransmitting();

  /**
   * @return Transmission end time if any transmission active
   */
  public abstract int getTransmissionEndTime();

  /**
   * Returns true if this radio is receiving data.
   * 
   * @see #getLastPacketReceived()
   * @return True if radio is receiving data
   */
  public abstract boolean isReceiving();

  /**
   * If a packet is being received, it will be interfered and dropped. The
   * interference will continue until the given time, during which no other
   * radio traffic may be received. This method can be used to simulate
   * significant interference during transmissions.
   * 
   * @param endTime
   *          Time when interference stops
   */
  public abstract void interferReception(int endTime);

  /**
   * Returns true is this radio is currently hearing noise from another
   * transmission.
   * 
   * @return True if this radio is interfered
   */
  public abstract boolean isInterfered();
  
  /**
   * @return Current output power (dBm)
   */
  public abstract double getCurrentOutputPower();

  /**
   * @return Current output power indicator (1-100)
   */
  public abstract int getCurrentOutputPowerIndicator();

  /**
   * @return Current surrounding signal strength
   */
  public abstract double getCurrentSignalStrength();

  /**
   * Sets surrounding signal strength.
   * 
   * @param signalStrength
   *          Current surrounding signal strength
   */
  public abstract void setCurrentSignalStrength(double signalStrength);

}

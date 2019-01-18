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
 * $Id: Radio.java,v 1.7 2007/05/30 10:52:57 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import se.sics.cooja.*;

/**
 * A Radio represents a mote radio transceiver.
 * 
 * A radio can support different abstraction levels such as transmitting and
 * receiving on a byte or packet-basis. In order to support communication
 * between different levels the general rule in COOJA is that all radios at a
 * lower abstraction level must also implement all higher levels.
 * 
 * @see PacketRadio
 * @see ByteRadio
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Packet Radio")
public abstract class Radio extends MoteInterface {

  /**
   * Events that radios should notify observers about.
   */
  public enum RadioEvent {
    UNKNOWN, HW_OFF, HW_ON, RECEPTION_STARTED, RECEPTION_FINISHED, RECEPTION_INTERFERED, TRANSMISSION_STARTED, TRANSMISSION_FINISHED, PACKET_TRANSMITTED, BYTE_TRANSMITTED
  }

  /**
   * Signal that a new reception just begun. This method should normally be
   * called from the radio medium.
   * 
   * @see #signalReceptionEnd()
   */
  public abstract void signalReceptionStart();

  /**
   * Signal that the current reception was ended. This method should normally be
   * called from the radio medium.
   * 
   * @see #signalReceptionStart()
   */
  public abstract void signalReceptionEnd();

  /**
   * Returns last event at this radio. This method should be used to learn the
   * reason when a radio notifies a change to observers.
   * 
   * @return Last radio event
   */
  public abstract RadioEvent getLastEvent();

  /**
   * Returns true if this radio is transmitting, or just finished transmitting,
   * data.
   * 
   * @see #isReceiving()
   * @return True if radio is transmitting data
   */
  public abstract boolean isTransmitting();

  /**
   * Returns true if this radio is receiving data.
   * 
   * @see #isTransmitting()
   * @return True if radio is receiving data
   */
  public abstract boolean isReceiving();

  /**
   * Returns true if this radio had a connection that was dropped due to
   * interference.
   * 
   * @return True if this radio is interfered
   */
  public abstract boolean isInterfered();

  /**
   * Interferes with any current reception. If this method is called, the packet
   * will be dropped. This method can be used to simulate radio interference
   * such as high background noise.
   */
  public abstract void interfereAnyReception();

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
   * Sets surrounding signal strength. This method should normally be called by
   * the radio medium.
   * 
   * @param signalStrength
   *          Current surrounding signal strength
   */
  public abstract void setCurrentSignalStrength(double signalStrength);

  /**
   * Returns the current radio channel number.
   * 
   * @return Current channel number
   */
  public abstract int getChannel();

  /**
   * Returns the radio position.
   * This is typically the position of the mote.
   * 
   * @return Radio position
   */
  public abstract Position getPosition();

  /**
   * This is a convenience function.
   * It returns the mote, if any, that holds this radio.
   * 
   * @return Mote
   */
  public abstract Mote getMote();
  
}

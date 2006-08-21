/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: Radio.java,v 1.1 2006/08/21 12:12:59 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import se.sics.cooja.*;

/**
 * A Radio represents a mote radio transceiver. An implementation should notify
 * all observers both when packets are received and transmitted. The static
 * constants should be used for describing the radio status when the observers
 * are notified.
 * 
 * @see #SENT_NOTHING
 * @see #SENT_SOMETHING
 * @see #HEARS_NOTHING
 * @see #HEARS_PACKET
 * @see #HEARS_NOISE
 * @author Fredrik Osterlind
 */
@ClassDescription("Packet Radio")
public abstract class Radio extends MoteInterface {

  /**
   * This radio has not sent anything last tick.
   */
  public static final int SENT_NOTHING = 1;

  /**
   * This radio has sent something last tick.
   */
  public static final int SENT_SOMETHING = 2;

  /**
   * This radio is not hearing anything.
   */
  public static final int HEARS_NOTHING = 1;

  /**
   * This radio is hearing exactly one packet right now.
   */
  public static final int HEARS_PACKET = 2;

  /**
   * This radio is hearing a lot of noise right now (may be several packets).
   */
  public static final int HEARS_NOISE = 3;

  /**
   * @return Last packet data sent from this radio.
   */
  public abstract byte[] getLastPacketSent();

  /**
   * @return Last packet data received by this radio.
   */
  public abstract byte[] getLastPacketReceived();

  /**
   * Send given packet data to this radio. The radio may or may not receive the
   * data correctly depending on the current listen state.
   * 
   * @param data
   */
  public abstract void receivePacket(byte[] data);

  /**
   * @return Current send state
   * @see #SENT_NOTHING
   * @see #SENT_SOMETHING
   */
  public abstract int getSendState();

  /**
   * @return Current listen state
   * 
   * @see #setListenState(int)
   * @see #HEARS_NOTHING
   * @see #HEARS_PACKET
   * @see #HEARS_NOISE
   */
  public abstract int getListenState();
  
  /**
   * Changes listen state to given state
   * @param newState
   *          New listen state
   *          
   * @see #getListenState()
   * @see #HEARS_NOTHING
   * @see #HEARS_PACKET
   * @see #HEARS_NOISE
   */
  public abstract void setListenState(int newState);
  
  /**
   * Advances listen state one step. If listen state was 'hears nothing', it
   * will become 'hears packet'. If it was 'hears packet', it will become 'hears
   * noise'. If it was 'hears noise', it will stay that way.
   * 
   * @see #getListenState()
   */
  public abstract void advanceListenState();

}

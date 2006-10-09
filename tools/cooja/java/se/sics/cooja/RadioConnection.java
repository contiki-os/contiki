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
 * $Id: RadioConnection.java,v 1.3 2006/10/09 13:38:38 fros4943 Exp $
 */

package se.sics.cooja;

import java.util.Vector;

import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;

/**
 * RadioConnection represents a radio connection between a sending radio
 * and zero or more receiving radios.
 * By registering as an observer to the current radio medium, all
 * radio connections and data sent in that medium can be accessed.
 *
 * Each radio is associated with a position and some radio data.
 * Often the destinations' and source's data will refer to the same object,
 * but some radio mediums may want to distort the transferred data, hence
 * resulting in different data sent and received.
 *
 * @see RadioMedium
 * @author Fredrik Osterlind
 */
public class RadioConnection {
  private Radio sourceRadio;
  private Position sourcePosition;
  private byte[] sourceData;

  private Vector<Radio> destinationRadios = new Vector<Radio>();
  private Vector<Position> destinationPositions = new Vector<Position>();
  private Vector<byte[]> destinationData = new Vector<byte[]>();

  /**
   * Set source of this connection.
   *
   * @param radio Source radio
   * @param position Source position
   * @param data Source data
   */
  public void setSource(Radio radio, Position position, byte[] data) {
    sourceRadio = radio;
    sourcePosition = position;
    sourceData = data;
  }

  /**
   * Add a connection destination.
   *
   * @param radio Source radio
   * @param position Source position
   * @param data Source data
   */
  public void addDestination(Radio radio, Position position, byte[] data) {
    destinationRadios.add(radio);
    destinationPositions.add(position);
    destinationData.add(data);
  }
  
  /**
   * Remove a connection destination.
   *
   * @param radio Destination to remove
   */
  public void removeDestination(Radio radio) {
    int pos = destinationRadios.indexOf(radio);
    if (pos >= 0) {
      destinationRadios.remove(pos);
      destinationPositions.remove(pos);
      destinationData.remove(pos);
    }
  }

  /**
   * @return Source radio
   */
  public Radio getSourceRadio() {
    return sourceRadio;
  }

  /**
   * @return Source position
   */
  public Position getSourcePosition() {
    return sourcePosition;
  }

  /**
   * Returns the data actually sent by source radio.
   * @return Source data
   */
  public byte[] getSourceData() {
    return sourceData;
  }

  /**
   * @return Array of destination radios
   */
  public Radio[] getDestinationRadios() {
    Radio[] radioArrayType;
    Radio[] radioArray;

    radioArrayType = new Radio[destinationRadios.size()];
    radioArray = (Radio[]) destinationRadios.toArray(radioArrayType);

    return radioArray;
  }

  /**
   * @return Array of destination positions
   */
  public Position[] getDestinationPositons() {
    Position[] positionArrayType;
    Position[] positionArray;

    positionArrayType = new Position[destinationPositions.size()];
    positionArray = (Position[]) destinationPositions.toArray(positionArrayType);

    return positionArray;
  }

  /**
   * Returns an array of data actually received by each radio.
   * @return Array of destination data
   */
  public byte[][] getDestinationData() {
    byte[][] dataArrayType;
    byte[][] dataArray;

    dataArrayType = new byte[destinationData.size()][];
    dataArray = (byte[][]) destinationData.toArray(dataArrayType);

    return dataArray;
  }

}

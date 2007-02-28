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
 * $Id: RadioConnection.java,v 1.4 2007/02/28 09:47:55 fros4943 Exp $
 */

package se.sics.cooja;

import java.util.Vector;

import se.sics.cooja.interfaces.Radio;

/**
 * A radio connection represents a connection between a source radio and zero or
 * more destination and interfered radios. Typically the destinations are able
 * to receive data sent by the source radio, and the interfered radios are not.
 * 
 * @see RadioMedium
 * @author Fredrik Osterlind
 */
public class RadioConnection {
  private Radio source;

  private Vector<Radio> destinations = new Vector<Radio>();

  private Vector<Radio> interfered = new Vector<Radio>();

  /**
   * Creates a new radio connection with given source and no destinations.
   * 
   * @param sourceRadio
   *          Source radio
   */
  public RadioConnection(Radio sourceRadio) {
    this.source = sourceRadio;
  }

  /**
   * Set source of this connection.
   * 
   * @param radio
   *          Source radio
   */
  public void setSource(Radio radio) {
    source = radio;
  }

  /**
   * Adds destination radio.
   * 
   * @param radio
   *          Radio
   */
  public void addDestination(Radio radio) {
    destinations.add(radio);
  }

  /**
   * Adds interfered radio.
   * 
   * @param radio
   *          Radio
   */
  public void addInterfered(Radio radio) {
    interfered.add(radio);
  }

  /**
   * Removes destination radio.
   * 
   * @param radio
   *          Radio
   */
  public void removeDestination(Radio radio) {
    destinations.remove(radio);
  }

  /**
   * Removes interfered radio.
   * 
   * @param radio
   *          Radio
   */
  public void removeInterfered(Radio radio) {
    interfered.remove(radio);
  }

  /**
   * @return Source radio
   */
  public Radio getSource() {
    return source;
  }

  /**
   * @return All destinations of this connection
   */
  public Radio[] getDestinations() {
    Radio[] radioArrayType;
    Radio[] radioArray;

    radioArrayType = new Radio[destinations.size()];
    radioArray = (Radio[]) destinations.toArray(radioArrayType);

    return radioArray;
  }

  /**
   * @return All radios interfered by this connection
   */
  public Radio[] getInterfered() {
    Radio[] radioArrayType;
    Radio[] radioArray;

    radioArrayType = new Radio[interfered.size()];
    radioArray = (Radio[]) interfered.toArray(radioArrayType);

    return radioArray;
  }

}

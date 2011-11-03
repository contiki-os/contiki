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
 * $Id: RadioConnection.java,v 1.12 2009/11/25 15:21:15 fros4943 Exp $
 */

package se.sics.cooja;

import java.util.ArrayList;

import org.apache.log4j.Logger;

import se.sics.cooja.interfaces.Radio;

/**
 * A radio connection represents a connection between a single source radio and 
 * any number of destination and interfered radios.
 * 
 * Note that a destination node may be interfered, and that an interfered radio 
 * does not need to be a destination. Interfered radios may be added during the
 * connection lifetime. 
 * 
 * Radio medium implementations may differ slightly in how they forward connection data 
 * to destination and interfered radios.
 * Typically, however, all destination radios (including those that are interfered)
 * receive the connection data.
 * And the interfered non-destination radios do not receive the connection data.
 * 
 * @see RadioMedium
 * @author Fredrik Osterlind
 */
public class RadioConnection {
  private static Logger logger = Logger.getLogger(RadioConnection.class);

  private static int ID = 0; /* Unique radio connection ID. For internal use */
  private int id;

  private Radio source;
  
  private ArrayList<Radio> allDestinations = new ArrayList<Radio>();
  private ArrayList<Long> allDestinationDelays = new ArrayList<Long>();

  private ArrayList<Radio> allInterfered = new ArrayList<Radio>();
  private ArrayList<Radio> onlyInterfered = new ArrayList<Radio>();
  private ArrayList<Radio> destinationsNonInterfered = new ArrayList<Radio>();
  
  private long startTime;

  /**
   * Creates a new radio connection with given source and no destinations.
   *
   * @param sourceRadio Source radio
   */
  public RadioConnection(Radio sourceRadio) {
    this.source = sourceRadio;
    startTime = sourceRadio.getMote().getSimulation().getSimulationTime();

    this.id = ID++;
  }

  /**
   * @return Radio connection start time
   */
  public long getStartTime() {
    return startTime;
  }

  /**
   * @return Start time of ongoing reception
   */
  public long getReceptionStartTime() {
    /* TODO XXX: This may currently return the start time of an ongoing 
     * interference. */
    return getStartTime();
  }
  
  /**
   * Add (non-interfered) destination radio to connection.
   *
   * @param radio Radio
   */
  public void addDestination(Radio radio) {
    addDestination(radio, new Long(0));
  }
  
  /**
   * Experimental: remove destination.
   *
   * @param radio Radio
   */
  public void removeDestination(Radio radio) {
    int idx = allDestinations.indexOf(radio);
    if (idx < 0) {
      logger.fatal("Radio is not a connection destination: " + radio);
      return;
    }
    
    allDestinations.remove(idx);
    allDestinationDelays.remove(idx);
    destinationsNonInterfered.remove(radio);
    onlyInterfered.remove(radio);
  }

  /**
   * Add (non-interfered) destination radio to connection.
   * 
   * @param radio Radio
   * @param delay Radio propagation delay (us)
   */
  public void addDestination(Radio radio, Long delay) {
    if (isDestination(radio)) {
      logger.fatal("Radio is already a destination: " + radio);
      return;
    }
    allDestinations.add(radio);
    allDestinationDelays.add(delay);
    destinationsNonInterfered.add(radio);
    onlyInterfered.remove(radio);
  }

  /**
   * @param radio Radio
   * @return Radio propagation delay (us)
   */
  public long getDestinationDelay(Radio radio) {
    int idx = allDestinations.indexOf(radio);
    if (idx < 0) {
      logger.fatal("Radio is not a connection destination: " + radio);
      return 0;
    }
    return allDestinationDelays.get(idx);

  }

  /**
   * Adds interfered radio to connection.
   * Note that the radio may or may not already be a destination.
   *
   * @param radio Radio
   * @see #getDestinations()
   * @see #getAllDestinations()
   */
  public void addInterfered(Radio radio) {
    if (isInterfered(radio)) {
      logger.fatal("Radio is already interfered: " + radio);
      return;
    }

    allInterfered.add(radio);
    destinationsNonInterfered.remove(radio);
    if (!isDestination(radio)) {
      onlyInterfered.add(radio);
    }
  }

  /**
   * @param radio Radio
   * @return True if radio is a non-interfered destination in this connection
   */
  public boolean isDestination(Radio radio) {
    return destinationsNonInterfered.contains(radio);
  }

  /**
   * @param radio Radio
   * @return True if radio is interfered in this connection
   */
  public boolean isInterfered(Radio radio) {
    return allInterfered.contains(radio);
  }

  /**
   * @return Source radio
   */
  public Radio getSource() {
    return source;
  }

  /**
   * @see #getAllDestinations()
   * @return All non-interfered destinations
   */
  public Radio[] getDestinations() {
    return destinationsNonInterfered.toArray(new Radio[0]);
  }

  /**
   * @see #getDestinations()
   * @return All destination radios, including radios that became
   * interfered after the connection started.
   */
  public Radio[] getAllDestinations() {
    return allDestinations.toArray(new Radio[0]);
  }

  /**
   * @return All radios interfered by this connection, including destinations
   */
  public Radio[] getInterfered() {
    return allInterfered.toArray(new Radio[0]);
  }

  public Radio[] getInterferedNonDestinations() {
    return onlyInterfered.toArray(new Radio[0]);
  }

  public String toString() {
    if (destinationsNonInterfered.size() == 0) {
      return id + ": Radio connection: " + source.getMote() + " -> none";
    }
    if (destinationsNonInterfered.size() == 1) {
      return id + ": Radio connection: " + source.getMote() + " -> " + destinationsNonInterfered.get(0).getMote();
    }

    return id + ": Radio connection: " + source.getMote() + " -> " + destinationsNonInterfered.size() + " motes";

  }

}

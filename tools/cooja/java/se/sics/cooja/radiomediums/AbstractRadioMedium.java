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
 * $Id: AbstractRadioMedium.java,v 1.2 2007/04/23 08:25:50 fros4943 Exp $
 */

package se.sics.cooja.radiomediums;

import java.util.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;

/**
 * Abstract radio medium provides basic functionality for implementing radio 
 * mediums.
 * 
 * It handles radio registrations, radio loggers, active connections and 
 * observes all registered radio interfaces.
 * 
 * @author Fredrik Osterlind
 */
public abstract class AbstractRadioMedium extends RadioMedium {
  private static Logger logger = Logger.getLogger(AbstractRadioMedium.class);

  private Vector<Radio> registeredRadios = new Vector<Radio>();
  
  private Vector<RadioConnection> activeConnections = new Vector<RadioConnection>();

  private Vector<RadioConnection> finishedConnections = new Vector<RadioConnection>();

  private boolean isTickObserver = false;

  private class RadioMediumObservable extends Observable {
    private void setRadioMediumChanged() {
      setChanged();
    }
  }

  private RadioMediumObservable radioMediumObservable = new RadioMediumObservable();

  private RadioConnection[] lastTickConnections = null;

  private ConnectionLogger myLogger = null;

  /**
   * This constructor should always be called from implemented radio mediums.
   * 
   * @param simulation Simulation
   */
  public AbstractRadioMedium(Simulation simulation) {
  }
  
  /**
   * @return All registered radios
   */
  public Vector<Radio> getRegisteredRadios() {
    return registeredRadios;
  }

  /**
   * @return All active connections
   */
  public Vector<RadioConnection> getActiveConnections() {
    return activeConnections;
  }

  /**
   * Creates a new connection from given radio.
   * 
   * This method is responsible for determining which radio should receive or be 
   * interfered by the transmission.
   * 
   * @param radio
   *          Transmitting radio
   * @return New registered connection
   */
  abstract public RadioConnection createConnections(Radio radio);
  
  /**
   * Forward given packet from source to all destinations in given connection.
   * This method is called sometime during an active connection.
   * 
   * @param connection
   *          Radio connection
   * @param packetData
   *          Packet data
   */
  public void forwardPacket(RadioConnection connection, byte[] packetData) {
    Radio sourceRadio = connection.getSource();
    if (packetData == null || packetData.length == 0) {
      logger.warn("Trying to forward null packet");
      return;
    }

    if (sourceRadio instanceof ByteRadio) {
      // Byte radios only forwards packets to packet radios
      for (Radio receivingRadio : connection.getDestinations()) {
        if (receivingRadio instanceof ByteRadio) {
          // Do nothing (data will be forwarded on byte-per-byte basis)
        } else if (receivingRadio instanceof PacketRadio) {
          // Forward packet data
          ((PacketRadio) receivingRadio).setReceivedPacket(packetData);
        } else {
          logger.warn("Packet was not forwarded correctly");
        }
      }
    } else if (sourceRadio instanceof PacketRadio) {
      // Packet radios forwards packets to all packet radios
      for (Radio receivingRadio : connection.getDestinations()) {
        if (receivingRadio instanceof PacketRadio) {
          // Forward packet data
          ((PacketRadio) receivingRadio).setReceivedPacket(packetData);
        } else {
          logger.warn("Packet was not forwarded correctly");
        }
      }
    }
  }

  /**
   * This method should update all radio interfaces' signal strengths according to 
   * the current active connections.
   */
  abstract public void updateSignalStrengths();

  /**
   * Remove given radio from any active connections.
   * This method can be called if a radio node falls asleep or is removed.
   * 
   * @param radio Radio
   */
  private void removeFromActiveConnections(Radio radio) {
    // Abort any reception
    if (radio.isReceiving()) {
      radio.interfereAnyReception();
      radio.signalReceptionEnd();
    }

    // Remove radio from all active connections
    RadioConnection connToRemove = null;
    for (RadioConnection conn : activeConnections) {
      conn.removeDestination(radio);
      conn.removeInterfered(radio);

      if (conn.getSource() == radio) {
        // Radio is currently transmitting
        connToRemove = conn;
        for (Radio dstRadio : conn.getDestinations()) {
          dstRadio.interfereAnyReception();
          dstRadio.signalReceptionEnd();
        }
        for (Radio dstRadio : conn.getInterfered()) {
          dstRadio.signalReceptionEnd();
        }
      }
    }
    if (connToRemove != null)
      activeConnections.remove(connToRemove);
  }

  /**
   * This observer is responsible for detecting radio interface events, for example 
   * new transmissions.
   */
  private Observer radioEventsObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      if (!(obs instanceof Radio)) {
        logger.fatal("Radio event dispatched by non-radio object");
        return;
      }

      Radio radio = (Radio) obs;

      // Handle radio event
      final Radio.RadioEvent event = radio.getLastEvent();

      // Ignore reception events
      if (event == Radio.RadioEvent.RECEPTION_STARTED
          || event == Radio.RadioEvent.RECEPTION_INTERFERED
          || event == Radio.RadioEvent.RECEPTION_FINISHED) {
        return;
      }

      if (event == Radio.RadioEvent.HW_OFF) {
        // Destroy any(?) transfers
        removeFromActiveConnections(radio);

        // Recalculate signal strengths on all radios
        updateSignalStrengths();

        // Wake up tick observer
        radioMediumObservable.setRadioMediumChanged();

      } else if (event == Radio.RadioEvent.HW_ON) {
        // No action
        // TODO Maybe set signal strength levels now?

        // Recalculate signal strengths on all radios
        updateSignalStrengths();

        // Wake up tick observer
        radioMediumObservable.setRadioMediumChanged();

      } else if (event == Radio.RadioEvent.TRANSMISSION_STARTED) {
        /* Create radio connections */

        RadioConnection newConnection = createConnections((Radio) radio);
        if (newConnection != null) {
          activeConnections.add(newConnection);
        }

        // Recalculate signal strengths on all radios
        updateSignalStrengths();

        // Wake up tick observer
        radioMediumObservable.setRadioMediumChanged();

      } else if (event == Radio.RadioEvent.TRANSMISSION_FINISHED) {
        /* Remove active connection */

        // Find corresponding connection of radio
        RadioConnection connection = null;
        for (RadioConnection conn : activeConnections) {
          if (conn.getSource() == radio) {
            connection = conn;
            break;
          }
        }

        if (connection == null) {
          logger.fatal("Can't find active connection to remove");
        } else {
          activeConnections.remove(connection);
          finishedConnections.add(connection);
          for (Radio dstRadio : connection.getDestinations()) {
            dstRadio.signalReceptionEnd();
          }
          for (Radio dstRadio : connection.getInterfered()) {
            dstRadio.signalReceptionEnd();
          }
        }

        // Recalculate signal strengths on all radios
        updateSignalStrengths();

        // Wake up tick observer
        radioMediumObservable.setRadioMediumChanged();

      } else if (event == Radio.RadioEvent.BYTE_TRANSMITTED) {
        /* Forward byte */

        // Find corresponding connection of radio
        RadioConnection connection = null;
        for (RadioConnection conn : activeConnections) {
          if (conn.getSource() == radio) {
            connection = conn;
            break;
          }
        }

        if (connection == null) {
          logger.fatal("Can't find active connection to forward byte in");
        } else {
          byte b = ((ByteRadio) radio).getLastByteTransmitted();
          long timestamp = ((ByteRadio) radio).getLastByteTransmittedTimestamp();

          for (Radio dstRadio : connection.getDestinations()) {
            if (dstRadio instanceof ByteRadio) {
              ((ByteRadio) dstRadio).receiveByte(b, timestamp);
            }
          }
        }

      } else if (event == Radio.RadioEvent.PACKET_TRANSMITTED) {
        /* Forward packet */

        // Find corresponding connection of radio
        RadioConnection connection = null;
        for (RadioConnection conn : activeConnections) {
          if (conn.getSource() == radio) {
            connection = conn;
            break;
          }
        }

        if (connection == null) {
          logger.fatal("Can't find active connection to forward byte in");
        } else {
          byte[] packetData = ((PacketRadio) radio).getLastPacketTransmitted();
          forwardPacket(connection, packetData);
        }

      } else if (event == Radio.RadioEvent.UNKNOWN) {
        // Do nothing
      } else {
        logger.fatal("Unsupported radio event: " + event);
      }
    }
  };
  

  /**
   * This observer is responsible for making last tick connections available to
   * external observers, as well as forwarding finished connections to any
   * registered connection logger.
   * 
   * @see #getLastTickConnections()
   * @see #setConnectionLogger(ConnectionLogger)
   */
  private Observer tickObserver = new Observer() {
    public void update(Observable obs, Object obj) {

      // Reset any last tick connections
      if (lastTickConnections != null) {
        radioMediumObservable.setRadioMediumChanged();
        lastTickConnections = null;
      }

      // Do nothing if radio medium unchanged
      if (!radioMediumObservable.hasChanged())
        return;

      // Log any new finished connections
      if (finishedConnections.size() > 0) {
        lastTickConnections = new RadioConnection[finishedConnections.size()];
        for (int i = 0; i < finishedConnections.size(); i++)
          lastTickConnections[i] = finishedConnections.get(i);
        finishedConnections.clear();

        // Notify radio medium logger of the finished transmissions
        if (myLogger != null) {
          for (RadioConnection conn : lastTickConnections)
            myLogger.logConnection(conn);
        }
      }

      // Notify all other radio medium observers
      radioMediumObservable.notifyObservers();
    }
  };

  public void registerMote(Mote mote, Simulation sim) {
    registerRadioInterface(mote.getInterfaces().getRadio(), sim);
  }

  public void unregisterMote(Mote mote, Simulation sim) {
    unregisterRadioInterface(mote.getInterfaces().getRadio(), sim);
  }

  public void registerRadioInterface(Radio radio, Simulation sim) {
    if (radio != null) {
      if (!isTickObserver) {
        sim.addTickObserver(tickObserver);
        isTickObserver = true;
      }

      // Warn if radio medium does not support radio
      if (!(radio instanceof PacketRadio)) {
        logger
            .warn("Radio medium does not support radio (no transmission supported)");
      }

      // Register and start observing radio
      registeredRadios.add(radio);
      radio.addObserver(radioEventsObserver);

      // Set initial signal strenth
      updateSignalStrengths();
//TODO      radio.setCurrentSignalStrength(SS_NOTHING);
    }
  }

  public void unregisterRadioInterface(Radio radio, Simulation sim) {
    if (!registeredRadios.contains(radio)) {
      logger.warn("Could not find radio: " + radio + " to unregister");
      return;
    }

    radio.deleteObserver(radioEventsObserver);
    registeredRadios.remove(radio);

    removeFromActiveConnections(radio);
  }

  public void addRadioMediumObserver(Observer observer) {
    radioMediumObservable.addObserver(observer);
  }

  public Observable getRadioMediumObservable() {
    return radioMediumObservable;
  }

  public void deleteRadioMediumObserver(Observer observer) {
    radioMediumObservable.deleteObserver(observer);
  }

  public RadioConnection[] getLastTickConnections() {
    return lastTickConnections;
  }

  public void setConnectionLogger(ConnectionLogger connection) {
    myLogger = connection;
  }

}

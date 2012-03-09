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
 * $Id: AbstractRadioMedium.java,v 1.15 2010/10/12 10:29:43 fros4943 Exp $
 */

package se.sics.cooja.radiomediums;

import java.util.ArrayList;
import java.util.Observable;
import java.util.Observer;

import org.apache.log4j.Logger;

import se.sics.cooja.Mote;
import se.sics.cooja.RadioConnection;
import se.sics.cooja.RadioMedium;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.interfaces.CustomDataRadio;
import se.sics.cooja.interfaces.Radio;

/**
 * Abstract radio medium provides basic functionality for implementing radio
 * mediums.
 *
 * The radio medium forwards both radio packets and custom data objects.
 *
 * The registered radios' signal strengths are updated whenever the radio medium
 * changes. There are three fixed levels: no surrounding traffic heard, noise
 * heard and data heard.
 *
 * It handles radio registrations, radio loggers, active connections and
 * observes all registered radio interfaces.
 *
 * @author Fredrik Osterlind
 */
public abstract class AbstractRadioMedium extends RadioMedium {
  private static Logger logger = Logger.getLogger(AbstractRadioMedium.class);

  /* Signal strengths in dBm.
   * Approx. values measured on TmoteSky */
  public static final double SS_NOTHING = -100;
  public static final double SS_STRONG = -10;
  public static final double SS_WEAK = -95;

  private ArrayList<Radio> registeredRadios = new ArrayList<Radio>();

  private ArrayList<RadioConnection> activeConnections = new ArrayList<RadioConnection>();

  private RadioConnection lastConnection = null;

  private Simulation simulation = null;

  /* Book-keeping */
  public int COUNTER_TX = 0;
  public int COUNTER_RX = 0;
  public int COUNTER_INTERFERED = 0;

  public class RadioMediumObservable extends Observable {
    public void setRadioMediumChanged() {
      setChanged();
    }
    public void setRadioMediumChangedAndNotify() {
      setChanged();
      notifyObservers();
    }
  }

  private RadioMediumObservable radioMediumObservable = new RadioMediumObservable();

  /**
   * This constructor should always be called from implemented radio mediums.
   *
   * @param simulation Simulation
   */
  public AbstractRadioMedium(Simulation simulation) {
    this.simulation = simulation;
  }

  /**
   * @return All registered radios
   */
  public Radio[] getRegisteredRadios() {
    return registeredRadios.toArray(new Radio[0]);
  }

  /**
   * @return All active connections
   */
  public RadioConnection[] getActiveConnections() {
    /* NOTE: toArray([0]) creates array and handles synchronization */
    return activeConnections.toArray(new RadioConnection[0]);
  }

  /**
   * Creates a new connection from given radio.
   *
   * Determines which radios should receive or be interfered by the transmission.
   *
   * @param radio Source radio
   * @return New connection
   */
  abstract public RadioConnection createConnections(Radio radio);

  /**
   * Updates all radio interfaces' signal strengths according to
   * the current active connections.
   */
  public void updateSignalStrengths() {

    /* Reset signal strengths */
    for (Radio radio : getRegisteredRadios()) {
      radio.setCurrentSignalStrength(SS_NOTHING);
    }

    /* Set signal strength to strong on destinations */
    RadioConnection[] conns = getActiveConnections();
    for (RadioConnection conn : conns) {
      if (conn.getSource().getCurrentSignalStrength() < SS_STRONG) {
        conn.getSource().setCurrentSignalStrength(SS_STRONG);
      }
      for (Radio dstRadio : conn.getDestinations()) {
        if (conn.getSource().getChannel() >= 0 &&
            dstRadio.getChannel() >= 0 &&
            conn.getSource().getChannel() != dstRadio.getChannel()) {
          continue;
        }
        if (dstRadio.getCurrentSignalStrength() < SS_STRONG) {
          dstRadio.setCurrentSignalStrength(SS_STRONG);
        }
      }
    }

    /* Set signal strength to weak on interfered */
    for (RadioConnection conn : conns) {
      for (Radio intfRadio : conn.getInterfered()) {
        if (intfRadio.getCurrentSignalStrength() < SS_STRONG) {
          intfRadio.setCurrentSignalStrength(SS_STRONG);
        }
        if (conn.getSource().getChannel() >= 0 &&
            intfRadio.getChannel() >= 0 &&
            conn.getSource().getChannel() != intfRadio.getChannel()) {
          continue;
        }
        if (!intfRadio.isInterfered()) {
          /*logger.warn("Radio was not interfered");*/
          intfRadio.interfereAnyReception();
        }
      }
    }
  }


  /**
   * Remove given radio from any active connections.
   * This method can be called if a radio node falls asleep or is removed.
   *
   * @param radio Radio
   */
  private void removeFromActiveConnections(Radio radio) {
    /* This radio must not be a connection source */
    RadioConnection connection = getActiveConnectionFrom(radio);
    if (connection != null) {
      logger.fatal("Connection source turned off radio: " + radio);
    }

    /* Set interfered if currently a connection destination */
    for (RadioConnection conn : activeConnections) {
      if (conn.isDestination(radio)) {
        conn.addInterfered(radio);
        if (!radio.isInterfered()) {
          radio.interfereAnyReception();
        }
      }
    }
  }

  private RadioConnection getActiveConnectionFrom(Radio source) {
    for (RadioConnection conn : activeConnections) {
      if (conn.getSource() == source) {
        return conn;
      }
    }
    return null;
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

      final Radio.RadioEvent event = radio.getLastEvent();
      if (event == Radio.RadioEvent.RECEPTION_STARTED ||
          event == Radio.RadioEvent.RECEPTION_INTERFERED ||
          event == Radio.RadioEvent.RECEPTION_FINISHED ||
          event == Radio.RadioEvent.UNKNOWN) {
        /* Ignored */
        return;
      }

      if (event == Radio.RadioEvent.HW_ON) {

        /* Update signal strengths */
        updateSignalStrengths();

      } else if (event == Radio.RadioEvent.HW_OFF) {

        /* Remove any radio connections from this radio */
        removeFromActiveConnections(radio);

        /* Update signal strengths */
        updateSignalStrengths();

      } else if (event == Radio.RadioEvent.TRANSMISSION_STARTED) {
        /* Create new radio connection */

        if (radio.isReceiving()) {
          /* Radio starts transmitting when it should be receiving!
           * Ok, but it won't receive the packet */
          for (RadioConnection conn : activeConnections) {
            if (conn.isDestination(radio)) {
              conn.addInterfered(radio);
            }
          }
          radio.interfereAnyReception();
        }
        
        RadioConnection newConnection = createConnections(radio);
        activeConnections.add(newConnection);
        for (Radio r: newConnection.getAllDestinations()) {
          if (newConnection.getDestinationDelay(r) == 0) {
            r.signalReceptionStart();
          } else {

            /* EXPERIMENTAL: Simulating propagation delay */
            final Radio delayedRadio = r;
            TimeEvent delayedEvent = new TimeEvent(0) {
              public void execute(long t) {
                delayedRadio.signalReceptionStart();
              }
            };
            simulation.scheduleEvent(
                delayedEvent,
                simulation.getSimulationTime() + newConnection.getDestinationDelay(r));

          }
        }

        /* Update signal strengths */
        updateSignalStrengths();

        /* Notify observers */
        lastConnection = null;
        radioMediumObservable.setRadioMediumChangedAndNotify();

      } else if (event == Radio.RadioEvent.TRANSMISSION_FINISHED) {
        /* Remove radio connection */

        /* Connection */
        RadioConnection connection = getActiveConnectionFrom(radio);
        if (connection == null) {
          logger.fatal("No radio connection found");
          return;
        }

        activeConnections.remove(connection);
        lastConnection = connection;
        COUNTER_TX++;
        for (Radio dstRadio : connection.getAllDestinations()) {
          if (connection.getDestinationDelay(dstRadio) == 0) {
            dstRadio.signalReceptionEnd();
          } else {

            /* EXPERIMENTAL: Simulating propagation delay */
            final Radio delayedRadio = dstRadio;
            TimeEvent delayedEvent = new TimeEvent(0) {
              public void execute(long t) {
                delayedRadio.signalReceptionEnd();
              }
            };
            simulation.scheduleEvent(
                delayedEvent,
                simulation.getSimulationTime() + connection.getDestinationDelay(dstRadio));
          }
        }
        COUNTER_RX += connection.getDestinations().length;
        COUNTER_INTERFERED += connection.getInterfered().length;
        for (Radio intRadio : connection.getInterferedNonDestinations()) {
          intRadio.signalReceptionEnd();
        }

        /* Update signal strengths */
        updateSignalStrengths();

        /* Notify observers */
        radioMediumObservable.setRadioMediumChangedAndNotify();

      } else if (event == Radio.RadioEvent.CUSTOM_DATA_TRANSMITTED) {

        /* Connection */
        RadioConnection connection = getActiveConnectionFrom(radio);
        if (connection == null) {
          logger.fatal("No radio connection found");
          return;
        }

        /* Custom data object */
        Object data = ((CustomDataRadio) radio).getLastCustomDataTransmitted();
        if (data == null) {
          logger.fatal("No custom data object to forward");
          return;
        }

        for (Radio dstRadio : connection.getAllDestinations()) {

          if (!radio.getClass().equals(dstRadio.getClass()) ||
              !(radio instanceof CustomDataRadio)) {
            /* Radios communicate via radio packets */
            continue;
          }

          if (connection.getDestinationDelay(dstRadio) == 0) {
            ((CustomDataRadio) dstRadio).receiveCustomData(data);
          } else {

            /* EXPERIMENTAL: Simulating propagation delay */
            final CustomDataRadio delayedRadio = (CustomDataRadio) dstRadio;
            final Object delayedData = data;
            TimeEvent delayedEvent = new TimeEvent(0) {
              public void execute(long t) {
                delayedRadio.receiveCustomData(delayedData);
              }
            };
            simulation.scheduleEvent(
                delayedEvent,
                simulation.getSimulationTime() + connection.getDestinationDelay(dstRadio));

          }
        }

      } else if (event == Radio.RadioEvent.PACKET_TRANSMITTED) {

        /* Connection */
        RadioConnection connection = getActiveConnectionFrom(radio);
        if (connection == null) {
          logger.fatal("No radio connection found");
          return;
        }

        /* Radio packet */
        RadioPacket packet = radio.getLastPacketTransmitted();
        if (packet == null) {
          logger.fatal("No radio packet to forward");
          return;
        }

        for (Radio dstRadio : connection.getAllDestinations()) {

          if (radio.getClass().equals(dstRadio.getClass()) &&
              radio instanceof CustomDataRadio) {
            /* Radios instead communicate via custom data objects */
            continue;
          }

          /* Forward radio packet */
          if (connection.getDestinationDelay(dstRadio) == 0) {
            dstRadio.setReceivedPacket(packet);
          } else {

            /* EXPERIMENTAL: Simulating propagation delay */
            final Radio delayedRadio = dstRadio;
            final RadioPacket delayedPacket = packet;
            TimeEvent delayedEvent = new TimeEvent(0) {
              public void execute(long t) {
                delayedRadio.setReceivedPacket(delayedPacket);
              }
            };
            simulation.scheduleEvent(
                delayedEvent,
                simulation.getSimulationTime() + connection.getDestinationDelay(dstRadio));
          }

        }

      } else {
        logger.fatal("Unsupported radio event: " + event);
      }
    }
  };

  public void registerMote(Mote mote, Simulation sim) {
    registerRadioInterface(mote.getInterfaces().getRadio(), sim);
  }

  public void unregisterMote(Mote mote, Simulation sim) {
    unregisterRadioInterface(mote.getInterfaces().getRadio(), sim);
  }

  public void registerRadioInterface(Radio radio, Simulation sim) {
    if (radio == null) {
      logger.warn("No radio to register");
      return;
    }

    registeredRadios.add(radio);
    radio.addObserver(radioEventsObserver);

    /* Update signal strengths */
    updateSignalStrengths();
  }

  public void unregisterRadioInterface(Radio radio, Simulation sim) {
    if (!registeredRadios.contains(radio)) {
      logger.warn("No radio to unregister: " + radio);
      return;
    }

    radio.deleteObserver(radioEventsObserver);
    registeredRadios.remove(radio);

    removeFromActiveConnections(radio);

    /* Update signal strengths */
    updateSignalStrengths();
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

  public RadioConnection getLastConnection() {
    return lastConnection;
  }

}

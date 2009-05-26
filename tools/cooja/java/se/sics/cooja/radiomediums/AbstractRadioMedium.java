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
 * $Id: AbstractRadioMedium.java,v 1.10 2009/05/26 14:17:29 fros4943 Exp $
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

  private RadioConnection[] lastTickConnections = null;

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
  public Vector<Radio> getRegisteredRadios() {
    return registeredRadios;
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
   * @param radio
   *          Transmitting radio
   * @return New registered connection
   */
  abstract public RadioConnection createConnections(Radio radio);

  /**
   * Updates all radio interfaces' signal strengths according to
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
    if (connToRemove != null) {
      activeConnections.remove(connToRemove);
    }
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
      } else if (event == Radio.RadioEvent.HW_ON) {
        // No action
        // TODO Maybe set signal strength levels now?

        // Recalculate signal strengths on all radios
        updateSignalStrengths();

      } else if (event == Radio.RadioEvent.TRANSMISSION_STARTED) {
        /* Create radio connections */

        RadioConnection newConnection = createConnections(radio);
        activeConnections.add(newConnection);
        for (Radio r: newConnection.getDestinations()) {
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

        // Recalculate signal strengths on all radios
        updateSignalStrengths();

        /* Notify observers */
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
          logger.fatal("Can't find active connection to remove, source=" + radio);
        } else {
          activeConnections.remove(connection);
          lastConnection = connection;
          COUNTER_TX++;
          for (Radio dstRadio : connection.getDestinations()) {
            COUNTER_RX++;
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
          for (Radio dstRadio : connection.getInterfered()) {
            COUNTER_INTERFERED++;
            dstRadio.signalReceptionEnd();
          }
        }
        
        // Recalculate signal strengths on all radios
        updateSignalStrengths();

        /* Notify observers */
        radioMediumObservable.setRadioMediumChanged();
        radioMediumObservable.notifyObservers();

      } else if (event == Radio.RadioEvent.CUSTOM_DATA_TRANSMITTED) {
        /* Forward custom data, if any */

        // Find corresponding connection of radio
        RadioConnection connection = null;
        for (RadioConnection conn : activeConnections) {
          if (conn.getSource() == radio) {
            connection = conn;
            break;
          }
        }
        if (connection == null) {
          logger.fatal("Can't find active connection to forward custom data in");
          return;
        }

        Object data = ((CustomDataRadio) radio).getLastCustomDataTransmitted();
        if (data == null) {
          logger.fatal("Custom data object is null");
          return;
        }

        for (Radio dstRadio : connection.getDestinations()) {
          if (dstRadio instanceof CustomDataRadio) {
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
        }

      } else if (event == Radio.RadioEvent.PACKET_TRANSMITTED) {
        /* Forward packet, if any */

        // Find corresponding connection of radio
        RadioConnection connection = null;
        for (RadioConnection conn : activeConnections) {
          if (conn.getSource() == radio) {
            connection = conn;
            break;
          }
        }
        if (connection == null) {
          logger.fatal("Can't find active connection to forward packet in");
          return;
        }

        RadioPacket packet = radio.getLastPacketTransmitted();
        if (packet == null) {
          logger.fatal("Radio packet is null");
          return;
        }

        Radio srcRadio = connection.getSource();
        for (Radio dstRadio : connection.getDestinations()) {
          if (!(srcRadio instanceof CustomDataRadio) ||
              !(dstRadio instanceof CustomDataRadio)) {
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
        }

      } else if (event == Radio.RadioEvent.UNKNOWN) {
        // Do nothing
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
    if (radio != null) {
      // Register and start observing radio
      registeredRadios.add(radio);
      radio.addObserver(radioEventsObserver);

      // Set initial signal strength
      updateSignalStrengths();
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
    if (lastConnection == null) {
      return null;
    }

    /* XXX Method only returns a single connection */
    return new RadioConnection[] { lastConnection };
  }

}

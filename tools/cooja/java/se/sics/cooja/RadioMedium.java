/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: RadioMedium.java,v 1.6 2008/03/18 12:57:04 fros4943 Exp $
 */

package se.sics.cooja;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.*;
import org.jdom.Element;

import se.sics.cooja.interfaces.Radio;

/**
 * The abstract class RadioMedium should be implemented by all COOJA radio
 * mediums. Radios registered in this medium can both send and receive radio
 * data. Depending on the implementation of this interface, more or less
 * accurate radio behaviour imitation is aquired.
 *
 * Often a radio medium, at initialization, registers one or several dynamic
 * plugins. These plugins shows the user some radio medium specific details,
 * such as radio transmission radius etc.
 *
 * @author Fredrik Osterlind
 */
public abstract class RadioMedium {

  /**
   * Registers a mote to this medium.
   *
   * How radio data will be received from and sent to this mote depends on the
   * medium implementation. Common factors may be random, distance from sending
   * to receiving mote and interference with other radio transmitters in some
   * range.
   *
   * @param mote
   *          Mote to register
   * @param sim
   *          Simulation holding mote
   */
  public abstract void registerMote(Mote mote, Simulation sim);

  /**
   * Unregisters a mote from this medium.
   *
   * @param mote
   *          Mote to unregister
   * @param sim
   *          Simulation holding mote
   */
  public abstract void unregisterMote(Mote mote, Simulation sim);

  /**
   * Register a radio to this radio medium.
   *
   * Concerning radio data, this radio will be treated the same way as a mote's
   * radio. This method can be used to add non-mote radio devices, such as a
   * packet generator or a sniffer.
   *
   * @param radio
   *          Radio
   * @param sim
   *          Simulation holding radio
   */
  public abstract void registerRadioInterface(Radio radio, Simulation sim);

  /**
   * Unregister given radio interface from this medium.
   *
   * @param radio
   *          Radio interface to unregister
   * @param sim
   *          Simulation holding radio
   */
  public abstract void unregisterRadioInterface(Radio radio, Simulation sim);

  /**
   * Adds an observer which is notified after the radio connections has been
   * calculated. Often a radio medium is a tick observer and makes these
   * calculations after each tick loop. A radio medium observer may then gather
   * network data by being notified every time the radio medium has delivered
   * data. The radio medium observable MUST notify observers every time the
   * getLastTickConnections returns a new value, even if the new value is null.
   *
   * @see #getLastTickConnections()
   * @see #deleteRadioMediumObserver(Observer)
   * @param observer
   *          New observer
   */
  public abstract void addRadioMediumObserver(Observer observer);

  /**
   * @return Radio medium observable
   */
  public abstract Observable getRadioMediumObservable();

  /**
   * Deletes an radio medium observer.
   *
   * @see #addRadioMediumObserver(Observer)
   * @param observer
   *          Observer to delete
   */
  public abstract void deleteRadioMediumObserver(Observer observer);

  /**
   * Returns all connections made during last tick loop.
   *
   * Typically a radio medium is a tick observer and transfers data between
   * radios after each tick loop. When these calculations are finished, it will
   * in turn notify all radio medium observers. A radio medium observer may get
   * information about which connections were made by using this method. Observe
   * that this method may return null of no connections were made.
   *
   * @see RadioConnection
   * @return All connections made during last tick loop or null if none
   */
  public abstract RadioConnection[] getLastTickConnections();

  /**
   * Returns XML elements representing the current config of this radio medium.
   * This is fetched by the simulator for example when saving a simulation
   * configuration file. For example a radio medium may return user altered
   * range parameters. This method should however not return state specific
   * information such as a current radio status. (All nodes are restarted when
   * loading a simulation.)
   *
   * @see #setConfigXML(Collection, boolean)
   * @return XML elements representing the current radio medium config
   */
  public abstract Collection<Element> getConfigXML();

  /**
   * Sets the current radio medium config depending on the given XML elements.
   *
   * @see #getConfigXML()
   * @param configXML
   *          Config XML elements
   * @return True if config was set successfully, false otherwise
   */
  public abstract boolean setConfigXML(Collection<Element> configXML, boolean visAvailable);

  /**
   * This method creates an instance of the given class with the given
   * simulation constructor argument. Instead of calling the constructors
   * directly this method may be used.
   *
   * @return Radio medium instance
   */
  public static final RadioMedium generateRadioMedium(
      Class<? extends RadioMedium> radioMediumClass, Simulation simulation)
      throws NoSuchMethodException, InvocationTargetException,
      IllegalAccessException, InstantiationException {

    // Generating radio medium
    Constructor constr = radioMediumClass
        .getConstructor(new Class[] { Simulation.class });
    return (RadioMedium) constr.newInstance(new Object[] { simulation });
  }
}

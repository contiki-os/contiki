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
 * $Id: MoteInterface.java,v 1.2 2007/01/09 10:16:42 fros4943 Exp $
 */

package se.sics.cooja;

import java.util.Collection;
import java.util.Observable;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.jdom.Element;

/**
 * A mote interface represents a mote property.
 * Often this is a simulated hardware peripheral such as a button or a led.
 * This can also be a property the mote software itself is unaware of, 
 * for example the current position of the mote.
 * 
 * Interfaces are the main way for the simulator to interact with a simulated mote.
 * 
 * Interfaces are divided into active and passive interfaces, and are handled differently.
 * In order to create a passive interfaces, the class should also implement the dummy Java interface PassiveMoteInterface.
 * For an explanation of the differences of active and passive interfaces see class PassiveMoteInterface.
 * 
 * @see PassiveMoteInterface
 * @author Fredrik Osterlind
 */
public abstract class MoteInterface extends Observable {
  private static Logger logger = Logger.getLogger(MoteInterface.class);

  /**
   * This method creates an instance of the given class with the given mote as constructor
   * argument. Instead of calling the interface constructors directly this method may be used.
   *
   * @param interfaceClass Mote interface class
   * @param mote Mote that will hold the interface
   * @return Mote interface instance
   */
  public static final MoteInterface generateInterface(Class<? extends MoteInterface> interfaceClass, Mote mote) {
    try {
      // Generating interface
      MoteInterface instance = (MoteInterface) interfaceClass.getConstructor(
          new Class[] { Mote.class }).newInstance(new Object[] { mote });
      
      return instance;
    } catch (Exception e) {
      logger.fatal("Exception when creating " + interfaceClass + ": " + e);
      return null;
    }
  }

  /**
   * Actions to be performed just before the holding mote is ticked
   */
  public abstract void doActionsBeforeTick();

  /**
   * Actions to be performed just after the holding mote has been ticked
   */
  public abstract void doActionsAfterTick();

  /**
   * Returns a panel with interesting data for this interface.
   * This could for example show last messages sent/received for
   * a radio interface, or logged message for a log interface.
   * 
   * All panels returned from this method must later be released 
   * for memory reasons. 
   * 
   * If returned panel is null, this interface will not be visualized.
   * 
   * @see #releaseInterfaceVisualizer(JPanel)
   * @return Interface visualizer or null
   */
  public abstract JPanel getInterfaceVisualizer();

  /**
   * This method should be called when a visualizer panel is no longer in use.
   * Any resources of that panel, for example registered observers, will be released.
   * 
   * @see #getInterfaceVisualizer()
   * @param panel A interface visualizer panel fetched earlier for this mote interface.
   */
  public abstract void releaseInterfaceVisualizer(JPanel panel);
  
  /**
   * Returns approximated energy consumed (mQ) during the current tick.
   * If the interface is active, this information must be available after the doActionsAfterTick method.
   * If the interface is passive, this information must be available after the doActionsBeforeTick method.
   * 
   * The interface is responsible to gather information about the current internal state,
   * and calculate whatever energy it needs in that state and during one tick.
   *
   * If the holding mote is dead, this method will not be called.
   * If the holding mote is sleeping and this interface is active, this method will not be called.
   *
   * For example, a radio transmitter or a PIR sensor often has a much higher energy
   * usage than a button sensor which virtually needs no energy at all.
   * If the radio is turned off in hardware, it should return a zero energy consumption.
   * If the radio is sending something which would take longer than one tick, it may either return
   * the total energy used directly, or a smaller amount during several ticks.
   * 
   * This method may typically be used by the passive interface battery, which sums up
   * all energy used during one tick and decreases the battery energy left.
   *
   * @see se.sics.cooja.interfaces.Battery
   * @return Energy consumption of this device during the current tick
   */
  public abstract double energyConsumptionPerTick();

  /**
   * Returns XML elements representing the current config of this mote interface.
   * This is fetched by the simulator for example when saving a simulation configuration file.
   * For example an IP interface may return one element with the mote IP address.
   * This method should however not return state specific information such as a log history.
   * (All nodes are restarted when loading a simulation.)
   * 
   * @see #setConfigXML(Collection)
   * @return XML elements representing the current interface config
   */
  public abstract Collection<Element> getConfigXML();

  /**
   * Sets the current mote interface config depending on the given XML elements.
   * 
   * @see #getConfigXML()
   * @param configXML Config XML elements
   */
  public abstract void setConfigXML(Collection<Element> configXML, boolean visAvailable);
  
}

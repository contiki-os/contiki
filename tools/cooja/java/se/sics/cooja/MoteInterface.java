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
 */

package se.sics.cooja;

import java.util.Collection;
import java.util.Observable;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.interfaces.PolledAfterActiveTicks;
import se.sics.cooja.interfaces.PolledAfterAllTicks;
import se.sics.cooja.interfaces.PolledBeforeActiveTicks;
import se.sics.cooja.interfaces.PolledBeforeAllTicks;

/**
 * A mote interface represents a mote property. Typically, this is a simulated
 * hardware peripheral such as a button or LEDs. This may also be a property
 * that the mote itself is unaware of, for example the current position of
 * the mote.
 *
 * Interfaces are the main way for the simulator to interact with a simulated
 * mote.
 *
 * Each interface can be polled before and after mote ticks.
 * This is controlled by implementing the correct Java interfaces,
 * such as PolledBeforeActiveTicks.
 *
 * @see PolledBeforeActiveTicks
 * @see PolledAfterActiveTicks
 * @see PolledBeforeAllTicks
 * @see PolledAfterAllTicks
 *
 * @author Fredrik Osterlind
 */
public abstract class MoteInterface extends Observable {
  private static Logger logger = Logger.getLogger(MoteInterface.class);

  /**
   * This method creates an instance of the given class with the given mote as
   * constructor argument. Instead of calling the interface constructors
   * directly this method may be used.
   *
   * @param interfaceClass
   *          Mote interface class
   * @param mote
   *          Mote that will hold the interface
   * @return Mote interface instance
   */
  public static final MoteInterface generateInterface(
      Class<? extends MoteInterface> interfaceClass, Mote mote) {
    try {
      MoteInterface instance = interfaceClass.getConstructor(
          new Class[] { Mote.class }).newInstance(new Object[] { mote });

      return instance;
    } catch (Exception e) {
      logger.fatal("Exception when calling constructor of " + interfaceClass, e);
      return null;
    }
  }

  /**
   * Returns a panel visualizing this interface. This could for
   * example show last messages sent/received for a radio interface, or logged
   * message for a log interface.
   *
   * All panels returned from this method must later be released for memory
   * reasons.
   *
   * This method may return null.
   *
   * @see #releaseInterfaceVisualizer(JPanel)
   * @return Interface visualizer or null
   */
  public abstract JPanel getInterfaceVisualizer();

  /**
   * This method should be called when a visualizer panel is no longer in use.
   * Any resources of that panel, for example registered observers, will be
   * released.
   *
   * @see #getInterfaceVisualizer()
   * @param panel
   *          A interface visualizer panel fetched earlier for this mote
   *          interface.
   */
  public abstract void releaseInterfaceVisualizer(JPanel panel);

  /**
   * Returns XML elements representing the current config of this mote
   * interface. This is fetched by the simulator for example when saving a
   * simulation configuration file. For example an IP interface may return one
   * element with the mote IP address. This method should however not return
   * state specific information such as a log history. (All nodes are restarted
   * when loading a simulation.)
   *
   * @see #setConfigXML(Collection, boolean)
   * @return XML elements representing the current interface config
   */
  public abstract Collection<Element> getConfigXML();

  /**
   * Sets the current mote interface config depending on the given XML elements.
   *
   * @see #getConfigXML()
   * @param configXML
   *          Config XML elements
   * @param visAvailable
   *          Is this object allowed to show a visualizer?
   */
  public abstract void setConfigXML(Collection<Element> configXML,
      boolean visAvailable);
  
  /**
   * Called to free resources used by the mote interface.
   * This method is called when the mote is removed from the simulation.
   */
  public void removed() {
  }
  
  /**
   * Called when all mote interfaces have been added to mote.
   */
  public void added() {
  }

}

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.contikios.cooja;

import java.util.Collection;
import org.contikios.cooja.mote.memory.MemoryInterface;

import org.jdom.Element;

/**
 * A simulated mote.
 *
 * All motes have an interface handler, a mote type and a mote memory.
 *
 * @see org.contikios.cooja.MoteInterfaceHandler
 * @see org.contikios.cooja.MoteMemory
 * @see org.contikios.cooja.MoteType
 *
 * @author Fredrik Osterlind
 */
public interface Mote {

  /**
   * @return Unique mote ID
   */
  public int getID();

  /**
   * Returns the interface handler of this mote.
   *
   * @see #setInterfaces(MoteInterfaceHandler)
   * @return Mote interface handler
   */
  public MoteInterfaceHandler getInterfaces();

  /**
   * Returns the memory of this mote.
   *
   * @see #setMemory(MoteMemory)
   * @return Mote memory
   */
  public MemoryInterface getMemory();

  /**
   * Returns mote type.
   *
   * @see #setType(MoteType)
   * @return Mote type
   */
  public MoteType getType();

  /**
   * Returns simulation which holds this mote.
   *
   * @see #setSimulation(Simulation)
   * @return Simulation
   */
  public Simulation getSimulation();

  /**
   * Returns XML elements representing the current config of this mote. This is
   * fetched by the simulator for example when saving a simulation configuration
   * file. For example a mote may return the configs of all its interfaces. This
   * method should however not return state specific information.
   * (All nodes are restarted when loading a simulation.)
   *
   * @see #setConfigXML(Simulation, Collection, boolean)
   * @return XML elements representing the current mote config
   */
  public abstract Collection<Element> getConfigXML();

  /**
   * Sets the current mote config depending on the given XML elements.
   *
   * @param simulation
   *          Simulation holding this mote
   * @param configXML
   *          Config XML elements
   *
   * @see #getConfigXML()
   */
  public abstract boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML, boolean visAvailable);

  /**
   * Called when mote is removed from simulation
   */
  public void removed();

  public void setProperty(String key, Object obj);
  public Object getProperty(String key);
}

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
 */

package org.contikios.cooja;

import java.util.Collection;

import javax.swing.JInternalFrame;

import org.jdom.Element;

/**
 * COOJA plugin. For graphical plugins, extend abstract class VisPlugin.
 * A plugin should also use ClassDecription and PluginType.
 *
 * @see org.contikios.cooja.ClassDescription
 * @see org.contikios.cooja.PluginType
 * @see org.contikios.cooja.VisPlugin
 *
 * @author Fredrik Osterlind
 */
public interface Plugin {

  /**
   * Graphical component of plugin (if any)
   */
  public JInternalFrame getCooja();


  /**
   * This method is called to activate a new plugin, after constructing it.
   * If a simulation is loaded, this method is called after {@link #setConfigXML(Collection, boolean)}.
   * 
   * @see #setConfigXML(Collection, boolean)
   * @see #closePlugin()
   */
  public void startPlugin();

  /**
   * This method is called when an opened plugin is about to close.
   * It should release any resources such as registered observers or
   * opened interface visualizers.
   */
  public void closePlugin();

  /**
   * Returns XML elements representing the current config of this plugin. This
   * is fetched by the simulator for example when saving a simulation
   * configuration file. For example a plugin may return the current size and
   * position. This method should however not return state specific information
   * such as the value of a mote LED, or total number of motes. (All nodes are
   * restarted when loading a simulation.)
   *
   * @see #setConfigXML(Collection, boolean)
   * @return XML elements representing the current radio medium config
   */
  public Collection<Element> getConfigXML();

  /**
   * Sets the current plugin config depending on the given XML elements.
   *
   * @see #getConfigXML()
   * @param configXML
   *          Config XML elements
   * @return True if config was set successfully, false otherwise
   */
  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable);

}

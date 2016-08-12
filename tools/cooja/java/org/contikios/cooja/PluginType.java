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
 * 
 */

package org.contikios.cooja;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Annotation type to identify a plugin type.
 * 
 * @author Fredrik Osterlind
 */
@Retention(RetentionPolicy.RUNTIME)
public @interface PluginType {
  public static final int UNDEFINED_PLUGIN = 0;

  /**
   * Mote Plugin
   * 
   * A mote plugin concerns one specific mote.
   * 
   * An example of such a plugin may be to display some mote information in a
   * frame.
   * 
   * Mote plugins can not be instantiated from the regular menu bar, but are
   * instead started from other plugins, for example a visualizer that let's a
   * user select a mote.
   * 
   * When constructed, a mote plugin is given a mote, the current active
   * simulation and the GUI object.
   * 
   * If the current simulation is removed, so are all instances of this plugin.
   */
  public static final int MOTE_PLUGIN = 1;

  /**
   * Simulation Plugin
   * 
   * A simulation plugin concerns one specific simulation.
   * 
   * An example of such a plugin may be to display number of motes and current
   * simulation time in a window.
   * 
   * Simulation plugins are available via the plugins menubar.
   * 
   * When constructed, a simulation plugin is given the current active
   * simulation and the GUI object.
   * 
   * If the current simulation is removed, so are all instances of this plugin.
   */
  public static final int SIM_PLUGIN = 2;

  /**
   * COOJA Plugin
   * 
   * A COOJA plugin does not depend on the current simulation (if any).
   * 
   * An example of such a plugin may be a control panel where a user can save
   * and load different simulations.
   * 
   * COOJA plugins are available via the plugins menubar.
   * 
   * When constructed, a COOJA plugin is given the current GUI.
   */
  public static final int COOJA_PLUGIN = 3;

  /**
   * Simulation Standard Plugin
   * 
   * This is treated exactly like a Simulation Plugin, with the only difference
   * that this will automatically be opened when a new simulation is created.
   * 
   * @see #SIM_PLUGIN
   */
  public static final int SIM_STANDARD_PLUGIN = 4;

  /**
   * COOJA Standard Plugin
   * 
   * This is treated exactly like a COOJA Plugin, with the only difference that
   * this will automatically be opened when the simulator is started.
   * 
   * @see #COOJA_PLUGIN
   */
  public static final int COOJA_STANDARD_PLUGIN = 5;
  
  /**
   * Simulation Control Plugin
   * 
   * A Simulation Control Plugin indicates control over the simulation. If COOJA
   * is loaded in nogui mode, it will terminate if no controll plugin is present.
   * 
   * COOJA plugins are available via the plugins menubar.
   * 
   * When constructed, a COOJA plugin is given the current GUI.  
   */
  public static final int SIM_CONTROL_PLUGIN = 6;

  int value();
}

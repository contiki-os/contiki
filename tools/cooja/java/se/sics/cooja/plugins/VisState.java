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
 * $Id: VisState.java,v 1.3 2006/10/23 16:15:16 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.util.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.Mote.State;

/**
 * A State Visualizer indicates mote states by painting them in different colors.
 * Active motes are green, sleeping motes are gray and dead motes are read.
 * 
 * The inner color indicates the mote type.
 *
 * A VisState observes both the simulation and all mote states.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("State Visualizer")
@VisPluginType(VisPluginType.SIM_PLUGIN)
public class VisState extends Visualizer2D {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(VisState.class);

  private Simulation simulation;
  
  private static final Color moteTypeColors[] = new Color[] {
    Color.MAGENTA,
    Color.CYAN,
    Color.ORANGE,
    Color.GREEN,
    Color.BLUE,
    Color.RED,
    Color.YELLOW,
  };

  private Observer simObserver = null; // Watches simulation changes
  private Observer stateObserver = null; // Watches mote state changes

  /**
   * Creates a new state visualizer.
   * 
   * @param simulationToVisualize Simulation to visualize
   */
  public VisState(Simulation simulationToVisualize) {
    super(simulationToVisualize);
    setTitle("State Visualizer");
    
    simulation = simulationToVisualize;
    
    // Always observe all motes in simulation
    stateObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        getCurrentCanvas().repaint();
      }
    };
    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        // Register (or reregister) as observer on all motes
        for (int i=0; i < simulation.getMotesCount(); i++) {
          Mote mote = simulation.getMote(i);
          if (mote != null) {
            mote.addStateObserver(stateObserver);
          }
        }
      }
    });
    simObserver.update(null, null);
    
  }
  
  public Color[] getColorOf(Mote mote) {
    Color[] returnColors = new Color[2];
    
    // If mote is sleeping, make outer circle blue 
    if (mote.getState() == Mote.State.LPM)
      returnColors[1] = Color.GRAY;
    
    // If mote is dead, make outer circle red
    else if (mote.getState() == State.DEAD)
      returnColors[1] = Color.RED;
    
    else
      returnColors[1] = Color.GREEN; // make outer circle green
    
    
    // Associate different colors with different mote types
    Vector<MoteType> allTypes = simulation.getMoteTypes();
    int numberOfTypes = allTypes.size();
    
    for (int colCounter=0; colCounter < numberOfTypes && colCounter < moteTypeColors.length; colCounter++) {
      if (mote.getType() == allTypes.get(colCounter)) {
        returnColors[0] = moteTypeColors[colCounter];
        return returnColors;
      }
    } 
    
    returnColors[0] = Color.WHITE;
    return returnColors;
  }

  public void closePlugin() {
    if (simObserver != null) {
      simulation.deleteObserver(simObserver);

      // Delete all state observers
      for (int i=0; i < simulation.getMotesCount(); i++) {
        Mote mote = simulation.getMote(i);
        if (mote != null) {
          mote.deleteStateObserver(stateObserver);
        }
      }
    }

    super.closePlugin();
  }
}

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
 * $Id: VisBattery.java,v 1.4 2007/01/09 09:49:24 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.util.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.Battery;

/**
 * A Battery Visualizer indicates mote energy levels by painting them in
 * different colors. The mote is painted in a grayscale where white is max
 * energy and black is no energy left. If a mote has no battery interface or
 * infinite energy, it is painted blue. If a mote is dead it is painted red.
 * 
 * A VisBattery observers both the simulation and all mote batteries.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Battery Visualizer")
@PluginType(PluginType.SIM_PLUGIN)
public class VisBattery extends Visualizer2D {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(VisBattery.class);

  private Simulation simulation;

  private Observer simObserver = null; // Watches simulation changes
  private Observer batteryObserver = null; // Watches mote battery changes

  /**
   * Creates a new battery visualizer.
   * 
   * @param simulationToVisualize
   *          Simulation to visualize
   */
  public VisBattery(Simulation simulationToVisualize, GUI gui) {
    super(simulationToVisualize, gui);
    setTitle("Battery Visualizer");

    simulation = simulationToVisualize;

    // Always observe all motes in simulation
    batteryObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        getCurrentCanvas().repaint();
      }
    };
    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        // Register (or reregister) as observer on all motes
        for (int i = 0; i < simulation.getMotesCount(); i++) {
          Battery battery = simulation.getMote(i).getInterfaces().getBattery();
          if (battery != null) {
            battery.addObserver(batteryObserver);
          }
        }
      }
    });
    simObserver.update(null, null);

  }

  public Color[] getColorOf(Mote mote) {
    if (mote.getState() == Mote.State.DEAD)
      return new Color[]{Color.RED};

    Battery battery = mote.getInterfaces().getBattery();
    if (battery == null) {
      return new Color[]{Color.BLUE};
    }

    if (battery.hasInfiniteEnergy()) {
      return new Color[]{Color.BLUE};
    }

    double currentEnergy = battery.getCurrentEnergy();

    if (currentEnergy < 0.0) {
      return new Color[]{Color.RED};
    }

    int grayValue = (int) (255 * (currentEnergy / battery.getInitialEnergy()));
    return new Color[]{new Color(grayValue, grayValue, grayValue)};
  }

  public void closePlugin() {
    if (simObserver != null) {
      simulation.deleteObserver(simObserver);

      // Delete all state observers
      for (int i = 0; i < simulation.getMotesCount(); i++) {
        Battery battery = simulation.getMote(i).getInterfaces().getBattery();
        if (battery != null) {
          battery.deleteObserver(batteryObserver);
        }
      }
    }

    super.closePlugin();
  }


}

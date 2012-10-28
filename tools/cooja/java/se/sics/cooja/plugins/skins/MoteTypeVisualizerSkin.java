/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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

package se.sics.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;
import se.sics.cooja.plugins.Visualizer.MoteMenuAction;

/**
 * Mote type visualizer skin.
 * 
 * Colors motes according to their mote type.
 * Adds a mote menu option to remove all motes of a particular mote type.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Mote type")
public class MoteTypeVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(MoteTypeVisualizerSkin.class);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  private static final Color[][] COLORS = new Color[][] {
    new Color[] {Color.GREEN},
    new Color[] {Color.ORANGE},
    new Color[] {Color.MAGENTA},
    new Color[] {Color.YELLOW},
    new Color[] {Color.CYAN},
    new Color[] {Color.BLUE},
    new Color[] {Color.RED},
  };
  
  public void setActive(Simulation simulation, Visualizer vis) {
    this.simulation = simulation;
    this.visualizer = vis;

    /* Register menu actions */
    visualizer.registerMoteMenuAction(DeleteAllAction.class);
  }

  public void setInactive() {
    /* Unregister menu actions */
    visualizer.unregisterMoteMenuAction(DeleteAllAction.class);
  }

  public Color[] getColorOf(Mote mote) {
    MoteType[] types = simulation.getMoteTypes();
    MoteType type = mote.getType();
    for (int i=0; i < COLORS.length; i++) {
      if (types[i] == type) {
        return COLORS[i];
      }
    }
    return null;
  }

  public void paintBeforeMotes(Graphics g) {
  }

  public void paintAfterMotes(Graphics g) {
  }

  public static class DeleteAllAction implements MoteMenuAction {
    public boolean isEnabled(Visualizer visualizer, Mote mote) {
      return true;
    }

    public String getDescription(Visualizer visualizer, Mote mote) {
      return "Delete all motes of type: " + mote.getType().getDescription();
    }

    public void doAction(Visualizer visualizer, Mote mote) {
      /* Remove all motes of this type */
      /* TODO Confirm? */
      Simulation simulation = mote.getSimulation();
      Mote[] motes = simulation.getMotes();
      for (Mote m: motes) {
        if (m.getType() == mote.getType()) {
          simulation.removeMote(m);
        }
      }
    }
  };

  public Visualizer getVisualizer() {
    return visualizer;
  }
}

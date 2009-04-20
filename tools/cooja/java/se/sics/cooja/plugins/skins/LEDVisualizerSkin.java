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
 * $Id: LEDVisualizerSkin.java,v 1.3 2009/04/20 16:16:25 fros4943 Exp $
 */

package se.sics.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.util.Observable;
import java.util.Observer;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.LED;
import se.sics.cooja.interfaces.MoteID;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;

/**
 * Visualizer skin for LEDs.
 *
 * Paints three LEDs left to each mote.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("LEDs")
public class LEDVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(LEDVisualizerSkin.class);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  private Observer ledObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      visualizer.repaint();
    }
  };
  private Observer simObserver = new Observer() {
    public void update(Observable obs, Object obj) {

      /* Observe LEDs */
      for (Mote mote: simulation.getMotes()) {
        LED led = mote.getInterfaces().getLED();
        if (led != null) {
          led.addObserver(ledObserver);
        }
      }
      visualizer.repaint();
    }
  };

  public void setActive(Simulation simulation, Visualizer vis) {
    this.simulation = simulation;
    this.visualizer = vis;

    simulation.addObserver(simObserver);
    simObserver.update(null, null);
  }

  public void setInactive() {
    simulation.deleteObserver(simObserver);
    for (Mote mote: simulation.getMotes()) {
      LED led = mote.getInterfaces().getLED();
      if (led != null) {
        led.deleteObserver(ledObserver);
      }
    }
  }

  public Color[] getColorOf(Mote mote) {
    return null;
  }

  public void paintSkin(Graphics g) {
    /* Paint LEDs left of each mote */
    Mote[] allMotes = simulation.getMotes();
    for (Mote mote: allMotes) {
      LED leds = mote.getInterfaces().getLED();
      if (leds == null) {
        continue;
      }

      Position pos = mote.getInterfaces().getPosition();
      Point pixel = visualizer.transformPositionToPixel(pos);

      int x = pixel.x - 2*Visualizer.MOTE_RADIUS;
      
      int y = pixel.y - Visualizer.MOTE_RADIUS;
      g.setColor(Color.RED);
      if (leds.isRedOn()) {
        g.fillRect(x, y, 7, 4);
      } else {
        g.drawRect(x, y, 7, 4);
      }

      y += 6;
      g.setColor(Color.GREEN);
      if (leds.isGreenOn()) {
        g.fillRect(x, y, 7, 4);
      } else {
        g.drawRect(x, y, 7, 4);
      }

      y += 6;
      g.setColor(Color.BLUE);
      if (leds.isYellowOn()) {
        g.fillRect(x, y, 7, 4);
      } else {
        g.drawRect(x, y, 7, 4);
      }

    }

    g.setColor(Color.BLACK);
  }

  public Visualizer getVisualizer() {
    return visualizer;
  }
}

/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
package org.contikios.cooja.plugins.skins;

import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Point;
import java.util.Set;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.SupportedArguments;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.plugins.Visualizer;
import org.contikios.cooja.plugins.VisualizerSkin;
import org.contikios.cooja.radiomediums.DGRMDestinationRadio;
import org.contikios.cooja.radiomediums.DestinationRadio;
import org.contikios.cooja.radiomediums.DirectedGraphMedium;

@ClassDescription("Radio environment (DGRM)")
@SupportedArguments(radioMediums = {DirectedGraphMedium.class})
public class DGRMVisualizerSkin implements VisualizerSkin {

  private static final Logger logger = Logger.getLogger(DGRMVisualizerSkin.class);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  @Override
  public void setActive(Simulation simulation, Visualizer vis) {
    if (!(simulation.getRadioMedium() instanceof DirectedGraphMedium)) {
      logger.fatal("Cannot activate DGRM skin for unknown radio medium: " + simulation.getRadioMedium());
      return;
    }
    this.simulation = simulation;
    this.visualizer = vis;
  }

  @Override
  public void setInactive() {
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }
  }

  @Override
  public Color[] getColorOf(Mote mote) {
    if (visualizer.getSelectedMotes().contains(mote)) {
      return new Color[]{Color.CYAN};
    }
    return null;
  }

  @Override
  public void paintBeforeMotes(Graphics g) {
    Set<Mote> selectedMotes = visualizer.getSelectedMotes();
    if (simulation == null || selectedMotes == null) {
      return;
    }

    for (final Mote selectedMote : selectedMotes) {
      if (selectedMote.getInterfaces().getRadio() == null) {
        continue;
      }

      /* Paint transmission and interference range for selected mote */
      Position motePos = selectedMote.getInterfaces().getPosition();

      Point pixelCoord = visualizer.transformPositionToPixel(motePos);
      int x = pixelCoord.x;
      int y = pixelCoord.y;
      Radio selectedRadio = selectedMote.getInterfaces().getRadio();

      FontMetrics fm = g.getFontMetrics();
      g.setColor(Color.BLACK);

      DirectedGraphMedium radioMedium = (DirectedGraphMedium) simulation.getRadioMedium();

      /* Print transmission success probabilities */
      DestinationRadio[] dests = radioMedium.getPotentialDestinations(selectedRadio);
      if (dests == null || dests.length == 0) {
        String msg = "No edges";
        int msgWidth = fm.stringWidth(msg);
        g.setColor(Color.BLACK);
        g.drawString(msg, x - msgWidth / 2, y + 2 * Visualizer.MOTE_RADIUS + 3);
        continue;
      }
      String msg = dests.length + " edges";
      int msgWidth = fm.stringWidth(msg);
      g.setColor(Color.BLACK);
      g.drawString(msg, x - msgWidth / 2, y + 2 * Visualizer.MOTE_RADIUS + 3);
      /* Draw LQI/RSSI edges */
      for (DestinationRadio r : dests) {
        double prob = ((DGRMDestinationRadio) r).ratio;
        double rssi = ((DGRMDestinationRadio) r).signal;
        double pos_rssi = rssi + 100;
        int lqi = ((DGRMDestinationRadio) r).lqi;
        float red = (float) (1 - prob * pos_rssi / 90 * lqi / 100);
        if (red > 1) {
          red = 1;
        }
        if (red < 0) {
          red = 0;
        }
        float green = (float) (prob * pos_rssi / 90 * lqi / 100);
        if (green > 1) {
          green = 1;
        }
        if (green < 0) {
          green = 0;
        }
        if (prob == 0.0d) {
          continue;
        }
        Position pos = r.radio.getPosition();
        Point pixel = visualizer.transformPositionToPixel(pos);
        g.setColor(new Color(red, green, 0.0f));
        g.drawString("LQI:	" + lqi, (x + pixel.x) / 2, (y + pixel.y) / 2);
        g.drawString("RSSI: " + rssi, (x + pixel.x) / 2, (y + pixel.y) / 2 + g.getFontMetrics().getHeight());
        g.drawLine(x, y, pixel.x, pixel.y);
        /* Draw success ratio only if single mote selected */
        if (selectedMotes.size() == 1) {
          g.setColor(Color.BLACK);
          msg = String.format("%1.1f%%", 100.0 * prob);
          msgWidth = fm.stringWidth(msg);
          g.drawString(msg, pixel.x - msgWidth / 2, pixel.y + 2 * Visualizer.MOTE_RADIUS + 3);
        }
      }
    }
  }

  @Override
  public void paintAfterMotes(Graphics g) {
  }

  @Override
  public Visualizer getVisualizer() {
    return visualizer;
  }
}

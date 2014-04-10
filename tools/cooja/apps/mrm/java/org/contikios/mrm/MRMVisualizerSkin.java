/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */
package org.contikios.mrm;

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
import org.contikios.mrm.ChannelModel.RadioPair;
import org.contikios.mrm.ChannelModel.TxPair;

@ClassDescription("Radio environment (MRM)")
@SupportedArguments(radioMediums = {MRM.class})
public class MRMVisualizerSkin implements VisualizerSkin {

  private static final Logger logger = Logger.getLogger(MRMVisualizerSkin.class);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  @Override
  public void setActive(Simulation simulation, Visualizer vis) {
    if (!(simulation.getRadioMedium() instanceof MRM)) {
      logger.fatal("Cannot activate MRM skin for unknown radio medium: " + simulation.getRadioMedium());
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
      final Position sPos = selectedMote.getInterfaces().getPosition();

      /* Paint transmission and interference range for selected mote */
      Position motePos = selectedMote.getInterfaces().getPosition();

      Point pixelCoord = visualizer.transformPositionToPixel(motePos);
      int x = pixelCoord.x;
      int y = pixelCoord.y;

      FontMetrics fm = g.getFontMetrics();
      g.setColor(Color.BLACK);

      MRM radioMedium = (MRM) simulation.getRadioMedium();

      /* Print transmission success probabilities */
      Mote[] dests = simulation.getMotes();
      if (dests == null || dests.length == 0) {
        String msg = "No edges";
        int msgWidth = fm.stringWidth(msg);
        g.setColor(Color.BLACK);
        g.drawString(msg, x - msgWidth / 2, y + 2 * Visualizer.MOTE_RADIUS + 3);
        return;
      }
      g.setColor(Color.BLACK);
      int edges = 0;
      for (Mote d : dests) {
        if (d == selectedMote) {
          continue;
        }
        final Radio dRadio = d.getInterfaces().getRadio();
        TxPair txPair = new RadioPair() {
          @Override
          public Radio getFromRadio() {
            return selectedMote.getInterfaces().getRadio();
          }

          @Override
          public Radio getToRadio() {
            return dRadio;
          }
        };
        double probArr[] = radioMedium.getChannelModel().getProbability(
                txPair,
                Double.NEGATIVE_INFINITY
        );
        double prob = probArr[0];
        double ss = probArr[1];

        if (prob == 0.0d) {
          continue;
        }
        edges++;
        String msg = String.format("%1.1f%%, %1.2fdB", 100.0 * prob, ss);
        Point pixel = visualizer.transformPositionToPixel(d.getInterfaces().getPosition());
        int msgWidth = fm.stringWidth(msg);
        g.setColor(new Color(1 - (float) prob, (float) prob, 0.0f));
        g.drawLine(x, y, pixel.x, pixel.y);
        g.setColor(Color.BLACK);
        g.drawString(msg, pixel.x - msgWidth / 2, pixel.y + 2 * Visualizer.MOTE_RADIUS + 3);
      }

      String msg = dests.length + " edges";
      int msgWidth = fm.stringWidth(msg);
      g.setColor(Color.BLACK);
      g.drawString(msg, x - msgWidth / 2, y + 2 * Visualizer.MOTE_RADIUS + 3);
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

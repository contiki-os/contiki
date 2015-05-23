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

package org.contikios.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.plugins.Visualizer;
import org.contikios.cooja.plugins.VisualizerSkin;

/**
 * Background grid visualizer skin.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("10m background grid")
public class GridVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(GridVisualizerSkin.class);

  private Visualizer visualizer = null;

  public void setActive(Simulation simulation, Visualizer vis) {
    this.visualizer = vis;
  }

  public void setInactive() {
  }

  public Color[] getColorOf(Mote mote) {
    return null;
  }

  public void paintBeforeMotes(Graphics g) {

    /* Background grid every 10 meters */
    Position upperLeft = 
      visualizer.transformPixelToPosition(-10, -10);
    upperLeft.setCoordinates(
        ((int)(upperLeft.getXCoordinate()/10))*10,
        ((int)(upperLeft.getYCoordinate()/10))*10,
        0);
    Position lowerRight = 
      visualizer.transformPixelToPosition(visualizer.getWidth(), visualizer.getHeight());
    lowerRight.setCoordinates(
        ((int)(lowerRight.getXCoordinate()/10))*10,
        ((int)(lowerRight.getYCoordinate()/10))*10,
        0);

    if ((lowerRight.getXCoordinate() - upperLeft.getXCoordinate())/10.0 < 200 &&
        (lowerRight.getYCoordinate() - upperLeft.getYCoordinate())/10.0 < 200) {
      /* X axis */
      for (double x = upperLeft.getXCoordinate(); x <= lowerRight.getXCoordinate(); x += 10.0) {
        int pixel = visualizer.transformPositionToPixel(x, 0, 0).x;
        if (x % 100 == 0) {
          g.setColor(Color.GRAY);
        } else {
          g.setColor(Color.LIGHT_GRAY);
        }
        g.drawLine(
            pixel,
            0,
            pixel,
            visualizer.getHeight()
        );
      }
      /* Y axis */
      for (double y = upperLeft.getYCoordinate(); y <= lowerRight.getYCoordinate(); y += 10.0) {
        int pixel = visualizer.transformPositionToPixel(0, y, 0).y;
        if (y % 100 == 0) {
          g.setColor(Color.GRAY);
        } else {
          g.setColor(Color.LIGHT_GRAY);
        }
        g.drawLine(
            0,
            pixel,
            visualizer.getWidth(),
            pixel
        );
      }
    }
  }

  public void paintAfterMotes(Graphics g) {
  }

  public Visualizer getVisualizer() {
    return visualizer;
  }
}

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
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Point;
import java.util.Observable;
import java.util.Observer;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.Simulation;
import se.sics.cooja.SimEventCentral.LogOutputEvent;
import se.sics.cooja.SimEventCentral.LogOutputListener;
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.SerialPort;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;

/**
 * Visualizer skin for Log output.
 *
 * Paints the last log message above each mote.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Log output: printf()'s")
public class LogVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(LogVisualizerSkin.class);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  private LogOutputListener logOutputListener = new LogOutputListener() {
    public void moteWasAdded(Mote mote) {
      visualizer.repaint();
    }
    public void moteWasRemoved(Mote mote) {
      visualizer.repaint();
    }
    public void newLogOutput(LogOutputEvent ev) {
      visualizer.repaint();
    }
    public void removedLogOutput(LogOutputEvent ev) {
    }
  };

  public void setActive(Simulation simulation, Visualizer vis) {
    this.simulation = simulation;
    this.visualizer = vis;

    simulation.getEventCentral().addLogOutputListener(logOutputListener);
  }

  public void setInactive() {
    simulation.getEventCentral().removeLogOutputListener(logOutputListener);
  }

  public Color[] getColorOf(Mote mote) {
    return null;
  }

  public void paintBeforeMotes(Graphics g) {
  }

  public void paintAfterMotes(Graphics g) {
    FontMetrics fm = g.getFontMetrics();
    g.setColor(Color.BLACK);

    /* Paint last output below motes */
    Mote[] allMotes = simulation.getMotes();
    for (Mote mote: allMotes) {
      String msg = null;
      for (MoteInterface mi: mote.getInterfaces().getInterfaces()) {
        if (!(mi instanceof Log)) {
          continue;
        }
        Log log = (Log) mi;
        if (log.getLastLogMessage() == null) {
          continue;
        }
        msg = log.getLastLogMessage();
      }
      if (msg == null) {
        continue;
      }

      Position pos = mote.getInterfaces().getPosition();
      Point pixel = visualizer.transformPositionToPixel(pos);

      int msgWidth = fm.stringWidth(msg);
      g.drawString(msg, pixel.x - msgWidth/2, pixel.y - Visualizer.MOTE_RADIUS);
    }
  }

  public Visualizer getVisualizer() {
    return visualizer;
  }
}

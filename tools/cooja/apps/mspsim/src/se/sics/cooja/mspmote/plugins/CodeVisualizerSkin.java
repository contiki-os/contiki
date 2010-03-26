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
 * $Id: CodeVisualizerSkin.java,v 1.2 2010/03/26 12:29:11 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Observable;
import java.util.Observer;

import javax.swing.Timer;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteType;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.util.DebugInfo;

/**
 * Code visualizer skin for MSPSim motes.
 *
 * Writes the current code line below each mote.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Code")
public class CodeVisualizerSkin implements VisualizerSkin {
  private Simulation simulation = null;
  private Visualizer visualizer = null;

  private Timer repaintTimer = new Timer(100, new ActionListener() {
    public void actionPerformed(ActionEvent e) {
      if (simulation.isRunning()) {
        visualizer.repaint();
      }
    }
  });

  private Observer simulationObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      visualizer.repaint();
    }
  };

  public void setActive(Simulation simulation, Visualizer vis) {
    this.simulation = simulation;
    this.visualizer = vis;
    simulation.addObserver(simulationObserver);
    repaintTimer.start();
  }

  public void setInactive() {
    simulation.deleteObserver(simulationObserver);
    repaintTimer.stop();
  }

  public Color[] getColorOf(Mote mote) {
    return null;
  }

  public void paintBeforeMotes(Graphics g) {
  }

  private static String getMoteString(Mote mote) {
    if (!(mote instanceof MspMote)) {
      return null;
    }
    try {
      DebugInfo debugInfo = 
        ((MspMoteType)mote.getType()).getELF().getDebugInfo(((MspMote)mote).getCPU().reg[MSP430.PC]);
      if (debugInfo == null) {
        return null;
      }
      return debugInfo.getFile() + ":" + debugInfo.getLine();
    } catch (Exception e) {
      return "?";
    }
  }

  public void paintAfterMotes(Graphics g) {
    FontMetrics fm = g.getFontMetrics();
    g.setColor(Color.BLACK);

    Mote[] motes = simulation.getMotes();
    for (Mote mote: motes) {
      String msg = getMoteString(mote);
      if (msg == null) {
        continue;
      }
      Position pos = mote.getInterfaces().getPosition();
      Point pixel = visualizer.transformPositionToPixel(pos);
      int msgWidth = fm.stringWidth(msg);
      g.drawString(msg, pixel.x - msgWidth/2, pixel.y + 2*Visualizer.MOTE_RADIUS + 3);
    }
  }

  public Visualizer getVisualizer() {
    return visualizer;
  }
}

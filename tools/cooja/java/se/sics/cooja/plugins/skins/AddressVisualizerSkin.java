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
 * $Id: AddressVisualizerSkin.java,v 1.1 2009/04/20 16:15:47 fros4943 Exp $
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
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.RimeAddress;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;

/**
 * Visualizer skin for mote addresses.
 *
 * Paints the address below each mote.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Addresses: IP or Rime")
public class AddressVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(AddressVisualizerSkin.class);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  private Observer addrObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      visualizer.repaint();
    }
  };
  private Observer simObserver = new Observer() {
    public void update(Observable obs, Object obj) {

      /* Observe Rime and IP addresses */
      for (Mote mote: simulation.getMotes()) {
        IPAddress ipAddr = mote.getInterfaces().getIPAddress();
        if (ipAddr != null) {
          ipAddr.addObserver(addrObserver);
        }
        RimeAddress rimeAddr = mote.getInterfaces().getRimeAddress();
        if (rimeAddr != null) {
          rimeAddr.addObserver(addrObserver);
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
      IPAddress ipAddr = mote.getInterfaces().getIPAddress();
      if (ipAddr != null) {
        ipAddr.deleteObserver(addrObserver);
      }
      RimeAddress rimeAddr = mote.getInterfaces().getRimeAddress();
      if (rimeAddr != null) {
        rimeAddr.deleteObserver(addrObserver);
      }
    }
  }

  public Color[] getColorOf(Mote mote) {
    return null;
  }

  public void paintSkin(Graphics g) {
    FontMetrics fm = g.getFontMetrics();

    /* Paint last output below motes */
    Mote[] allMotes = simulation.getMotes();
    for (Mote mote: allMotes) {
      String msg = null;
      {
        IPAddress ipAddr = mote.getInterfaces().getIPAddress();
        if (ipAddr != null) {
          msg = ipAddr.getIPString();
        }
      }

      if (msg == null) {
        RimeAddress rimeAddr = mote.getInterfaces().getRimeAddress();
        if (rimeAddr != null) {
          msg = rimeAddr.getAddressString();
        }
      }
      
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

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
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.util.Observable;
import java.util.Observer;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.SimEventCentral.MoteCountListener;
import org.contikios.cooja.interfaces.IPAddress;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.RimeAddress;
import org.contikios.cooja.plugins.Visualizer;
import org.contikios.cooja.plugins.VisualizerSkin;
import org.contikios.cooja.plugins.Visualizer.MoteMenuAction;

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
  private MoteCountListener newMotesListener = new MoteCountListener() {
    public void moteWasAdded(Mote mote) {
      IPAddress ipAddr = mote.getInterfaces().getIPAddress();
      if (ipAddr != null) {
        ipAddr.addObserver(addrObserver);
      }
      RimeAddress rimeAddr = mote.getInterfaces().getRimeAddress();
      if (rimeAddr != null) {
        rimeAddr.addObserver(addrObserver);
      }
    }
    public void moteWasRemoved(Mote mote) {
      IPAddress ipAddr = mote.getInterfaces().getIPAddress();
      if (ipAddr != null) {
        ipAddr.deleteObserver(addrObserver);
      }
      RimeAddress rimeAddr = mote.getInterfaces().getRimeAddress();
      if (rimeAddr != null) {
        rimeAddr.deleteObserver(addrObserver);
      }
    }
  };

  public void setActive(Simulation simulation, Visualizer vis) {
    this.simulation = simulation;
    this.visualizer = vis;

    simulation.getEventCentral().addMoteCountListener(newMotesListener);
    for (Mote m: simulation.getMotes()) {
      newMotesListener.moteWasAdded(m);
    }
    
    /* Register menu actions */
    visualizer.registerMoteMenuAction(CopyAddressAction.class);
  }

  public void setInactive() {
    simulation.getEventCentral().removeMoteCountListener(newMotesListener);
    for (Mote m: simulation.getMotes()) {
      newMotesListener.moteWasRemoved(m);
    }

    /* Unregister menu actions */
    visualizer.unregisterMoteMenuAction(CopyAddressAction.class);
  }

  public Color[] getColorOf(Mote mote) {
    return null;
  }

  public void paintBeforeMotes(Graphics g) {
  }

  private static String getMoteString(Mote mote) {
    IPAddress ipAddr = mote.getInterfaces().getIPAddress();
    if ((ipAddr != null) && (ipAddr.hasIP())) {
      if (ipAddr.getLocalIP() == null) {
        return "";
      }
      return ipAddr.getLocalIP().toString();
    }

    RimeAddress rimeAddr = mote.getInterfaces().getRimeAddress();
    if (rimeAddr != null) {
      return rimeAddr.getAddressString();
    }
    return null;
  }
  
  public void paintAfterMotes(Graphics g) {
    FontMetrics fm = g.getFontMetrics();
    g.setColor(Color.BLACK);

    /* Paint last output below motes */
    Mote[] allMotes = simulation.getMotes();
    for (Mote mote: allMotes) {
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

  public static class CopyAddressAction implements MoteMenuAction {
    public boolean isEnabled(Visualizer visualizer, Mote mote) {
      return true;
    }

    public String getDescription(Visualizer visualizer, Mote mote) {
      return "Copy address to clipboard: \"" + getMoteString(mote) + "\"";
    }

    public void doAction(Visualizer visualizer, Mote mote) {
      Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
      StringSelection stringSelection = new StringSelection(getMoteString(mote));
      clipboard.setContents(stringSelection, null);
    }
  };
  public Visualizer getVisualizer() {
    return visualizer;
  }
}

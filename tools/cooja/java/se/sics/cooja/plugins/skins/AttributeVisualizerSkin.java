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
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.interfaces.MoteAttributes;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;

/**
 * Visualizer skin for mote attributes.
 *
 * @see MoteAttributes
 * @author Fredrik Osterlind
 */
@ClassDescription("Mote attributes")
public class AttributeVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(AttributeVisualizerSkin.class);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  private Observer attributesObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      visualizer.repaint();
    }
  };
  private MoteCountListener newMotesListener = new MoteCountListener() {
    public void moteWasAdded(Mote mote) {
      MoteAttributes intf = mote.getInterfaces().getInterfaceOfType(MoteAttributes.class);
      if (intf != null) {
        intf.addObserver(attributesObserver);
      }
    }
    public void moteWasRemoved(Mote mote) {
      MoteAttributes intf = mote.getInterfaces().getInterfaceOfType(MoteAttributes.class);
      if (intf != null) {
        intf.deleteObserver(attributesObserver);
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
  }

  public void setInactive() {
    simulation.getEventCentral().removeMoteCountListener(newMotesListener);
    for (Mote m: simulation.getMotes()) {
      newMotesListener.moteWasRemoved(m);
    }
  }

  public Color[] getColorOf(Mote mote) {
    String[] as = getAttributesStrings(mote);
    if (as == null) {
      return null;
    }

    Color color = null;
    for (String a: as) {
      if (a.startsWith("color=")) {
        String colorString = a.substring("color=".length());
        color = parseAttributeColor(colorString);
      }
    }
    if (color == null) {
      return null;
    }
    return new Color[] { color };
  }

  private Color parseAttributeColor(String colorString) {
    if (colorString.equalsIgnoreCase("red")) {
      return Color.RED;
    } else if (colorString.equalsIgnoreCase("green")) {
      return Color.GREEN;
    } else if (colorString.equalsIgnoreCase("blue")) {
      return Color.BLUE;
    } else if (colorString.equalsIgnoreCase("orange")) {
      return Color.ORANGE;
    } else if (colorString.equalsIgnoreCase("pink")) {
      return Color.PINK;
    } else {
      try {
        return Color.decode(colorString);
      } catch (NumberFormatException e) {
      }
      logger.warn("Unknown color attribute: " + colorString);
      return null;
    }
  }
  public void paintBeforeMotes(Graphics g) {
  }

  private static String[] getAttributesStrings(Mote mote) {
    MoteAttributes intf = mote.getInterfaces().getInterfaceOfType(MoteAttributes.class);
    if (intf == null) {
      return null;
    }
    String text = intf.getText();
    if (text == null) {
      return null;
    }
    
    return text.split("\n");
  }
  
  public void paintAfterMotes(Graphics g) {
    FontMetrics fm = g.getFontMetrics();
    g.setColor(Color.BLACK);

    /* Paint attributes below motes */
    Mote[] allMotes = simulation.getMotes();
    for (Mote mote: allMotes) {
      String[] as = getAttributesStrings(mote);
      if (as == null) {
        continue;
      }
      
      Position pos = mote.getInterfaces().getPosition();
      Point pixel = visualizer.transformPositionToPixel(pos);

      int y = pixel.y + 2*Visualizer.MOTE_RADIUS + 3;
      for (String a: as) {
        if (a.startsWith("color=")) {
          /* Ignore */
          continue;
        }

        Color color = null;
        if (a.contains(";")) {
          String[] args = a.split(";");
          color = parseAttributeColor(args[1]);
          a = args[0];
        }
        if (color != null) {
          g.setColor(color);
        }
        
        int msgWidth = fm.stringWidth(a);
        g.drawString(a, pixel.x - msgWidth/2, y);
        y += fm.getHeight();

        if (color != null) {
          g.setColor(Color.BLACK);
        }
      }
    }
  }

  public Visualizer getVisualizer() {
    return visualizer;
  }
}

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
 * $Id: TrafficVisualizerSkin.java,v 1.5 2010/02/26 07:46:26 nifi Exp $
 */

package se.sics.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Polygon;
import java.util.ArrayDeque;
import java.util.Observable;
import java.util.Observer;

import javax.swing.Box;
import javax.swing.JLabel;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.RadioConnection;
import se.sics.cooja.Simulation;
import se.sics.cooja.SimEventCentral.MoteCountListener;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;
import se.sics.cooja.plugins.Visualizer.SimulationMenuAction;
import se.sics.cooja.radiomediums.AbstractRadioMedium;

/**
 * Radio traffic visualizer skin.
 * 
 * Transmitting motes are painted blue. Receiving motes are painted green.
 * Interfered motes are painted red. Motes without radios are painted gray. All
 * other motes are painted white.
 * 
 * In contrast to the {@link UDGMVisualizerSkin}, this skin listens to all mote
 * radios, not to the radio medium. The radio traffic skin also displays a
 * history.
 * 
 * @see UDGMVisualizerSkin
 * @author Fredrik Osterlind
 */
@ClassDescription("Radio traffic")
public class TrafficVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(TrafficVisualizerSkin.class);
  
  private Simulation simulation = null;
  private Visualizer visualizer = null;

  private Box counters;

  private final int MAX_HISTORY_SIZE = 200;
  private boolean showHistory = true;
  private ArrayDeque<RadioConnectionArrow> history = new ArrayDeque<RadioConnectionArrow>();

  private AbstractRadioMedium radioMedium;
  private Observer radioObserver, radioMediumObserver;

  public void setActive(final Simulation simulation, Visualizer vis) {
    if (!(simulation.getRadioMedium() instanceof AbstractRadioMedium)) {
      logger.fatal("Radio medium type not supported: "
          + simulation.getRadioMedium());
      return;
    }
    this.radioMedium = (AbstractRadioMedium) simulation.getRadioMedium();
    this.simulation = simulation;
    this.visualizer = vis;

    final JLabel txCounter = new JLabel("TX: " + radioMedium.COUNTER_TX);
    final JLabel rxCounter = new JLabel("RX: " + radioMedium.COUNTER_RX);
    final JLabel interferedCounter = new JLabel("INT: "
        + radioMedium.COUNTER_INTERFERED);

    counters = Box.createHorizontalBox();
    counters.add(txCounter);
    counters.add(Box.createHorizontalStrut(10));
    counters.add(rxCounter);
    counters.add(Box.createHorizontalStrut(10));
    counters.add(interferedCounter);

    /* visualizer.getCurrentCanvas().add(counters); */

    /* Start observing radio medium and radios */
    radioMedium.addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        txCounter.setText("TX: " + radioMedium.COUNTER_TX);
        rxCounter.setText("RX: " + radioMedium.COUNTER_RX);
        interferedCounter.setText("INT: " + +radioMedium.COUNTER_INTERFERED);

        if (showHistory) {
          RadioConnection last = radioMedium.getLastConnection();
          if (last != null && history.size() < MAX_HISTORY_SIZE) {
            history.add(new RadioConnectionArrow(last));
          }
        }
        visualizer.repaint();
      }
    });
    radioObserver = new Observer() {
      public void update(Observable o, Object arg) {
        visualizer.repaint();
      }
    };
    simulation.getEventCentral().addMoteCountListener(new MoteCountListener() {
      public void moteWasAdded(Mote mote) {
        Radio r = mote.getInterfaces().getRadio();
        if (r != null) {
          r.addObserver(radioObserver);
        }
      }

      public void moteWasRemoved(Mote mote) {
        Radio r = mote.getInterfaces().getRadio();
        if (r != null) {
          r.deleteObserver(radioObserver);
        }
        history.clear();
      }
    });
    for (Mote mote : simulation.getMotes()) {
      Radio r = mote.getInterfaces().getRadio();
      if (r != null) {
        r.addObserver(radioObserver);
      }
    }

    simulation.addMillisecondObserver(new Observer() {
      public void update(Observable obs, Object obj) {
        if((simulation.getSimulationTimeMillis() % 100) == 0) {
          RadioConnectionArrow[] historyArr = history.toArray(new RadioConnectionArrow[0]);
          if(historyArr.length > 0) {
            visualizer.repaint();
          }
          for (RadioConnectionArrow connArrow : historyArr) {
            if (connArrow == null) {
              continue;
            }
            connArrow.increaseAge();
            if(connArrow.getAge() >= connArrow.getMaxAge()) {
              history.remove(connArrow);            
            }
          }
        }
        
      }
    });
    /* Register menu actions */
    visualizer.registerSimulationMenuAction(ToggleHistoryAction.class);
  }

  public void setInactive() {
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }

    visualizer.getCurrentCanvas().remove(counters);

    /* Stop observing radio medium and radios */
    radioMedium.deleteRadioMediumObserver(radioMediumObserver);
    for (Mote mote : simulation.getMotes()) {
      Radio r = mote.getInterfaces().getRadio();
      if (r != null) {
        r.addObserver(radioObserver);
      }
    }

    /* Unregister menu actions */
    visualizer.unregisterSimulationMenuAction(ToggleHistoryAction.class);
  }

  public Color[] getColorOf(Mote mote) {
    return null;
  }

  private Polygon arrowPoly = new Polygon();
  private void drawArrow(Graphics g, int xSource, int ySource, int xDest, int yDest, int delta) {
    double dx = xSource - xDest;
    double dy = ySource - yDest;
    double dir = Math.atan2(dx, dy);
    double len = Math.sqrt(dx * dx + dy * dy);
    dx /= len;
    dy /= len;
    len -= delta;
    xDest = xSource - (int) (dx * len);
    yDest = ySource - (int) (dy * len);
    g.drawLine(xDest, yDest, xSource, ySource);

    final int size = 8;
    arrowPoly.reset();
    arrowPoly.addPoint(xDest, yDest);
    arrowPoly.addPoint(xDest + xCor(size, dir + 0.5), yDest + yCor(size, dir + 0.5));
    arrowPoly.addPoint(xDest + xCor(size, dir - 0.5), yDest + yCor(size, dir - 0.5));
    arrowPoly.addPoint(xDest, yDest);
    g.fillPolygon(arrowPoly);
  }

  private int yCor(int len, double dir) {
    return (int)(0.5 + len * Math.cos(dir));
  }

  private int xCor(int len, double dir) {
    return (int)(0.5 + len * Math.sin(dir));
  }
  public void paintBeforeMotes(Graphics g) {
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }

    if (showHistory) {
      /* Paint history in gray */
      RadioConnectionArrow[] historyArr = history.toArray(new RadioConnectionArrow[0]);
      for (RadioConnectionArrow connArrow : historyArr) {
        if (connArrow == null) {
          continue;
        }
        float colorHistoryIndex = (float)connArrow.getAge() / (float)connArrow.getMaxAge();
        g.setColor(new Color(colorHistoryIndex, colorHistoryIndex, 1.0f));
        Radio source = connArrow.getConnection().getSource();
        Point sourcePoint = visualizer.transformPositionToPixel(source
            .getPosition());
        for (Radio destRadio : connArrow.getConnection().getDestinations()) {
          Position destPos = destRadio.getPosition();
          Point destPoint = visualizer.transformPositionToPixel(destPos);          
          drawArrow(g, sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y, 8);
        }
      }
    }

    /* Paint active connections in black */
    RadioConnection[] conns = radioMedium.getActiveConnections();
    if (conns != null) {
      g.setColor(Color.BLACK);
      for (RadioConnection conn : conns) {
        if (conn == null) {
          continue;
        }
        Radio source = conn.getSource();
        Point sourcePoint = visualizer.transformPositionToPixel(source
            .getPosition());
        for (Radio destRadio : conn.getDestinations()) {
          if (destRadio == null) {
            continue;
          }
          Position destPos = destRadio.getPosition();
          Point destPoint = visualizer.transformPositionToPixel(destPos);
          drawArrow(g, sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y, 8);          
        }
      }
    }
  }

  public void paintAfterMotes(Graphics g) {
  }

  public static class ToggleHistoryAction implements SimulationMenuAction {
    public boolean isEnabled(Visualizer visualizer, Simulation simulation) {
      return true;
    }

    public String getDescription(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      boolean showing = false;
      for (VisualizerSkin skin : skins) {
        if (skin instanceof TrafficVisualizerSkin) {
          showing = ((TrafficVisualizerSkin) skin).showHistory;
        }
      }
      if (showing) {
        return "Hide traffic history";
      }
      return "Show traffic history";
    }

    public void doAction(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      for (VisualizerSkin skin : skins) {
        if (skin instanceof TrafficVisualizerSkin) {
          ((TrafficVisualizerSkin) skin).showHistory = !((TrafficVisualizerSkin) skin).showHistory;
          visualizer.repaint();
        }
      }
    }
  };

  public Visualizer getVisualizer() {
    return visualizer;
  }
  
  private class RadioConnectionArrow {
    private RadioConnection conn;
    private int age;
    private final int MAX_AGE = 10;
    RadioConnectionArrow(RadioConnection conn) {
      this.conn = conn;
      this.age = 0;
    }
    public void increaseAge() {
      age++;
    }
    public int getAge() {
      return age;
    }
    public RadioConnection getConnection() {
      return conn;
    }
    public int getMaxAge() {
      return MAX_AGE;
    }
  }
}

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
 * $Id: TrafficVisualizerSkin.java,v 1.2 2009/10/19 17:37:50 fros4943 Exp $
 */

package se.sics.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
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

  private static final boolean DRAW_ARROWS = true;
  private static final Color COLOR_HISTORY = new Color(100, 100, 100, 100);

  private Simulation simulation = null;
  private Visualizer visualizer = null;

  private Box counters;

  private final static int HISTORY_SIZE = 8;
  private boolean showHistory = false;
  private ArrayDeque<RadioConnection> history = new ArrayDeque<RadioConnection>();

  private AbstractRadioMedium radioMedium;
  private Observer radioObserver, radioMediumObserver;

  public void setActive(Simulation simulation, Visualizer vis) {
    if (!(simulation.getRadioMedium() instanceof AbstractRadioMedium)) {
      logger.fatal("Radio medium type not supported: " + simulation.getRadioMedium());
      return;
    }
    this.radioMedium = (AbstractRadioMedium) simulation.getRadioMedium();
    this.simulation = simulation;
    this.visualizer = vis;

    final JLabel txCounter = new JLabel("TX: " + radioMedium.COUNTER_TX);
    final JLabel rxCounter = new JLabel("RX: " + radioMedium.COUNTER_RX);
    final JLabel interferedCounter = new JLabel("INT: " + radioMedium.COUNTER_INTERFERED);

    counters = Box.createHorizontalBox();
    counters.add(txCounter);
    counters.add(Box.createHorizontalStrut(10));
    counters.add(rxCounter);
    counters.add(Box.createHorizontalStrut(10));
    counters.add(interferedCounter);

    visualizer.getCurrentCanvas().add(counters);

    /* Start observing radio medium and radios */
    radioMedium.addRadioMediumObserver(radioMediumObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        txCounter.setText("TX: " + radioMedium.COUNTER_TX);
        rxCounter.setText("RX: " + radioMedium.COUNTER_RX);
        interferedCounter.setText("INT: " +  + radioMedium.COUNTER_INTERFERED);

        if (showHistory) {
          RadioConnection[] past = radioMedium.getLastTickConnections();
          if (past != null) {
            for (RadioConnection con: past) {
              history.add(con);
            }
            while (history.size() > HISTORY_SIZE) {
              history.removeFirst();
            }
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
    for (Mote mote: simulation.getMotes()) {
      Radio r = mote.getInterfaces().getRadio();
      if (r != null) {
        r.addObserver(radioObserver);
      }
    }

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
    for (Mote mote: simulation.getMotes()) {
      Radio r = mote.getInterfaces().getRadio();
      if (r != null) {
        r.addObserver(radioObserver);
      }
    }

    /* Unregister menu actions */
    visualizer.unregisterSimulationMenuAction(ToggleHistoryAction.class);
  }

  public Color[] getColorOf(Mote mote) {
    if (simulation == null) {
      /* Skin was never activated */
      return null;
    }

    Radio moteRadio = mote.getInterfaces().getRadio();
    if (moteRadio == null) {
      return null;
    }

    if (!moteRadio.isReceiverOn()) {
      return new Color[] { Color.GRAY };
    }

    if (moteRadio.isTransmitting()) {
      return new Color[] { Color.BLUE };
    }

    if (moteRadio.isInterfered()) {
      return new Color[] { Color.RED };
    }

    if (moteRadio.isReceiving()) {
      return new Color[] { Color.GREEN };
    }

    return null;
  }

  public void paintBeforeMotes(Graphics g) {
  }

  public void paintAfterMotes(Graphics g) {
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }

    if (showHistory) {
      /* Paint history in gray */
      RadioConnection[] historyArr = history.toArray(new RadioConnection[0]);
      for (RadioConnection conn : historyArr) {
        g.setColor(COLOR_HISTORY);
        Radio source = conn.getSource();
        Point sourcePoint = visualizer.transformPositionToPixel(source.getPosition());
        for (Radio destRadio : conn.getDestinations()) {
          Position destPos = destRadio.getPosition();
          Point destPoint = visualizer.transformPositionToPixel(destPos);
          g.drawLine(sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y);
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
        Radio source = conn.getSource(); // XXX Must not be null!
        Point sourcePoint = visualizer.transformPositionToPixel(source.getPosition());
        for (Radio destRadio : conn.getDestinations()) {
          if (destRadio == null) {
            continue;
          }
          Position destPos = destRadio.getPosition();
          Point destPoint = visualizer.transformPositionToPixel(destPos);
          g.drawLine(sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y);

          /* Draw arrows */
          if (DRAW_ARROWS) {
            Point centerPoint = new Point(
                destPoint.x/2 + sourcePoint.x/2,
                destPoint.y/2 + sourcePoint.y/2
            );
            int startAngle = (int) (-180 * Math.atan2(destPoint.y - sourcePoint.y, destPoint.x - sourcePoint.x)/Math.PI - 90);
            g.drawArc(centerPoint.x-5, centerPoint.y-5, 10, 10, startAngle, 180);
          }
        }
      }
    }
  }

  public static class ToggleHistoryAction implements SimulationMenuAction {
    public boolean isEnabled(Visualizer visualizer, Simulation simulation) {
      return true;
    }

    public String getDescription(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      boolean showing = false;
      for (VisualizerSkin skin: skins) {
        if (skin instanceof TrafficVisualizerSkin) {
          showing = ((TrafficVisualizerSkin)skin).showHistory;
        }
      }
      if (showing) {
        return "Hide traffic history";
      }
      return "Show traffic history";
    }

    public void doAction(Visualizer visualizer, Simulation simulation) {
      VisualizerSkin[] skins = visualizer.getCurrentSkins();
      for (VisualizerSkin skin: skins) {
        if (skin instanceof TrafficVisualizerSkin) {
          ((TrafficVisualizerSkin)skin).showHistory = !((TrafficVisualizerSkin)skin).showHistory;
          visualizer.repaint();
        }
      }
    }
  };

  public Visualizer getVisualizer() {
    return visualizer;
  }
}

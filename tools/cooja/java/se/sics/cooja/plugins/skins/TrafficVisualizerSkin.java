/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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

package se.sics.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Polygon;
import java.util.ArrayList;
import java.util.Observable;
import java.util.Observer;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.RadioConnection;
import se.sics.cooja.Simulation;
import se.sics.cooja.SupportedArguments;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;
import se.sics.cooja.radiomediums.AbstractRadioMedium;

/**
 * Radio traffic history visualizer skin.
 *
 * @see UDGMVisualizerSkin
 * @author Fredrik Osterlind
 */
@ClassDescription("Radio traffic")
@SupportedArguments(radioMediums = {AbstractRadioMedium.class})
public class TrafficVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(TrafficVisualizerSkin.class);

  private final int MAX_HISTORY_SIZE = 200;

  private boolean active = false;
  private Simulation simulation = null;
  private Visualizer visualizer = null;
  private AbstractRadioMedium radioMedium = null;

  private ArrayList<RadioConnectionArrow> historyList = new ArrayList<RadioConnectionArrow>();
  private RadioConnectionArrow[] history = null;

  private Observer radioMediumObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      RadioConnection last = radioMedium.getLastConnection();
      if (last != null && historyList.size() < MAX_HISTORY_SIZE) {
        historyList.add(new RadioConnectionArrow(last));
        history = historyList.toArray(new RadioConnectionArrow[0]);
        visualizer.repaint(500);
      }
    }
  };
  private TimeEvent ageArrowsTimeEvent = new TimeEvent(0) {
    public void execute(long t) {
      if (!active) {
        return;
      }

      if (historyList.size() > 0) {
        boolean hasOld = false;

        /* Increase age */
        for (RadioConnectionArrow connArrow : historyList) {
          connArrow.increaseAge();
          if(connArrow.getAge() >= connArrow.getMaxAge()) {
            hasOld = true;
          }
        }

        /* Remove too old arrows */
        if (hasOld) {
          RadioConnectionArrow[] historyArr = historyList.toArray(new RadioConnectionArrow[0]);
          for (RadioConnectionArrow connArrow : historyArr) {
            if(connArrow.getAge() >= connArrow.getMaxAge()) {
              historyList.remove(connArrow);
            }
          }
          historyArr = historyList.toArray(new RadioConnectionArrow[0]);
        }

        visualizer.repaint(500);
      }

      /* Reschedule myself */
      simulation.scheduleEvent(this, t + 100*Simulation.MILLISECOND);
    }
  };

  public void setActive(final Simulation simulation, Visualizer vis) {
    this.radioMedium = (AbstractRadioMedium) simulation.getRadioMedium();
    this.simulation = simulation;
    this.visualizer = vis;
    this.active = true;

    simulation.invokeSimulationThread(new Runnable() {
      public void run() {
        historyList.clear();
        history = null;

        /* Start observing radio medium for transmissions */
        radioMedium.addRadioMediumObserver(radioMediumObserver);

        /* Fade away arrows */
        simulation.scheduleEvent(ageArrowsTimeEvent, simulation.getSimulationTime() + 100*Simulation.MILLISECOND);
      }
    });
  }

  public void setInactive() {
    this.active = false;
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }

    /* Stop observing radio medium */
    radioMedium.deleteRadioMediumObserver(radioMediumObserver);
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
    RadioConnectionArrow[] historyCopy = history;
    if (historyCopy == null) {
      return;
    }
    for (RadioConnectionArrow connArrow : historyCopy) {
      float colorHistoryIndex = (float)connArrow.getAge() / (float)connArrow.getMaxAge();
      g.setColor(new Color(colorHistoryIndex, colorHistoryIndex, 1.0f));
      Radio source = connArrow.getConnection().getSource();
      Point sourcePoint = visualizer.transformPositionToPixel(source.getPosition());
      for (Radio destRadio : connArrow.getConnection().getDestinations()) {
        Position destPos = destRadio.getPosition();
        Point destPoint = visualizer.transformPositionToPixel(destPos);
        drawArrow(g, sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y, 8);
      }
    }
  }

  public void paintAfterMotes(Graphics g) {
  }

  public Visualizer getVisualizer() {
    return visualizer;
  }

  private static class RadioConnectionArrow {
    private RadioConnection conn;
    private int age;
    private static final int MAX_AGE = 10;
    RadioConnectionArrow(RadioConnection conn) {
      this.conn = conn;
      this.age = 0;
    }
    public void increaseAge() {
      if (age < MAX_AGE) {
        age++;
      }
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

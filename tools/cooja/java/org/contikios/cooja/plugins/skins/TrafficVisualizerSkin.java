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

package org.contikios.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Polygon;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Observable;
import java.util.Observer;

import org.apache.log4j.Logger;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.RadioConnection;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.SupportedArguments;
import org.contikios.cooja.TimeEvent;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.plugins.Visualizer;
import org.contikios.cooja.plugins.VisualizerSkin;
import org.contikios.cooja.radiomediums.AbstractRadioMedium;

/**
 * Radio traffic history visualizer skin.
 *
 * @see UDGMVisualizerSkin
 * @author Fredrik Osterlind
 */
@ClassDescription("Radio traffic")
@SupportedArguments(radioMediums = {AbstractRadioMedium.class})
public class TrafficVisualizerSkin implements VisualizerSkin {
  private static final Logger logger = Logger.getLogger(TrafficVisualizerSkin.class);

  private final int MAX_HISTORY_SIZE = 200;
  private final float TRANSMITTED_COLOR_RGB[] = Color.BLUE.getRGBColorComponents(null);
  private final float UNTRANSMITTED_COLOR_RGB[] = Color.RED.getRGBColorComponents(null);

  private boolean active = false;
  private Simulation simulation = null;
  private Visualizer visualizer = null;
  private AbstractRadioMedium radioMedium = null;

  private final List<RadioConnectionArrow> historyList = new LinkedList<>();

  private Observer radioMediumObserver = new Observer() {
    @Override
    public void update(Observable obs, Object obj) {
      RadioConnection last = radioMedium.getLastConnection();
      if (last != null && historyList.size() < MAX_HISTORY_SIZE) {
        synchronized(historyList) {
          historyList.add(new RadioConnectionArrow(last));
          visualizer.repaint(500);
        }
      }
    }
  };

  private final TimeEvent ageArrowsTimeEvent = new TimeEvent(0) {
    @Override
    public void execute(long t) {
      if (!active) {
        return;
      }

      if (historyList.size() > 0) {

        synchronized (historyList) {
          /* Increase age and remove too old arrows */
          Iterator<RadioConnectionArrow> iter = historyList.iterator();
          while (iter.hasNext()) {
            RadioConnectionArrow rca = iter.next();
            /* Try to increase age and remove if max age was reached */
            if (!rca.increaseAge()) {
              iter.remove();
            }
          }
        }

        visualizer.repaint(500);
      }

      /* Reschedule myself */
      simulation.scheduleEvent(this, t + 100*Simulation.MILLISECOND);
    }
  };

  @Override
  public void setActive(final Simulation simulation, Visualizer vis) {
    this.radioMedium = (AbstractRadioMedium) simulation.getRadioMedium();
    this.simulation = simulation;
    this.visualizer = vis;
    this.active = true;

    simulation.invokeSimulationThread(new Runnable() {
      @Override
      public void run() {
        historyList.clear();

        /* Start observing radio medium for transmissions */
        radioMedium.addRadioTransmissionObserver(radioMediumObserver);

        /* Fade away arrows */
        simulation.scheduleEvent(ageArrowsTimeEvent, simulation.getSimulationTime() + 100*Simulation.MILLISECOND);
      }
    });
  }

  @Override
  public void setInactive() {
    this.active = false;
    if (simulation == null) {
      /* Skin was never activated */
      return;
    }

    /* Stop observing radio medium */
    radioMedium.deleteRadioTransmissionObserver(radioMediumObserver);
  }

  @Override
  public Color[] getColorOf(Mote mote) {
    return null;
  }

  private final Polygon arrowPoly = new Polygon();
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

  @Override
  public void paintBeforeMotes(Graphics g) {
    synchronized (historyList) {
      for (RadioConnectionArrow connArrow : historyList) {
        float colorHistoryIndex = 1.0f - connArrow.getAge();
        Radio source = connArrow.getConnection().getSource();
        Point sourcePoint = visualizer.transformPositionToPixel(source.getPosition());
        /* If there is no destination, paint red circles to indicate untransmitted message */
        if (connArrow.getConnection().getDestinations().length == 0) {
          g.setColor(new Color(UNTRANSMITTED_COLOR_RGB[0], UNTRANSMITTED_COLOR_RGB[1], UNTRANSMITTED_COLOR_RGB[2], colorHistoryIndex));
          g.drawOval(sourcePoint.x - 20, sourcePoint.y - 20, 40, 40);
          g.drawOval(sourcePoint.x - 30, sourcePoint.y - 30, 60, 60);
          continue;
        }
        g.setColor(new Color(TRANSMITTED_COLOR_RGB[0], TRANSMITTED_COLOR_RGB[1], TRANSMITTED_COLOR_RGB[2], colorHistoryIndex));
        for (Radio destRadio : connArrow.getConnection().getDestinations()) {
          Position destPos = destRadio.getPosition();
          Point destPoint = visualizer.transformPositionToPixel(destPos);
          drawArrow(g, sourcePoint.x, sourcePoint.y, destPoint.x, destPoint.y, 8);
        }
      }
    }
  }

  @Override
  public void paintAfterMotes(Graphics g) {
  }

  @Override
  public Visualizer getVisualizer() {
    return visualizer;
  }

  private static class RadioConnectionArrow {

    private static final int MAX_AGE = 10;
    private final RadioConnection conn;
    private int age;

    RadioConnectionArrow(RadioConnection conn) {
      this.conn = conn;
      this.age = 0;
    }

    /**
     * Increases age of radio connection if possible or indicates max age.
     *
     * @return true if max age was not reached yet, false, if max age was
     * reached
     */
    public boolean increaseAge() {
      if (age < MAX_AGE) {
        age++;
        return true;
      } else {
        return false;
      }
    }

    /**
     * Returns relative age of radio connection
     *
     * @return Relative age (0.0 - 1.0)
     */
    public float getAge() {
      return (float) age / (float) MAX_AGE;
    }

    /**
     * Returns radio connection
     *
     * @return radio connection
     */
    public RadioConnection getConnection() {
      return conn;
    }
  }
}

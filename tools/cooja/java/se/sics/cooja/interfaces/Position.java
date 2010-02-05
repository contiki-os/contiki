/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: Position.java,v 1.6 2010/02/05 09:07:58 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.text.NumberFormat;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * Mote 3D position.
 *
 * <p>
 * This observable notifies when the position is changed.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Position")
public class Position extends MoteInterface {
  private static Logger logger = Logger.getLogger(Position.class);
  private Mote mote = null;
  private double[] coords = new double[3];

  /**
   * Creates a position for given mote with coordinates (x=0, y=0, z=0).
   *
   * @param mote
   *          Led's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public Position(Mote mote) {
    this.mote = mote;

    coords[0] = 0.0f;
    coords[1] = 0.0f;
    coords[2] = 0.0f;
  }

  /**
   * Set position to (x,y,z).
   *
   * @param x New X coordinate
   * @param y New Y coordinate
   * @param z New Z coordinate
   */
  public void setCoordinates(double x, double y, double z) {
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    this.setChanged();
    this.notifyObservers(mote);
  }

  /**
   * @return X coordinate
   */
  public double getXCoordinate() {
    return coords[0];
  }

  /**
   * @return Y coordinate
   */
  public double getYCoordinate() {
    return coords[1];
  }

  /**
   * @return Z coordinate
   */
  public double getZCoordinate() {
    return coords[2];
  }

  /**
   * Calculates distance from this position to given position.
   *
   * @param pos Compared position
   * @return Distance
   */
  public double getDistanceTo(Position pos) {
    return Math.sqrt(Math.abs(coords[0] - pos.getXCoordinate())
        * Math.abs(coords[0] - pos.getXCoordinate())
        + Math.abs(coords[1] - pos.getYCoordinate())
        * Math.abs(coords[1] - pos.getYCoordinate())
        + Math.abs(coords[2] - pos.getZCoordinate())
        * Math.abs(coords[2] - pos.getZCoordinate()));
  }

  /**
   * Calculates distance from associated mote to another mote.
   *
   * @param m Another mote
   * @return Distance
   */
  public double getDistanceTo(Mote m) {
    return getDistanceTo(m.getInterfaces().getPosition());
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    final NumberFormat form = NumberFormat.getNumberInstance();

    final JLabel positionLabel = new JLabel();
    positionLabel.setText("x=" + form.format(getXCoordinate()) + " "
        + "y=" + form.format(getYCoordinate()) + " "
        + "z=" + form.format(getZCoordinate()));

    panel.add(positionLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        positionLabel.setText("x=" + form.format(getXCoordinate()) + " "
            + "y=" + form.format(getYCoordinate()) + " "
            + "z=" + form.format(getZCoordinate()));
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    // X coordinate
    element = new Element("x");
    element.setText(Double.toString(getXCoordinate()));
    config.add(element);

    // Y coordinate
    element = new Element("y");
    element.setText(Double.toString(getYCoordinate()));
    config.add(element);

    // Z coordinate
    element = new Element("z");
    element.setText(Double.toString(getZCoordinate()));
    config.add(element);

    return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    double x = 0, y = 0, z = 0;

    for (Element element : configXML) {
      if (element.getName().equals("x")) {
        x = Double.parseDouble(element.getText());
      }

      if (element.getName().equals("y")) {
        y = Double.parseDouble(element.getText());
      }

      if (element.getName().equals("z")) {
        z = Double.parseDouble(element.getText());
      }
    }

    setCoordinates(x, y, z);
  }

}

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
 * $Id: VisTraffic.java,v 1.5 2007/01/09 09:49:24 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.*;
import java.util.*;
import javax.swing.SwingUtilities;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.Position;

/**
 * A Traffic Visualizer visualizes radio traffic by painting lines between
 * communicating motes.
 * 
 * A VisTraffic observers the current simulation radio medium.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Traffic Visualizer")
@PluginType(PluginType.SIM_PLUGIN)
public class VisTraffic extends Visualizer2D {
  protected boolean USE_ALPHA = false;

  protected boolean USE_HISTORY = true;

  protected int MAX_PAINTED_CONNS = 50;

  private static final long serialVersionUID = 1L;

  private static Logger logger = Logger.getLogger(VisTraffic.class);

  private RadioMedium radioMedium;

  public Vector<PaintedConnection> allPaintedConnections = new Vector<PaintedConnection>();

  private Simulation simulation;

  private Observer radioObserver = null;

  /**
   * Creates a new VisTraffic visualizer.
   * 
   * @param simulationToVisualize
   *          Simulation to visualize
   */
  public VisTraffic(Simulation simulationToVisualize, GUI gui) {
    super(simulationToVisualize, gui);
    setTitle("Traffic Visualizer");
    simulation = simulationToVisualize;

    radioMedium = simulationToVisualize.getRadioMedium();

    // Listen to radio medium and paint any new data transfers
    simulationToVisualize.getRadioMedium().addRadioMediumObserver(
        radioObserver = new Observer() {
          public void update(Observable obs, Object obj) {
            if (radioMedium == null)
              return;

            final RadioConnection[] connsToAdd = radioMedium
                .getLastTickConnections();
            if (connsToAdd != null && connsToAdd.length > 0) {

              SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                  if (!USE_HISTORY)
                    allPaintedConnections.clear();

                  for (RadioConnection conn : connsToAdd) {
                    if (conn != null) {
                      Color connColor = getColorOf(conn);
                      int duration = getDurationOf(conn);
                      if (connColor != null && duration > 0)
                        allPaintedConnections.add(0, new PaintedConnection(
                            conn, duration, connColor));
                    }
                  }

                  getCurrentCanvas().repaint();
                }
              });
            }
          }
        });
  }

  /**
   * Paints given connection on given graphics.
   * 
   * @param connection
   *          Connection
   * @param g2d
   *          Graphics to paint on
   */
  protected void paintConnection(PaintedConnection connection, Graphics g2d) {
    Point sourcePixelPosition = transformPositionToPixel(connection.radioConnection
        .getSourcePosition());
    g2d.setColor(connection.getColor(simulation.isRunning()));
    for (Position destPosition : connection.radioConnection
        .getDestinationPositons()) {
      Point destPixelPosition = transformPositionToPixel(destPosition);
      g2d.drawLine(sourcePixelPosition.x, sourcePixelPosition.y,
          destPixelPosition.x, destPixelPosition.y);
    }
  }

  /**
   * Returns color the given connection should be painted in. If returned color
   * is null, the connection will not be painted.
   * 
   * @param connection
   *          Connection
   * @return Color
   */
  protected Color getColorOf(RadioConnection connection) {
    return Color.BLACK;
  }

  /**
   * Returns duration the given connection should be visible. If negative, the
   * connection will not be painted. Observe that the duration is the number of
   * repaints, not related to time.
   * 
   * @param connection
   *          Connection
   * @return Duration in repaints
   */
  protected int getDurationOf(RadioConnection connection) {
    return 10;
  }

  public void closePlugin() {
    super.closePlugin();

    // Remove radio observer
    radioMedium.deleteRadioMediumObserver(radioObserver);
  }

  public Color[] getColorOf(Mote m) {
    return null;
  }

  protected void visualizeSimulation(Graphics g) {

    // Clean up old connections
    Vector<PaintedConnection> newPaintedConnections = new Vector<PaintedConnection>();
    for (PaintedConnection conn : allPaintedConnections)
      if (!conn.shouldBeRemoved())
        newPaintedConnections.add(conn);
    allPaintedConnections = newPaintedConnections;
    if (allPaintedConnections.size() > MAX_PAINTED_CONNS)
      allPaintedConnections.setSize(MAX_PAINTED_CONNS);

    for (PaintedConnection conn : allPaintedConnections)
      paintConnection(conn, (Graphics2D) g);
  }

  public class PaintedConnection {
    public RadioConnection radioConnection;

    private int duration;

    private int colorVal;

    private int repaintsLeft;

    private Color staticColor;

    /**
     * @param conn
     *          Radio connection to visualize
     * @param duration
     *          Number of repaints
     * @param color
     *          Base color of painted connection
     */
    public PaintedConnection(RadioConnection conn, int duration, Color color) {
      radioConnection = conn;
      colorVal = color.getRGB() & 0xffffff;
      repaintsLeft = duration;
      this.duration = duration;
      staticColor = color;
    }

    /**
     * Get color this connection should be painted in.
     * 
     * @param isRunning
     *          True if current simulation is running
     * @return Color
     */
    public Color getColor(boolean isRunning) {
      if (isRunning)
        repaintsLeft--;

      if (!USE_ALPHA)
        return staticColor;

      int alpha = 127 + 128 * repaintsLeft / duration;
      return new Color(colorVal | (alpha << 24), true);
    }

    public boolean shouldBeRemoved() {
      return repaintsLeft <= 0;
    }
  }

}

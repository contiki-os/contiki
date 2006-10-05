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
 * $Id: VisTraffic.java,v 1.2 2006/10/05 15:13:48 fros4943 Exp $
 */

package se.sics.cooja.plugins;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.util.Observable;
import java.util.Observer;

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
@VisPluginType(VisPluginType.SIM_PLUGIN)
public class VisTraffic extends Visualizer2D {
  private static final long serialVersionUID = 1L;
  private RadioMedium radioMedium;
  
  /**
   * The image painted on screen at repaint().
   */
  public BufferedImage image;
  
  private Position oldSmallPosition = null;
  private Position oldBigPosition = null;
  private Simulation simulation;
  private int oldNrMotes;
  
  private Observer radioObserver = null;
  
  /**
   * Creates a new VisTraffic visualizer.
   * 
   * @param simulationToVisualize
   *          Simulation to visualize
   */
  public VisTraffic(Simulation simulationToVisualize) {
    super(simulationToVisualize);
    setTitle("Traffic Visualizer");
    simulation = simulationToVisualize;
    oldNrMotes = simulation.getMotesCount();
    
    radioMedium = simulationToVisualize.getRadioMedium();

    // Repaint when radio medium has sent data
    simulationToVisualize.getRadioMedium().addRadioMediumObserver(radioObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        Graphics2D g2d = image.createGraphics();
        g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.2f));
        if (radioMedium != null && radioMedium.getLastTickConnections() != null) {
          for (RadioConnection conn: radioMedium.getLastTickConnections()) {
            if (conn != null)
              paintConnection(conn, g2d);
          }
        }
        getCurrentCanvas().repaint();
      }
    });

  }
  
  /**
   * Paints given connection on given graphics.
   * 
   * @param connection Connection
   * @param g2d Graphics to paint on
   */
  protected void paintConnection(RadioConnection connection, Graphics g2d) {
    Point sourcePixelPosition = transformPositionToPixel(connection.getSourcePosition());
    for (Position destPosition: connection.getDestinationPositons()) {
      Point destPixelPosition = transformPositionToPixel(destPosition);
      g2d.setColor(getColorOf(connection));
      g2d.drawLine(sourcePixelPosition.x, sourcePixelPosition.y,
          destPixelPosition.x, destPixelPosition.y);
      
    }
  }
  
  /**
   * Returns color a specific connection should be painted in.
   * 
   * @param connection Connection
   */
  protected Color getColorOf(RadioConnection connection) {
    return Color.BLACK;
  }
  
  protected void calculateTransformations() {
    super.calculateTransformations();
    Dimension imageDimension = getCurrentCanvas().getPreferredSize();
    if (imageDimension.height <= 0 || imageDimension.width <= 0)
      return;
    
    // Recreate image if ..
    if (
        // .. it hasn't been painted before
        oldSmallPosition == null ||
        oldBigPosition == null ||
        image == null ||
        simulation == null ||
        
        // .. mote changes may have changed the transformation.
        simulation.getMotesCount() != oldNrMotes ||
        
        // .. visualization window has changed the transformation.
        imageDimension.height != image.getHeight() ||
        imageDimension.width != image.getWidth()
    ) {
      if (simulation != null)
        oldNrMotes = simulation.getMotesCount();
      
      BufferedImage oldImage = image;
      image = new BufferedImage(imageDimension.width, imageDimension.height, BufferedImage.TYPE_4BYTE_ABGR);
      image.createGraphics().setColor(Color.WHITE);
      image.createGraphics().fillRect(0, 0, imageDimension.width, imageDimension.height);
      
      // If old data exists, keep it
      if (oldSmallPosition != null && oldBigPosition != null) {
        Point oldSmallPixelPos = transformPositionToPixel(oldSmallPosition);
        Point oldBigPixelPos = transformPositionToPixel(oldBigPosition);
        image.createGraphics().drawImage(oldImage,
            oldSmallPixelPos.x,
            oldSmallPixelPos.y,
            oldBigPixelPos.x-oldSmallPixelPos.x,
            oldBigPixelPos.y-oldSmallPixelPos.y,
            null);
      }
      
      oldSmallPosition = transformPixelToPositon(new Point(0,0));
      oldBigPosition = transformPixelToPositon(new Point(imageDimension.width, imageDimension.height));
    }
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
    g.drawImage(image, 0, 0, null);
  }
  
}

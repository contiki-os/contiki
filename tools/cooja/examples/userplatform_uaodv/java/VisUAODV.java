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
 * $Id: VisUAODV.java,v 1.3 2007/02/28 09:50:51 fros4943 Exp $
 */

import java.awt.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;
import se.sics.cooja.plugins.*;

/**
 * VisUAODV is a 2D graphical visualizer for simulations with motes running
 * UAODV protocol.
 * RREQs are painted red, and RREPs green.
 * The rest of sent data is painted black.
 *
 * Interactions with motes are available via registered mote plugins.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("uAODV Visualizer")
@PluginType(PluginType.SIM_PLUGIN)
public class VisUAODV extends VisTraffic {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(VisUAODV.class);
  
  /**
   * Creates a new VisUAODV visualizer.
   * @param simulationToVisualize Simulation to visualize
   */
  public VisUAODV(Simulation simulationToVisualize, GUI gui) {
    super(simulationToVisualize, gui);
    setTitle("uAODV Visualizer");
  }
  
  protected void paintConnection(RadioConnection connection, Graphics g2d) {
    Point sourcePixelPosition = transformPositionToPixel(connection.getSource().getPosition());
    for (Radio destRadio: connection.getDestinations()) {
      Position destPosition = destRadio.getPosition();
      Point destPixelPosition = transformPositionToPixel(destPosition);
      g2d.setColor(getColorOf(connection));

      byte[] packet = ((PacketRadio)destRadio).getLastPacketReceived();
      if (isRouteReply(packet)) {
        ((Graphics2D) g2d).setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f));
      }

      g2d.drawLine(sourcePixelPosition.x, sourcePixelPosition.y,
          destPixelPosition.x, destPixelPosition.y);

      int hopCount = getHopCount(packet);
      if (hopCount >= 0)
        g2d.drawString("" + hopCount, sourcePixelPosition.x, sourcePixelPosition.y);
    }
  }
  
  protected Color getColorOf(RadioConnection conn) {
    byte[] packet = ((PacketRadio)conn.getSource()).getLastPacketReceived();

    if (isRouteRequest(packet))
      return Color.RED;
    else if (isRouteReply(packet))
      return Color.GREEN;
    else
      return Color.BLACK;
  }

  private boolean isRouteRequest(byte[] data) {
    if (data.length > 28)
      return data[28] == 1;
    return false;
  }
  private boolean isRouteReply(byte[] data) {
    if (data.length > 28)
      return data[28] == 2;
    return false;
  }
  private int getHopCount(byte[] data) {
    if (data.length > 31)
      return (int) data[31];
    return -1;
  }
  
}

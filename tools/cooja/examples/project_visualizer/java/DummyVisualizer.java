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
 * $Id: DummyVisualizer.java,v 1.2 2009/02/18 12:07:42 fros4943 Exp $
 */

import java.awt.*;
import java.util.Random;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.plugins.*;

/**
 * Dummy visualizer.
 *
 * Extend Visualizer2D to show node positions.
 * Extend VisTraffic to show radio connections.
 *
 * @author Fredrik Österlind
 */
@ClassDescription("Dummy Visualizer")
@PluginType(PluginType.SIM_PLUGIN)
public class DummyVisualizer extends Visualizer2D {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(DummyVisualizer.class);

  public DummyVisualizer(Simulation simulationToVisualize, GUI gui) {
    super(simulationToVisualize, gui);
    setTitle("Dummy Visualizer");

    logger.debug("Creating Dummy visualizer");
  }

  public Color[] getColorOf(Mote m) {
    Random random = new Random(); /* Do not use main random generator */
    Color moteColors[] = new Color[2];

    /* Outer color */
    moteColors[0] = new Color(random.nextInt(256), random.nextInt(256), random.nextInt(256));

    /* Inner color */
    moteColors[1] = new Color(random.nextInt(256), random.nextInt(256), random.nextInt(256));

    return moteColors;
  }

  protected Color getColorOf(RadioConnection conn) {
    RadioPacket radioPacket = conn.getSource().getLastPacketReceived();

    /* TODO Analyze data - determine color */

    Random random = new Random(); /* Do not use main random generator */
    return new Color(random.nextInt(256), random.nextInt(256), random.nextInt(256));
  }

}

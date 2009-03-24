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
 * $Id: BasicVisualizerSkin.java,v 1.1 2009/03/24 15:46:29 fros4943 Exp $
 */

package se.sics.cooja.plugins.skins;

import java.awt.Color;
import java.awt.Graphics;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.Simulation;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;

/**
 * Basic visualizer skin. Paints all motes black.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("[select visualizer skin]")
public class BasicVisualizerSkin implements VisualizerSkin {
  private static Logger logger = Logger.getLogger(BasicVisualizerSkin.class);

  private Visualizer visualizer = null;

  public void setActive(Simulation simulation, Visualizer vis) {
    this.visualizer = vis;
  }

  public void setInactive() {
  }

  public Color[] getColorOf(Mote mote) {
    return new Color[] { Color.BLACK };
  }

  public void paintSkin(Graphics g) {
    /* Just paint motes */
    visualizer.paintSkinGeneric(g);
  }
}

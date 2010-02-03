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
 * $Id: VisualizerSkin.java,v 1.4 2010/02/03 15:49:24 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.Color;
import java.awt.Graphics;

import se.sics.cooja.Mote;
import se.sics.cooja.Simulation;

/**
 * Visualizer skin used by visualizer plugin.
 *
 * @see Visualizer
 * @author Fredrik Osterlind
 */
public interface VisualizerSkin {

  /**
   * Activates skin.
   * When the plugin is repainted, this skin is responsible to paint the canvas.
   *
   * @param sim Simulation
   * @param visualizer Visualizer plugin
   *
   * @see #paintBeforeMotes(Graphics)
   * @see #paintAfterMotes(Graphics)
   * @see #setInactive()
   */
  public void setActive(Simulation sim, Visualizer visualizer);

  /**
   * Deactivates skin.
   * Should release resources such as simulation observers.
   *
   * @see #setActive(Simulation, Visualizer)
   */
  public void setInactive();

  /**
   * Returns mote colors.
   * Used by skin generic visualization.
   * May return a single color, or two colors.
   *
   * @see Visualizer#paintMotes(Graphics)
   *
   * @param mote Mote
   * @return Color[] { Inner color, Outer color }
   */
  public Color[] getColorOf(Mote mote);

  /**
   * Called every time the visualizer plugin is repainted.
   * Paints graphics that should be underneath the motes.
   *
   * @param g Graphics
   */
  public void paintBeforeMotes(Graphics g);

  /**
   * Called every time the visualizer plugin is repainted.
   * Paints graphics that should be on top of the motes.
   *
   * @param g Graphics
   */
  public void paintAfterMotes(Graphics g);

  /**
   * @return Visualizer plugin where this skin is showing
   */
  public Visualizer getVisualizer();

}

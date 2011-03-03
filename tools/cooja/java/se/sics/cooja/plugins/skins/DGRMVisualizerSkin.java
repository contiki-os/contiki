/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: DGRMVisualizerSkin.java,v 1.1 2010/09/24 12:49:14 fros4943 Exp $
 */

package se.sics.cooja.plugins.skins;

import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Point;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.plugins.Visualizer;
import se.sics.cooja.plugins.VisualizerSkin;
import se.sics.cooja.radiomediums.DGRMDestinationRadio;
import se.sics.cooja.radiomediums.DestinationRadio;
import se.sics.cooja.radiomediums.DirectedGraphMedium;

@ClassDescription("Radio environment (DGRM)")
public class DGRMVisualizerSkin implements VisualizerSkin {
	private static Logger logger = Logger.getLogger(DGRMVisualizerSkin.class);

	private Simulation simulation = null;
	private Visualizer visualizer = null;

	public void setActive(Simulation simulation, Visualizer vis) {
		if (!(simulation.getRadioMedium() instanceof DirectedGraphMedium)) {
			logger.fatal("Cannot activate DGRM skin for unknown radio medium: " + simulation.getRadioMedium());
			return;
		}
		this.simulation = simulation;
		this.visualizer = vis;
	}

	public void setInactive() {
		if (simulation == null) {
			/* Skin was never activated */
			return;
		}
	}

	public Color[] getColorOf(Mote mote) {
	  Mote selectedMote = visualizer.getSelectedMote();
	  if (mote == selectedMote) {
	    return new Color[] { Color.CYAN };
	  }
	  return null;
	}

	public void paintBeforeMotes(Graphics g) {
          Mote selectedMote = visualizer.getSelectedMote();
		if (simulation == null 
				|| selectedMote == null
				|| selectedMote.getInterfaces().getRadio() == null) {
			return;
		}

		/* Paint transmission and interference range for selected mote */
		Position motePos = selectedMote.getInterfaces().getPosition();

		Point pixelCoord = visualizer.transformPositionToPixel(motePos);
		int x = pixelCoord.x;
		int y = pixelCoord.y;
		Radio selectedRadio = selectedMote.getInterfaces().getRadio();

		FontMetrics fm = g.getFontMetrics();
		g.setColor(Color.BLACK);

		DirectedGraphMedium radioMedium = (DirectedGraphMedium) simulation.getRadioMedium();
		
		/* Print transmission success probabilities */
		DestinationRadio[] dests = radioMedium.getPotentialDestinations(selectedRadio);
		if (dests == null || dests.length == 0) {
			String msg = "No edges";
			int msgWidth = fm.stringWidth(msg);
			g.setColor(Color.BLACK);
			g.drawString(msg, x - msgWidth/2, y + 2*Visualizer.MOTE_RADIUS + 3);
			return;
		}
		String msg = dests.length + " edges";
		int msgWidth = fm.stringWidth(msg);
		g.setColor(Color.BLACK);
		g.drawString(msg, x - msgWidth/2, y + 2*Visualizer.MOTE_RADIUS + 3);
		for (DestinationRadio r: dests) {
			double prob = ((DGRMDestinationRadio)r).ratio;
			if (prob == 0.0d) {
				continue;
			}
			msg = String.format("%1.1f%%", 100.0*prob);
			Position pos = r.radio.getPosition();
			Point pixel = visualizer.transformPositionToPixel(pos);
			msgWidth = fm.stringWidth(msg);
			g.setColor(new Color(1-(float)prob, (float)prob, 0.0f));
			g.drawLine(x, y, pixel.x, pixel.y);
			g.setColor(Color.BLACK);
			g.drawString(msg, pixel.x - msgWidth/2, pixel.y + 2*Visualizer.MOTE_RADIUS + 3);
		}
	}

	public void paintAfterMotes(Graphics g) {
	}

	public Visualizer getVisualizer() {
		return visualizer;
	}
}

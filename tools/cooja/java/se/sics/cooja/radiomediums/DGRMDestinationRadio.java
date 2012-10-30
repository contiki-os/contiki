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
 */

package se.sics.cooja.radiomediums;

import java.util.Collection;

import org.jdom.Element;

import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.Radio;

public class DGRMDestinationRadio extends DestinationRadio {
	public double ratio = 1.0; /* Link success ratio (per packet). */
	public double signal = AbstractRadioMedium.SS_STRONG; /* RSSI */
	public long delay = 0; /* EXPERIMENTAL: Propagation delay (us). */
	public int lqi = 105;

	public DGRMDestinationRadio() {
		super();
	}
	public DGRMDestinationRadio(Radio dest) {
		super(dest);
	}

	protected Object clone() {
		DGRMDestinationRadio clone = new DGRMDestinationRadio(this.radio);
		clone.ratio = this.ratio;
		clone.delay = this.delay;
		clone.signal = this.signal;
		clone.lqi = this.lqi;
		return clone;
	}
	
	public Collection<Element> getConfigXML() {
		Collection<Element> config = super.getConfigXML();
		Element element;

		element = new Element("ratio");
		element.setText("" + ratio);
		config.add(element);

		element = new Element("signal");
		element.setText("" + signal);
		config.add(element);

		element = new Element("lqi");
		element.setText("" + lqi);
		config.add(element);
		
		
		element = new Element("delay");
		element.setText("" + delay);
		config.add(element);

		return config;
	}
	
	public boolean setConfigXML(final Collection<Element> configXML, Simulation simulation) {
		if (!super.setConfigXML(configXML, simulation)) {
			return false;
		}
		for (Element element : configXML) {
			if (element.getName().equals("ratio")) {
				ratio = Double.parseDouble(element.getText());
			} else if (element.getName().equals("signal")) {
				signal = Double.parseDouble(element.getText());
			} else if (element.getName().equals("lqi")) {
				lqi = Integer.parseInt(element.getText());
			} else if (element.getName().equals("delay")) {
				delay = Long.parseLong(element.getText());
			}
		}
		return true;
	}
}

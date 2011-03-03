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
 * $Id: DirectedGraphMedium.java,v 1.8 2010/12/02 15:25:50 fros4943 Exp $
 */

package se.sics.cooja.radiomediums;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.Radio;

public class DestinationRadio {
	private static Logger logger = Logger.getLogger(DestinationRadio.class);

	public Radio radio; /* destination radio */
	public DestinationRadio() {
	}
	public DestinationRadio(Radio dest) {
		this.radio = dest;
	}

	public String toString() {
		return radio.getMote().toString();
	}

	public Collection<Element> getConfigXML() {
		ArrayList<Element> config = new ArrayList<Element>();
		Element element;

		element = new Element("radio");
		element.setText("" + radio.getMote().getID());
		config.add(element);
		return config;
	}

	public boolean setConfigXML(Collection<Element> configXML, Simulation simulation) {
		for (Element element : configXML) {
			if (element.getName().equals("radio")) {
				radio = simulation.getMoteWithID(Integer.parseInt(element.getText())).getInterfaces().getRadio();
				if (radio == null) {
					throw new RuntimeException("No mote with ID " + element.getText());
				}
			}
		}
		return true;
	}
}

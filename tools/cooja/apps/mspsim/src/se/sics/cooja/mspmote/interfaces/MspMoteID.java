/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: MspMoteID.java,v 1.16 2010/03/08 14:26:12 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Mote;
import se.sics.cooja.MoteTimeEvent;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.MoteID;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteMemory;
import se.sics.mspsim.core.CPUMonitor;

/**
 * Mote ID.
 *
 * @author Fredrik Osterlind
 */
public class MspMoteID extends MoteID {
	private static Logger logger = Logger.getLogger(MspMoteID.class);

	private MspMote mote;
	private MspMoteMemory moteMem = null;

	private boolean writeFlashHeader = true;
	private int moteID = -1;

	/**
	 * Creates an interface to the mote ID at mote.
	 *
	 * @param mote ID
	 * @see Mote
	 * @see se.sics.cooja.MoteInterfaceHandler
	 */
	public MspMoteID(Mote m) {
		this.mote = (MspMote) m;
		this.moteMem = (MspMoteMemory) mote.getMemory();

		final MoteTimeEvent writeIDEvent = new MoteTimeEvent(mote, 0) {
			public void execute(long t) {
				setMoteID(moteID);
			}
		};

		if (moteMem.variableExists("node_id")) {
			this.mote.getCPU().setBreakPoint(moteMem.getVariableAddress("node_id"), new CPUMonitor() {
				public void cpuAction(int type, int adr, int data) {
					if (type != MEMORY_WRITE) {
						return;
					}
					if (data == moteID) {
						return;
					}
					Simulation s = mote.getSimulation();
					s.scheduleEvent(writeIDEvent, s.getSimulationTime());
				}
			});
		}
		if (moteMem.variableExists("TOS_NODE_ID")) {
			this.mote.getCPU().setBreakPoint(moteMem.getVariableAddress("TOS_NODE_ID"), new CPUMonitor() {
				public void cpuAction(int type, int adr, int data) {
					if (type != MEMORY_WRITE) {
						return;
					}
					if (data == moteID) {
						return;
					}
					Simulation s = mote.getSimulation();
					s.scheduleEvent(writeIDEvent, s.getSimulationTime());
				}
			});
		}
		if (moteMem.variableExists("ActiveMessageAddressC__addr")) {
		  this.mote.getCPU().setBreakPoint(moteMem.getVariableAddress("ActiveMessageAddressC__addr"), new CPUMonitor() {
		    public void cpuAction(int type, int adr, int data) {
		      if (type != MEMORY_WRITE) {
		        return;
		      }
		      if (data == moteID) {
		        return;
		      }
		      Simulation s = mote.getSimulation();
		      s.scheduleEvent(writeIDEvent, s.getSimulationTime());
		    }
		  });
		}
		if (moteMem.variableExists("ActiveMessageAddressC$addr")) {
		  this.mote.getCPU().setBreakPoint(moteMem.getVariableAddress("ActiveMessageAddressC$addr"), new CPUMonitor() {
		    public void cpuAction(int type, int adr, int data) {
		      if (type != MEMORY_WRITE) {
		        return;
		      }
		      if (data == moteID) {
		        return;
		      }
		      Simulation s = mote.getSimulation();
		      s.scheduleEvent(writeIDEvent, s.getSimulationTime());
		    }
		  });
		}
	}

	public int getMoteID() {
		return moteID;
	}

	public void setMoteID(int newID) {
		if (moteID != newID) {
			mote.idUpdated(newID);
			setChanged();
		}
		moteID = newID;

		if (moteMem.variableExists("node_id")) {
			moteMem.setIntValueOf("node_id", moteID);

			if (writeFlashHeader) {
				/* Write to external flash */
				SkyFlash flash = mote.getInterfaces().getInterfaceOfType(SkyFlash.class);
				if (flash != null) {
					flash.writeIDheader(moteID);
				}
				writeFlashHeader = false;
			}
			/* Experimental: set Contiki random seed variable if it exists */
			if (moteMem.variableExists("rseed")) {
				moteMem.setIntValueOf("rseed", (int) (mote.getSimulation().getRandomSeed() + newID));
			}
		}
		if (moteMem.variableExists("TOS_NODE_ID")) {
			moteMem.setIntValueOf("TOS_NODE_ID", moteID);
		}
		if (moteMem.variableExists("ActiveMessageAddressC__addr")) {
			moteMem.setIntValueOf("ActiveMessageAddressC__addr", newID);
		}
		if (moteMem.variableExists("ActiveMessageAddressC$addr")) {
			moteMem.setIntValueOf("ActiveMessageAddressC$addr", newID);
		}

		notifyObservers();
	}

	public JPanel getInterfaceVisualizer() {
		JPanel panel = new JPanel();
		final JLabel idLabel = new JLabel();

		idLabel.setText("Mote ID: " + getMoteID());

		panel.add(idLabel);

		Observer observer;
		this.addObserver(observer = new Observer() {
			public void update(Observable obs, Object obj) {
				idLabel.setText("Mote ID: " + getMoteID());
			}
		});

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
		ArrayList<Element> config = new ArrayList<Element>();
		Element element = new Element("id");
		element.setText(Integer.toString(getMoteID()));
		config.add(element);
		return config;
	}

	public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
		for (Element element : configXML) {
			if (element.getName().equals("id")) {
				setMoteID(Integer.parseInt(element.getText()));
			}
		}
	}
}

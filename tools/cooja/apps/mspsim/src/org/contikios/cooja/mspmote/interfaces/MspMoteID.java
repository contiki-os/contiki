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
 */

package org.contikios.cooja.mspmote.interfaces;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;

import org.contikios.cooja.Mote;
import org.contikios.cooja.interfaces.MoteID;
import org.contikios.cooja.mote.memory.VarMemory;
import org.contikios.cooja.mspmote.MspMote;
import se.sics.mspsim.core.Memory;
import se.sics.mspsim.core.MemoryMonitor;

/**
 * Mote ID.
 *
 * @author Fredrik Osterlind
 */
public class MspMoteID extends MoteID {
	private static Logger logger = Logger.getLogger(MspMoteID.class);

	private MspMote mote;
	private VarMemory moteMem = null;

	private boolean writeFlashHeader = true;
	private int moteID = -1;

	private MemoryMonitor memoryMonitor;
	
	/**
	 * Creates an interface to the mote ID at mote.
	 *
	 * @param mote ID
	 * @see Mote
	 * @see org.contikios.cooja.MoteInterfaceHandler
	 */
	public MspMoteID(Mote m) {
		this.mote = (MspMote) m;
		this.moteMem = new VarMemory(mote.getMemory());
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

		/* Write node-unique infomem entry, used to configure node_id and node_mac */
		byte[] infomemCurrent = mote.getMemory().getMemorySegment(0x1980, 10);
		if (infomemCurrent != null) {
			/* Only write to infomem is nothing else resides there */
			boolean ffOnly = true;
			for (byte b: infomemCurrent) {
				if (b != (byte) 0xff) {
					ffOnly = false;
					break;
				}
			}

			if (ffOnly) {
				byte[] infomem = new byte[10];
				infomem[0] = (byte) 0xab; /* magic */
				infomem[1] = (byte) 0xcd; /* magic */
				infomem[2] = (byte) 0x02;
				infomem[3] = (byte) 0x12;
				infomem[4] = (byte) 0x74;
				infomem[5] = (byte) 0x00;
				infomem[6] = (byte) 0x00;
				infomem[7] = (byte) 0x01;
				infomem[8] = (byte) ((newID << 8) & 0xFF);
				infomem[9] = (byte) (newID & 0xFF);
				mote.getMemory().setMemorySegment(0x1980, infomem);
			}
		}
		
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
		if (memoryMonitor == null) {
		    memoryMonitor = new MemoryMonitor.Adapter() {

		        @Override
		        public void notifyWriteAfter(int dstAddress, int data, Memory.AccessMode mode) {
		            byte[] id = new byte[2];
		            id[0] = (byte) (moteID & 0xff);
		            id[1] = (byte) ((moteID >> 8) & 0xff);
		            mote.getMemory().setMemorySegment(dstAddress & ~1, id);
		        }

		    };

                    addMonitor("node_id", memoryMonitor);
                    addMonitor("TOS_NODE_ID", memoryMonitor);
                    addMonitor("ActiveMessageAddressC__addr", memoryMonitor);
                    addMonitor("ActiveMessageAddressC$addr", memoryMonitor);
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

	public void removed() {
	  super.removed();
	  if (memoryMonitor != null) {
	      removeMonitor("node_id", memoryMonitor);
	      removeMonitor("TOS_NODE_ID", memoryMonitor);
	      removeMonitor("ActiveMessageAddressC__addr", memoryMonitor);
	      removeMonitor("ActiveMessageAddressC$addr", memoryMonitor);
	      memoryMonitor = null;
	  }
	}

	private void addMonitor(String variable, MemoryMonitor monitor) {
	    if (moteMem.variableExists(variable)) {
	        int address = (int) moteMem.getVariableAddress(variable);
	        if ((address & 1) != 0) {
	            // Variable can not be a word - must be a byte
	        } else {
	            mote.getCPU().addWatchPoint(address, monitor);
	            mote.getCPU().addWatchPoint(address + 1, monitor);
	        }
	    }
	}

        private void removeMonitor(String variable, MemoryMonitor monitor) {
            if (moteMem.variableExists(variable)) {
                int address = (int) moteMem.getVariableAddress(variable);
                mote.getCPU().removeWatchPoint(address, monitor);
                mote.getCPU().removeWatchPoint(address + 1, monitor);
            }
        }
}

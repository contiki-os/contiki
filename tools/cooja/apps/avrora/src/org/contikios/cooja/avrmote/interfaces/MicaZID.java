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
 */

package org.contikios.cooja.avrmote.interfaces;

import java.util.Collection;
import java.util.Vector;

import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteTimeEvent;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.TimeEvent;
import org.contikios.cooja.avrmote.MicaZMote;
import org.contikios.cooja.interfaces.MoteID;
import org.contikios.cooja.mote.memory.MemoryInterface;
import org.contikios.cooja.mote.memory.MemoryInterface.SegmentMonitor;
import org.contikios.cooja.mote.memory.VarMemory;

public class MicaZID extends MoteID {

    private static final boolean PERSISTENT_SET_ID = true;

    private static Logger logger = Logger.getLogger(MicaZID.class);

    private int moteID = -1; /* TODO Implement */

    private VarMemory moteMem;
    boolean tosID = false;
    boolean contikiID = false;
    private MicaZMote mote;
    private int persistentSetIDCounter = 1000;

    TimeEvent persistentSetIDEvent = new MoteTimeEvent(mote, 0) {
        public void execute(long t) {
            if (persistentSetIDCounter-- > 0) {
                setMoteID(moteID);
                if (t + mote.getInterfaces().getClock().getDrift() < 0) {
                    /* Wait until node is booting */
                    mote.getSimulation().scheduleEvent(this, -mote.getInterfaces().getClock().getDrift());
                } else {
                    mote.getSimulation().scheduleEvent(this, t + Simulation.MILLISECOND / 16);
                }
            }
        }
    };


    public MicaZID(Mote mote) {
        this.mote = (MicaZMote) mote;
        this.moteMem = new VarMemory(mote.getMemory());

        if (moteMem.variableExists("node_id")) {
            contikiID = true;

            int addr = (int) moteMem.getVariableAddress("node_id");
            moteMem.addVarMonitor(
                            SegmentMonitor.EventType.READWRITE, 
                            "node_id", 
                            new SegmentMonitor() {

                @Override
                public void memoryChanged(MemoryInterface memory, SegmentMonitor.EventType type, long address) {
                    if (type == EventType.READ) {
                        System.out.println("Read from node_id.");
                    } else {
                        System.out.println("Writing to node_id.");
                    }
                }
            });
        }

        if (moteMem.variableExists("TOS_NODE_ID")) {
            tosID = true;
        }


        if (PERSISTENT_SET_ID) {
            mote.getSimulation().invokeSimulationThread(new Runnable() {
                public void run() {
                    persistentSetIDEvent.execute(MicaZID.this.mote.getSimulation().getSimulationTime());
                };
            });
        }
    }

    public int getMoteID() {
        if (contikiID) {
            return moteMem.getIntValueOf("node_id");
        }

        if (tosID) {
            return moteMem.getIntValueOf("TOS_NODE_ID");
        }     
        return moteID;
    }

    public void setMoteID(int newID) {
        moteID = newID;
        if (contikiID) {
            mote.setEEPROM(0, 0xad);
            mote.setEEPROM(1, 0xde);
            mote.setEEPROM(2, newID);
            mote.setEEPROM(3, newID >> 8);
            System.out.println("Setting node id: " + newID);
            moteMem.setIntValueOf("node_id", newID);
        }
        if (tosID) {
            moteMem.setIntValueOf("TOS_NODE_ID", newID);
            moteMem.setIntValueOf("ActiveMessageAddressC$addr", newID);
        }
        setChanged();
        notifyObservers();
        return;
    }


    public JPanel getInterfaceVisualizer() {
        return null;
    }

    public void releaseInterfaceVisualizer(JPanel panel) {
    }

    public Collection<Element> getConfigXML() {
        Vector<Element> config = new Vector<Element>();
        Element element;

        // Infinite boolean
        element = new Element("id");
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

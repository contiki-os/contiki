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

package se.sics.cooja.mspmote.interfaces;

import java.util.Collection;

import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteMemory;
import se.sics.mspsim.core.Memory;
import se.sics.mspsim.core.MemoryMonitor;

/**
 * Observes writes to a special (hardcoded) Contiki variable: cooja_debug_ptr.
 * When the pointer is changed, the string that the pointer points to 
 * is outputted as log output from this mote interface.
 * 
 * Contiki code example:
 *  cooja_debug_ptr = "Almost non-intrusive debug output";
 * or simply:
 *  COOJA_DEBUG("Almost non-intrusive debug output");
 *  
 * @author Fredrik Osterlind
 */
@ClassDescription("Debugging output")
public class MspDebugOutput extends Log {
  private static Logger logger = Logger.getLogger(MspDebugOutput.class);

  private final static String CONTIKI_POINTER = "cooja_debug_ptr";
  
  private MspMote mote;
  private MspMoteMemory mem;
  
  private String lastLog = null;
  private MemoryMonitor memoryMonitor = null;
  
  public MspDebugOutput(Mote mote) {
    this.mote = (MspMote) mote;
    this.mem = (MspMoteMemory) this.mote.getMemory();

    if (!mem.variableExists(CONTIKI_POINTER)) {
      /* Disabled */
      return;
    }
    this.mote.getCPU().addWatchPoint(mem.getVariableAddress(CONTIKI_POINTER),
        memoryMonitor = new MemoryMonitor.Adapter() {
        @Override
        public void notifyWriteAfter(int adr, int data, Memory.AccessMode mode) {
          String msg = extractString(mem, data);
          if (msg != null && msg.length() > 0) {
            lastLog = "DEBUG: " + msg;
            setChanged();
            notifyObservers(MspDebugOutput.this.mote);
          }
      }
    });
  }

  private String extractString(MspMoteMemory mem, int address) {
    StringBuilder sb = new StringBuilder();
    while (true) {
      byte[] data = mem.getMemorySegment(address, 8);
      address += 8;
      for (byte b: data) {
        if (b == 0) {
          return sb.toString();
        }
        sb.append((char)b);
        if (sb.length() > 128) {
          /* Maximum size */
          return sb.toString() + "...";
        }
      }
    }
  }
  
  public Mote getMote() {
    return mote;
  }

  public String getLastLogMessage() {
    return lastLog;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    /* Observed Contiki pointer is hardcoded */
  }

  public JPanel getInterfaceVisualizer() {
    return null;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public void removed() {
    super.removed();

    if (memoryMonitor != null) {
      mote.getCPU().removeWatchPoint(mem.getVariableAddress(CONTIKI_POINTER), memoryMonitor);
    }
  }
}

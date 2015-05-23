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
 */

package org.contikios.cooja;

import java.util.Collection;

import javax.swing.JInternalFrame;
import javax.swing.event.InternalFrameAdapter;
import javax.swing.event.InternalFrameEvent;

import org.jdom.Element;

import org.contikios.cooja.plugins.SimControl;

/**
 * Visualized plugins can extend VisPlugin for basic visualization functionality.
 * VisPlugin extends JInternalFrame, the graphical component used by plugins.
 * VisPlugin implementations may hence directly add buttons to themselves.
 *
 * Note that plugins of this type can only be started if COOJA is visualized.
 * Hence, these plugins will not be started during nightly Contiki tests.
 *
 * @see SimControl
 * @see PluginRequiresVisualizationException
 * @author Fredrik Osterlind
 */
public abstract class VisPlugin extends JInternalFrame implements Plugin {

  public VisPlugin(String title, final Cooja gui) {
    this(title, gui, true);
  }

  public VisPlugin(String title, final Cooja gui, boolean requiresVis) {
    super(title, true, true, true, true);

    if (requiresVis && !Cooja.isVisualized()) {
      throw new PluginRequiresVisualizationException();
    }

    setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);

    addInternalFrameListener(new InternalFrameAdapter() {
      public void internalFrameClosing(InternalFrameEvent e) {
        gui.removePlugin(VisPlugin.this, true);
      }
      public void internalFrameActivated(InternalFrameEvent e) {
        /* Highlight mote in COOJA */
        Plugin p = VisPlugin.this;
        if (p instanceof MotePlugin) {
          gui.signalMoteHighlight(((MotePlugin)p).getMote());
        }
        gui.loadQuickHelp(VisPlugin.this);
      }
    }
    );
  }

  public JInternalFrame getCooja() {
    return this;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return false;
  }

  public void startPlugin() {
  }
  public void closePlugin() {
  }
  
  public static class PluginRequiresVisualizationException extends RuntimeException {
  }
}

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
 * $Id: VisPlugin.java,v 1.1 2006/08/21 12:12:56 fros4943 Exp $
 */

package se.sics.cooja;

import javax.swing.JInternalFrame;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;

/**
 * Abstract class VisPlugin should be implemented by plugins for COOJA Simulator.
 * By extending JInternalFrame, the visual apperence is decided by the plugin itself.
 *
 * An implemented plugin should be registered at runtime using the following method:
 * GUI.registerPlugin(Class<? extends VisPlugin>, String)
 *
 * For example how to implement a plugin see classes SimControl or Vis2D.
 *
 * @author Fredrik Osterlind
 */
public abstract class VisPlugin extends JInternalFrame {

  /**
   * Sets frame title
   * @param title Frame title
   */
  public VisPlugin(String title) {
    super(title, true, true, true, true);
    final VisPlugin thisPlugin = this;
    
    // Close via gui
    setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
    // Detect frame events
    addInternalFrameListener(new InternalFrameListener() {
      public void internalFrameClosing(InternalFrameEvent e) {
        GUI.currentGUI.removePlugin(thisPlugin, true);
      }
      public void internalFrameClosed(InternalFrameEvent e) {
        // NOP
      }
      public void internalFrameOpened(InternalFrameEvent e) {
        // NOP
      }
      public void internalFrameIconified(InternalFrameEvent e) {
        // NOP
      }
      public void internalFrameDeiconified(InternalFrameEvent e) {
        // NOP
      }
      public void internalFrameActivated(InternalFrameEvent e) {
        // NOP
      }
      public void internalFrameDeactivated(InternalFrameEvent e) {
        // NOP
      }
    }
    );
  }
  
  /**
   * This method is called when an opened plugin is about to close.
   * It should release any resources such as registered observers or
   * opened interface visualizers.
   */
  public abstract void closePlugin();

}

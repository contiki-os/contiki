/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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

package se.sics.cooja.util;

import java.awt.event.ActionEvent;
import java.io.File;

import javax.swing.Action;
import javax.swing.JMenuItem;
import javax.swing.text.JTextComponent;

import jsyntaxpane.SyntaxDocument;
import jsyntaxpane.actions.DefaultSyntaxAction;

import org.apache.log4j.Logger;

import se.sics.cooja.WatchpointMote;

public class JSyntaxAddBreakpoint extends DefaultSyntaxAction {
  private static Logger logger = Logger.getLogger(JSyntaxAddBreakpoint.class);

  public JSyntaxAddBreakpoint() {
    super("addbreakpoint");
  }
  
  public void actionPerformed(ActionEvent e) {
    JMenuItem menuItem = (JMenuItem) e.getSource();
    Action action = menuItem.getAction();
    WatchpointMote watchpointMote = (WatchpointMote) action.getValue("WatchpointMote");
    if (watchpointMote == null) {
      logger.warn("Error: No source, cannot configure breakpoint");
      return;
    }

    File file = (File) action.getValue("WatchpointFile");
    Integer line = (Integer) action.getValue("WatchpointLine");
    Integer address = (Integer) action.getValue("WatchpointAddress");
    if (file == null || line == null || address == null) {
      logger.warn("Error: Bad breakpoint info, cannot add breakpoint");
      return;
    }

    watchpointMote.addBreakpoint(file, line, address);
  }
}

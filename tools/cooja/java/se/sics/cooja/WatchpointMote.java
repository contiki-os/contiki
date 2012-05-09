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
 * $Id: WatchpointMote.java,v 1.1 2009/06/11 10:02:11 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.File;

/**
 * @author Fredrik Osterlind
 */
public interface WatchpointMote extends Mote {

  public interface WatchpointListener {
    public void watchpointTriggered(Watchpoint watchpoint);
    public void watchpointsChanged();
  }

  /**
   * Adds a breakpoint listener.
   * The listener will be notified when breakpoints are added, removed or triggered.
   *
   * @param listener Action listener
   */
  public void addWatchpointListener(WatchpointListener listener);

  /**
   * Removes previously registered listener.
   *
   * @param listener Listeners
   */
  public void removeWatchpointListener(WatchpointListener listener);

  /**
   * @return All registered listeners
   */
  public WatchpointListener[] getWatchpointListeners();

  public Watchpoint addBreakpoint(File codeFile, int lineNr, int address);
  public void removeBreakpoint(Watchpoint watchpoint);
  public Watchpoint[] getBreakpoints();

  public boolean breakpointExists(int address);
  public boolean breakpointExists(File file, int lineNr);

  public int getExecutableAddressOf(File file, int lineNr);

}

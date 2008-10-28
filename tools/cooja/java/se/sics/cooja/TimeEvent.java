/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: TimeEvent.java,v 1.1 2008/10/28 12:51:22 fros4943 Exp $
 */

package se.sics.cooja;

/**
 * @author Joakim Eriksson (ported to COOJA by Fredrik Österlind)
 */
public abstract class TimeEvent {
  // For linking events...
  TimeEvent nextEvent;
  TimeEvent prevEvent;

  // Keeps track of where this is scheduled
  EventQueue scheduledIn = null;
  String name;

  protected int time;

  public TimeEvent(int time) {
    this.time = time;
  }

  public TimeEvent(int time, String name) {
    this.time = time;
    this.name = name;
  }

  public final int getTime() {
    return time;
  }

  public boolean isScheduled() {
    return scheduledIn != null;
  }

  public boolean remove() {
    if (scheduledIn != null) {
      return scheduledIn.removeEvent(this);
    }
    return false;
  }

  public abstract void execute(int t);

  public String getShort() {
    return "" + time + (name != null ? ": " + name : "");
  }

} // TimeEvent

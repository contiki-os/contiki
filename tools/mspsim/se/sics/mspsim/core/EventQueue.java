/**
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * EventQueue
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.core;

import java.io.PrintStream;

public class EventQueue {

  private TimeEvent first;
  public long nextTime;
  public int eventCount = 0;

  public EventQueue() {
  }

  public void addEvent(TimeEvent event, long time) {
    event.time = time;
    addEvent(event);
  }

  public void addEvent(TimeEvent event) {
    if (event.scheduledIn != null) {
      event.remove();
    }
    if (first == null) {
      first = event;
    } else {
      TimeEvent pos = first;
      TimeEvent lastPos = first;
      while (pos != null && pos.time < event.time) {
        lastPos = pos;
        pos = pos.nextEvent;
      }
      // Here pos will be the first TE after event
      // and lastPos the first before
      if (pos == first) {
        // Before all other
        event.nextEvent = pos;
        first = event;
      } else {
        event.nextEvent = pos;
        lastPos.nextEvent = event;
      }
    }
    if (first != null) {
      nextTime = first.time;
    } else {
      nextTime = 0;
    }
    event.scheduledIn = this;
    eventCount++;
  }

  // Not yet impl.
  public boolean removeEvent(TimeEvent event) {
    TimeEvent pos = first;
    TimeEvent lastPos = first;
//  System.out.println("Removing: " + event.getShort() + "  Before remove: ");
//  print();
    while (pos != null && pos != event) {
      lastPos = pos;
      pos = pos.nextEvent;
    }
    if (pos == null) return false;
    // pos == event!
    if (pos == first) {
      // remove it from first pos.
      first = pos.nextEvent;
    } else {
      // else link prev to next...
      lastPos.nextEvent = pos.nextEvent;
    }
    // unlink
    pos.nextEvent = null;

    if (first != null) {
      nextTime = first.time;
    } else {
      nextTime = 0;
    }
//  System.out.println("Removed =>");
//  print();
    event.scheduledIn = null;
    eventCount--;
    return true;
  }

  public TimeEvent popFirst() {
    TimeEvent tmp = first;
    if (tmp != null) {
      first = tmp.nextEvent;
      // Unlink.
      tmp.nextEvent = null;

      // No longer scheduled!
      tmp.scheduledIn = null;

      eventCount--;
    }

    if (first != null) {
      nextTime = first.time;
    } else {
      nextTime = 0;
    }
    return tmp;
  }

  public void removeAll() {
    TimeEvent t = first;
    while(t != null) {
      TimeEvent clr = t;
      t = t.nextEvent;
      clr.nextEvent = null;
      clr.time = 0;
      clr.scheduledIn = null;
    }
    first = null;
    eventCount = 0;
  }
  
  public void print(PrintStream out) {
    TimeEvent t = first;
    out.print("nxt: " + nextTime + " [");
    while(t != null) {
      out.print(t.getShort());
      t = t.nextEvent;
      if (t != null) out.print(", ");
    }
    out.println("]");
  }
} // LLEventQueue

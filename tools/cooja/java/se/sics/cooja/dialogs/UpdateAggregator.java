/*
 * Copyright (c) 2010, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: UpdateAggregator.java,v 1.2 2010/03/26 10:11:50 nifi Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.List;

import javax.swing.Timer;

import se.sics.cooja.plugins.LogListener;

/**
 * Help class to avoid EventQueue flooding by aggregating several update events.
 * 
 * To be used by plugins et. al. that receive updates at a high rate 
 * (such as new Log Output messages), and must handle them from the Event thread.
 * 
 * @author Fredrik Osterlind, Niclas Finne
 *
 * @param <A> Event whose update event are aggregated
 * @see LogListener
 */
public abstract class UpdateAggregator<A> {
  private static final int DEFAULT_MAX_PENDING = 256;
  private int maxPending;
  
  private ArrayList<A> pending;
  private Timer t;

  /**
   * @param interval Max interval (ms)
   */
  public UpdateAggregator(int interval) {
    this(interval, DEFAULT_MAX_PENDING);
  }
  /**
   * @param delay Max interval (ms)
   * @param maxEvents Max pending events (default 256)
   */
  public UpdateAggregator(int interval, int maxEvents) {
    this.maxPending = maxEvents;
    pending = new ArrayList<A>();
    t = new Timer(interval, new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        consume.run();
      }
    });
    t.setInitialDelay(0);
    t.setCoalesce(true);
    t.setRepeats(true);
  }

  /**
   * Consumer: called from event queue
   */
  private Runnable consume = new Runnable() {
    public void run() {
      if (pending.isEmpty()) {
        return;
      };

      List<A> q = getPending();
      if (q != null) {
        /* Handle objects */
        handle(q);
      }

      synchronized (UpdateAggregator.this) {
        UpdateAggregator.this.notifyAll();
      }
    }
  };
  
  /**
   * @param l All events since last update
   */
  protected abstract void handle(List<A> l);
  
  private synchronized List<A> getPending() {
    /* Queue pending packets */
    ArrayList<A> tmp = pending;
    pending = new ArrayList<A>();
    return tmp;
  }

  /**
   * @param a Add new event (any thread). May block.
   */
  public synchronized void add(A a) {
    try {
      while (pending.size() > maxPending) {
        /* Delay producer thread; events are coming in too fast */
        EventQueue.invokeLater(consume); /* Request immediate consume */
        wait(t.getDelay());
      }
    } catch (InterruptedException e) {
    }
    pending.add(a);
  }

  public void start() {
    t.start();
  }
  public void stop() {
    t.stop();
  }
}

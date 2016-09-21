/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 * This file is part of MSPSim.
 *
 * -----------------------------------------------------------------
 *
 * PortListenerProxy
 *
 * Author  : Niclas Finne
 * Created : Tue Dec 7 18:25:00 2011
 */

package se.sics.mspsim.core;
import se.sics.mspsim.util.ArrayUtils;

public class PortListenerProxy implements PortListener {

    private PortListener[] portListeners;

    public PortListenerProxy(PortListener listen1, PortListener listen2) {
        portListeners = new PortListener[] { listen1, listen2 };
    }

    public static PortListener addPortListener(PortListener portListener, PortListener listener) {
        if (portListener == null) {
            return listener;
        }
        if (portListener instanceof PortListenerProxy) {
            return ((PortListenerProxy)portListener).add(listener);
        }
        return new PortListenerProxy(portListener, listener);
    }

    public static PortListener removePortListener(PortListener portListener, PortListener listener) {
        if (portListener == listener) {
            return null;
        }
        if (portListener instanceof PortListenerProxy) {
            return ((PortListenerProxy)portListener).remove(listener);
        }
        return portListener;
    }

    public PortListener add(PortListener mon) {
        portListeners = ArrayUtils.add(PortListener.class, portListeners, mon);
        return this;
    }

    public PortListener remove(PortListener listener) {
        PortListener[] listeners = ArrayUtils.remove(portListeners, listener);
        if (listeners == null) {
            return null;
        }
        if (listeners.length == 1) {
            return listeners[0];
        }
        portListeners = listeners;
        return this;
    }

    @Override
    public void portWrite(IOPort source, int data) {
        PortListener[] listeners = this.portListeners;
        for(PortListener l : listeners) {
            l.portWrite(source, data);
        }
    }

}

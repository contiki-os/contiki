/**
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * SentillaUSBGui
 *
 * Author  : Joakim Eriksson, Niclas Finne
 * Created : 7 jul 2010
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.platform.sentillausb;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;

import se.sics.mspsim.core.StateChangeListener;
import se.sics.mspsim.platform.AbstractNodeGUI;

public class SentillaUSBGui extends AbstractNodeGUI {

    private static final long serialVersionUID = -692114865639672141L;

    private static final Color[] RED = new Color[] {
        new Color(0xff, 0x40, 0x40, 0xa0),
        new Color(0xff,0x40,0x40,0xff),
        new Color(0xff,0x60,0x60,0xff),
        new Color(0xff,0xa0,0xa0,0xff)
    };
    private static final Color[] GREEN = new Color[] {
        new Color(0x40, 0xff, 0x40, 0xa0),
        new Color(0x40,0xff,0x40,0xff),
        new Color(0x60,0xff,0x60,0xff),
        new Color(0xa0,0xff,0xa0,0xff)
    };

    private static final int LEDS_GREEN_X = 8;
    private static final int LEDS_RED_X = 126;
    private static final int LEDS_Y = 89;

    private static final Rectangle LEDS_CLIP =
        new Rectangle(LEDS_GREEN_X, LEDS_Y, LEDS_RED_X + 10 - LEDS_GREEN_X, 13);

    private final SentillaUSBNode node;
    private final StateChangeListener ledsListener = new StateChangeListener() {
        public void stateChanged(Object source, int oldState, int newState) {
            repaint(LEDS_CLIP);
        }
    };

    public SentillaUSBGui(SentillaUSBNode node) {
        super("SentillaUSBGui", "images/sentilla-usb.jpg");
        this.node = node;
    }

    @Override
    protected void startGUI() {
        node.getLeds().addStateChangeListener(ledsListener);
    }

    @Override
    protected void stopGUI() {
        node.getLeds().removeStateChangeListener(ledsListener);
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);

        Color old = g.getColor();
        ((Graphics2D)g).setRenderingHint(RenderingHints.KEY_ANTIALIASING,
            RenderingHints.VALUE_ANTIALIAS_ON);
        if (node.greenLed) {
            paintLed(g, LEDS_GREEN_X, GREEN);
        }
        if (node.redLed) {
            paintLed(g, LEDS_RED_X, RED);
        }
        g.setColor(old);
    }

    protected void paintLed(Graphics g, int x, Color[] colors) {
        g.setColor(colors[0]);
        g.fillOval(x, LEDS_Y, 10, 13);
        g.setColor(colors[1]);
        g.fillOval(x + 2, LEDS_Y + 2, 6, 7);
        g.setColor(colors[2]);
        g.fillOval(x + 3, LEDS_Y + 3, 4, 5);
        g.setColor(colors[3]);
        g.fillRect(x + 3, LEDS_Y + 3, 2, 2);
    }

}

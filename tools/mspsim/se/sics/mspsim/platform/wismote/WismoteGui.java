/**
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 */

package se.sics.mspsim.platform.wismote;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import se.sics.mspsim.chip.Leds;
import se.sics.mspsim.core.StateChangeListener;
import se.sics.mspsim.platform.AbstractNodeGUI;

/**
 * @author Niclas Finne
 */
public class WismoteGui extends AbstractNodeGUI {

    private static final long serialVersionUID = -8713047619139235630L;
    private static final int RED1_X = 172;
    private static final int GREEN_X = 182;
    private static final int RED2_X = 192;
    private static final int LED_Y = 40;
    private static final int LED_HEIGHT = 11;
    private static final int LED_WIDTH = 7;

    private static final Color GREEN_TRANS = new Color(0x40, 0xf0, 0x40, 0xa0);
    private static final Color RED_TRANS = new Color(0xf0, 0x40, 0x40, 0xa0);

    private static final Color GREEN_C = new Color(0xff60ff60);
    private static final Color RED_C = new Color(0xffff8000);

    private static final Color BUTTON_C = new Color(0x60ffffff);

    private static final Rectangle LEDS_BOUNDS = new Rectangle(RED1_X - 2,
            LED_Y - 1, RED2_X - RED1_X + LED_HEIGHT, LED_WIDTH);

    private final WismoteNode node;
    private final StateChangeListener ledsListener = new StateChangeListener() {
        public void stateChanged(Object source, int oldState, int newState) {
            repaint(LEDS_BOUNDS);
        }
    };
    private boolean buttonDown = false;
    private boolean resetDown = false;

    public WismoteGui(WismoteNode node) {
        super("WismoteGui", "images/wismote.jpg");
        this.node = node;
    }

    protected void startGUI() {
        MouseAdapter mouseHandler = new MouseAdapter() {

            // For the button sensor and reset button on the Sky nodes.
            public void mousePressed(MouseEvent e) {
                int x = e.getX();
                int y = e.getY();
                if (x > 6 && x < 19) {
                    if (y > 236 && y < 251) {
                        buttonDown = true;
                        WismoteGui.this.node.getButton().setPressed(true);
                        repaint(7, 237, 11, 13);
                    } else if (y > 268 && y < 282) {
                        resetDown = true;
                        repaint(7, 269, 11, 13);
                    }
                }
            }

            public void mouseReleased(MouseEvent e) {
                if (buttonDown) {
                    buttonDown = false;
                    WismoteGui.this.node.getButton().setPressed(false);
                    repaint(7, 237, 11, 13);

                } else if (resetDown) {
                    int x = e.getX();
                    int y = e.getY();
                    resetDown = false;
                    repaint(7, 269, 11, 13);
                    if (x > 6 && x < 19 && y > 268 && y < 282) {
                        WismoteGui.this.node.getCPU().reset();
                    }
                }
            }
        };

        this.addMouseListener(mouseHandler);
        node.getLeds().addStateChangeListener(ledsListener);
    }

    protected void stopGUI() {
        node.getLeds().removeStateChangeListener(ledsListener);
    }

    protected void paintComponent(Graphics g) {
        Color old = g.getColor();

        super.paintComponent(g);

        // Display all active LEDs
        Leds leds = node.getLeds();
        int l = leds.getLeds();
        if ((l & 1) != 0) {
            g.setColor(RED_TRANS);
            g.fillOval(RED1_X - 2, LED_Y - 1, LED_HEIGHT, LED_WIDTH);
            g.setColor(RED_C);
            g.fillOval(RED1_X, LED_Y, LED_HEIGHT - 5, LED_WIDTH - 2);
        }
        if ((l & 2) != 0) {
            g.setColor(GREEN_TRANS);
            g.fillOval(GREEN_X - 2, LED_Y - 1, LED_HEIGHT, LED_WIDTH);
            g.setColor(GREEN_C);
            g.fillOval(GREEN_X, LED_Y, LED_HEIGHT - 5, LED_WIDTH - 2);
        }
        if ((l & 4) != 0) {
            g.setColor(RED_TRANS);
            g.fillOval(RED2_X - 2, LED_Y - 1, LED_HEIGHT, LED_WIDTH);
            g.setColor(RED_C);
            g.fillOval(RED2_X, LED_Y, LED_HEIGHT - 5, LED_WIDTH - 2);
        }

        if (buttonDown) {
            g.setColor(BUTTON_C);
            g.fillOval(8, 236, 9, 9);
        }
        if (resetDown) {
            g.setColor(BUTTON_C);
            g.fillOval(8, 271, 9, 9);
        }
        g.setColor(old);
    }

}

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

package se.sics.mspsim.platform.z1;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import se.sics.mspsim.core.StateChangeListener;
import se.sics.mspsim.platform.AbstractNodeGUI;

public class Z1Gui extends AbstractNodeGUI {

    private static final long serialVersionUID = 7016480889484768582L;

    private static final int GREEN_Y = 88;
    private static final int BLUE_Y = 99;
    private static final int RED_Y = 77;
    private static final int LED_X = 184;
    private static final int LED_WIDTH = 11;
    private static final int LED_HEIGHT = 6;

    private static final Color BLUE_TRANS = new Color(0x80, 0x80, 0xff, 0xa0);
    private static final Color GREEN_TRANS = new Color(0x40, 0xf0, 0x40, 0xa0);
    private static final Color RED_TRANS = new Color(0xf0, 0x40, 0x40, 0xa0);

    private static final Color BLUE_C = new Color(0xffa0a0ff);
    private static final Color GREEN_C = new Color(0xff60ff60);
    private static final Color RED_C = new Color(0xffff8000);

    private static final Color BUTTON_C = new Color(0x60ffffff);

    private static final Rectangle LEDS_BOUNDS =
      new Rectangle(LED_X - 2, RED_Y - 2, LED_WIDTH + 1, BLUE_Y - RED_Y + LED_HEIGHT + 1);

    private boolean buttonDown = false;
    private boolean resetDown = false;

    private final Z1Node node;
    private final StateChangeListener ledsListener = new StateChangeListener() {
        public void stateChanged(Object source, int oldState, int newState) {
            repaint(LEDS_BOUNDS);
        }
    };

    public Z1Gui(Z1Node node) {
        super("Z1Gui", "images/z1.jpg");
        this.node = node;
    }

    protected void startGUI() {
        MouseAdapter mouseHandler = new MouseAdapter() {

            // For the button sensor and reset button on the Sky nodes.
            public void mousePressed(MouseEvent e) {
                int x = e.getX();
                int y = e.getY();
                if (x > 53 && x < 66) {
                    if (y > 231 && y < 246) {
                        buttonDown = true;
                        Z1Gui.this.node.getButton().setPressed(true);
                        repaint(53, 231, 14, 16);
                    } else if (y > 260 && y < 273) {
                        resetDown = true;
                        repaint(53, 260, 14, 13);
                    }
                }
            }

            public void mouseReleased(MouseEvent e) {
                if (buttonDown) {
                    buttonDown = false;
                    Z1Gui.this.node.getButton().setPressed(false);
                    repaint(53, 231, 14, 16);

                } else if (resetDown) {
                    int x = e.getX();
                    int y = e.getY();
                    resetDown = false;
                    if (x > 53 && x < 66 && y > 260 && y < 273) {
                        Z1Gui.this.node.getCPU().reset();
                    }
                    repaint(53, 260, 14, 13);
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

        // Display all active leds
        if (node.redLed) {
            g.setColor(RED_TRANS);
            g.fillOval(LED_X - 2, RED_Y - 1, LED_WIDTH, LED_HEIGHT);
            g.setColor(RED_C);
            g.fillOval(LED_X, RED_Y, LED_WIDTH - 5, LED_HEIGHT - 2);
        }
        if (node.greenLed) {
            g.setColor(GREEN_TRANS);
            g.fillOval(LED_X - 2, GREEN_Y - 1, LED_WIDTH, LED_HEIGHT);
            g.setColor(GREEN_C);
            g.fillOval(LED_X, GREEN_Y, LED_WIDTH - 5, LED_HEIGHT - 2);
        }
        if (node.blueLed) {
            g.setColor(BLUE_TRANS);
            g.fillOval(LED_X - 2, BLUE_Y - 1, LED_WIDTH, LED_HEIGHT);
            g.setColor(BLUE_C);
            g.fillOval(LED_X, BLUE_Y, LED_WIDTH - 5, LED_HEIGHT - 2);
        }
        if (buttonDown) {
            g.setColor(BUTTON_C);
            g.fillOval(55, 234, 9, 9);
        }
        if (resetDown) {
            g.setColor(BUTTON_C);
            g.fillOval(55, 262, 9, 9);
        }
        g.setColor(old);
    }

}

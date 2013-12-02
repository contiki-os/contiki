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
 */

package org.contikios.cooja.mspmote.interfaces;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.util.Collection;
import javax.swing.JPanel;
import org.jdom.Element;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.interfaces.LED;
import org.contikios.cooja.mspmote.MspMote;
import se.sics.mspsim.chip.Leds;
import se.sics.mspsim.core.StateChangeListener;

/**
 * @author Fredrik Osterlind, Niclas Finne
 */
@ClassDescription("Leds")
public class MspLED extends LED {

    private final Leds leds;
    private Color[] onColors;
    private Color[] offColors;

    public MspLED(Mote mote) {
        final MspMote mspMote = (MspMote) mote;
        leds = mspMote.getCPU().getChip(Leds.class);
        if (leds == null) {
            throw new IllegalStateException("Mote is not equipped with leds");
        }
        leds.addStateChangeListener(new StateChangeListener() {

            public void stateChanged(Object source, int oldState, int newState) {
                setChanged();
                notifyObservers();
            }

        });
    }

    private void checkColors() {
        if (onColors == null) {
            Color[] onColors = new Color[leds.getLedsCount()];
            Color[] offColors = new Color[leds.getLedsCount()];
            for (int i = 0, n = onColors.length; i < n; i++) {
                onColors[i] = new Color(leds.getLedsColor(i));
                offColors[i] = onColors[i].darker().darker();
            }
            this.onColors = onColors;
            this.offColors = offColors;
        }
    }

    public int getLedsCount() {
        return leds.getLedsCount();
    }

    public int getLeds() {
        return leds.getLeds();
    }

    public boolean isLedOn(int led) {
        return leds.isLedOn(led);
    }

    public Color getLedColor(int led) {
        checkColors();
        return onColors[led];
    }

    @Override
    public boolean isAnyOn() {
        return leds.getLeds() != 0;
    }

    @Override
    public boolean isGreenOn() {
        return leds.isLedOn(0);
    }

    @Override
    public boolean isYellowOn()  {
        return leds.isLedOn(1);
    }

    @Override
    public boolean isRedOn() {
        return leds.isLedOn(2);
    }

    @Override
    public JPanel getInterfaceVisualizer() {
        checkColors();
        return new LedsPanel();
    }

    @Override
    public void releaseInterfaceVisualizer(JPanel panel) {
        if (panel instanceof LedsPanel) {
            ((LedsPanel) panel).close();
        }
    }

    @Override
    public Collection<Element> getConfigXML() {
        return null;
    }

    @Override
    public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    }

    private class LedsPanel extends JPanel implements StateChangeListener {

        private static final long serialVersionUID = 2664611402441057313L;
        private static final int D = 25;
        private static final int S = D + 15;

        public LedsPanel() {
            super(null);

            Dimension d = new Dimension(5 + S * (leds.getLedsCount() - 1) + D + 5, 5 + D + 5);
            setMinimumSize(d);
            setPreferredSize(d);
            leds.addStateChangeListener(this);
        }

        public void close() {
            leds.removeStateChangeListener(this);
        }

        @Override
        public void stateChanged(Object source, int oldState, int newState) {
            repaint();
        }

        @Override
        protected void paintComponent(Graphics g) {
            final int count = leds.getLedsCount();
            final int height = getHeight();
            final int width = getWidth();
            final int y = (height - D) / 2;
            int x = width - D - (width - (S * (count - 1) + D)) / 2;
            int l = leds.getLeds();

            g.setColor(getBackground());
            g.fillRect(0, 0, width, height);

            ((Graphics2D)g).setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                    RenderingHints.VALUE_ANTIALIAS_ON);

            for (int i = 0; i < count; i++) {
                if ((l & 1) != 0) {
                    g.setColor(onColors[i]);
                    g.fillOval(x, y, D, D);
                    g.setColor(Color.BLACK);
                    g.drawOval(x, y, D, D);
                } else {
                    g.setColor(offColors[i]);
                    g.fillOval(x + 5, y + 5, D - 10, D - 10);
                }
                x -= S;
                l >>= 1;
            }
        }
    }

}

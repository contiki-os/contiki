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

package se.sics.cooja.mspmote.interfaces;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Collection;
import javax.swing.JButton;
import javax.swing.JPanel;
import org.jdom.Element;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.Simulation;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.interfaces.Button;
import se.sics.cooja.mspmote.MspMote;

/**
 * @author Fredrik Osterlind, Niclas Finne
 */
@ClassDescription("Button")
public class MspButton extends Button {

    private final Simulation sim;
    private final se.sics.mspsim.chip.Button button;

    public MspButton(Mote mote) {
        final MspMote mspMote = (MspMote) mote;
        sim = mote.getSimulation();
        button = mspMote.getCPU().getChip(se.sics.mspsim.chip.Button.class);
        if (button == null) {
            throw new IllegalStateException("Mote is not equipped with a button");
        }
    }

    @Override
    public void clickButton() {
        sim.invokeSimulationThread(new ButtonClick());
    }

    @Override
    public void pressButton() {
        sim.invokeSimulationThread(new Runnable() {
            public void run() {
                button.setPressed(true);
            }
        });
    }

    @Override
    public void releaseButton() {
        sim.invokeSimulationThread(new Runnable() {
            public void run() {
                button.setPressed(false);
            }
        });
    }

    @Override
    public boolean isPressed() {
        return button.isPressed();
    }

    @Override
    public JPanel getInterfaceVisualizer() {
        final JPanel panel = new JPanel();
        final JButton clickButton = new JButton("Click button");

        panel.add(clickButton);

        clickButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                clickButton();
            }
        });

        return panel;
    }

    @Override
    public void releaseInterfaceVisualizer(JPanel panel) {
    }

    @Override
    public Collection<Element> getConfigXML() {
        return null;
    }

    @Override
    public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    }

    private class ButtonClick extends TimeEvent implements Runnable {

        public ButtonClick() {
            super(0);
        }

        @Override
        public void run() {
            button.setPressed(true);
            sim.scheduleEvent(this, sim.getSimulationTime() + Simulation.MILLISECOND);
        }

        @Override
        public void execute(long t) {
            button.setPressed(false);
        }
    }
}

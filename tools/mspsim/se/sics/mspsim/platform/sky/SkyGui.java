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
 * SkyGui
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.platform.sky;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import se.sics.mspsim.core.StateChangeListener;
import se.sics.mspsim.platform.AbstractNodeGUI;

public class SkyGui extends AbstractNodeGUI {

  private static final long serialVersionUID = 7753659717805292786L;
  
  public static final int GREEN_Y = 40;
  public static final int BLUE_Y = 46;
  public static final int RED_Y = 34;
  public static final int LED_X = 10;

  public static final Color BLUE_TRANS = new Color(0x80,0x80,0xff,0xa0);
  public static final Color GREEN_TRANS = new Color(0x40, 0xf0, 0x40, 0xa0);
  public static final Color RED_TRANS = new Color(0xf0, 0x40, 0x40, 0xa0);

  public static final Color BLUE_C = new Color(0xffa0a0ff);
  public static final Color GREEN_C = new Color(0xff60ff60);
  public static final Color RED_C = new Color(0xffff8000);

  private static final Rectangle LEDS_BOUNDS = new Rectangle(LED_X, RED_Y, 9, BLUE_Y - RED_Y + 5);

  private final MoteIVNode node;
  private final StateChangeListener ledsListener = new StateChangeListener() {
      public void stateChanged(Object source, int oldState, int newState) {
          repaint(LEDS_BOUNDS);
      }
  };

  public SkyGui(MoteIVNode node) {
    super("SkyGui", "images/sky.jpg");
    this.node = node;
  }

  protected void startGUI() {
    MouseAdapter mouseHandler = new MouseAdapter() {

	private boolean buttonDown = false;
	private boolean resetDown = false;

	// For the button sensor and reset button on the Sky nodes.
	public void mousePressed(MouseEvent e) {
	  int x = e.getX();
	  int y = e.getY();
	  if (x > 122 && x < 135) {
	    if (y > 41 && y < 55) {
	      buttonDown = true;
	      SkyGui.this.node.getButton().setPressed(true);
	    } else if (y > 72 && y < 85) {
	      resetDown = true;
	    }
	  }
	}

	public void mouseReleased(MouseEvent e) {
	  if (buttonDown) {
	    buttonDown = false;
	    SkyGui.this.node.getButton().setPressed(false);

	  } else if (resetDown) {
	    int x = e.getX();
	    int y = e.getY();
	    resetDown = false;
	    if (x > 122 && x < 135 && y > 72 && y < 85) {
	      SkyGui.this.node.getCPU().reset();
	    }
	  }
	}
      };

    this.addMouseListener(mouseHandler);
    node.getLeds().addStateChangeListener(ledsListener);
  }

  protected void paintComponent(Graphics g) {
    Color old = g.getColor();

    super.paintComponent(g);

    // Display all active leds
    if (node.redLed) {
      g.setColor(RED_TRANS);
      g.fillOval(LED_X - 2, RED_Y - 1, 9, 5);
      g.setColor(RED_C);
      g.fillOval(LED_X, RED_Y, 4, 3);
    }
    if (node.greenLed) {
      g.setColor(GREEN_TRANS);
      g.fillOval(LED_X - 2, GREEN_Y - 1, 9, 5);
      g.setColor(GREEN_C);
      g.fillOval(LED_X, GREEN_Y, 4, 3);
    }
    if (node.blueLed) {
      g.setColor(BLUE_TRANS);
      g.fillOval(LED_X - 2, BLUE_Y - 1, 9, 5);
      g.setColor(BLUE_C);
      g.fillOval(LED_X, BLUE_Y, 4, 3);
    }
    g.setColor(old);
  }

  protected void stopGUI() {    
      node.getLeds().removeStateChangeListener(ledsListener);
  }

}

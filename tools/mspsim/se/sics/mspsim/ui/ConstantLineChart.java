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
 * ConstantLineChart
 *
 * Authors : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : April 26 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.ui;

import java.awt.Graphics2D;

public class ConstantLineChart extends AbstractChart {

  private int lineY;

  public ConstantLineChart(int y) {
    this.lineY = y;
  }

  public ConstantLineChart(String name, int y) {
    setName(name);
    this.lineY = y;
    updateMinMax();
  }

  public void drawChart(Graphics2D g, double xfac, double yfac, int width, int height) {
    int zero = height;
    if (getMinY() < 0) {
      zero += (int) (yfac * getMinY());
    }
    int y = (int) (zero - (yfac * lineY));
    g.drawLine(0, y, width, y);
  }

  @Override
  protected void updateMinMax() {
    this.minx = 0;
    this.maxx = 1;
    this.miny = this.maxy = lineY;
  }

}

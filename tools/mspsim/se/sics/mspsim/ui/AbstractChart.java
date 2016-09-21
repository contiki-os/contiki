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
 * AbstractChart
 *
 * Authors : Adam Dunkels, Joakim Eriksson, Niclas Finne
 * Created : May 3 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.ui;

import java.awt.Graphics2D;
import java.util.Hashtable;

/**
 *
 */
public abstract class AbstractChart implements Chart {

  protected String name;
  protected double minx, maxx;
  protected double miny, maxy;
  protected boolean autoscale = true;
  private Hashtable<String,Object> config = new Hashtable<String,Object>();

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public boolean isAutoscaling() {
    return autoscale;
  }

  public void setAutoscale() {
    if (!this.autoscale) {
      this.autoscale = true;
      updateMinMax();
    }
  }

  public void setMinMax(double minVal, double maxVal) {
    miny = minVal;
    maxy = maxVal;
    autoscale = false;
  }

  public double getMinX() {
    return minx;
  }

  public double getMaxX() {
    return maxx;
  }

  public double getMinY() {
    return miny;
  }

  public double getMaxY() {
    return maxy;
  }

  public Object getConfig(String param) {
    return getConfig(param, null);
  }

  public Object getConfig(String param, Object defVal) {
    Object retVal = config.get(param);
    return retVal != null ? retVal : defVal;
  }

  public void setConfig(String param, Object value) {
    if (value != null) {
      config.put(param, value);
    } else {
      config.remove(param);
    }
  }

  protected abstract void updateMinMax();

  public abstract void drawChart(Graphics2D g, double xfac, double yfac,
      int width, int height);

}

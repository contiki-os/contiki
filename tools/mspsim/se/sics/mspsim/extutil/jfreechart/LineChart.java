/**
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: $
 *
 * -----------------------------------------------------------------
 *
 * LineChart
 *
 * Author  : Joakim Eriksson
 * Created : 17 apr 2008
 * Updated : $Date:$
 *           $Revision:$
 */
package se.sics.mspsim.extutil.jfreechart;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import javax.swing.JComponent;
import javax.swing.JPanel;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.DefaultXYItemRenderer;
import org.jfree.data.general.Series;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

/**
 * @author joakim
 *
 */
public class LineChart extends JFreeWindowDataHandler {

  private JPanel panel;
  private JFreeChart chart;
  private XYSeriesCollection dataset;
  private DefaultXYItemRenderer renderer = new DefaultXYItemRenderer();

  public LineChart() {
    NumberAxis domain = new NumberAxis("Time");
    NumberAxis range = new NumberAxis("Value");
    XYPlot xyplot = new XYPlot();
    xyplot.setDomainAxis(domain);
    xyplot.setRangeAxis(range);
    xyplot.setDataset(dataset = new XYSeriesCollection());
    xyplot.setRenderer(renderer);

    domain.setAutoRange(true);
    domain.setAutoRangeIncludesZero(false);
    domain.setLowerMargin(0.0);
    domain.setUpperMargin(0.0);
    domain.setTickLabelsVisible(true);
    range.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
    chart = new JFreeChart("Data",  JFreeChart.DEFAULT_TITLE_FONT, xyplot, true);
    ChartPanel chartPanel = new ChartPanel(chart);
    panel = new JPanel();
    panel.setLayout(new BorderLayout());
    panel.setPreferredSize(new Dimension(400, 200));
    panel.add(chartPanel, BorderLayout.CENTER);
  }
 
  public JComponent getComponent() {
    return panel;
  }

  public int getDataSeriesCount() {
    return dataset.getSeriesCount();
  }

  public Series getDataSeries(int index) {
    while (index >= dataset.getSeriesCount()) {
      addSeries();
    }
    return dataset.getSeries(index);
  }

  public void setProperty(String param, String[] args) {
    if ("title".equals(param)) {
      chart.setTitle(args[0]);
    }
  }

  private void addSeries() {
    XYSeries dataSeries = new XYSeries("series " + (getDataSeriesCount() + 1));
    dataSeries.setMaximumItemCount(200);
//    renderer.setSeriesPaint(0, Color.black);
    renderer.setSeriesShapesVisible(getDataSeriesCount(), false);
    dataset.addSeries(dataSeries);
  }
  
  int point = 0;
  public void lineRead(String line) {
    String parts[] = line.trim().split(" ");
    while (parts.length > getDataSeriesCount()) {
      addSeries();
    }
    for (int i = 0; i < parts.length; i++) {
      dataset.getSeries(i).add(point, atod(parts[i], 0));
    }
    point++;
    panel.repaint();
  }
  
  @Override
  public void setProperty(int index, String param, String[] args) {
    super.setProperty(index, param, args);
    if ("color".equals(param)) {
      renderer.setSeriesPaint(index, Color.decode(args[0]));
      panel.repaint();
    }
  }
}

/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: SeqnoChartPanel.java,v 1.1 2010/08/31 13:05:40 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * TimeChartPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2010/08/31 13:05:40 $
 *           $Revision: 1.1 $
 */

package se.sics.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;

import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

import se.sics.contiki.collect.CollectServer;
import se.sics.contiki.collect.Node;
import se.sics.contiki.collect.SensorData;
import se.sics.contiki.collect.SensorDataAggregator;
import se.sics.contiki.collect.Visualizer;

/**
 *
 */
public class SeqnoChartPanel extends JPanel implements Visualizer {

  private static final long serialVersionUID = 4302047079820959307L;

  protected final CollectServer server;
  protected final String title;
  protected final XYSeriesCollection dataSet;
  protected final XYSeries series;
  protected final JFreeChart chart;
  protected final ChartPanel chartPanel;

  private Node[] selectedNodes;

  public SeqnoChartPanel(CollectServer server, String title,
      String chartTitle, String timeAxisLabel, String valueAxisLabel) {
    super(new BorderLayout());
    this.server = server;
    this.title = title;
    this.series = new XYSeries(chartTitle);
    this.dataSet = new XYSeriesCollection(this.series);
    this.chart = ChartFactory.createXYLineChart(
        chartTitle, timeAxisLabel, valueAxisLabel, this.dataSet, PlotOrientation.VERTICAL,
        false, false, false
    );
    this.chartPanel = new ChartPanel(chart);
    this.chartPanel.setPreferredSize(new Dimension(500, 270));
    add(chartPanel, BorderLayout.CENTER);
  }

  @Override
  public String getTitle() {
    return title;
  }

  @Override
  public Component getPanel() {
    return this;
  }

  @Override
  public void nodeAdded(Node node) {
    // Ignore
  }

  @Override
  public void nodesSelected(Node[] nodes) {
    if (this.selectedNodes != nodes) {
      this.selectedNodes = nodes;
      if (isVisible()) {
        updateCharts();
      }
    }
  }

  @Override
  public void nodeDataReceived(SensorData data) {
    if (isVisible() && selectedNodes != null) {
      Node node = data.getNode();
      for (int i = 0, n = selectedNodes.length; i < n; i++) {
        if (node == selectedNodes[i]) {
          updateCharts();
          break;
        }
      }
    }
  }

  @Override
  public void clearNodeData() {
    if (isVisible()) {
      updateCharts();
    }
  }

  private void updateCharts() {
    series.clear();
    if (this.selectedNodes != null) {
      int minSeqno = Integer.MAX_VALUE;
      int maxSeqno = Integer.MIN_VALUE;
      for(Node node: this.selectedNodes) {
        SensorDataAggregator sda = node.getSensorDataAggregator();
        if (sda.getMinSeqno() < minSeqno) {
          minSeqno = sda.getMinSeqno();
        }
        if (sda.getMaxSeqno() > maxSeqno) {
          maxSeqno = sda.getMaxSeqno();
        }
      }
      if (minSeqno < maxSeqno) {
        int[] seqnos = new int[maxSeqno - minSeqno + 1];
        for(Node node: this.selectedNodes) {
          for(int i = 0, n = node.getSensorDataCount(); i < n; i++) {
            seqnos[node.getSensorData(i).getSeqno() - minSeqno]++;
          }
        }
        for(int i = 0; i < seqnos.length; i++) {
          series.add(i + minSeqno, seqnos[i]);
        }
      }
    }
  }

  public double getRangeMinimumSize() {
    return chart.getXYPlot().getRangeAxis().getAutoRangeMinimumSize();
  }

  public void setRangeMinimumSize(double size) {
    chart.getXYPlot().getRangeAxis().setAutoRangeMinimumSize(size);
  }

  public void setVisible(boolean visible) {
    if (visible) {
      updateCharts();
    } else {
      series.clear();
    }
    super.setVisible(visible);
  }

}

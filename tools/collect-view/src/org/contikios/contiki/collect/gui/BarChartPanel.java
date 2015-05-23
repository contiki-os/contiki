/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *
 * -----------------------------------------------------------------
 *
 * PowerPanel
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 5 jul 2008
 * Updated : $Date: 2010/11/03 14:53:05 $
 *           $Revision: 1.1 $
 */

package org.contikios.contiki.collect.gui;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.GradientPaint;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JPanel;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.renderer.category.BarRenderer;
import org.jfree.data.category.DefaultCategoryDataset;
import org.contikios.contiki.collect.CollectServer;
import org.contikios.contiki.collect.Node;
import org.contikios.contiki.collect.SensorData;
import org.contikios.contiki.collect.Visualizer;

/**
 *
 */
public abstract class BarChartPanel extends JPanel implements Visualizer {

  private static final long serialVersionUID = 7664283678708048061L;

  protected final CollectServer server;
  protected final String category;
  protected final String title;
  protected final String[] categories;
  protected final JFreeChart chart;
  protected final ChartPanel chartPanel;
  protected final DefaultCategoryDataset dataset;

  private boolean isShowingAllNodes = false;
  private int categoryOrder = 0;

  protected BarChartPanel(CollectServer server, String category, String title,
      String chartTitle, String domainAxisLabel, String valueAxisLabel,
      String[] categories) {
    this(server, category, title, chartTitle, domainAxisLabel, valueAxisLabel, categories, true);
  }

  protected BarChartPanel(CollectServer server, String category, String title,
      String chartTitle, String domainAxisLabel, String valueAxisLabel,
      String[] categories, boolean stackedChart) {
    super(new BorderLayout());
    this.server = server;
    this.category = category;
    this.title = title;
    this.categories = categories;

    /* Create chart with power of all nodes */
    dataset = new DefaultCategoryDataset();
    if (stackedChart) {
      this.chart = ChartFactory.createStackedBarChart(chartTitle,
          domainAxisLabel, valueAxisLabel, dataset, PlotOrientation.VERTICAL,
          categories.length > 1, true, false);
    } else {
      this.chart = ChartFactory.createBarChart(chartTitle,
          domainAxisLabel, valueAxisLabel, dataset, PlotOrientation.VERTICAL,
          categories.length > 1, true, false);
      if (categories.length > 1) {
        CategoryPlot plot = chart.getCategoryPlot();
        BarRenderer renderer = (BarRenderer) plot.getRenderer();
        renderer.setItemMargin(0);
      }
    }
    this.chartPanel = new ChartPanel(chart, false);
    this.chartPanel.setPreferredSize(new Dimension(500, 270));
    if (categories.length > 1) {
      this.chartPanel.addMouseListener(new MouseAdapter() {
        public void mouseClicked(MouseEvent e) {
          categoryOrder++;
          updateCharts();
        }
      });
    }

    CategoryPlot plot = (CategoryPlot) chart.getPlot();
    BarRenderer renderer = (BarRenderer) plot.getRenderer();
    if (categories.length < 3) {
      renderer.setDrawBarOutline(false);

      GradientPaint gp = new GradientPaint(0.0f, 0.0f, Color.RED,
          0.0f, 0.0f, new Color(128, 0, 0));
      renderer.setSeriesPaint(0, gp);
      if (categories.length > 1) {
        gp = new GradientPaint(0.0f, 0.0f, Color.BLUE,
            0.0f, 0.0f, new Color(0, 0, 128));
        renderer.setSeriesPaint(1, gp);
      }
    } else {
      renderer.setDrawBarOutline(true);
    }

    add(chartPanel, BorderLayout.CENTER);
  }

  @Override
  public String getCategory() {
    return category;
  }
  
  @Override
  public String getTitle() {
    return title;
  }

  @Override
  public Component getPanel() {
    return this;
  }

  public boolean isShowingAllNodes() {
    return isShowingAllNodes;
  }
  
  public void setShowingAllNodes(boolean isShowingAllNodes) {
    if (this.isShowingAllNodes != isShowingAllNodes) {
      this.isShowingAllNodes = isShowingAllNodes;
      if (isVisible()) {
        updateCharts();
      }
    }
  }
  
  @Override
  public void nodeAdded(Node node) {
    if (isVisible()) {
      int count = node.getSensorDataCount();
      if (count > 0 || isShowingAllNodes) {
        addNode(node);
      }
      if (count > 0) {
        addSensorData(node.getSensorData(count - 1));
      }
    }
  }

  @Override
  public void nodesSelected(Node[] nodes) {
  }

  @Override
  public void nodeDataReceived(SensorData data) {
    if (isVisible()) {
      addSensorData(data);
    }
  }

  @Override
  public void clearNodeData() {
    if (isVisible()) {
      updateCharts();
    }
  }

  private void updateCharts() {
    dataset.clear();
    Node[] nodes = server.getNodes();
    if (nodes != null) {
      for (int i = 0, n = nodes.length; i < n; i++) {
        int count = nodes[i].getSensorDataCount();
        if (count > 0 || isShowingAllNodes) {
          addNode(nodes[i]);
        }
        if (count > 0) {
          addSensorData(nodes[i].getSensorData(count - 1));
        }
      }
    }
  }

  private void addNode(Node node) {
    String name = node.getName();
    for (int j = 0, m = categories.length; j < m; j++) {
      dataset.addValue(0, categories[(j + categoryOrder) % categories.length], name);
    }
  }

  public void setVisible(boolean visible) {
    if (visible) {
      updateCharts();
    } else {
      dataset.clear();
    }
    super.setVisible(visible);
  }

  protected abstract void addSensorData(SensorData data);

}

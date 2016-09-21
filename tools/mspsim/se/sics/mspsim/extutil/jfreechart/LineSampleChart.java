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

public class LineSampleChart extends JFreeWindowDataHandler {

  private JPanel panel;
  private XYSeriesCollection dataset;
  private XYSeries dataSeries;
  private JFreeChart chart;
  private DefaultXYItemRenderer renderer;

  public LineSampleChart() {
    NumberAxis domain = new NumberAxis("Index");
    NumberAxis range = new NumberAxis("Value");
    XYPlot xyplot = new XYPlot();
    xyplot.setDomainAxis(domain);
    xyplot.setRangeAxis(range);
    // xyplot.setBackgroundPaint(Color.black);
    xyplot.setDataset(dataset = new XYSeriesCollection());

    renderer = new DefaultXYItemRenderer();
    renderer.setSeriesPaint(0, Color.black);
    renderer.setSeriesShapesVisible(0, false);
    xyplot.setRenderer(renderer);

    domain.setAutoRange(true);
    domain.setLowerMargin(0.0);
    domain.setUpperMargin(0.0);

    domain.setTickLabelsVisible(true);
    range.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
    chart = new JFreeChart("Test",
        JFreeChart.DEFAULT_TITLE_FONT, xyplot, true);
    ChartPanel chartPanel = new ChartPanel(chart);
    panel = new JPanel();
    panel.setLayout(new BorderLayout());
    panel.setPreferredSize(new Dimension(400, 200));
    panel.add(chartPanel, BorderLayout.CENTER);
    
    dataSeries = new XYSeries("-");
    dataSeries.setMaximumItemCount(200);
    dataset.addSeries(dataSeries);
  }
  
  public JComponent getComponent() {
    return panel;
  }

  public void lineRead(String line) {
    String parts[] = line.trim().split(" ");
    dataSeries.clear();
    for (int i = 0; i < parts.length; i++) {
      dataSeries.add(i, atod(parts[i], 0));
    }
    panel.repaint();
  }

  public void setProperty(String param, String[] args) {
    if ("title".equals(param)) {
      chart.setTitle(args[0]);
    }
  }

  public int getDataSeriesCount() {
    return 1;
  }

  public Series getDataSeries(int index) {
    return dataSeries;
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

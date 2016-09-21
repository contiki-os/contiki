package se.sics.mspsim.extutil.jfreechart;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Date;

import javax.swing.Timer;

import org.jfree.data.time.Millisecond;
import org.jfree.data.time.TimeSeries;

import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.util.DataSource;

public class DataSourceSampler implements ActionListener {

  private MSP430Core cpu;
  private int interval = 100;
  private Timer timer;
  private ArrayList<TimeSource> sources = new ArrayList<TimeSource>();

  public DataSourceSampler(MSP430Core cpu) {
    this.cpu = cpu;
    timer = new Timer(interval, this);
    timer.start();
  }

  public void stop() {
      timer.stop();
  }
  
  public void start() {
      timer.start();
  }
  
  public TimeSource addDataSource(DataSource source, TimeSeries ts) {
    TimeSource times = new TimeSource(cpu, source, ts);
    sources.add(times);
    return times;
  }
  
  public void removeDataSource(TimeSource source) {
    sources.remove(source);
  }
  
  public void setInterval(int intMsek) {
    interval = intMsek;
    timer.setDelay(interval);
  }

  private void sampleAll() {
    if (sources.size() > 0) {
      TimeSource[] srcs = (TimeSource[]) sources.toArray(new TimeSource[0]);
      for (int i = 0; i < srcs.length; i++) {
        if (srcs[i] != null)
          srcs[i].update();
      }
    }
    
//    test.add(new Millisecond(), Math.random() * 100);
//    test2.add(new Millisecond(), Math.random() * 100);
  }

  public void actionPerformed(ActionEvent arg0) {
    sampleAll();
  }
    
  private static class TimeSource {

    private MSP430Core cpu;
    private DataSource dataSource;
    private TimeSeries timeSeries;
    private long lastUpdate;

    TimeSource(MSP430Core cpu, DataSource ds, TimeSeries ts) {
      this.cpu = cpu;
      dataSource = ds;
      timeSeries = ts;
    }
    
    public void update() {
      long time = cpu.cycles / 2;
      if (time > lastUpdate) {
        lastUpdate = time;
        timeSeries.add(new Millisecond(new Date(time)), dataSource.getValue());
      } else {
//        System.out.println("IGNORING TIME " + time);
      }
    }
    
  }
  
}

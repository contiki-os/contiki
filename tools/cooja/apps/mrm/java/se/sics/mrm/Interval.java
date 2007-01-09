package se.sics.mrm;

import java.util.Vector;
import org.apache.log4j.Logger;

/**
 * This class represents a interval. Some operations on these intervals exist,
 * such as intersecting a interval with another and subtracting an interval from
 * another.
 * 
 * @author Fredrik Osterlind
 */
class Interval {
  private static Logger logger = Logger.getLogger(Interval.class);
  
  private double lowValue;
  private double highValue;
  
  /**
   * Creates a new double interval.
   * The given low value must be lower than the given high value.
   * 
   * @param lowValue Low interval border (< End interval border)
   * @param highValue High interval border
   */
  public Interval(double lowValue, double highValue) {
    this.lowValue = Math.min(lowValue, highValue);
    this.highValue = highValue;
  }
  
  /**
   * Set new values of interval
   * 
   * @param newLow New low value
   * @param newHigh New high value
   */
  public void setInterval(double newLow, double newHigh) {
    lowValue = newLow;
    highValue = newHigh;
  }
  
  /**
   * @return Low border value
   */
  public double getLow() {
    return lowValue;
  }
  
  /**
   * @return High border value
   */
  public double getHigh() {
    return highValue;
  }
  
  /**
   * @return Size of interval
   */
  public double getSize() {
    return highValue - lowValue;
  }
  
  /**
   * Returns the intersection between this interval and the given
   * interval or null if no intersection exists.
   * 
   * @param interval Other interval
   * @return Intersection interval
   */
  public Interval intersectWith(Interval interval) {
    // Given interval higher than this interval
    if (highValue <= interval.getLow())
      return null;
    
    // Given interval lower than this interval
    if (lowValue >= interval.getHigh())
      return null;
    
    // Given interval covers this interval
    if (lowValue >= interval.getLow() &&
        highValue <= interval.getHigh())
      return this;
    
    // Given interval inside this interval
    if (lowValue <= interval.getLow() &&
        highValue >= interval.getHigh())
      return interval;
    
    // Given interval overlaps lower part of this interval
    if (lowValue >= interval.getLow() &&
        highValue >= interval.getHigh())
      return new Interval(lowValue, interval.getHigh());
    
    // Given interval overlaps upper part of this interval
    if (lowValue <= interval.getLow() &&
        highValue <= interval.getHigh())
      return new Interval(interval.getLow(), highValue);
    
    logger.fatal("DoubleInterval.intersectWithInterval(), error!");
    return null;
  }
  
  /**
   * Checks if the given interval is a subset of this interval.
   * 
   * @param interval Other interval
   * @return True if this interval contains the given interval
   */
  public boolean contains(Interval interval) {
    return getLow() <= interval.getLow() && getHigh() >= interval.getHigh();
  }
  
  /**
   * Returns new intervals consisting of this interval with the given interval removed.
   * These can either be null (if entire interval was removed), 
   * one interval (if upper or lower part, or nothing was removed) or two intervals
   * (if middle part of interval was removed).
   * 
   * @param interval Other interval
   * @return New intervals
   */
  public Vector<Interval> subtract(Interval interval) {
    Vector<Interval> returnIntervals = new Vector<Interval>();
    
    // Given interval higher than this interval
    if (highValue <= interval.getLow()) {
      returnIntervals.add(this);
      return returnIntervals;
    }
    
    // Given interval lower than this interval
    if (lowValue >= interval.getHigh()) {
      returnIntervals.add(this);
      return returnIntervals;
    }
    
    // Given interval covers this interval
    if (lowValue >= interval.getLow() &&
        highValue <= interval.getHigh()) {
      return null;
    }
    
    // Given interval inside this interval
    if (lowValue <= interval.getLow() &&
        highValue >= interval.getHigh()) {
      returnIntervals.add(new Interval(lowValue, interval.getLow()));
      returnIntervals.add(new Interval(interval.getHigh(), highValue));
      return returnIntervals;
    }
    
    // Given interval overlaps lower part of this interval
    if (lowValue >= interval.getLow() &&
        highValue >= interval.getHigh()) {
      returnIntervals.add(new Interval(interval.getHigh(), highValue));
      return returnIntervals;
    }
    
    // Given interval overlaps upper part of this interval
    if (lowValue <= interval.getLow() &&
        highValue <= interval.getHigh()) {
      returnIntervals.add(new Interval(lowValue, interval.getLow()));
      return returnIntervals;
    }
    
    logger.fatal("DoubleInterval.subtractInterval(), error!");
    return null;
  }
  
  /**
   * Subtracts given interval from all intervals in given vector.
   * This method never returns null (but empty vectors).
   * 
   * @param initialIntervals Initial intervals
   * @param interval Interval to subtract
   * @return New intervals
   */
  static public Vector<Interval> subtract(Vector<Interval> initialIntervals, Interval interval) {
    Vector<Interval> newIntervals = new Vector<Interval>();
    for (int i=0; i < initialIntervals.size(); i++) {
      Vector<Interval> tempIntervals = initialIntervals.get(i).subtract(interval);
      if (tempIntervals != null)
        newIntervals.addAll(tempIntervals);
    }
    return newIntervals;
  }
  
  /**
   * @return True if interval does not have a length.
   */
  public boolean isEmpty() {
    if (highValue <= lowValue) 
      return true;
    return false;
  }
  
  public String toString() {
    if (isEmpty())
      return "Double interval: (null)";
    else
      return "Double interval: " + 
      getLow() + 
      " -> " + 
      getHigh();
  }
}
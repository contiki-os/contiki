package se.sics.arm;

import java.awt.geom.Line2D;
import java.awt.geom.Point2D;

/**
 * @author Fredrik Osterlind
 */
class RayData {
  
  enum RayType { ORIGIN, REFRACTION, REFLECTION, DIFFRACTION, DESTINATION }
  RayType type;
  
  private Point2D sourcePoint = null;
  private Line2D line = null;

  private int limitOverall = 0;
  private int limitRefracted = 0;
  private int limitReflected = 0;
  private int limitDiffracted = 0;

  /**
   * Creates a new ray data instance.
   * A ray can be of the following types:
   * 
   *  ORIGIN - starting at source point, line should be null
   *  REFRACTED - ray was refracted at an obstacle, 
   *   source is intersection point with obstacle, line should be null
   *  REFLECTED - an interval of rays is reflected at given line,
   *   the source point should be a "psuedo-source" is located behind it
   *   (as if one was looking from given source point through given line)
   *  DIFFRACTED - a ray is diffracted at given source point,
   *   line should be null
   *  DESTINATION
   * @param type Type of ray (one of the above)
   * @param sourcePoint See above
   * @param line See above (may be null)
   * @param limitOverall Maximum numbers of sub rays this ray may produce
   * @param limitRefracted Maximum numbers of refracted sub rays this ray may produce
   * @param limitReflected Maximum numbers of reflected sub rays this ray may produce
   * @param limitDiffracted Maximum numbers of diffracted sub rays this ray may produce
   */
  public RayData(
      RayType type,
      Point2D sourcePoint,
      Line2D line,
      int limitOverall,
      int limitRefracted,
      int limitReflected,
      int limitDiffracted
  ) {
    this.type = type;
    this.sourcePoint = sourcePoint;
    this.line = line;
    this.limitOverall = limitOverall;
    this.limitRefracted = limitRefracted;
    this.limitReflected = limitReflected;
    this.limitDiffracted = limitDiffracted;
  }
  
  public RayType getType() {
    return type;
  }
  
  public Point2D getSourcePoint() {
    return sourcePoint;
  }

  public Line2D getLine() {
    return line;
  }

  public int getSubRaysLimit() {
    return Math.min(limitOverall, limitRefracted + limitReflected + limitDiffracted);
  }
  
  public int getRefractedSubRaysLimit() {
    return Math.min(limitOverall, limitRefracted);
  }
  
  public int getReflectedSubRaysLimit() {
    return Math.min(limitOverall, limitReflected);
  }
  
  public int getDiffractedSubRaysLimit() {
    return Math.min(limitOverall, limitDiffracted);
  }
  
}

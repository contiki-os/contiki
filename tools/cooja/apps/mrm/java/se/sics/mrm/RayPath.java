package se.sics.mrm;

import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.util.Vector;

/**
 * @author Fredrik Osterlind
 */
public class RayPath {
  private Vector<Point2D> points = new Vector<Point2D>();
  private Vector<RayData.RayType> types = new Vector<RayData.RayType>();
  
  public void addPoint(Point2D point, RayData.RayType type) {
    points.insertElementAt(point, 0);
    types.insertElementAt(type, 0);
  }

  public int getSubPathCount() {
    return points.size() - 1;
  }

  public Line2D getSubPath(int pos) {
    return new Line2D.Double(points.get(pos), points.get(pos + 1));
  }

  public Point2D getPoint(int i) {
    return points.get(i);
  }
  
  public RayData.RayType getType(int i) {
    return types.get(i);
  }
  
  public String toString() {
    if (points.size() != types.size())
      return "Malformed ray path (differing sizes)";

    if (points.size() == 0)
      return "Empty ray path";
    
    if (types.firstElement() != RayData.RayType.ORIGIN && types.lastElement() != RayData.RayType.ORIGIN)
      return "Malformed ray path (not closed)";
      
    if (types.firstElement() != RayData.RayType.DESTINATION && types.lastElement() != RayData.RayType.DESTINATION)
      return "Malformed ray path (not closed)";

    if (types.firstElement() == types.lastElement())
      return "Malformed ray path (last == first element)";

    String retVal = "";
    for (int i=0; i < types.size(); i++) {
      RayData.RayType currentType = types.get(i);
      if (currentType == RayData.RayType.DESTINATION)
        retVal = retVal + " DEST ";
      else if (currentType == RayData.RayType.DIFFRACTION)
        retVal = retVal + " DIFF ";
      else if (currentType == RayData.RayType.ORIGIN)
        retVal = retVal + " ORIG ";
      else if (currentType == RayData.RayType.REFLECTION)
        retVal = retVal + " REFL ";
      else if (currentType == RayData.RayType.REFRACTION)
        retVal = retVal + " REFR ";
      else
        retVal = retVal + " ???? ";
    }
    return retVal;
    
  }
  
}

/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 */

package org.contikios.mrm;

import java.awt.Point;
import java.awt.geom.*;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Vector;
import org.apache.log4j.Logger;
import org.jdom.Element;

/**
 * This class represents an area with obstacles.
 * Obstacles may only be of rectangular shape.
 * 
 * @author Fredrik Osterlind
 */
class ObstacleWorld {
  private static Logger logger = Logger.getLogger(ObstacleWorld.class);
  
  // All registered obstacles
  private Vector<Rectangle2D> allObstacles = null;
  
  // All registered obstacles, with spatial information
  private int spatialResolution = 10;
  private Vector<Rectangle2D>[][] allObstaclesSpatial = new Vector[spatialResolution][spatialResolution];
  private boolean obstaclesOrganized = false;
  
  // Outer bounds of all obstacles
  private Rectangle2D outerBounds = null;
  
  
  /**
   * Creates a new obstacle world without any obstacles.
   */
  public ObstacleWorld() {
    // No obstacles present so far
    allObstacles = new Vector<Rectangle2D>();
    
    for (int x=0; x < spatialResolution; x++)
      for (int y=0; y < spatialResolution; y++) 
        allObstaclesSpatial[x][y] = new Vector<Rectangle2D>();
    
    outerBounds = new Rectangle2D.Double(0,0,0,0);
  }
  
  /**
   * @return The total number of registered obstacles
   */
  public int getNrObstacles() {
    return allObstacles.size();
  }
  
  /**
   * This method can be used to find extreme coordinates of all obstacles.
   * 
   * @return Outer bounds of all registered obstacles
   */
  public Rectangle2D getOuterBounds() {
    return outerBounds;
  }
  
  /**
   * Returns obstacle registered at given position.
   * The coordinates of an obstacles should never
   * be changed directly on an object returned by this method.
   * 
   * @param i Obstacle position
   * @return Obstacle at given position
   */
  public Rectangle2D getObstacle(int i) {
    return allObstacles.get(i);
  }
  
  /**
   * @return All registered obstacles
   */
  public Vector<Rectangle2D> getAllObstacles() {
    return allObstacles;
  }
  
  /**
   * Returns at least all registered obstacles that contains given point.
   * Note that obstacles close to but not containing the point may also
   * be returned.
   * 
   * @param center Center point
   * @return All obstacles containing or near center
   */
  public Vector<Rectangle2D> getAllObstaclesNear(Point2D center) {
    double boxWidth = outerBounds.getWidth() / (double) spatialResolution;
    double boxHeight = outerBounds.getHeight() / (double) spatialResolution;
    double areaStartX = outerBounds.getMinX();
    double areaStartY = outerBounds.getMinY();
    
    double centerX = (center.getX() - areaStartX)/boxWidth;
    double centerY = (center.getY() - areaStartY)/boxHeight;

    Vector<Rectangle2D> allNearObstacles = new Vector<Rectangle2D>();

    Point pointToAdd = new Point((int) centerX, (int) centerY);
    if (pointToAdd.x >= 0 &&
        pointToAdd.x < allObstaclesSpatial.length &&
        pointToAdd.y >= 0 &&
        pointToAdd.y < allObstaclesSpatial[0].length)
      allNearObstacles.addAll(allObstaclesSpatial[pointToAdd.x][pointToAdd.y]);

    // Add borders if needed
    boolean addedXBorder = false;
    boolean addedYBorder = false;
    if (Math.floor(centerX) == centerX) {
      pointToAdd = new Point((int) centerX-1, (int) centerY);
      if (pointToAdd.x >= 0 &&
          pointToAdd.x < allObstaclesSpatial.length &&
          pointToAdd.y >= 0 &&
          pointToAdd.y < allObstaclesSpatial[0].length) {
        allNearObstacles.addAll(allObstaclesSpatial[pointToAdd.x][pointToAdd.y]);
        addedXBorder = true;
      }
    }

    if (Math.floor(centerY) == centerY) {
      pointToAdd = new Point((int) centerX, (int) centerY-1);
      if (pointToAdd.x >= 0 &&
          pointToAdd.x < allObstaclesSpatial.length &&
          pointToAdd.y >= 0 &&
          pointToAdd.y < allObstaclesSpatial[0].length) {
        allNearObstacles.addAll(allObstaclesSpatial[pointToAdd.x][pointToAdd.y]);
        addedYBorder = true;
      }
    }
    
    if (addedXBorder && addedYBorder) {
      pointToAdd = new Point((int) centerX-1, (int) centerY-1);
      allNearObstacles.addAll(allObstaclesSpatial[pointToAdd.x][pointToAdd.y]);
    }
      
    return allNearObstacles;
  }

  /**
   * Returns at least all registered obstacles inside the given angle
   * interval when at the given center point. Note that obstacles partly or 
   * completely outside the interval may also be returned.
   * All obstacles are preferably returned in order of distance from given
   * center point, although this is not guaranteed.
   * 
   * @param center Center point
   * @param angleInterval Angle interval
   * @return All obstacles in given angle interval
   */
  public Vector<Rectangle2D> getAllObstaclesInAngleInterval(Point2D center, AngleInterval angleInterval) {
    Vector<Rectangle2D> obstaclesToReturn = new Vector<Rectangle2D>();
    if (!obstaclesOrganized) {
      reorganizeSpatialObstacles();
    }
    
    double boxWidth = outerBounds.getWidth() / (double) spatialResolution;
    double boxHeight = outerBounds.getHeight() / (double) spatialResolution;
    double areaStartX = outerBounds.getMinX();
    double areaStartY = outerBounds.getMinY();
    
    // Calculate which boxes to check (and in which order)
    Point centerInArray = new Point(
        (int) ((center.getX() - areaStartX)/boxWidth),
        (int) ((center.getY() - areaStartY)/boxHeight)
    );
    Vector<Point> pointsToCheck = new Vector<Point>();
    
    int currentDistance = 0;
    while (currentDistance < 2*spatialResolution) {
      
      if (currentDistance > 0) {
        int currentX = centerInArray.x - currentDistance;
        int currentY = centerInArray.y - currentDistance;
        
        // Step right
        while (currentX < centerInArray.x + currentDistance) {
          if (currentX >= 0 &&
              currentX < allObstaclesSpatial.length &&
              currentY >= 0 &&
              currentY < allObstaclesSpatial[0].length)
            pointsToCheck.add(new Point(currentX, currentY));
          currentX++;
        }
        
        // Step right
        while (currentY < centerInArray.y + currentDistance) {
          if (currentX >= 0 &&
              currentX < allObstaclesSpatial.length &&
              currentY >= 0 &&
              currentY < allObstaclesSpatial[0].length)
            pointsToCheck.add(new Point(currentX, currentY));
          currentY++;
        }
        
        // Step left
        while (currentX > centerInArray.x - currentDistance) {
          if (currentX >= 0 &&
              currentX < allObstaclesSpatial.length &&
              currentY >= 0 &&
              currentY < allObstaclesSpatial[0].length)
            pointsToCheck.add(new Point(currentX, currentY));
          currentX--;
        }
        
        // Step up
        while (currentY > centerInArray.y - currentDistance) {
          if (currentX >= 0 &&
              currentX < allObstaclesSpatial.length &&
              currentY >= 0 &&
              currentY < allObstaclesSpatial[0].length)
            pointsToCheck.add(new Point(currentX, currentY));
          currentY--;
        }
            
      } else {
        if (centerInArray.x >= 0 &&
            centerInArray.x < allObstaclesSpatial.length &&
            centerInArray.y >= 0 &&
            centerInArray.y < allObstaclesSpatial[0].length) {
          pointsToCheck.add(new Point(centerInArray.x, centerInArray.y));
        }
      }
      currentDistance++;  
    }
      
    for (int pointNr=0; pointNr < pointsToCheck.size(); pointNr++) {
      // Check which obstacles should be in this box
      boolean hit = false;
      int x = pointsToCheck.get(pointNr).x;
      int y = pointsToCheck.get(pointNr).y;
      
      // Test if we are inside test box
      if (!hit) {
        if (new Rectangle2D.Double(
            areaStartX + x*boxWidth, 
            areaStartY + y*boxHeight, 
            boxWidth, 
            boxHeight).contains(center)) {
          hit = true;
          for (int i=0; i < allObstaclesSpatial[x][y].size(); i++) {
            if (!obstaclesToReturn.contains(allObstaclesSpatial[x][y].get(i)))
              obstaclesToReturn.add(allObstaclesSpatial[x][y].get(i));
          }
        }
      }
      
      // Test first diagonal
      if (!hit) {
        AngleInterval testInterval = AngleInterval.getAngleIntervalOfLine(
            center,
            new Line2D.Double(
                areaStartX + x*boxWidth, 
                areaStartY + y*boxHeight, 
                areaStartX + (x+1)*boxWidth, 
                areaStartY + (y+1)*boxHeight)
        );
        if (testInterval.intersects(angleInterval)) {
          hit = true;
          for (int i=0; i < allObstaclesSpatial[x][y].size(); i++) {
            if (!obstaclesToReturn.contains(allObstaclesSpatial[x][y].get(i)))
              obstaclesToReturn.add(allObstaclesSpatial[x][y].get(i));
          }
        }
      }
        
      // Test second diagonal
      if (!hit) {
        AngleInterval testInterval = AngleInterval.getAngleIntervalOfLine(
            center,
            new Line2D.Double(
                areaStartX + x*boxWidth, 
                areaStartY + (y+1)*boxHeight,
                areaStartX + (x+1)*boxWidth, 
                areaStartY + y*boxHeight)
        );
        if (testInterval.intersects(angleInterval)) {
          hit = true;
          for (int i=0; i < allObstaclesSpatial[x][y].size(); i++) {
            if (!obstaclesToReturn.contains(allObstaclesSpatial[x][y].get(i)))
              obstaclesToReturn.add(allObstaclesSpatial[x][y].get(i));
          }
        }
      }
    }
        
    return obstaclesToReturn;
  }
  
  /**
   * Removes all registered obstacles.
   */
  public void removeAll() {
    allObstacles.removeAllElements();
    for (int x=0; x < spatialResolution; x++)
      for (int y=0; y < spatialResolution; y++) 
        allObstaclesSpatial[x][y].removeAllElements();
    
    outerBounds = new Rectangle2D.Double(0,0,0,0);
  }
  
  /**
   * Returns true of given point is on a corner of
   * any of the structures build from the obstacles.
   * Internally this method checks how many of four point 
   * close to and located around given point (diagonally) are 
   * inside any obstacle.
   * This method returns true if exactly one point is inside an obstacle.
   * 
   * @param point Point to check
   * @return True of point is on a corner, false otherwise
   */
  public boolean pointIsNearCorner(Point2D point) {
    double boxWidth = outerBounds.getWidth() / (double) spatialResolution;
    double boxHeight = outerBounds.getHeight() / (double) spatialResolution;
    double areaStartX = outerBounds.getMinX();
    double areaStartY = outerBounds.getMinY();

    // Which obstacles should be checked
    Point centerInArray = new Point(
        (int) ((point.getX() - areaStartX)/boxWidth),
        (int) ((point.getY() - areaStartY)/boxHeight)
    );
    Vector<Rectangle2D> allObstaclesToCheck = null;
    if (centerInArray.x < 0)
      centerInArray.x = 0;
    if (centerInArray.x >= spatialResolution)
      centerInArray.x = spatialResolution-1;
    if (centerInArray.y < 0)
      centerInArray.y = 0;
    if (centerInArray.y >= spatialResolution)
      centerInArray.y = spatialResolution-1;
    
    allObstaclesToCheck = allObstaclesSpatial[centerInArray.x][centerInArray.y];

    if (allObstaclesToCheck.size() == 0) {
      return false;
    } 
    
    // Create the four point to check
    double deltaDistance = 0.01; // 1 cm TODO Change this?
    Point2D point1 = new Point2D.Double(point.getX() - deltaDistance, point.getY() - deltaDistance);
    Point2D point2 = new Point2D.Double(point.getX() - deltaDistance, point.getY() + deltaDistance);
    Point2D point3 = new Point2D.Double(point.getX() + deltaDistance, point.getY() - deltaDistance);
    Point2D point4 = new Point2D.Double(point.getX() + deltaDistance, point.getY() + deltaDistance);

    int containedPoints = 0;
    Enumeration<Rectangle2D> allObstaclesToCheckEnum = allObstaclesToCheck.elements();
    while (allObstaclesToCheckEnum.hasMoreElements()) {
      Rectangle2D obstacleToCheck = allObstaclesToCheckEnum.nextElement();
      if (obstacleToCheck.contains(point1))
        containedPoints++;
      if (obstacleToCheck.contains(point2))
        containedPoints++;
      if (obstacleToCheck.contains(point3))
        containedPoints++;
      if (obstacleToCheck.contains(point4))
        containedPoints++;

      // Abort if already to many contained points
      if (containedPoints > 1) {
        return false;
      }
    }
    
    return (containedPoints == 1);
  }
  
  /**
   * Checks if specified obstacle can be merged with any existing obstacle
   * in order to reduce the total number of obstacles. And in that case a merge
   * is performed and this method returns the new obstacle object.
   * The checking is performed by looping through all existing obstacles and 
   * for each one comparing the union area of it and the given obstacle to the 
   * area sum of the two. And since obstacles are not allowed to overlap, if the 
   * union area is equal to the area sum, they can be merged.
   * If a merge is performed, another may be made possible so this method
   * should be looped until returning null.
   * 
   * This method does not notify observers of changes made!
   * 
   * @return New object of a merge was performed, null otherwise
   */
  private Rectangle2D mergeObstacle(Rectangle2D mergeObstacle) {
    double mergeObstacleArea = mergeObstacle.getWidth() * mergeObstacle.getHeight();
    double mergeObstacleTolerance = mergeObstacleArea * 0.01; // 1%
    
    // Loop through all existing obstacles (but ignore itself)
    for (int i=0; i < getNrObstacles(); i++) {
      Rectangle2D existingObstacle = getObstacle(i);
      if (!existingObstacle.equals(mergeObstacle)) {
        double existingObstacleArea = existingObstacle.getWidth() * existingObstacle.getHeight();
        Rectangle2D unionObstacle = existingObstacle.createUnion(mergeObstacle);
        double unionArea = unionObstacle.getWidth() * unionObstacle.getHeight();
        
        // Fault-tolerance
        double faultTolerance = Math.min(mergeObstacleTolerance, existingObstacleArea*0.01);
        
        // Compare areas
        if (unionArea - faultTolerance <= existingObstacleArea + mergeObstacleArea) {
          // Remove both old obstacles, add union
          removeObstacle(mergeObstacle);
          removeObstacle(existingObstacle);
          addObstacle(unionObstacle, false);
          
          obstaclesOrganized = false;
          return unionObstacle;
        }
      }
    }
    
    return null;
  }
  
  /**
   * Register new obstacle with given attributes.
   * This method will try to merge this obstacle with other already existing obstacles.
   * 
   * @param startX Start X coordinate
   * @param startY Start Y coordinate
   * @param width Width
   * @param height Height
   */
  public void addObstacle(double startX, double startY, double width, double height) {
    addObstacle(startX, startY, width, height, true);
  }
  
  /**
   * Register new obstacle with given attributes.
   * This method will, depending on given argument, try to merge 
   * this obstacle with other already existing obstacles.
   * 
   * @param startX Start X coordinate
   * @param startY Start Y coordinate
   * @param width Width
   * @param height Height
   * @param merge Should this obstacle, if possible, be merged with existing obstacles
   */
  public void addObstacle(double startX, double startY, double width, double height, boolean merge) {
    Rectangle2D newRect = new Rectangle2D.Double(startX, startY, width, height);
    addObstacle(newRect, merge);
  }
  
  /**
   * Registers a given obstacle.
   * This method will try to merge this obstacle with other already existing obstacles.
   * 
   * @param obstacle New obstacle
   */
  public void addObstacle(Rectangle2D obstacle) {
    addObstacle(obstacle, true);
  }
  
  /**
   * Registers a given obstacle.
   * This method will, depending on the given argument, try to 
   * merge this obstacle with other already existing obstacles.
   * 
   * @param obstacle New obstacle
   */
  public void addObstacle(Rectangle2D obstacle, boolean merge) {
    // TODO Should we keep the rounding?
    obstacle.setRect(
        Math.round(obstacle.getMinX()*1000.0) / 1000.0,
        Math.round(obstacle.getMinY()*1000.0) / 1000.0,
        Math.round(obstacle.getWidth()*1000.0) / 1000.0,
        Math.round(obstacle.getHeight()*1000.0) / 1000.0
    );
  
    allObstacles.add(obstacle);
    outerBounds = outerBounds.createUnion(obstacle);
    
    if (merge) {
      // Check if obstacle can be merged with another obstacle
      Rectangle2D mergedObstacle = mergeObstacle(obstacle);
      
      // Keep merging...
      while (mergedObstacle != null)
        mergedObstacle = mergeObstacle(mergedObstacle);
    }
    
    obstaclesOrganized = false;
  }
  
  /**
   * Remove the given obstacle, if it exists.
   * 
   * @param obstacle Obstacle to remove
   */
  public void removeObstacle(Rectangle2D obstacle) {
    allObstacles.remove(obstacle);
    
    recreateOuterBounds();
    obstaclesOrganized = false;
  }
  
  /**
   * This method recreates the outer bounds of 
   * this obstacle area by checking all registered
   * obstacles.
   * This method should never have to be called directly
   * by a user.
   */
  public void recreateOuterBounds() {
    outerBounds = new Rectangle2D.Double(0,0,0,0);
    for (int i=0; i < allObstacles.size(); i++) {
      outerBounds = outerBounds.createUnion(allObstacles.get(i));
    }
    obstaclesOrganized = false;
  }
  
  /**
   * Reorganizes all registered obstacles in order to speed up
   * searches for obstacles in spatial areas.
   * This method is run automatically 
   */
  public void reorganizeSpatialObstacles() {
    // Remove all spatial obstacles
    for (int x=0; x < spatialResolution; x++)
      for (int y=0; y < spatialResolution; y++) 
        allObstaclesSpatial[x][y].removeAllElements();
    
    double boxWidth = outerBounds.getWidth() / (double) spatialResolution;
    double boxHeight = outerBounds.getHeight() / (double) spatialResolution;
    double currentBoxMinX = outerBounds.getMinX();
    double currentBoxMinY = outerBounds.getMinY();
    
    // For each box, add obstacles that belong there
    for (int x=0; x < spatialResolution; x++)
      for (int y=0; y < spatialResolution; y++) {
        // Check which obstacles should be in this box
        Rectangle2D boxToCheck = new Rectangle2D.Double(currentBoxMinX + x*boxWidth, currentBoxMinY + y*boxHeight, boxWidth, boxHeight);
        for (int i=0; i < allObstacles.size(); i++) {
          if (allObstacles.get(i).intersects(boxToCheck)) {
            allObstaclesSpatial[x][y].add(allObstacles.get(i));
          }
        }
      }
    
    obstaclesOrganized = true;
    
    //printObstacleGridToConsole();
  }
  
  /**
   * Prints a description of all obstacles to the console
   */
  public void printObstacleGridToConsole() {
    logger.info("<<<<<<< printObstacleGridToConsole >>>>>>>");
    logger.info(". Number of obstacles:\t" + getNrObstacles());
    logger.info(". Outer boundary min:\t" + getOuterBounds().getMinX() + ", " + getOuterBounds().getMinY());
    logger.info(". Outer boundary max:\t" + getOuterBounds().getMaxX() + ", " + getOuterBounds().getMaxY());
    
    Vector<Rectangle2D> uniqueSpatialObstacles = new Vector<Rectangle2D>();
    for (int x=0; x < spatialResolution; x++)
      for (int y=0; y < spatialResolution; y++) 
        for (int i=0; i < allObstaclesSpatial[x][y].size(); i++) 
          if (!uniqueSpatialObstacles.contains(allObstaclesSpatial[x][y].get(i)))
            uniqueSpatialObstacles.add(allObstaclesSpatial[x][y].get(i));
    logger.info(". Unique spatial obstacles:\t" + uniqueSpatialObstacles.size());
    
    int allSpatialObstacles = 0;
    for (int x=0; x < spatialResolution; x++)
      for (int y=0; y < spatialResolution; y++) 
        for (int i=0; i < allObstaclesSpatial[x][y].size(); i++) 
          allSpatialObstacles++;
    logger.debug(". All spatial obstacles:\t" + allSpatialObstacles);
    
    logger.info(". Spatial map counts:");
    for (int y=0; y < spatialResolution; y++) {
      for (int x=0; x < spatialResolution; x++) {
        System.out.print(allObstaclesSpatial[x][y].size() + " ");
      }
      System.out.println("");
    }
    
  }
  
  /**
   * Returns XML elements representing the current obstacles.
   * 
   * @see #setConfigXML(Collection)
   * @return XML elements representing the obstacles
   */
  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    for (Rectangle2D rect: allObstacles) {
      element = new Element("obst");
      element.setText(rect.getMinX() + ";" + rect.getMinY() + ";" + rect.getWidth() + ";" + rect.getHeight());
      config.add(element);
    }
    return config;
  }

  /**
   * Sets the current obstacles depending on the given XML elements.
   * 
   * @see #getConfigXML()
   * @param configXML
   *          Config XML elements
   * @return True if config was set successfully, false otherwise
   */
  public boolean setConfigXML(Collection<Element> configXML) {
    for (Element element : configXML) {
      if (element.getName().equals("obst")) {
        String rectValues[] = element.getText().split(";");
        Rectangle2D newObst = new Rectangle2D.Double(
            Double.parseDouble(rectValues[0]),
            Double.parseDouble(rectValues[1]),
            Double.parseDouble(rectValues[2]),
            Double.parseDouble(rectValues[3]));
        this.addObstacle(newObst, false);
      }
    }
    return true;    
  }

}


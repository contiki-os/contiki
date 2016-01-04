package pt.inescid.cnm.nodegenerator;

import java.lang.Math;

import org.apache.log4j.Logger;

import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.TimeEvent;

import pt.inescid.cnm.nodegenerator.NodeGenerator;

public class MobileMote{
  private Mote mote;
  private Thread movementThread;
  private NodeGenerator context;
  private double step;
  private double pause_min;
  private double pause_max;
  private double velocity_min;
  private double velocity_max;
  
  private double[] current_destination;
  private double[] current_step;
  private boolean moving;
  
  private static Logger logger = Logger.getLogger(MobileMote.class);
  
  private NextMove nextMove;
  
  public MobileMote(Mote m, NodeGenerator ctx){
    this.mote = m;
    this.context = ctx;
    this.step = 0.1;
    
    this.pause_min = 10;
    this.pause_max = 30;
    
    this.velocity_min = 1;
    this.velocity_max = 10; 
    
    ctx.getSimulation().invokeSimulationThread(new Runnable() {
      public void run() {
        //long periodStart = ctx.getSimulation().getSimulationTime();
        /*logger.debug("periodStart: " + periodStart);*/
        NextMove nextMove = new NextMove(0,Integer.toString(m.getID()));
        nextMove.execute(MobileMote.this.context.getSimulation().getSimulationTime());
      }
    });
    
  }
  
  public double[] getMotePosition(){
    Position p = mote.getInterfaces().getPosition();
    double[] ret = new double[2];
    ret[0] = p.getXCoordinate();
    ret[1] = p.getYCoordinate();
    return ret;
  }
  
  public void setMotePosition(double x, double y){
    Position p = mote.getInterfaces().getPosition();
    p.setCoordinates(x,y,p.getZCoordinate());
  }
  
  private double[] getNextDestination(){
    double r = Math.random()*(context.getBoundary() + 50);
    double a = Math.random()*2*Math.PI;
    double[] destiny = new double[2];
    destiny[0] = r * Math.cos(a);
    destiny[1] = r * Math.sin(a);
    return destiny;
  }
  
  private double[] getStepSumToDestination(double[] dst){
    double v = Math.random()*(velocity_max-velocity_min) + velocity_min;
    double[] curr = getMotePosition();
    double direction = Math.atan2(dst[1]-curr[1],dst[0]-curr[0]);
    double length = Math.hypot(Math.abs(dst[0]-curr[0]),Math.abs(dst[1]-curr[1]));
    double[] step_sum = new double[2];
    step_sum[0] = step*v*Math.cos(direction);
    step_sum[1] = step*v*Math.sin(direction);
    return step_sum;
  }
  
  private boolean isPastDestination(double[] curr, double[] next){
    /*double cx = curr[0];
    double dx = current_destination[0];
    if(cx < 0 && dx < 0) return (cx < dx);
    else if(cx < 0 && dx < 0) return (cx > dx);
    else if(dx == 0) return true;
    else return false;*/
    // ((cx -> dx -> nx) ou (nx -> dx -> cx)) e (y)
    double cx = curr[0];
    double nx = next[0];
    double dx = current_destination[0];
    double cy = curr[1];
    double ny = next[1];
    double dy = current_destination[1];
    return  ((cx < dx && dx < nx) || (nx < dx && dx < cx)) &&
        ((cy < dy && dy < ny) || (ny < dy && dy < cy));   
    
    
  }
  
  private class NextMove extends TimeEvent{
    String id;
    
    public NextMove(long t){
      super(t);
    }
    
    public NextMove(long t, String id){
      super(t,id);
      this.id = id;
    }
    
    public void execute(long t) {
      if(moving){ 
        double[] curr = getMotePosition();
        double posX = curr[0] + current_step[0];
        double posY = curr[1] + current_step[1];
        double[] next = {posX,posY};
        if(!isPastDestination(curr,next)){
          setMotePosition(posX,posY);
          context.getSimulation().scheduleEvent(this,t+(long)(step*1000.0*Simulation.MILLISECOND));
        }else {
          moving = false;
          double pause = Math.random() * (pause_max - pause_min) + pause_min;
          context.getSimulation().scheduleEvent(this,t+(long)(pause*1000.0*Simulation.MILLISECOND));
        }
      }else{
        current_destination = getNextDestination();
        current_step = getStepSumToDestination(current_destination);
        moving = true;
        context.getSimulation().scheduleEvent(this,t+(long)(step*1000.0*Simulation.MILLISECOND));
      }
      
    }
  };
  
  
}

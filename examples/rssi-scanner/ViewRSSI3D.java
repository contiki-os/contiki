/**
 * 3D RSSI Viewer - view RSSI values of 802.15.4 channels
 * ---------------------------------------------------
 * Note: run the rssi-scanner on a Sky, Z1 or sentilla node connected to USB
 * then start with
 * make login | java ViewRSSI3D
 *
 * @author Joakim Eriksson, SICS
 *
 */
import java.io.*;
import java.awt.*;
import java.awt.geom.*;
import javax.swing.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;

public class ViewRSSI3D extends JPanel {
  public InputStream inputstr;

  public static final int WIDTH = 6;
  public static final int HEIGHT = 4;

  public static final int MAX_HISTORY = 50;
  public static final int MAX_VALUES = 86;

  private int[][] values = new int[MAX_HISTORY][MAX_VALUES];
  private int pos = 0;

  public ViewRSSI3D() {
    super(null);
    setBackground(Color.black);
    setOpaque(true);
    setPreferredSize(new Dimension(800, 400));
  }

  long last = 0;
  Polygon pol = new Polygon();
  float[] dist = {0.0f, 0.3f, 0.6f, 1.0f};
  Color[] colors = {Color.RED, Color.YELLOW, Color.GREEN, Color.BLUE};
  int[] lastPos = new int[MAX_VALUES];

  public void paint(Graphics g) {
    //long t = System.currentTimeMillis();
    Graphics2D gfx = (Graphics2D) g;
    gfx.setColor(Color.black);
    gfx.fillRect(0, 0, getWidth(), getHeight());
    int oldPos = 0;
    int oldJPos = 0;
    int lastColor = -1;

    int w = (getWidth() - MAX_HISTORY * 4) / MAX_VALUES;
    int h = w * HEIGHT / WIDTH;
    w = (getWidth() - MAX_HISTORY * h) / MAX_VALUES;
    int xs = h * MAX_HISTORY;
    int ys = (getHeight() - h * MAX_HISTORY) / 2;

    for (int i = 0, n = MAX_HISTORY; i < n; i++) {
      Point2D start = new Point2D.Float(xs - i * h, ys - 100 + i * h);
      Point2D end = new Point2D.Float(xs - i * h, ys + i * h);
      LinearGradientPaint p =
        new LinearGradientPaint(start, end, dist, colors);

      gfx.setPaint(p);
      for (int j = 0, m = MAX_VALUES; j < m; j++) {
        int val = (int) values[(i + pos) % MAX_HISTORY][j];
        if (i > 0 && j > 0) {
          pol.reset();
          pol.addPoint(xs + (j - 1) * w - i * h, ys + i * h - oldPos);
          pol.addPoint(xs + j * w - i * h, ys + i * h - val);
          pol.addPoint(xs + j * w - (i - 1) * h,
                       ys + (i - 1) * h - lastPos[j]);
          pol.addPoint(xs + (j - 1) * w - (i - 1) * h,
                       ys + (i - 1) * h - oldJPos);

          gfx.fillPolygon(pol);
        }
        oldPos = val;
        oldJPos = lastPos[j];
      }
      gfx.setColor(Color.black);
      for (int j = 0, m = MAX_VALUES; j < m; j++) {
        int val = (int) values[(i + pos) % MAX_HISTORY][j];
        if (i > 0 && j > 0) {
          pol.reset();
          pol.addPoint(xs + (j - 1) * w - i * h, ys + i * h - oldPos);
          pol.addPoint(xs + j * w - i * h, ys + i * h - val);
          pol.addPoint(xs + j * w - (i - 1) * h,
                       ys + (i - 1) * h - lastPos[j]);
          pol.addPoint(xs + (j - 1) * w - (i - 1) * h,
                       ys + (i - 1) * h - oldJPos);

          gfx.drawPolygon(pol);
        }
        oldPos = val;
        oldJPos = lastPos[j];
        lastPos[j] = val;
      }
    }

    gfx.setColor(Color.gray);
    gfx.drawLine(xs - (MAX_HISTORY - 1) * h, ys + (MAX_HISTORY - 1) * h,
                 xs - (MAX_HISTORY - 1) * h + (MAX_VALUES - 1) * w,
                 ys + (MAX_HISTORY - 1) * h);
    gfx.drawLine(xs - (MAX_HISTORY - 1) * h + (MAX_VALUES - 1) * w,
                 ys + (MAX_HISTORY - 1) * h,
                 xs + (MAX_VALUES - 1) * w, ys);
    // System.out.println("Time: " + (System.currentTimeMillis() - t) + " last: " + (System.currentTimeMillis() - last));
    // last = System.currentTimeMillis();
  }

  public void handleInput() throws IOException {
    BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));    while(true) {
      String line = reader.readLine();
      if (line != null && line.startsWith("RSSI:")) {
        try {
          String[] parts = line.substring(5).split(" ");
          for (int i = 0, n = parts.length; i < n; i++) {
            values[pos][i] = Integer.parseInt(parts[i]);
          }
        } catch (Exception e) {
          e.printStackTrace(); /* Report error, but do not fail... */
        }
        pos = (pos + 1) % MAX_HISTORY;
        repaint();
      }
    }
  }


  public static void main(String[] args) throws IOException {
    ViewRSSI3D mc = new ViewRSSI3D();
    JFrame window = new JFrame("3D RSSI Viewer");
    window.add(mc);
    window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    window.pack();
    window.setVisible(true);
    mc.handleInput();
  }
}

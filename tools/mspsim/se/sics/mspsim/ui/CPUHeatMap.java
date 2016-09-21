package se.sics.mspsim.ui;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.image.BufferedImage;

import javax.swing.JComponent;
import javax.swing.Timer;

import se.sics.mspsim.core.Memory;
import se.sics.mspsim.core.MemoryMonitor;
import se.sics.mspsim.core.MSP430Core;

public class CPUHeatMap extends JComponent implements MemoryMonitor {

    private static final long serialVersionUID = -7964848220064713887L;

    private Timer ticker;
    
    private ManagedWindow window;
    private BufferedImage heatmap;
    private int[] heatR;
    private int[] heatW;
    private int[] heatE;
    private int heatMax = 0;
    private int mode = 1;
    
    private MSP430Core cpu;
    
    public CPUHeatMap(MSP430Core cpu, WindowManager windowManager) {
        this.cpu = cpu;
        heatR = new int[cpu.MAX_MEM];
        heatW = new int[cpu.MAX_MEM];
        heatE = new int[cpu.MAX_MEM];
        
        window = windowManager.createWindow("CPU Heat Map");
        heatmap = new BufferedImage(128, 512, BufferedImage.TYPE_INT_RGB);
        setPreferredSize(new Dimension(140, 530));
        setOpaque(true);
        window.add(this);
        
        ticker = new Timer(50, new ActionListener() {
            public void actionPerformed(ActionEvent arg0) {
                repaint();
            }
        });
        ticker.start();

        setFocusable(true);
        addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent ke) {
                if (ke.getKeyChar() == 'm') {
                    mode = mode ^ 1;
                }
            }
        });

        window.setVisible(true);
    }

    public void close() {
        if (ticker != null) {
            ticker.stop();
        }
        if (window != null) {
            window.setVisible(false);
            window = null;
        }
    }

    public void updateImage() {
        double factor = 250.0 / heatMax;
        switch (mode) {
        case 0:
            int me, mr, mw;
            for (int i = 0; i < cpu.MAX_MEM; i++) {
                mw = me = mr = 0;
                if (heatW[i] > 0) mw = 80;
                if (heatR[i] > 0) mr = 80;
                if (heatE[i] > 0) me = 80;
                
                heatmap.setRGB(i & 127, i / 128,
                        ((int)(factor * heatW[i] + mw) << 16) |
                        ((int)(factor * heatR[i] + mr) << 8) | 
                        (int)(factor * heatE[i]) + me);
            }
            break;
        case 1:
            /* slow... */
            for (int i = 0; i < cpu.MAX_MEM; i++) {
                int r = heatW[i];
                int g = heatR[i];
                int b = heatE[i];

                heatW[i] = (99 * r) / 100;
                heatR[i] = (99 * g) / 100;
                heatE[i] = (99 * b) / 100;

                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;
                
                heatmap.setRGB(i & 127, i / 128, (r << 16) | (g << 8) | b);
            }
            break;
        }
    }
    
    public void paint(Graphics g) {
        updateImage();
        g.drawImage(heatmap, 0, 0, getWidth(), getHeight(), this);
    }

    private void cpuAction(int adr, Memory.AccessType type) {
        int val = 0;
        int f = 1;
        if (mode == 1) f = 40;
        switch (type) {
        case EXECUTE:
            val = heatE[adr] = heatE[adr] + f;
            break;
        case READ:
            val = heatR[adr] = heatR[adr] + f;
            break;
        case WRITE:
            val = heatW[adr] = heatW[adr] + f;
            break;
        }
        if (val > heatMax) {
            heatMax = val;
        }
    }

    @Override
    public void notifyReadBefore(int addr, Memory.AccessMode mode, Memory.AccessType type) {
        cpuAction(addr, type);
    }

    @Override
    public void notifyReadAfter(int addr, Memory.AccessMode mode, Memory.AccessType type) {
    }

    @Override
    public void notifyWriteBefore(int dstAddress, int data, Memory.AccessMode mode) {
        cpuAction(dstAddress, Memory.AccessType.WRITE);
    }

    @Override
    public void notifyWriteAfter(int dstAddress, int data, Memory.AccessMode mode) {
    }

}

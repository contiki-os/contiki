package com.thingsquare.cooja.mspsim;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.interfaces.LED;
import se.sics.cooja.mspmote.Exp5438Mote;
import se.sics.mspsim.core.IOPort;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.PortListener;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Trxeb LEDs")
public class TrxebLEDs extends LED {
	private static Logger logger = Logger.getLogger(TrxebLEDs.class);

	private Exp5438Mote mspMote;

	private boolean redOn = false;
	private boolean yellowOn = false;
	private boolean greenOn = false;
	private boolean blueOn = false;

	private static final Color RED = new Color(255, 0, 0);
	private static final Color DARK_RED = new Color(100, 0, 0);
	private static final Color YELLOW = new Color(255, 255, 0);
	private static final Color DARK_YELLOW = new Color(184,134,11);
	private static final Color GREEN = new Color(0, 255, 0);
	private static final Color DARK_GREEN = new Color(0, 100, 0);
	private static final Color BLUE = new Color(0, 0, 255);
	private static final Color DARK_BLUE = new Color(0, 0, 100);

	public TrxebLEDs(Mote mote) {
		mspMote = (Exp5438Mote) mote;

		IOUnit unit = mspMote.getCPU().getIOUnit("P4");
		if (unit instanceof IOPort) {
			((IOPort) unit).addPortListener(new PortListener() {
				public void portWrite(IOPort source, int data) {
					redOn = (data & (1<<0)) == 0;
					yellowOn = (data & (1<<1)) == 0;
					greenOn = (data & (1<<2)) == 0;
					blueOn = (data & (1<<3)) == 0;
					setChanged();
					notifyObservers();
				}
			});
		}
	}

	public boolean isAnyOn() {
		return redOn || yellowOn || greenOn || blueOn;
	}

	public boolean isGreenOn() {
		return greenOn;
	}

	public boolean isRedOn() {
		return redOn;
	}

	public boolean isYellowOn()  {
		return yellowOn;
	}

	public boolean isBlueOn()  {
		return blueOn;
	}

	public JPanel getInterfaceVisualizer() {
		final JPanel panel = new JPanel() {
			private static final long serialVersionUID = 1L;
			public void paintComponent(Graphics g) {
				super.paintComponent(g);

				int x = 20;
				int y = 25;
				int d = 25;

				if (isRedOn()) {
					g.setColor(RED);
					g.fillOval(x, y, d, d);
					g.setColor(Color.BLACK);
					g.drawOval(x, y, d, d);
				} else {
					g.setColor(DARK_RED);
					g.fillOval(x + 5, y + 5, d-10, d-10);
				}

				x += 40;

				if (isYellowOn()) {
					g.setColor(YELLOW);
					g.fillOval(x, y, d, d);
					g.setColor(Color.BLACK);
					g.drawOval(x, y, d, d);
				} else {
					g.setColor(DARK_YELLOW);
					g.fillOval(x + 5, y + 5, d-10, d-10);
				}

				x += 40;

				if (isGreenOn()) {
					g.setColor(GREEN);
					g.fillOval(x, y, d, d);
					g.setColor(Color.BLACK);
					g.drawOval(x, y, d, d);
				} else {
					g.setColor(DARK_GREEN);
					g.fillOval(x + 5, y + 5, d-10, d-10);
				}

				x += 40;

				if (isBlueOn()) {
					g.setColor(BLUE);
					g.fillOval(x, y, d, d);
					g.setColor(Color.BLACK);
					g.drawOval(x, y, d, d);
				} else {
					g.setColor(DARK_BLUE);
					g.fillOval(x + 5, y + 5, d-10, d-10);
				}
			}
		};

		Observer observer;
		this.addObserver(observer = new Observer() {
			public void update(Observable obs, Object obj) {
				panel.repaint();
			}
		});

		// Saving observer reference for releaseInterfaceVisualizer
		panel.putClientProperty("intf_obs", observer);
		panel.setMinimumSize(new Dimension(140, 60));
		panel.setPreferredSize(new Dimension(140, 60));
		return panel;
	}

	public void releaseInterfaceVisualizer(JPanel panel) {
		Observer observer = (Observer) panel.getClientProperty("intf_obs");
		if (observer == null) {
			logger.fatal("Error when releasing panel, observer is null");
			return;
		}

		this.deleteObserver(observer);
	}


	public Collection<Element> getConfigXML() {
		return null;
	}

	public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
	}

}

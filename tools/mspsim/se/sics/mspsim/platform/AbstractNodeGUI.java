/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * This file is part of MSPSim.
 *
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * AbstractNodeGUI
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 9 okt 2009
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.platform;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.io.File;
import java.net.URL;

import javax.swing.ImageIcon;
import javax.swing.JComponent;

import se.sics.mspsim.ui.ManagedWindow;
import se.sics.mspsim.ui.WindowManager;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.ServiceComponent;

public abstract class AbstractNodeGUI extends JComponent implements ServiceComponent {

    private static final long serialVersionUID = 1435276301923987019L;

    private final String windowName;
    private final String nodeImageName;

    private String name;
    private ComponentRegistry registry;

    private ImageIcon nodeImage;
    private ManagedWindow window;

    private ServiceComponent.Status status = Status.STOPPED;


    protected AbstractNodeGUI(String windowName, String imageName) {
        this.windowName = windowName;
        this.nodeImageName = imageName;
        setBackground(Color.black);
        setOpaque(true);
    }

    public Status getStatus() {
        return status;
    }

    public String getName() {
        return name;
    }

    protected ImageIcon getNodeImage() {
        return nodeImage;
    }

    protected ComponentRegistry getRegistry() {
        return registry;
    }

    public final void init(String name, ComponentRegistry registry) {
        this.name = name;
        this.registry = registry;
    }

    public final void start() {
        File fp = new File(nodeImageName);
        if (!fp.canRead()) {
            URL imageURL = getImageURL(nodeImageName);
            if (imageURL == null
                    && !nodeImageName.startsWith("images/")
                    && !nodeImageName.startsWith("/images/")) {
                imageURL = getImageURL("images/" + nodeImageName);
            }
            if (imageURL != null) {
                nodeImage = new ImageIcon(imageURL);
            } else {
                throw new IllegalStateException("image not found: " + nodeImageName);
            }
        } else {
            nodeImage = new ImageIcon(nodeImageName);
        }
        if (nodeImage.getIconWidth() == 0 || nodeImage.getIconHeight() == 0) {
          // Image not found
          throw new IllegalStateException("image not found: " + nodeImageName);
        }
        setPreferredSize(new Dimension(nodeImage.getIconWidth(),
                                       nodeImage.getIconHeight()));

        WindowManager wm = registry.getComponent(WindowManager.class);
        window = wm.createWindow(windowName);
        window.add(this);
        window.pack();

        startGUI();

        status = Status.STARTED;
        window.setVisible(true);
    }

    private URL getImageURL(String image) {
        URL imageURL = getClass().getResource(image);
        if (imageURL == null && !image.startsWith("/")) {
            imageURL = getClass().getResource("/" + image);
        }
        return imageURL;
    }

    public final void stop() {
        status = Status.STOPPED;
        stopGUI();
        if (window != null) {
            window.setVisible(false);
            window = null;
        }
    }

    protected abstract void startGUI();

    protected abstract void stopGUI();

    protected void paintComponent(Graphics g) {
        Color old = g.getColor();
        g.setColor(getBackground());
        g.fillRect(0, 0, getWidth(), getHeight());

        ImageIcon nodeImage = getNodeImage();
        nodeImage.paintIcon(this, g, 0, 0);

        g.setColor(old);
    }
}

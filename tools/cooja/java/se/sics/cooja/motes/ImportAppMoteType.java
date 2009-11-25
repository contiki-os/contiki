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
 *
 */

package se.sics.cooja.motes;

import java.awt.Container;
import java.io.File;
import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.Collection;

import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.AbstractionLevelDescription;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.util.ArrayUtils;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Imported App Mote Type")
@AbstractionLevelDescription("Application level")
public class ImportAppMoteType extends AbstractApplicationMoteType {
  private static Logger logger = Logger.getLogger(ImportAppMoteType.class);

  private Simulation simulation;

  private File moteClassFile = null;
  private Class<? extends AbstractApplicationMote> moteClass = null;
  private Constructor<? extends AbstractApplicationMote> moteConstructor = null;

  public ImportAppMoteType() {
    super();
  }

  public ImportAppMoteType(String identifier) {
    super(identifier);
    setDescription("Imported App Mote Type #" + identifier);
  }

  private class TestClassLoader extends ClassLoader {
    private File file;
    public TestClassLoader(File f) {
      file = f;
    }
    public TestClassLoader(ClassLoader parent, File f) {
      super(parent);
      file = f;
    }
    public Class<?> findClass(String name) {
      byte[] data = loadClassData(name);
      return defineClass(data, 0, data.length);
    }
    private byte[] loadClassData(String name) {
      return ArrayUtils.readFromFile(file);
    }
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();

    Element element = new Element("moteclass");
    element.setText(simulation.getGUI().createPortablePath(moteClassFile).getPath());
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML, boolean visAvailable)
  throws MoteTypeCreationException {
    this.simulation = simulation;

    for (Element element : configXML) {
      String name = element.getName();
      if (name.equals("moteclass")) {
        moteClassFile = simulation.getGUI().restorePortablePath(new File(element.getText()));
      }
    }

    return configureAndInit(GUI.getTopParentContainer(), simulation, visAvailable);
  }

  public boolean configureAndInit(Container parentContainer,
      Simulation simulation, boolean visAvailable) 
  throws MoteTypeCreationException {
    this.simulation = simulation;

    if (!super.configureAndInit(parentContainer, simulation, visAvailable)) {
      return false;
    }

    boolean updateSuggestion = false;
    if (visAvailable) {
      /* Select mote class file */
      JFileChooser fileChooser = new JFileChooser();
      fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
      fileChooser.setDialogTitle("Select application class file");
      if (moteClassFile != null) {
        fileChooser.setSelectedFile(moteClassFile);
      } else {
        updateSuggestion = true;
        fileChooser.setSelectedFile(new File(GUI.getExternalToolsSetting("IMPORT_APP_LAST", "mymote.class")));
      }
      fileChooser.setFileFilter(new FileFilter() {
        public boolean accept(File file) {
          if (file.isDirectory()) {
            return true;
          }
          if (file.getName().endsWith(".class")) {
            return true;
          }
          return false;
        }
        public String getDescription() {
          return "Java class extending " + AbstractApplicationMoteType.class.getName();
        }
        public String toString() {
          return ".class";
        }
      });

      int reply = fileChooser.showOpenDialog(GUI.getTopParentContainer());
      if (reply != JFileChooser.APPROVE_OPTION) {
        return false;
      }

      moteClassFile = fileChooser.getSelectedFile();
    }

    if (moteClassFile == null) {
      throw new MoteTypeCreationException("Unknown mote class file");
    }
    if (!moteClassFile.exists()) {
      throw new MoteTypeCreationException("Mote class file does not exist");
    }
    if (updateSuggestion) {
      GUI.setExternalToolsSetting("IMPORT_APP_LAST", moteClassFile.getPath());
    }

    /* Load class */
    TestClassLoader loader = new TestClassLoader(moteClassFile);
    try {
      moteClass = 
        (Class<? extends AbstractApplicationMote>) 
        loader.loadClass("ignored class name");
      moteConstructor = 
        (Constructor<? extends AbstractApplicationMote>) 
        moteClass.getConstructor(new Class[] { MoteType.class, Simulation.class });
    } catch (Exception e) {
      throw (MoteTypeCreationException) 
      new MoteTypeCreationException("Error when loading class from: " + moteClassFile.getPath()).initCause(e);
    } catch (Throwable t) {
      throw (MoteTypeCreationException) 
      new MoteTypeCreationException("Error when loading class from: " + moteClassFile.getPath()).initCause(t);
    }

    /* XXX Extending GUI's project class loader. Nasty. */
    if (simulation.getGUI().projectDirClassLoader == null) {
      simulation.getGUI().projectDirClassLoader = 
        new TestClassLoader(ClassLoader.getSystemClassLoader(), moteClassFile);
    } else {
      simulation.getGUI().projectDirClassLoader = 
        new TestClassLoader(simulation.getGUI().projectDirClassLoader, moteClassFile);
    }

    setDescription("Imported Mote Type #" + moteClassFile.getName());

    return true;
  }

  public Mote generateMote(Simulation simulation) {
    try {
      return moteConstructor.newInstance(ImportAppMoteType.this, simulation);
    } catch (Exception e) {
      throw (RuntimeException) new RuntimeException("Error when generating mote").initCause(e);
    }
  }
}

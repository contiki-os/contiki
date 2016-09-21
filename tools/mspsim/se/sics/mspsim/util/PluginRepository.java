/**
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * PluginRepository
 *
 * Author  : Joakim Eriksson, Niclas Finne
 * Created : Wed Feb 13 19:51:00 2008
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.util;
import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.jar.Attributes;
import java.util.jar.JarFile;
import java.util.jar.Manifest;

public class PluginRepository implements ActiveComponent {

  private URLClassLoader classLoader;

  @Override
  public void init(String name, ComponentRegistry registry) {
    File dir = new File("plugins");
    if (dir.isDirectory()) {
      File[] files = dir.listFiles(new JarFilter());
      if (files != null) {
	try {
	  URL[] jarFiles = new URL[files.length];
	  String[] plugins = new String[files.length];
	  int pluginCount = 0;
	  for (int i = 0, n = files.length; i < n; i++) {
	    URL jarURL = files[i].toURI().toURL();
// 	    System.out.println("JAR FILE: " + jarURL);
	    JarFile jarFile = new JarFile(files[i]);
	    Manifest mf = jarFile.getManifest();
	    Attributes attr = mf.getMainAttributes();
	    String pluginBundle = attr.getValue("mspsim-plugin");
	    if (pluginBundle != null) {
	      plugins[pluginCount++] = pluginBundle;
	    }
	    jarFiles[i] = jarURL;
	    jarFile.close();
	  }

	  classLoader = URLClassLoader.newInstance(jarFiles);

	  for (int i = 0; i < pluginCount; i++) {
	    try {
	      PluginBundle bundle = (PluginBundle) classLoader.loadClass(plugins[i]).newInstance();
	      //	          System.out.println("PluginBundle: " + bundles[i].getClass()
	      //	                             + "  (" + plugins[i] + ')');
	      bundle.init(registry);
	    } catch (Exception e) {
	      // TODO: handle exception
	    }
	  }
	} catch (IOException e) {
	  e.printStackTrace();
	}
      }
    }
  }

  public Class<?> loadClass(String name) throws ClassNotFoundException {
    if (classLoader == null) {
      return Class.forName(name);
    }
    return classLoader.loadClass(name);
  }

  @Override
  public void start() {
  }

  private static class JarFilter implements FileFilter {

    public boolean accept(File f) {
      if (f.isFile() && f.canRead()) {
	String name = f.getName().toLowerCase();
	return name.endsWith(".jar");
      }
      return false;
    }

  }

}

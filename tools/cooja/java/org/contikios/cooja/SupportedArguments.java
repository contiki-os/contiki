/*
 * Copyright (c) 2012, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.contikios.cooja;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

import javax.swing.MenuElement;

import org.contikios.cooja.plugins.Visualizer;
import org.contikios.cooja.plugins.skins.DGRMVisualizerSkin;

/**
 * With this annotation, Cooja components (e.g. mote plugins) can be activated
 * or deactivated depending on the given argument (e.g. mote). This may for
 * example be used by a mote plugin that only accepts emulated motes, and that
 * consequently should be hidden in other non-emulated motes' plugin menues.
 *
 * See below code usage examples.
 *
 * @see Cooja#createMotePluginsSubmenu(Mote)
 * @see Visualizer#populateSkinMenu(MenuElement)
 * @see DGRMVisualizerSkin
 *
 * @author Fredrik Osterlind
 */
@Retention(RetentionPolicy.RUNTIME)
public @interface SupportedArguments {

  /**
   * @return List of accepted mote classes.
   */
  Class<? extends Mote>[] motes() default { Mote.class };

  /**
   * @return List of accepted radio medium classes.
   */
  Class<? extends RadioMedium>[] radioMediums() default { RadioMedium.class };

  /**
   * @return List of required mote interfaces.
   */
  Class<? extends MoteInterface>[] moteInterfaces() default { MoteInterface.class };
}

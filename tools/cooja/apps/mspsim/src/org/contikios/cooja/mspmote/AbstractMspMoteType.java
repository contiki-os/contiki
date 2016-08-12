/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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
 * -----------------------------------------------------------------
 *
 * AbstractMspMoteType
 *
 * Authors : Fredrik Osterlind, Niclas Finne
 */

package org.contikios.cooja.mspmote;
import java.awt.Container;
import java.awt.Image;
import java.io.File;
import java.net.URL;

import javax.swing.*;

import org.apache.log4j.Logger;

import org.contikios.cooja.*;
import org.contikios.cooja.dialogs.*;

/**
 *
 */
public abstract class AbstractMspMoteType extends MspMoteType {

    protected final Logger logger = Logger.getLogger(getClass());

    public abstract String getMoteType();

    public abstract String getMoteName();

    protected abstract String getMoteImage();

    @Override
    public boolean configureAndInit(Container parentContainer, Simulation simulation, boolean visAvailable)
            throws MoteTypeCreationException {

        /* SPECIAL CASE: Cooja started in applet.
         * Use preconfigured Contiki firmware */
        if (Cooja.isVisualizedInApplet()) {
            String firmware = Cooja.getExternalToolsSetting(getMoteType().toUpperCase() + "_FIRMWARE", "");
            if (!firmware.equals("")) {
                setContikiFirmwareFile(new File(firmware));
                JOptionPane.showMessageDialog(Cooja.getTopParentContainer(),
                        "Creating mote type from precompiled firmware: " + firmware,
                        "Compiled firmware file available", JOptionPane.INFORMATION_MESSAGE);
            } else {
                JOptionPane.showMessageDialog(Cooja.getTopParentContainer(),
                        "No precompiled firmware found",
                        "Compiled firmware file not available", JOptionPane.ERROR_MESSAGE);
                return false;
            }
        }

        /* If visualized, show compile dialog and let user configure */
        if (visAvailable) {

            /* Create unique identifier */
            if (getIdentifier() == null) {
                int counter = 0;
                boolean identifierOK = false;
                while (!identifierOK) {
                    identifierOK = true;

                    counter++;
                    setIdentifier(getMoteType() + counter);

                    for (MoteType existingMoteType : simulation.getMoteTypes()) {
                        if (existingMoteType == this) {
                            continue;
                        }
                        if (existingMoteType.getIdentifier().equals(getIdentifier())) {
                            identifierOK = false;
                            break;
                        }
                    }
                }
            }

            /* Create initial description */
            if (getDescription() == null) {
                setDescription(getMoteName() + " Mote Type #" + getIdentifier());
            }

            return MspCompileDialog.showDialog(parentContainer, simulation, this, getMoteType());
        }

        /* Not visualized: Compile Contiki immediately */
        if (getIdentifier() == null) {
            throw new MoteTypeCreationException("No identifier");
        }

        final MessageList compilationOutput = visAvailable ? new MessageListUI() : new MessageListText();

        if (getCompileCommands() != null) {
            /* Handle multiple compilation commands one by one */
            String[] arr = getCompileCommands().split("\n");
            for (String cmd: arr) {
                cmd = cmd.trim();
                if (cmd.isEmpty()) {
                    continue;
                }

                try {
                    CompileContiki.compile(
                            cmd,
                            null,
                            null /* Do not observe output firmware file */,
                            getContikiSourceFile().getParentFile(),
                            null,
                            null,
                            compilationOutput,
                            true
                            );
                } catch (Exception e) {
                    MoteTypeCreationException newException =
                            new MoteTypeCreationException("Mote type creation failed: " + e.getMessage(), e);
                    newException.setCompilationOutput(compilationOutput);

                    /* Print last 10 compilation errors to console */
                    MessageContainer[] messages = compilationOutput.getMessages();
                    for (int i = Math.max(messages.length - 10, 0); i < messages.length; i++) {
                        logger.fatal(">> " + messages[i]);
                    }

                    logger.fatal("Compilation error: " + e.getMessage());
                    throw newException;
                }
            }
        }

        if (getContikiFirmwareFile() == null
                || !getContikiFirmwareFile().exists()) {
            throw new MoteTypeCreationException(
                    "Contiki firmware file does not exist: "
                            + getContikiFirmwareFile());
        }
        return true;
    }

    @Override
    public Icon getMoteTypeIcon() {
        String imageName = getMoteImage();
        if (imageName == null) {
            return null;
        }
        URL imageURL = this.getClass().getClassLoader().getResource(imageName);
        if (imageURL == null) {
            return null;
        }
        ImageIcon icon = new ImageIcon(imageURL);
        if (icon.getIconHeight() > 0 && icon.getIconWidth() > 0) {
            Image image = icon.getImage().getScaledInstance(
                    (200 * icon.getIconWidth() / icon.getIconHeight()), 200,
                    Image.SCALE_DEFAULT);
            return new ImageIcon(image);
        }
        return null;
    }

    @Override
    public File getExpectedFirmwareFile(File source) {
        File parentDir = source.getParentFile();
        String sourceNoExtension = source.getName();
        if (sourceNoExtension.endsWith(".c")) {
            sourceNoExtension = sourceNoExtension.substring(0, source.getName().length() - 2);
        }
        return new File(parentDir, sourceNoExtension + '.' + getMoteType());
    }

    protected Class<? extends MoteInterface>[] createMoteInterfaceList(Class<? extends MoteInterface>... interfaceList) {
        return interfaceList;
    }

}

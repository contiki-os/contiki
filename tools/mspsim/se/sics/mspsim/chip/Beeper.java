/**
 * Copyright (c) 2007-2010, Swedish Institute of Computer Science.
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
 * Beeper
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.chip;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.FloatControl;
import javax.sound.sampled.SourceDataLine;

import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.TimeEvent;

/**
 * Beeper for the ESB...
 */
public class Beeper extends Chip {

    public static final int MODE_OFF = 0;
    public static final int MODE_ON = 1;
    public static final int MODE_MAX = MODE_ON;

    public static final int SAMPLE_RATE = 44000;
    public static final int FRQ_1 = 2200;
    public static final int WAVE_LEN = (SAMPLE_RATE / FRQ_1);

    // One second of the sound in buffer
    private static byte[] buffer;
    private static byte[] quiet;

    private boolean beepOn = false;
    private int beepCtrl;
    private boolean isSoundEnabled = false;

    private SourceDataLine dataLine;
    private FloatControl volume;

    private TimeEvent soundEvent;

    public Beeper(MSP430Core cpu) {
        super("Beeper", cpu);
        setMode(MODE_OFF);
    }

    private void initSound() {
        if (quiet == null) {
            quiet = new byte[WAVE_LEN];
        }
        if (buffer == null) {
            byte[] buf = new byte[WAVE_LEN];
            double f1 = 0;
            for (int i = 0, n = WAVE_LEN; i < n; i++) {
                f1 = Math.sin(i * 3.141592 * 2 / WAVE_LEN) * 40;
                f1 += Math.sin(i * 3.141592 * 4 / WAVE_LEN) * 30;
                buf[i] = (byte) (f1);
            }
            buffer = buf;
        }
        if (soundEvent == null) {
            soundEvent = new TimeEvent(0, "Beeper") {
                public void execute(long t) {
                    if (isSoundEnabled) {
                        ioTick(t);
                        cpu.scheduleCycleEvent(this, cpu.cycles + 1000);
                    }
                }
            };
        }
        AudioFormat af = new AudioFormat(SAMPLE_RATE, 8, 1, true, false);
        DataLine.Info dli = new DataLine.Info(SourceDataLine.class, af, 16384);
        try {
            dataLine = (SourceDataLine) AudioSystem.getLine(dli);
            if (dataLine == null) {
                logw(WarningType.EMULATION_ERROR, "No audio data line available");
            } else {
                dataLine.open(dataLine.getFormat(), 16384);
                volume = (FloatControl) dataLine.getControl(FloatControl.Type.MASTER_GAIN);
            }
        } catch (Exception e) {
            logw(WarningType.EMULATION_ERROR, "Could not get audio data line: " + e);
        }
        if (dataLine != null) {
            isSoundEnabled = true;
            dataLine.start();
        }
    }

    private void shutdownSound() {
        isSoundEnabled = false;
        if (dataLine != null) {
            dataLine.close();
            dataLine = null;
            volume = null;
        }
    }

    public boolean isSoundEnabled() {
        return isSoundEnabled;
    }

    public void setSoundEnabled(boolean sound) {
        if (this.isSoundEnabled != sound) {
            if (sound) {
                initSound();
            } else {
                shutdownSound();
            }
        }
    }

    public int getVolume() {
        return volume == null ? 0 : (int) volume.getValue();
    }

    public void setVolume(int vol) {
        if (volume != null) {
            volume.setValue(vol);
        }
    }

    public void beepOn(boolean beep) {
        if (beepOn != beep) {
            beepOn = beep;
            setMode(beepOn ? MODE_ON : MODE_OFF);
            if (DEBUG) log(beepOn ? "BEEPING" : "SILENT");
            if (beepOn && isSoundEnabled) {
                beepCtrl = 7;
                if (!soundEvent.isScheduled()) {
                    cpu.scheduleTimeEvent(soundEvent, cpu.getTime() + 2);
                }
            }
        }
    }

    private void ioTick(long time) {
        // Avoid blocking using timer...
        if (isSoundEnabled && dataLine != null) {
            if (dataLine.available() > WAVE_LEN * 2) {
                if (beepCtrl > 0) {
                    dataLine.write(buffer, 0, WAVE_LEN);
                    if (!beepOn) {
                        beepCtrl--;
                    }
                } else {
                    dataLine.write(quiet, 0, WAVE_LEN);
                }
            }
        }
    }

    public int getModeMax() {
        return MODE_MAX;
    }

    public String info() {
        return "Volume: " + getVolume() + " Beep: " + (beepOn ? "on" : "off")
        + " Sound Enabled: " + isSoundEnabled;
    }

    /* just return some value */
    public int getConfiguration(int parameter) {
        return beepOn ? 1 : 0;
    }

}

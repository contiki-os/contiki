/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: ScriptParser.java,v 1.1 2009/01/08 16:27:48 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.script.ScriptException;

import org.apache.log4j.Logger;

public class ScriptParser {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ScriptParser.class);

  private long timeoutTime = -1;
  private String timeoutCode = null;

  private String code = null;

  public ScriptParser(String code) throws ScriptSyntaxErrorException {

    code = fixNewlines(code);

    code = stripMultiLineComments(code);

    code = stripSingleLineComments(code);

    code = parseTimeout(code);

    code = parseTimeoutWithAction(code);

    code = replaceYields(code);

    code = replaceWaitUntils(code);

    code = replaceYieldThenWaitsUntils(code);

    this.code = code;
  }

  private String fixNewlines(String code) {
    code = code.replaceAll("\r\n", "\n");
    code = "\n" + code + "\n";
    return code;
  }

  private String stripSingleLineComments(String code) {
    /* TODO Handle strings */
    Pattern pattern = Pattern.compile("//.*\n");
    Matcher matcher = pattern.matcher(code);
    code = matcher.replaceAll("\n");
    return code;
  }

  private String stripFirstComment(String code) {
    int first = code.indexOf('"');
    if (first < 0) {
      return code;
    }
    int second = code.indexOf('"', first+1);

    code = code.substring(0, first) + code.substring(second+1, code.length());
    return code;
  }

  private String stripMultiLineComments(String code) {
    /* TODO Handle strings */
    Pattern pattern =
      Pattern.compile("/\\*([^*]|[\n]|(\\*+([^*/]|[\n])))*\\*+/");
    Matcher matcher = pattern.matcher(code);

    while (matcher.find()) {
      String match = matcher.group();
      int newLines = match.split("\n").length;
      String replacement = "";
      for (int i=0; i < newLines; i++) {
        replacement += "\n";
      }
      code = matcher.replaceFirst(replacement);
      matcher.reset(code);
    }
    return code;
  }

  private String parseTimeout(String code) throws ScriptSyntaxErrorException {
    Pattern pattern = Pattern.compile(
        "TIMEOUT\\(" +
        "([0-9]+)" /* timeout */ +
        "\\)"
    );
    Matcher matcher = pattern.matcher(code);

    if (!matcher.find()) {
      return code;
    }

    if (timeoutTime > 0) {
      throw new ScriptSyntaxErrorException("Only one timeout handler allowed");
    }

    timeoutTime = Long.parseLong(matcher.group(1));
    timeoutCode = ";";

    matcher.reset(code);
    code = matcher.replaceFirst(";");

    matcher.reset(code);
    if (matcher.find()) {
      throw new ScriptSyntaxErrorException("Only one timeout handler allowed");
    }
    return code;
  }

  private String parseTimeoutWithAction(String code) throws ScriptSyntaxErrorException {
    Pattern pattern = Pattern.compile(
        "TIMEOUT\\(" +
        "([0-9]+)" /* timeout */ +
        "[\\s]*,[\\s]*" +
        "(.*)" /* code */ +
        "\\)"
    );
    Matcher matcher = pattern.matcher(code);

    if (!matcher.find()) {
      return code;
    }

    if (timeoutTime > 0) {
      throw new ScriptSyntaxErrorException("Only one timeout handler allowed");
    }

    timeoutTime = Long.parseLong(matcher.group(1));
    timeoutCode = matcher.group(2);

    matcher.reset(code);
    code = matcher.replaceFirst(";");

    matcher.reset(code);
    if (matcher.find()) {
      throw new ScriptSyntaxErrorException("Only one timeout handler allowed");
    }
    return code;
  }

  private String replaceYields(String code) throws ScriptSyntaxErrorException {
    Pattern pattern = Pattern.compile(
        "YIELD\\(\\)"
    );
    return pattern.matcher(code).replaceAll("SCRIPT_SWITCH()");
  }

  private String replaceYieldThenWaitsUntils(String code) throws ScriptSyntaxErrorException {
    Pattern pattern = Pattern.compile(
        "YIELD_THEN_WAIT_UNTIL\\(" +
        "(.*)" /* expression */ +
        "\\)"
    );
    Matcher matcher = pattern.matcher(code);

    while (matcher.find()) {
      code = matcher.replaceFirst(
          "do { " +
          " SCRIPT_SWITCH();" +
          "} while (!(" + matcher.group(1) + "))");
      matcher.reset(code);
    }

    return code;
  }

  private String replaceWaitUntils(String code) throws ScriptSyntaxErrorException {
    Pattern pattern = Pattern.compile(
        "WAIT_UNTIL\\(" +
        "(.*)" /* expression */ +
        "\\)"
    );
    Matcher matcher = pattern.matcher(code);

    while (matcher.find()) {
      code = matcher.replaceFirst(
          "while (!(" + matcher.group(1) + ")) { " +
          " SCRIPT_SWITCH()" +
      "}");
      matcher.reset(code);
    }

    return code;
  }

  public String getJSCode() {
    return
    "function SCRIPT_KILL() { " +
    " SEMAPHORE_SIM.release(100); " +
    " throw('test script killed'); " +
    "};\n" +
    "function SCRIPT_TIMEOUT() { " +
    timeoutCode + "; " +
    " log.log('TEST TIMEOUT\\n'); " +
    " log.testFailed(); " +
    " while (!SHUTDOWN) { " +
    "  SEMAPHORE_SIM.release(); " +
    "  SEMAPHORE_SCRIPT.acquire(); " /* SWITCH BLOCKS HERE! */ +
    " } " +
    " SCRIPT_KILL(); " +
    "};\n" +
    "function SCRIPT_SWITCH() { " +
    " SEMAPHORE_SIM.release(); " +
    " SEMAPHORE_SCRIPT.acquire(); " /* SWITCH BLOCKS HERE! */ +
    " if (SHUTDOWN) { SCRIPT_KILL(); } " +
    " if (TIMEOUT) { SCRIPT_TIMEOUT(); } " +
    " msg = mote.getInterfaces().getLog().getLastLogMessage(); " +
    " node.setMoteMsg(mote, msg); " +
    "};\n" +
    "function run() { " +
    "SEMAPHORE_SIM.acquire(); " +
    "SEMAPHORE_SCRIPT.acquire(); " + /* STARTUP BLOCKS HERE! */
    "if (SHUTDOWN) { SCRIPT_KILL(); } " +
    "if (TIMEOUT) { SCRIPT_TIMEOUT(); } " +
    "msg = mote.getInterfaces().getLog().getLastLogMessage(); " +
    "node.setMoteMsg(mote, msg); " +
    code + "\n" +
    "while (true) { SCRIPT_SWITCH(); } " /* SCRIPT ENDED */+
    "};";
  }

  public long getTimeoutTime() {
    return timeoutTime;
  }

  public class ScriptSyntaxErrorException extends ScriptException {
    public ScriptSyntaxErrorException(String msg) {
      super(msg);
    }
  }

}

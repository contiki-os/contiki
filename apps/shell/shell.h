/** \addtogroup apps
 * @{ */

/**
 * \defgroup shell The Contiki shell
 * @{
 *
 * The Contiki shell provides both interactive and batch processing
 * for Contiki.
 *
 * The shell consists of two parts: the shell application and a shell
 * back-end. The shell application contains all the logic of the
 * shell, whereas the shell back-end provides I/O for the
 * shell. Examples of shell back-ends are a serial I/O shell back-end,
 * that allows the shell to operate over a serial connection, and a
 * telnet server back-end, that allows the shell to operate over a
 * TCP/IP telnet connection.
 *
 */

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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Main header file for the Contiki shell
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#ifndef __SHELL_H__
#define __SHELL_H__

#include "sys/process.h"

/**
 * \brief      Holds a information about a shell command
 *
 *             This structure contains information about a shell
 *             command. It is an opaque structure with no user-visible
 *             elements.
 *
 */
struct shell_command {
  struct shell_command *next;
  char *command;
  char *description;
  struct process *process;
  struct shell_command *child;
};

/**
 * \name       Shell back-end API
 *
 *             The shell back-end API contains functions that are used
 *             by shell back-ends.
 * @{
 */

/**
 * \brief      Initialize the shell.
 *
 *             This function initializes the shell. It typically is
 *             called from the shell back-end.
 *
 */
void shell_init(void);

/**
 * \brief      Start the shell.
 *
 *             This function starts the shell and prints out the shell
 *             prompt. It typically is called by the shell back-end to
 *             start a new shell session.
 *
 */
void shell_start(void);

/**
 * \brief      Send a line of input to the shell
 * \param commandline A pointer to a string that contains the command line
 * \param commandline_len Length of the command line, in bytes
 *
 *             This function is called by a shell back-end to send an
 *             incoming command line to the shell. The shell parses
 *             the command line and starts any commands found in the
 *             command line.
 *
 */
void shell_input(char *commandline, int commandline_len);

/**
 * \brief      Stop the shell
 *
 *             This function stops all running commands. It typically
 *             is called by a shell back-end to to indicate that the
 *             user has quit the shell.
 *
 */
void shell_stop(void);

/**
 * \brief      Quit the shell
 *
 *             This function is called by a shell back-end to stop the
 *             shell processes.
 *
 */
void shell_quit(void);

/**
 * @}
 */

/**
 * \name       Shell back-end callback functions
 *
 *             These callback functions are called from the shell to
 *             the shell back-end. The shell back-end must implement
 *             all back-end callback functions.
 * @{
 */


/**
 * \brief      Print a prompt
 * \param prompt A suggested prompt
 *
 *             This function is called by the shell to print a
 *             prompt. The shell back-end may show the suggested
 *             prompt, or another prompt.
 *
 */
void shell_prompt(char *prompt);

/**
 * \brief      Print a line of output from the shell
 * \param data1 A pointer to the first half of the data
 * \param size1 The size of the first half of the data
 * \param data2 A pointer to the second half of the data
 * \param size2 The size of the second half of the data
 *
 *
 *             This function is called by a shell command to output
 *             data. The output is split into two halves to make it
 *             easier for shell commands to output data that contains
 *             a static part (such as a static string) and a dynamic
 *             part (a dynamically generated string).
 *
 */
void shell_default_output(const char *data1, int size1,
			  const char *data2, int size2);

/**
 * \brief      Request shell exit
 *
 *             This function is called by the shell to request exiting
 *             the current session. The shell back-end will later call
 *             shell_stop() when the session was successfully exited.
 *
 */
void shell_exit(void);

/**
 * @}
 */

/**
 * \name       Shell command API
 *
 *             These functions are used by shell commands.
 * @{
 */


/**
 * \brief      Define a shell command
 * \param name The variable name of the shell command definition
 * \param command A string with the name of the shell command
 * \param description A string that contains a one-line description of the command
 * \param process A pointer to the process that implements the shell command
 *
 *             This macro defines and declares a shell command (struct
 *             shell_command). This is used with the
 *             shell_register_command() function to register the
 *             command with the shell.
 *
  * \hideinitializer
 */
#define SHELL_COMMAND(name, command, description, process) \
static struct shell_command name = { NULL, command, \
                                     description, process }


/**
 * \brief      Output data from a shell command
 * \param c    The command that outputs data
 * \param data1 A pointer to the first half of the data
 * \param size1 The size of the first half of the data
 * \param data2 A pointer to the second half of the data
 * \param size2 The size of the second half of the data
 *
 *             This function is called by a shell command to output
 *             data. The output is split into two halves to make it
 *             easier for shell commands to output data that contains
 *             a static part (such as a static string) and a dynamic
 *             part (a dynamically generated string).
 *
 */
void shell_output(struct shell_command *c,
		  void *data1, int size1,
		  const void *data2, int size2);
/**
 * \brief      Output strings from a shell command
 * \param c    The command that outputs data
 * \param str1 A pointer to the first half of the string
 * \param str2 A pointer to the second half of the string
 *
 *             This function is called by a shell command to output a
 *             string. Internally, the function uses the
 *             shell_output() function to output the data. The output
 *             is split into two halves to make it easier for shell
 *             commands to output data that contains a static part
 *             (such as a static string) and a dynamic part (a
 *             dynamically generated string).
 *
 */
void shell_output_str(struct shell_command *c,
		      char *str1, const char *str2);

/**
 * \brief      Register a command with the shell
 * \param c    A pointer to a shell command structure, defined with SHELL_COMMAND()
 *
 *             This function registers a shell command with the
 *             shell. After becoming registered, the shell command
 *             will appear in the list of available shell commands and
 *             is possible to invoke by a user. The shell command must
 *             have been defined with the SHELL_COMMAND() macro.
 *
 */
void shell_register_command(struct shell_command *c);

/**
 * \brief      Unregister a previously registered shell command
 * \param c    A pointer to a shell command structure
 *
 *             This function unregisters a shell command that has
 *             previously been registered with eht
 *             shell_register_command() function.
 *
 */
void shell_unregister_command(struct shell_command *c);

/**
 * \brief      Start a shell command from another shell command
 * \param commandline A pointer to a string that contains the command line
 * \param commandline_len Length of the command line, in bytes
 * \param child A pointer to the shell command that starts the command
 * \param started_process A pointer to a shell command pointer that will be filled in with a pointer to the started command structure
 * \retval     A shell_retval indicating if the command was started as a foreground or a background process
 *
 *             This function starts a command, or a set of
 *             commands. The function is called by a shell command to
 *             start other shell commands.
 *
 */
int shell_start_command(char *commandline, int commandline_len,
			struct shell_command *child,
			struct process **started_process);

/**
 * @}
 */

/**
 * \name       Shell convenience functions
 *
 *             These functions assist shell commands in parsing
 *             command lines
 * @{
 */

/**
 * \brief      Convert a string to a number
 * \param str  The input string
 * \param retstr A pointer to a pointer to a string, is filled in with a pointer to the data after the number in the input string
 * \retval     The converted number
 *
 *             This function converts a string to a number. The
 *             function returns the converted number and a pointer to
 *             the data that follows the number in the input string.
 *
 */
unsigned long shell_strtolong(const char *str, const char **retstr);

unsigned long shell_time(void);
void shell_set_time(unsigned long seconds);

/**
 * @}
 */

/**
 * \name       Shell variables, definitions, and return values
 *
 * @{
 */

enum shell_retval {
  SHELL_FOREGROUND,
  SHELL_BACKGROUND,
  SHELL_NOTHING,
};

/**
 * \brief      The event number for shell input data
 *
 *             The shell sends data as Contiki events to shell command
 *             processes. This variable contains the number of the
 *             Contiki event.
 *
 */
extern int shell_event_input;

/**
 * \brief      Structure for shell input data
 *
 *             The shell sends data as Contiki events to shell command
 *             processes. This structure contains the data in the
 *             event. The data is split into two halves, data1 and
 *             data2. The length of the data in the two halves is
 *             given by len1 and len2.
 *
 */
struct shell_input {
  char *data1;
  const char *data2;
  int len1, len2;
};

/**
 * @}
 */

#include "shell-base64.h"
#include "shell-blink.h"
#include "shell-checkpoint.h"
#include "shell-collect-view.h"
#include "shell-coffee.h"
#include "shell-download.h"
#include "shell-exec.h"
#include "shell-file.h"
#include "shell-httpd.h"
#include "shell-irc.h"
#include "shell-memdebug.h"
#include "shell-netfile.h"
#include "shell-netperf.h"
#include "shell-netstat.h"
#include "shell-ping.h"
#include "shell-power.h"
#include "shell-powertrace.h"
#include "shell-ps.h"
#include "shell-reboot.h"
#include "shell-rime-debug.h"
#include "shell-rime-debug-runicast.h"
#include "shell-rime-neighbors.h"
#include "shell-rime-netcmd.h"
#include "shell-rime-ping.h"
#include "shell-rime-sendcmd.h"
#include "shell-rime-sniff.h"
#include "shell-rime-unicast.h"
#include "shell-rime.h"
#include "shell-rsh.h"
#include "shell-run.h"
#include "shell-sendtest.h"
#include "shell-sensortweet.h"
#include "shell-sky.h"
#include "shell-tcpsend.h"
#include "shell-text.h"
#include "shell-time.h"
#include "shell-tweet.h"
#include "shell-udpsend.h"
#include "shell-vars.h"
#include "shell-wget.h"

#endif /* __SHELL_H__ */


/** @} */
/** @} */

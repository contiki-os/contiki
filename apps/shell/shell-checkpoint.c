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
 * This file is part of the Contiki operating system.
 *
 * $Id: shell-checkpoint.c,v 1.2 2009/10/27 10:46:24 fros4943 Exp $
 */

#include "contiki.h"
#include "shell.h"

#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/checkpoint.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_checkpoint_process, "checkpoint");
SHELL_COMMAND(checkpoint_command,
          "checkpoint",
          "checkpoint <filename>: checkpoint local state to file",
          &shell_checkpoint_process);
PROCESS(shell_rollback_process, "rollback");
SHELL_COMMAND(rollback_command,
          "rollback",
          "rollback <filename>: rollback local state from file",
          &shell_rollback_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_checkpoint_process, ev, data)
{
  int fd = 0;

  PROCESS_BEGIN();

  /* Make sure file does not already exist */
  cfs_remove(data);

  cfs_coffee_reserve(data, checkpoint_arch_size());
  fd = cfs_open(data, CFS_WRITE);

  if(fd < 0) {
    shell_output_str(&checkpoint_command,
             "checkpoint: could not open file for writing: ", data);
  } else {
    shell_output_str(&checkpoint_command, "checkpoint to: ", data);
    checkpoint_checkpoint(fd);
    cfs_close(fd);
    shell_output_str(&checkpoint_command, "checkpointing done", "");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_rollback_process, ev, data)
{
  int fd = 0;

  PROCESS_BEGIN();

  fd = cfs_open(data, CFS_READ);

  if(fd < 0) {
    shell_output_str(&rollback_command,
             "rollback: could not open file for reading: ", data);
  } else {
    shell_output_str(&rollback_command, "rollback from: ", data);
    checkpoint_rollback(fd);
    cfs_close(fd);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_checkpoint_init(void)
{
  checkpoint_init();
  shell_register_command(&checkpoint_command);
  shell_register_command(&rollback_command);
}
/*---------------------------------------------------------------------------*/

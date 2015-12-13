/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-coffee-example cc2538dk Coffee Filesystem Example Project
 *
 *   Coffee filesystem example for CC2538 on SmartRF06EB.
 *
 *   This example shows how Coffee should be used. The example also verifies
 *   the Coffee functionality.
 *
 * @{
 *
 * \file
 *     Example demonstrating Coffee on the cc2538dk platform
 */
#include "contiki.h"
#include "cfs/cfs.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(coffee_test_process, "Coffee test process");
AUTOSTART_PROCESSES(&coffee_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coffee_test_process, ev, data)
{
  char message[32];
  char buf[100];
  char *filename = "msg_file";
  int fd_write, fd_read;
  int n;

  PROCESS_BEGIN();

  /* Step 1 */
  strcpy(message, "#1.hello world.");
  strcpy(buf, message);
  printf("step 1: %s\n", buf);

  /* Step 2: writing to cfs */
  fd_write = cfs_open(filename, CFS_WRITE);
  if(fd_write != -1) {
    n = cfs_write(fd_write, message, sizeof(message));
    cfs_close(fd_write);
    printf("step 2: successfully written to cfs. wrote %i bytes\n", n);
  } else {
    printf("ERROR: could not write to memory in step 2.\n");
  }

  /* Step 3: reading from cfs */
  strcpy(buf, "empty string");
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read!=-1) {
    cfs_read(fd_read, buf, sizeof(message));
    printf("step 3: %s\n", buf);
    cfs_close(fd_read);
  } else {
    printf("ERROR: could not read from memory in step 3.\n");
  }

  /* Step 4: adding more data to cfs */
  strcpy(buf, "empty string");
  strcpy(message, "#2.contiki is amazing!");
  fd_write = cfs_open(filename, CFS_WRITE | CFS_APPEND);
  if(fd_write != -1) {
    n = cfs_write(fd_write, message, sizeof(message));
    cfs_close(fd_write);
    printf("step 4: successfully appended data to cfs. wrote %i bytes  \n", n);
  } else {
    printf("ERROR: could not write to memory in step 4.\n");
  }

  /* Step 5: seeking specific data from cfs */
  strcpy(buf, "empty string");
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read != -1) {
    cfs_read(fd_read, buf, sizeof(message));
    printf("step 5: #1 - %s\n", buf);
    cfs_seek(fd_read, sizeof(message), CFS_SEEK_SET);
    cfs_read(fd_read, buf, sizeof(message));
    printf("step 5: #2 - %s\n", buf);
    cfs_close(fd_read);
  } else {
    printf("ERROR: could not read from memory in step 5.\n");
  }

  /* Step 6: remove the file from cfs */
  cfs_remove(filename);
  fd_read = cfs_open(filename, CFS_READ);
  if(fd_read == -1) {
    printf("Successfully removed file\n");
  } else {
    printf("ERROR: could read from memory in step 6.\n");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

/*
 * Copyright (c) 2014, Eistec AB.
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
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Mulle platform port of the Contiki operating system.
 *
 */

/**
 * \file
 *         Syscall implementations for K60 CPU.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */


#include <unistd.h>
#include <sys/reent.h>
#include <sys/times.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
//~ #undef errno
//~ extern int errno;

#include "K60.h"
#include "devopttab.h"
#include "devicemap.h"
#include "cfs.h"

/* Empty environment definition */
char *__env[1] = { 0 };
char **environ = __env;

/* Align all sbrk arguments to this many bytes */
#define DYNAMIC_MEMORY_ALIGN 4

/* ************************ */
/* Process control syscalls */
/* ************************ */

void
_exit(int code) {
  volatile int status; /* volatile to prevent optimizations to remove the variable from memory */
  status = code;
  (void)status; /* Suppress compiler warnings about unused variable */

  /* See local variable `status` during debugger break. */
  DEBUGGER_BREAK(BREAK_EXIT);
  NVIC_SystemReset();
  while(1);
}

int
_fork_r(struct _reent *r) {
  /* return "not supported" */
  r->_errno = ENOTSUP;
  return -1;
}

int
_execve_r(struct _reent *r, const char *name, char *const *argv, char *const *env) {
  /* Not supported */
  (void)name; /* Suppress compiler warnings about unused parameters */
  (void)argv;
  (void)env;

  r->_errno = ENOMEM;
  return -1;
}

int
_kill_r(struct _reent *r, int  pid, int  sig) {
  /* Not supported */
  (void)pid; /* Suppress compiler warnings about unused parameters */
  (void)sig;

  r->_errno = EINVAL;
  return -1;
}

pid_t
_getpid(void)
{
  /** \todo Return some process identifier on getpid() */
  return 1;
}

clock_t
_times_r(struct _reent *r, struct tms *buf)
{
  /* Not supported, yet */
  (void)buf; /* Suppress compiler warnings about unused parameters */

  r->_errno = EACCES;
  return  -1;
}

int
_wait_r(struct _reent *r, int *status)
{
  /* Not supported, yet */
  (void)status; /* Suppress compiler warnings about unused parameters */

  r->_errno = ECHILD;
  return -1;
}

/* ******************************** */
/* File descriptor related syscalls */
/* ******************************** */

/** Internal helper for generating FDs */
static int get_next_dev_fd() {
  int fd;
  for (fd = 0; fd < MAX_OPEN_DEVICES; ++fd) {
    if (devoptab_list[fd] == NULL) {
      return fd;
    }
  }
  return -1;
}

int
_open_r(struct _reent *r, const char *name, int flags, int mode) {
  unsigned int i;
  int fd;
  int cfs_flags = 0;
  /* Search for devices */
  for (i = 0; i < devoptab_name_list.len; ++i) {
    if (strcmp(devoptab_name_list.data[i].name, name) == 0) {
      /* Device found */
      fd = get_next_dev_fd();
      if (fd < 0)
      {
        /* No free FDs. */
        /* ENFILE means too many file descriptors open, system-wide. */
        r->_errno = ENFILE;
        return -1;
      }
      /* Set up device operations table and call open method */
      devoptab_list[fd] = devoptab_name_list.data[i].devoptab;
      /* open_r method is mandatory */
      devoptab_list[fd]->open_r(r, name, flags, mode);
      return fd;
    }
  }
  /* Not a device name, try searching for files. */
  /* Translate POSIX O_* flags to CFS */
  if (flags & O_APPEND) {
    cfs_flags |= CFS_APPEND;
  }
  if (flags & O_RDWR) {
    cfs_flags |= CFS_READ | CFS_WRITE;
  }
  else if (flags & O_RDONLY) {
    cfs_flags |= CFS_READ;
  }
  else if (flags & O_WRONLY) {
    cfs_flags |= CFS_WRITE;
  }
  fd = cfs_open(name, cfs_flags);
  if (fd < 0) {
    /* Not found or whatever, CFS doesn't tell us why it failed. */
    r->_errno = ENOENT;
    return -1;
  }
  fd += MAX_OPEN_DEVICES; /* Remap from CFS FD number */
  return fd;
}

int
_close_r(struct _reent *r, int fd) {
  int ret;

  if (fd >= MAX_OPEN_DEVICES) {
    /* CFS file */
    fd -= MAX_OPEN_DEVICES; /* Remap to CFS FD number */
    cfs_close(fd);
    return 0; /* cfs_close does not indicate failures */
  }
  if (devoptab_list[fd] == NULL) {
    /* nothing mapped on that FD */
    r->_errno = EBADF;
    return -1;
  }
  if (devoptab_list[fd]->close_r == NULL) {
    /* Function not implemented */
    r->_errno = ENOSYS;
    return -1;
  }
  /* Call method from device operations table */
  ret = devoptab_list[fd]->close_r(r, fd);
  if (ret == 0) {
    /* Successfully closed, clear out device operations table entry to free up
     * the file descriptor. */
    devoptab_list[fd] = NULL;
  }
  return ret;
}

ssize_t
_read_r(struct _reent *r, int fd, void *ptr, size_t len) {
  if (fd >= MAX_OPEN_DEVICES) {
    int ret;
    /* CFS file */
    fd -= MAX_OPEN_DEVICES; /* Remap to CFS FD number */
    /* this is not really reentrant */
    ret = cfs_read(fd, ptr, len);
    if (ret < 0) {
      r->_errno = EBADF;
    }
    return ret;
  }
  if (devoptab_list[fd] == NULL) {
    /* nothing mapped on that FD */
    r->_errno = EBADF;
    return -1;
  }
  if (devoptab_list[fd]->read_r == NULL) {
    /* Function not implemented */
    r->_errno = ENOSYS;
    return -1;
  }
  /* Call method from device operations table */
  return devoptab_list[fd]->read_r(r, fd, ptr, len);
}

ssize_t
_write_r(struct _reent *r, int fd, const void *ptr, size_t len) {
  if (fd >= MAX_OPEN_DEVICES) {
    int ret;
    /* CFS file */
    fd -= MAX_OPEN_DEVICES; /* Remap to CFS FD number */
    /* this is not really reentrant */
    ret = cfs_write(fd, (const char *)ptr, len);
    if (ret < 0) {
      r->_errno = EBADF;
    }
    return ret;
  }
  if (devoptab_list[fd] == NULL) {
    /* nothing mapped on that FD */
    r->_errno = EBADF;
    return -1;
  }
  if (devoptab_list[fd]->write_r == NULL) {
    /* Function not implemented */
    r->_errno = ENOSYS;
    return -1;
  }
  /* Call method from device operations table */
  return devoptab_list[fd]->write_r(r, fd, ptr, len);
}

off_t
_lseek_r(struct _reent *r, int fd, off_t offset, int whence) {
  if (fd >= MAX_OPEN_DEVICES) {
    int ret;
    /* CFS file */
    fd -= MAX_OPEN_DEVICES; /* Remap to CFS FD number */
    /* CFS_SEEK_* macros used by the CFS whence parameter is assumed to
     * correspond with POSIX constants */
    /* this is not really reentrant */
    ret = cfs_seek(fd, offset, whence);
    if (ret < 0) {
      r->_errno = EBADF;
    }
    return ret;
  }
  if (devoptab_list[fd] == NULL) {
    /* nothing mapped on that FD */
    r->_errno = EBADF;
    return -1;
  }
  if (devoptab_list[fd]->lseek_r == NULL) {
    /* Function not implemented */
    r->_errno = ENOSYS;
    return -1;
  }
  /* Call method from device operations table */
  return devoptab_list[fd]->lseek_r(r, fd, offset, whence);
}

int
_fstat_r(struct _reent *r, int fd, struct stat *st) {
  if (fd >= MAX_OPEN_DEVICES) {
    /* CFS file */
    st->st_mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO; /* regular file, 0777 perms (-rwxrwxrwx) */
    /** \todo Handle file size with fstat */
    /* st->st_uid = 0; */
    /* st->st_gid = 0; */
    /* st->st_size = 0; */
    return 0;
  }
  if (devoptab_list[fd] != NULL) {
    /* Check device operations table to determine mode */
    st->st_mode = devoptab_list[fd]->st_mode;
    return 0;
  } else {
    /* nothing mapped on that FD */
    r->_errno = EBADF;
    return -1;
  }
}

int
_isatty_r(struct _reent *r, int fd) {
  if (fd >= MAX_OPEN_DEVICES) {
    /* CFS file, not a TTY */
    r->_errno = ENOTTY;
    return 0;
  }
  if (devoptab_list[fd] != NULL) {
    /* Check device operations table to determine if it is considered a TTY */
    if (devoptab_list[fd]->isatty == 0) {
      r->_errno = ENOTTY;
    }
    return devoptab_list[fd]->isatty;
  } else {
    /* nothing mapped on that FD */
    r->_errno = EBADF;
    return -1;
  }
}

/* **************************** */
/* File system related syscalls */
/* **************************** */

int
_stat_r(struct _reent *r, const char *file, struct stat *st) {
  /* not supported, yet */
  (void)file; /* Suppress compiler warnings about unused parameters */
  (void)st;
  r->_errno = ENOENT;
  return -1;
}

int
_link_r(struct _reent *r, const char *old, const char *new)
{
  /* not supported, yet */
  (void)old; /* Suppress compiler warnings about unused parameters */
  (void)new;
  r->_errno = EMLINK;
  return -1;
}

int
_unlink_r(struct _reent *r, const char *name)
{
  /* not supported, yet */
  (void)name; /* Suppress compiler warnings about unused parameters */
  r->_errno = ENOENT;
  return -1;
}

/* ********************************** */
/* Memory management related syscalls */
/* ********************************** */

/* Beginning of unallocated RAM, defined by the linker script */
/* extern int _end; */
extern int _heap_start;
/* End of RAM area available for allocation */
extern int _heap_end;
/* Current edge of dynamically allocated space */
static void* current_break = (void *)(&_heap_start);

/**
 * Move the program break.
 *
 * This function can increase the size of the allocated memory.
 */
void* _sbrk_r(struct _reent *r, ptrdiff_t increment) {
  /* Align memory increment to nearest DYNAMIC_MEMORY_ALIGN bytes upward */
  if (increment % DYNAMIC_MEMORY_ALIGN)
  {
    increment += DYNAMIC_MEMORY_ALIGN - (increment % DYNAMIC_MEMORY_ALIGN);
  }
  if ((current_break + increment) < ((void *)(&_heap_end)))
  {
    void* ret = (void*)current_break;
    current_break += increment;
    return ret;
  }
  else
  {
    r->_errno = ENOMEM;
    return (void *)(-1);
  }
}

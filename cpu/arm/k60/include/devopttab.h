#ifndef DEVOPTTAB_H_
#define DEVOPTTAB_H_

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <reent.h>

/* From http://neptune.billgatliff.com/newlib.html
 * A simple "device operations" table, with function pointers for all the kinds
 * of activities you would expect a stream-like device to support.
 */
typedef struct {
   const char *name;
   const int isatty;
   const int st_mode;
   int  (*open_r  )(struct _reent *r, const char *path, int flags, int mode);
   int  (*close_r )(struct _reent *r, int fd);
   long (*write_r )(struct _reent *r, int fd, const char *ptr, int len);
   long (*read_r  )(struct _reent *r, int fd, char *ptr, int len);
   long (*lseek_r )(struct _reent *r, int fd, int ptr, int dir);
   long (*fstat_r )(struct _reent *r, int fd, char *ptr, int len);
} devoptab_t;

#endif

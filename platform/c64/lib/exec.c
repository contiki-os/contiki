/*
** Program-chaining function for Commodore platforms.
**
** This copy of the cc65 system library function makes smaller code by using
** Contiki's Personal File System (instead of POSIX) functions.
**
** 2016-03-16, Greg King
**
** This function exploits the program-chaining feature in CBM BASIC's ROM.
**
** CC65's CBM programs have a BASIC program stub.  We start those programs by
** RUNning that stub; it SYSes to the Machine Language code.  Normally, after
** the ML code exits, the BASIC ROM continues running the stub.  But, it has
** no more statements; so, the program stops.
**
** This function puts the desired program's name and device number into a LOAD
** statement.  Then, it points BASIC to that statement, so that the ROM will run
** that statement after this program quits.  The ROM will load the next program,
** and will execute it (because the LOAD will be seen in a running program).
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <device.h>

#include "cfs.h"


/* The struct below is a line of BASIC code.  It sits in the LOWCODE segment
** to make sure that it won't be hidden by a ROM when BASIC is re-enabled.
** The line is:
**  0 CLR:LOAD""+""                    ,01
** After this function has written into the line, it might look like this:
**  0 CLR:LOAD""+"program name"        ,08
**
** When BASIC's LOAD command asks the Kernal to load a file, it gives the
** Kernal a pointer to a file-name string.  CC65's CBM programs use that
** pointer to give a copy of the program's name to main()'s argv[0] parameter.
** But, when BASIC uses a string literal that is in a program, it points
** directly to that literal -- in the models that don't use banked RAM
** (Pet/CBM, VIC-20, and 64).  The literal is overwritten by the next program
** that is loaded.  So, argv[0] would point to machine code.  String operations
** create a new result string -- even when that operation changes nothing.  The
** result is put in the string space at the top of BASIC's memory.  So, the ""+
** in this BASIC line guarantees that argv[0] will get a name from a safe place.
*/
#pragma data-name(push, "LOWCODE")
static struct line {
  const char end_of_line;               /* fake previous line */
  const struct line* const next;
  const unsigned line_num;
  const char CLR_token, colon, LOAD_token, quotes[2], add_token, quote;
  char name[21];
  const char comma;
  char unit[3];
} basic = {
  '\0', &basic + 1,                     /* high byte of link must be non-zero */
  0, 0x9C, ':', 0x93, "\"\"", 0xAA, '\"',
  "\"                    ",             /* format: "123:1234567890123456\"" */
  ',', "01"
};
#pragma data-name(pop)

/* These values are platform-specific. */
extern const void* vartab;              /* points to BASIC program variables */
#pragma zpsym("vartab")
extern const void* memsize;             /* points to top of BASIC RAM */
#pragma zpsym("memsize")
extern const struct line* txtptr;       /* points to BASIC code */
#pragma zpsym("txtptr")
extern char basbuf[];                   /* BASIC's input buffer */
extern void basbuf_len[];
#pragma zpsym("basbuf_len")


int __fastcall__
exec(const char *progname, const char *cmdline)
{
  static int fd;
  static unsigned char dv, n;

  /* Exclude devices that can't load files. */
  /* (Use hand optimization, to make smaller code.) */
  dv = getcurrentdevice();
  if(dv < 8 && __AX__ != 1 || __AX__ > 30) {
    return _mappederrno(9);             /* illegal device number */
  }
  utoa(dv, basic.unit, 10);

  /* Tape files can be openned only once; skip this test for the Datasette. */
  if(dv != 1) {
    /* Don't try to run a program that can't be found. */
    fd = cfs_open(progname, CFS_READ);
    if(fd < 0) {
      return -1;
    }
    cfs_close(fd);
  }

  n = 0;
  do {
    if((basic.name[n] = progname[n]) == '\0') {
      break;
    }
  } while(++n < 20);                    /* truncate long names */
  basic.name[n] = '\"';

/* This next part isn't needed by machines that put
** BASIC source and variables in different RAM banks.
*/
#if !defined(__C128__)
  /* cc65 program loads might extend beyond the end of the RAM that is allowed
  ** for BASIC.  Then, the LOAD statement would complain that it is "out of
  ** memory".  Some pointers that say where to put BASIC program variables
  ** must be changed, so that we do not get that error.  One pointer is
  ** changed here; a BASIC CLR statement changes the others.  Some space is
  ** needed for the file-name string.  Subtracting an entire RAM page allows
  ** better optimization of this expression.
  */
  vartab = (char*)memsize - 0x0100;
#endif

  /* Build the next program's argument list. */
  basbuf[0] = 0x8F;                     /* REM token */
  basbuf[1] = '\0';
  if(cmdline != NULL) {
    strncat(basbuf, cmdline, (size_t)basbuf_len - 2);
  }

  /* Tell the ROM where to find that BASIC program. */
  txtptr = &basic;

  /* (The return code, in ST [status], will be destroyed by LOAD.
  ** So, don't bother to set it here.)
  */
  exit(__AX__);
}

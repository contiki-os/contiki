/*-
 * Copyright (c) 1983, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* This file implements a subset of the profiling support functions.
   It has been copied and adapted from mcount.c, gmon.c and gmon.h in
   the glibc sources.
   Since we do not have access to a timer interrupt in the simulator
   the histogram and basic block information is not generated.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* Fraction of text space to allocate for histogram counters here, 1/2.  */
#define	HISTFRACTION	2

/* Fraction of text space to allocate for from hash buckets.
   The value of HASHFRACTION is based on the minimum number of bytes
   of separation between two subroutine call points in the object code.
   Given MIN_SUBR_SEPARATION bytes of separation the value of
   HASHFRACTION is calculated as:
  
  	HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof (short) - 1);
  
   For example, on the VAX, the shortest two call sequence is:
  
  	calls	$0,(r0)
  	calls	$0,(r0)
  
   which is separated by only three bytes, thus HASHFRACTION is
   calculated as:
  
  	HASHFRACTION = 3 / (2 * 2 - 1) = 1
  
   Note that the division above rounds down, thus if MIN_SUBR_FRACTION
   is less than three, this algorithm will not work!
  
   In practice, however, call instructions are rarely at a minimal
   distance.  Hence, we will define HASHFRACTION to be 2 across all
   architectures.  This saves a reasonable amount of space for
   profiling data structures without (in practice) sacrificing
   any granularity.  */
#define	HASHFRACTION	2

/* Percent of text space to allocate for tostructs.
   This is a heuristic; we will fail with a warning when profiling
   programs with a very large number of very small functions, but
   that's normally OK.
   2 is probably still a good value for normal programs.
   Profiling a test case with 64000 small functions will work if
   you raise this value to 3 and link statically (which bloats the
   text size, thus raising the number of arcs expected by the heuristic).  */
#define ARCDENSITY	3

/* Always allocate at least this many tostructs.  This hides the
   inadequacy of the ARCDENSITY heuristic, at least for small programs.  */
#define MINARCS		50

/* Maximum number of arcs we want to allow.
   Used to be max representable value of ARCINDEX minus 2, but now 
   that ARCINDEX is a long, that's too large; we don't really want 
   to allow a 48 gigabyte table.
   The old value of 1<<16 wasn't high enough in practice for large C++
   programs; will 1<<20 be adequate for long?  FIXME  */
#define MAXARCS		(1L << 20)

#define	SCALE_1_TO_1	0x10000L

#define	GMON_MAGIC	"gmon"	/* Magic cookie.  */
#define GMON_VERSION	1	/* Version number.  */


/* General rounding functions.  */
#define ROUNDDOWN(x ,y)	(((x) / (y)) * (y))
#define ROUNDUP(x, y)	((((x) + (y) - 1) / (y)) * (y))

struct tostruct
{
  unsigned long selfpc;
  unsigned long	count;
  unsigned long	link;
};

/* Possible states of profiling.  */
enum profiling_state
{
  GMON_PROF_OFF,
  GMON_PROF_ON,
  GMON_PROF_BUSY,
  GMON_PROF_ERROR
};

/* The profiling data structures are housed in this structure.  */
struct gmonparam
{
  enum profiling_state state;
  unsigned short *     kcount;
  unsigned long	       kcountsize;
  unsigned long	*      froms;
  unsigned long	       fromssize;
  struct tostruct *    tos;
  unsigned long	       tossize;
  long		       tolimit;
  unsigned long	       lowpc;
  unsigned long	       highpc;
  unsigned long	       textsize;
  unsigned long	       hashfraction;
  long		       log_hashfraction;
};

/* Raw header as it appears in the gmon.out file (without padding).
   This header always comes first and is then followed by a series
   records defined below.  */
struct gmon_hdr
{
  char cookie[4];
  char version[4];
  char spare[3 * 4];
};

/* Types of records in this file.  */
typedef enum
{
  GMON_TAG_TIME_HIST = 0,
  GMON_TAG_CG_ARC = 1,
} GMON_Record_Tag;

struct gmon_cg_arc_record
{
  char tag;                             /* Set to GMON_TAG_CG_ARC.  */
  char from_pc[sizeof (char *)];	/* Address within caller's body.  */
  char self_pc[sizeof (char *)];	/* Address within callee's body.  */
  char count[4];			/* Number of arc traversals.  */
};


/* Forward declarations.  */
void _mcount_internal (unsigned long);
void _monstartup (unsigned long, unsigned long);
void _mcleanup (void);

static struct gmonparam _gmonparam;

void
_mcount_internal (unsigned long frompc)
{
  unsigned long      selfpc = frompc;
  unsigned long *    frompcindex;
  struct tostruct *  top;
  struct tostruct *  prevtop;
  struct gmonparam * p;
  unsigned long      toindex;
  int                i;

  p = & _gmonparam;

  /* Check that we are profiling and that we aren't recursively invoked.
     NB/ This version is not thread-safe.  */
  if (p->state != GMON_PROF_ON)
    return;
  p->state = GMON_PROF_BUSY;

  /* Check that frompcindex is a reasonable pc value.
     For example: signal catchers get called from the stack,
     not from text space.  Too bad.  */
  frompc -= p->lowpc;
  if (frompc > p->textsize)
    goto done;

  i = frompc >> p->log_hashfraction;

  frompcindex = p->froms + i;
  toindex = * frompcindex;

  if (toindex == 0)
    {
      /* First time traversing this arc.  */
      toindex = ++ p->tos[0].link;
      if (toindex >= p->tolimit)
	/* Halt further profiling.  */
	goto overflow;

      * frompcindex = toindex;
      top = p->tos + toindex;
      top->selfpc = selfpc;
      top->count = 1;
      top->link = 0;
      goto done;
    }

  top = p->tos + toindex;

  if (top->selfpc == selfpc)
    {
      /* Arc at front of chain: usual case.  */
      top->count ++;
      goto done;
    }

  /* Have to go looking down chain for it.
     Top points to what we are looking at,
     prevtop points to previous top.
     We know it is not at the head of the chain.  */
  for (;;)
    {
      if (top->link == 0)
	{
	  /* Top is end of the chain and none of the chain
	     had top->selfpc == selfpc.  So we allocate a
	     new tostruct and link it to the head of the
	     chain.  */
	  toindex = ++ p->tos[0].link;
	  if (toindex >= p->tolimit)
	    goto overflow;

	  top = p->tos + toindex;
	  top->selfpc = selfpc;
	  top->count = 1;
	  top->link = * frompcindex;
	  * frompcindex = toindex;
	  goto done;
	}

      /* Otherwise, check the next arc on the chain.  */
      prevtop = top;
      top = p->tos + top->link;

      if (top->selfpc == selfpc)
	{
	  /* There it is.  Increment its count
	     move it to the head of the chain.  */
	  top->count ++;
	  toindex = prevtop->link;
	  prevtop->link = top->link;
	  top->link = * frompcindex;
	  * frompcindex = toindex;
	  goto done;
	}
    }

 done:
  p->state = GMON_PROF_ON;
  return;

 overflow:
  p->state = GMON_PROF_ERROR;
  return;
}

void
_monstartup (unsigned long lowpc, unsigned long highpc)
{
  char * cp;
  struct gmonparam * p = & _gmonparam;

  /* If the calloc() function has been instrumented we must make sure
     that it is not profiled until we are ready.  */
  p->state = GMON_PROF_BUSY;

  /* Round lowpc and highpc to multiples of the density we're using
     so the rest of the scaling (here and in gprof) stays in ints.  */
  p->lowpc            = ROUNDDOWN (lowpc, HISTFRACTION * sizeof (* p->kcount));
  p->highpc           = ROUNDUP (highpc, HISTFRACTION * sizeof (* p->kcount));
  p->textsize         = p->highpc - p->lowpc;
  p->kcountsize       = ROUNDUP (p->textsize / HISTFRACTION, sizeof (*p->froms));
  p->hashfraction     = HASHFRACTION;
  p->log_hashfraction = -1;
  p->log_hashfraction = ffs (p->hashfraction * sizeof (*p->froms)) - 1;
  p->fromssize        = p->textsize / HASHFRACTION;
  p->tolimit          = p->textsize * ARCDENSITY / 100;

  if (p->tolimit < MINARCS)
    p->tolimit = MINARCS;
  else if (p->tolimit > MAXARCS)
    p->tolimit = MAXARCS;
  
  p->tossize          = p->tolimit * sizeof (struct tostruct);

  cp = calloc (p->kcountsize + p->fromssize + p->tossize, 1);
  if (cp == NULL)
    {
      write (2, "monstartup: out of memory\n", 26);
      p->tos = NULL;
      p->state = GMON_PROF_ERROR;
      return;
    }

  p->tos = (struct tostruct *) cp;
  cp += p->tossize;
  p->kcount = (unsigned short *) cp;
  cp += p->kcountsize;
  p->froms = (unsigned long *) cp;

  p->tos[0].link = 0;
  p->state = GMON_PROF_ON;
}


static void
write_call_graph (int fd)
{
#define NARCS_PER_WRITE	32

  struct gmon_cg_arc_record raw_arc[NARCS_PER_WRITE]
    __attribute__ ((aligned (__alignof__ (char *))));
  unsigned long from_index;
  unsigned long to_index;
  unsigned long from_len;
  unsigned long frompc;
  int nfilled;

  for (nfilled = 0; nfilled < NARCS_PER_WRITE; ++ nfilled)
    raw_arc[nfilled].tag = GMON_TAG_CG_ARC;

  nfilled = 0;
  from_len = _gmonparam.fromssize / sizeof (*_gmonparam.froms);

  for (from_index = 0; from_index < from_len; ++from_index)
    {
      if (_gmonparam.froms[from_index] == 0)
	continue;

      frompc = _gmonparam.lowpc;
      frompc += (from_index * _gmonparam.hashfraction
		 * sizeof (*_gmonparam.froms));

      for (to_index = _gmonparam.froms[from_index];
	   to_index != 0;
	   to_index = _gmonparam.tos[to_index].link)
	{
	  struct gmon_cg_arc_record * arc = raw_arc + nfilled;

	  memcpy (arc->from_pc, & frompc, sizeof (arc->from_pc));
	  memcpy (arc->self_pc, & _gmonparam.tos[to_index].selfpc, sizeof (arc->self_pc));
	  memcpy (arc->count,   & _gmonparam.tos[to_index].count, sizeof (arc->count));

	  if (++ nfilled == NARCS_PER_WRITE)
	    {
	      write (fd, raw_arc, sizeof raw_arc);
	      nfilled = 0;
	    }
	}
    }

  if (nfilled > 0)
    write (fd, raw_arc, nfilled * sizeof (raw_arc[0]));
}

#include <errno.h>

static void
write_gmon (void)
{
  struct gmon_hdr ghdr __attribute__ ((aligned (__alignof__ (int))));
  int fd;

  fd = open ("gmon.out", O_CREAT|O_TRUNC|O_WRONLY, 0666);
  if (fd < 0)
    {
      write (2, "_mcleanup: could not create gmon.out\n", 37);
      return;
    }

  /* Write gmon.out header: */
  memset (& ghdr, '\0', sizeof (ghdr));
  memcpy (ghdr.cookie, GMON_MAGIC, sizeof (ghdr.cookie));
  * (unsigned long *) ghdr.version = GMON_VERSION;
  write (fd, & ghdr, sizeof (ghdr));

  /* We do not have histogram or basic block information,
     so we do not generate these parts of the gmon.out file.  */
  
  /* Write call-graph.  */
  write_call_graph (fd);

  close (fd);
}

void
_mcleanup (void)
{
  if (_gmonparam.state != GMON_PROF_ERROR)
    {
      _gmonparam.state = GMON_PROF_OFF;
      write_gmon ();
    }

  /* Free the memory.  */
  if (_gmonparam.tos != NULL)
    {
      free (_gmonparam.tos);
      _gmonparam.tos = NULL;
    }
}

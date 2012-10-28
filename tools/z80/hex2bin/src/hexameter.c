/*
 * Copyright (c) 2003-2009, Takahide Matsutsuka.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */

/*
 * A main file for hexameter.
 */

#define MAX_PATH 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "ihx2bin.h"

#define MAXFILES 256

typedef struct {
  char* output;
  char* dir;
  char *files[MAXFILES];
  unsigned int defsize;
  struct ConvertDefinition defs[DEF_MAX];
  unsigned char verbose;
  // number of the ihx files
  int length;
  // output file size
  int size;
  // output file wriiten size
  int written;
} Configuration;

static
const char* IHXEXT = ".ihx";

static
const char *strcasestr(const char *haystack, const char *needle) {
  int haypos;
  int needlepos;

  haypos = 0;
  while (haystack[haypos]) {
    if (tolower (haystack[haypos]) == tolower(needle[0])) {
      needlepos = 1;
      while ( (needle[needlepos]) &&
              (tolower(haystack[haypos + needlepos])
               == tolower(needle[needlepos])) )
          ++needlepos;
      if (! needle[needlepos]) return (haystack + haypos);
    }
    ++haypos;
  }
  return NULL;
}

static
char *changeExt(const char *path, const char* ext) {
  char *p;
  char *tail;
  char *changed;
  int len;
  int extlen = strlen(ext);

  for (tail = (char*) path; *tail != 0; tail++);
  for (p = tail; p > path; p--) {
    if (*p == '.') {
      len = p - path;
      changed = (char*) malloc(len + extlen + 2);

      strncpy(changed, path, len + 1);
      strcpy(changed + len + 1, ext);
      return changed;
    }
  }

  len = strlen(path);
  changed = (char*) malloc(len + extlen + 2);
  strncpy(changed, path, len);
  *(changed + len) = '.';
  strcpy(changed + len + 1, ext);
  return changed;
}

static
unsigned char analyzeOption(int argc, char **argv, Configuration *config) {
  int c;
  char *defval;

  opterr = 0;
  while ((c = getopt(argc, argv, "hvo:d:b:")) != EOF) {
    switch (c) {
    case 'v':
      config->verbose = 1;
      break;
    case 'o':
      config->output = optarg;
      break;
    case 'b':
      sscanf(optarg, "%x", &config->size);
      break;
    case 'd':
      if (config->defsize >= DEF_MAX) {
	printf("excess number of definitions\n");
	return 1;
      }
      defval = strchr(optarg, '=');
      if (defval) {
	*defval = 0;
	config->defs[config->defsize].name = optarg;
	config->defs[config->defsize].value = defval + 1;
	config->defsize++;
      } else {
	printf("definition value required:%s\n", optarg);
	return 1;
      }

      break;
    case 'h':
      printf("Hexameter: Convert Intel HEX file (ihx) to binary file, ver. 2.1.3\n");
      printf("Copyright (c) 2003-2009 Takahide Matsutsuka <markn@markn.org>\n");
      printf("Usage: hexameter [options] <ihx|bin> [<ihx|bin>...]\n");
      printf("Options:\n");
      printf(" -v verbose output\n");
      printf(" -o <output file name>\n");
      printf(" -d <name>=<value> define property\n");
      printf(" -b <output file size in hexadecimal bytes>\n");
      printf(" -h show this help\n");
    
      return 1;
    default:
      printf("unknown option:%c\n", optopt);
      return 1;
    }
  }

  return 0;
}

#if 0
static
int isFileExists(const char *dir, const char *filename) {
  char path[MAX_PATH];
  FILE *f;

  if (!filename) {
    return 0;
  }
  if (dir) {
    strcpy(path, dir);
    strcpy(path + strlen(path), filename);
  } else {
    strcpy(path, filename);
  }
  if (!(f = fopen(path, "rb"))) {
    printf("cannot open: %s\n", filename);
    return 1;
  }
  fclose(f);

  return 0;
}
#endif

/**
 * @return 1 if given filename has an extension of ".ihx"
 */
static
int isIhx(const char *filename) {
  const char* pos = strcasestr(filename, IHXEXT);
  if (pos && pos[strlen(IHXEXT)] == 0) {
    return 1;
  }
  return 0;
}

static
int checkExistence(Configuration *config) {
  int i;
  FILE *f;

  for (i = 0; i < config->length; i++) {
    f = fopen(config->files[i], "r");
    if (!f) {
      printf("cannot open: %s\n", config->files[i]);
      return 1;
    }
    fclose(f);
  }
  return 0;
}

/**
 * @return writtn size in bytes
 */
static
int copy(FILE *out, const char* dir, const char* filename, unsigned char verbose) {
  FILE *in;
  int ch;
  char path[MAX_PATH];
  int bytes = 0;
  if (!filename) {
    return 0;
  }
  if (dir) {
    strcpy(path, dir);
    strcpy(path + strlen(path), filename);
  } else {
    strcpy(path, filename);
  }
  in = fopen(path, "rb");
  while ((ch = getc(in)) != EOF) {
    putc(ch, out);
    bytes++;
  }
  fclose(in);
  if (verbose) {
    printf("imported file: %s, size=%d\n", path, bytes);
  }
  return bytes;
}

static
int output(Configuration *config) {
  FILE *out;
  int i;

  if (!(out = fopen(config->output, "wb"))) {
    printf("cannot open output file:%s\n", config->output);
    return 1;
  }

  for (i = 0; i < config->length; i++) {
    struct ConvertInfo info;
    info.out = out;
    info.filename = config->files[i];
    info.verbose = config->verbose;
    info.defsize = config->defsize;
    info.defs = config->defs;

    if (isIhx(config->files[i])) {
      config->written += ihx2bin(&info);
    } else {
      config->written += copy(out, NULL, config->files[i], config->verbose);
    }
  }

  if (config->size) {
    if (config->verbose) {
      printf("Writing trailing bytes\n");
    }
    for (; config->size > config->written; config->written++) {
      putc(0, out);
    }
  }

  fclose(out);
  return 0;
}


int main(int argc, char **argv) {
  Configuration config;
  unsigned char r;

  memset(&config, 0, sizeof(Configuration));

#ifndef __CYGWIN__
  r = analyzeOption(argc, argv, &config);
  if (r) {
    return r;
  }
#endif
  while (optind < argc) {
#ifdef __CYGWIN__
    r = analyzeOption(argc, argv, &config);
    if (r) {
      return r;
    }
#endif
    if (config.length >= MAXFILES) {
      printf("too much files specified\n");
      return 1;
    }
    config.files[config.length] = argv[optind];
    config.length++;
    optind++;
  }

  if (config.length == 0) {
    printf("no input specified\n");
    return 1;
  }

  if (config.output == NULL) {
    config.output = changeExt(config.files[0], "bin");
  }

  r = checkExistence(&config);
  if (r) {
    return r;
  }
  if (config.verbose) {
    printf("Generating file: %s\n", config.output);
  }
  r = output(&config);
  if (r) {
    return r;
  }

  if (config.verbose) {
    printf("Done.\n");
  }
  return 0;
}

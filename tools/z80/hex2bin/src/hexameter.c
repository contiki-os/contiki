/*
 * Copyright (c) 2003-2008, Takahide Matsutsuka.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. The end-user documentation included with the redistribution, if
 *    any, must include the following acknowlegement:
 *       "This product includes software developed by Takahide Matsutsuka."
 *    Alternately, this acknowlegement may appear in the software itself,
 *    if and wherever such third-party acknowlegements normally appear.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * A main file for hex2cas.
 */


#if __CYGWIN32__
#include <windows.h>
#else
#include <stdlib.h>
#endif
#include <stdio.h>
#include <unistd.h>
#ifndef TRUE
#define TRUE 1
#endif

#include "ihx2bin.h"

#define MAXFILES 256

struct Configuration {
  char* output;
  char* prefix;
  char* suffix;
  char* dir;
  char archname[6];
  char *files[MAXFILES];
  unsigned char verbose;
  // number of the ihx files
  int length;
  // output file size
  int size;
  // output file wriiten size
  int written;
};

static char *changeExt(const char *path, const char* ext) {
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

static unsigned char analyzeOption(int argc, char **argv, struct Configuration *config) {
  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "hvo:p:s:n:b:")) != EOF) {
    int i;
    switch (c) {
    case 'v':
      config->verbose = TRUE;
      break;
    case 'o':
      config->output = optarg;
      break;
    case 'p':
      config->prefix = optarg;
      break;
    case 's':
      config->suffix = optarg;
      break;
    case 'b':
      sscanf(optarg, "%x", &config->size);
      break;
//     case 'n':
//       for (i = 0; i < 6; i++) {
//         if (optarg[i] == 0) {
//           break;
//       	}
//       	config->archname[i] = optarg[i];
//       }
//       break;
    case 'h':
      printf("%s : Convert Intel HEX file (ihx) to binary file, ver. 2.0.0\n", getprogname());
      printf("Copyright (c) 2003-2008 Takahide Matsutsuka <markn@markn.org>\n");
      printf("Usage: %s [options] <ihx> [<ihx>...]\n", argv[0]);
      printf("Options:\n");
      printf(" -v verbose output\n");
      printf(" -o <output file name>\n");
//       printf(" -n <cassette file name> (for NEC PC series)\n");
      printf(" -p <prefix file>\n");
      printf(" -s <suffix file>\n");
      printf(" -b <output file size in hexadecimal bytes>\n");
      printf(" -h print this help\n");
    
      return 1;
    default:
      printf("unknown option:%c\n", optopt);
      return 1;
    }
  }

  return 0;
}

static int isFileExists(const char *dir, const char *filename) {
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


static int checkExistence(struct Configuration *config) {
  int i;
  int r;
  FILE *f;
  fclose(f);
  if (r = isFileExists(config->dir, config->prefix)) {
    return r;
  }
  if (r = isFileExists(config->dir, config->suffix)) {
    return r;
  }
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
  char ch;
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
  if (verbose) {
    printf("importing file: %s\n", path);
  }
  in = fopen(path, "rb");
  while ((ch = getc(in)) != EOF) {
    putc(ch, out);
    bytes++;
  }
  return bytes;
}

static int output(struct Configuration *config) {
  FILE *out;
  int i;
  // TODO: ARCH FILE NAME

  if (!(out = fopen(config->output, "wb"))) {
    printf("cannot open output file:%s\n", config->output);
    return 1;
  }
  config->written += copy(out, config->dir, config->prefix, config->verbose);
  for (i = 0; i < config->length; i++) {
    config->written += ihx2bin(out, config->files[i], config->verbose);
  }
  config->written += copy(out, config->dir, config->suffix, config->verbose);

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
  struct Configuration config;
  unsigned char r;

  memset(&config, 0, sizeof(struct Configuration));

#if __CYGWIN32__
   char path[MAX_PATH];
   GetModuleFileName(NULL, path, MAX_PATH);
   int len = strlen(path);
   while (len > 0) {
     if (path[len] == '\\') {
       path[len + 1] = 0;
       break;
     }
     len--;
   }
   config.dir = path;
#endif

  while (optind < argc) {
    if (r = analyzeOption(argc, argv, &config)) {
      return r;
    }
    if (optind == argc) {
      break;
    }
    if (config.length == MAXFILES) {
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

  if (r = checkExistence(&config)) {
    return r;
  }
  if (config.verbose) {
    printf("Generating file: %s\n", config.output);
  }
  if (r = output(&config)) {
    return r;
  }

  if (config.verbose) {
    printf("Done.\n");
  }
  return 0;
}

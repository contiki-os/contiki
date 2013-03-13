/*
 * Copyright (c) 2002, Adam Dunkels.
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
 * This file is part of the Contiki desktop environment 
 *
 *
 */

/* htmlparser.c:
 *
 * Implements a very simplistic HTML parser. It recognizes HTML links
 * (<a href>-tags), HTML img alt tags, a few text flow break tags
G * (<br>, <p>, <h>), the <li> tag (but does not even try to
 * distinguish between <ol> or <ul>) as well as HTML comment tags
 * (<!-- -->).
 *
 * To save memory, the HTML parser is state machine driver, which
 * means that it will shave off one character from the HTML page,
 * process that character, and return to the next. Another way of
 * doing it would be to buffer a number of characters and process them
 * together.
 *
 * The main function in this file is the htmlparser_parse() function
 * which takes a htmlparser_state structur and a part of an HTML file
 * as an argument. The htmlparser_parse() function will call the
 * helper functions parse_char() and parse_tag(). Those functions will
 * in turn call the two callback functions htmlparser_char() and
 * htmlparser_tag(). Those functions must be implemented by the using
 * module (e.g., a web browser program).
 *
 * htmlparser_char() will be called for every non-tag character.
 *
 * htmlparser_tag() will be called whenever a full tag has been found.
 *
 */

#include <string.h>

#include "contiki.h"
#include "html-strings.h"
#include "www.h"

#include "htmlparser.h"

#if 1
#define PRINTF(x)
#else
#include <stdio.h>
#define PRINTF(x) printf x
#endif


/*-----------------------------------------------------------------------------------*/
#define ISO_A     0x41
#define ISO_B     0x42
#define ISO_E     0x45
#define ISO_F     0x46
#define ISO_G     0x47
#define ISO_H     0x48
#define ISO_I     0x49
#define ISO_L     0x4c
#define ISO_M     0x4d
#define ISO_P     0x50
#define ISO_R     0x52
#define ISO_T     0x54

#define ISO_a     (ISO_A | 0x20)
#define ISO_b     (ISO_B | 0x20)
#define ISO_e     (ISO_E | 0x20)
#define ISO_f     (ISO_F | 0x20)
#define ISO_g     (ISO_G | 0x20)
#define ISO_h     (ISO_H | 0x20)
#define ISO_i     (ISO_I | 0x20)
#define ISO_l     (ISO_L | 0x20)
#define ISO_m     (ISO_M | 0x20)
#define ISO_p     (ISO_P | 0x20)
#define ISO_r     (ISO_R | 0x20)
#define ISO_t     (ISO_T | 0x20)

#define ISO_ht    0x09
#define ISO_nl    0x0a
#define ISO_cr    0x0d
#define ISO_space 0x20
#define ISO_bang  0x21
#define ISO_citation 0x22
#define ISO_ampersand 0x26
#define ISO_citation2 0x27
#define ISO_asterisk 0x2a
#define ISO_dash  0x2d
#define ISO_slash 0x2f
#define ISO_semicolon  0x3b
#define ISO_lt    0x3c
#define ISO_eq    0x3d
#define ISO_gt    0x3e

#define ISO_rbrack 0x5b
#define ISO_lbrack 0x5d

#define MINORSTATE_NONE           0
#define MINORSTATE_TEXT           1 /* Parse normal text */
#define MINORSTATE_EXTCHAR        2 /* Check for semi-colon */
#define MINORSTATE_TAG            3 /* Check for name of tag. */
#define MINORSTATE_TAGEND         4 /* Scan for end of tag. */
#define MINORSTATE_TAGATTR        5 /* Parse tag attr. */
#define MINORSTATE_TAGATTRSPACE   6 /* Parse optional space after tag
				       attr. */
#define MINORSTATE_TAGATTRPARAM   7 /* Parse tag attr parameter. */
#define MINORSTATE_TAGATTRPARAMNQ 8 /* Parse tag attr parameter without
				  quotation marks. */
#define MINORSTATE_HTMLCOMMENT    9 /* Scan for HTML comment end */

#define MAJORSTATE_NONE       0
#define MAJORSTATE_BODY       1
#define MAJORSTATE_LINK       2
#define MAJORSTATE_FORM       3
#define MAJORSTATE_DISCARD    4


struct htmlparser_state {

  unsigned char minorstate;
  char tag[20];
  unsigned char tagptr;
  char tagattr[20];
  unsigned char tagattrptr;
  char tagattrparam[WWW_CONF_MAX_URLLEN + 1];
  unsigned char tagattrparamptr;
  unsigned char lastchar, quotechar;
  unsigned char majorstate, lastmajorstate;
  char linkurl[WWW_CONF_MAX_URLLEN + 1];

  char word[WWW_CONF_WEBPAGE_WIDTH];
  unsigned char wordlen;

#if WWW_CONF_FORMS
  char formaction[WWW_CONF_MAX_FORMACTIONLEN + 1];
  unsigned char inputtype;
  char inputname[WWW_CONF_MAX_INPUTNAMELEN + 1];
  char inputvalue[WWW_CONF_MAX_INPUTVALUELEN + 1];
  unsigned char inputvaluesize;
#endif /* WWW_CONF_FORMS */
};

static struct htmlparser_state s;

/*-----------------------------------------------------------------------------------*/
static char last[1] = {(char)0xff};

static const char *tags[] = {
#define TAG_FIRST       0
#define TAG_SLASHA      0
  html_slasha,
#define TAG_SLASHDIV    1
  html_slashdiv,
#define TAG_SLASHFORM   2
  html_slashform,
#define TAG_SLASHH      3
  html_slashh,
#define TAG_SLASHSCRIPT 4
  html_slashscript,
#define TAG_SLASHSELECT 5
  html_slashselect,
#define TAG_SLASHSTYLE  6
  html_slashstyle,
#define TAG_A           7
  html_a,
#define TAG_BODY        8
  html_body,
#define TAG_BR          9
  html_br,
#define TAG_FORM       10
  html_form,
#define TAG_FRAME      11
  html_frame,
#define TAG_H1         12
  html_h1,
#define TAG_H2         13
  html_h2,
#define TAG_H3         14
  html_h3,
#define TAG_H4         15
  html_h4,
#define TAG_IMG        16
  html_img,
#define TAG_INPUT      17
  html_input,
#define TAG_LI         18
  html_li,
#define TAG_P          19
  html_p,
#define TAG_SCRIPT     20
  html_script, 
#define TAG_SELECT     21
  html_select,
#define TAG_STYLE      22
  html_style,
#define TAG_TR         23
  html_tr,
#define TAG_LAST       24
  last,
};

/*-----------------------------------------------------------------------------------*/
static unsigned char CC_FASTCALL
iswhitespace(char c)
{
  return (c == ISO_space ||
	  c == ISO_nl ||
	  c == ISO_cr ||
	  c == ISO_ht);
}
/*-----------------------------------------------------------------------------------*/
#if WWW_CONF_FORMS
static void
init_input(void)
{
  s.inputtype = HTMLPARSER_INPUTTYPE_NONE;
  s.inputname[0] = s.inputvalue[0] =
  s.formaction[WWW_CONF_MAX_FORMACTIONLEN] =
  s.inputname[WWW_CONF_MAX_INPUTNAMELEN] =
  s.inputvalue[WWW_CONF_MAX_INPUTVALUELEN] = 0;
  s.inputvaluesize = 20; /* De facto default size */
}
#endif /* WWW_CONF_FORMS */
/*-----------------------------------------------------------------------------------*/
void
htmlparser_init(void)
{
  s.majorstate = s.lastmajorstate = MAJORSTATE_DISCARD;
  s.minorstate = MINORSTATE_TEXT;
  s.lastchar = 0;
#if WWW_CONF_FORMS
  s.formaction[0] = 0;
#endif /* WWW_CONF_FORMS */
}
/*-----------------------------------------------------------------------------------*/
static char CC_FASTCALL
lowercase(char c)
{
  /* XXX: This is a *brute force* approach to lower-case
     converting and should *not* be used anywhere else! It
     works for our purposes, however (i.e., HTML tags). */
  if(c > 0x40) {
    return (c & 0x1f) | 0x60;
  } else {
    return c;
  }
}
/*-----------------------------------------------------------------------------------*/
static void 
endtagfound(void)
{
  s.tag[s.tagptr] = 0;
  s.tagattr[s.tagattrptr] = 0;
  s.tagattrparam[s.tagattrparamptr] = 0;
}
/*-----------------------------------------------------------------------------------*/
static void CC_FASTCALL
switch_majorstate(unsigned char newstate)
{
  if(s.majorstate != newstate) {
    PRINTF(("Switching state from %d to %d (%d)\n", s.majorstate, newstate, s.lastmajorstate));
    s.lastmajorstate = s.majorstate;
    s.majorstate = newstate;
  }
}
/*-----------------------------------------------------------------------------------*/
static void CC_FASTCALL
add_char(unsigned char c)
{
  if(s.wordlen < WWW_CONF_WEBPAGE_WIDTH - 1 && c < 0x80) {
    s.word[s.wordlen] = c;
    ++s.wordlen;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
do_word(void)
{
  if(s.wordlen > 0) {
    if(s.majorstate == MAJORSTATE_LINK) {
      if(s.word[s.wordlen] != ISO_space) {
	add_char(ISO_space);
      }
    } else if(s.majorstate == MAJORSTATE_DISCARD) {
      s.wordlen = 0;
    } else {
      s.word[s.wordlen] = '\0';
      htmlparser_word(s.word, s.wordlen);
      s.wordlen = 0;
    }
  }
}
/*-----------------------------------------------------------------------------------*/
static void
newline(void)
{
  do_word();
  htmlparser_newline();
}
/*-----------------------------------------------------------------------------------*/
static unsigned char CC_FASTCALL
find_tag(char *tag)
{
  static unsigned char first, last, i, tabi;
  static char tagc;
  
  first = TAG_FIRST;
  last = TAG_LAST;
  i = 0;

  do {
    tagc = tag[i];

    if((tagc == 0 || tagc == ISO_slash) && tags[first][i] == 0) {
      return first;
    }

    tabi = first;

    /* First, find first matching tag from table. */
    while(tagc > (tags[tabi])[i] && tabi < last) {
      ++tabi;
    }
    first = tabi;

    /* Second, find last matching tag from table. */
    while(tagc == (tags[tabi])[i] && tabi < last) {
      ++tabi;
    }
    last = tabi;

    /* If first and last matching tags are equal, we have a non-match
       and return. Else we continue with the next character. */
    ++i;

  } while(last != first);
  return TAG_LAST;
}
/*-----------------------------------------------------------------------------------*/
static void
parse_tag(void)
{
  static char *tagattrparam;
  static unsigned char size;

  static char dummy;
  
  PRINTF(("Parsing tag '%s' '%s' '%s'\n", s.tag, s.tagattr, s.tagattrparam));

  switch(find_tag(s.tag)) {
  case TAG_P:
  case TAG_H1:
  case TAG_H2:
  case TAG_H3:
  case TAG_H4:
    newline();
    /* FALLTHROUGH */
  case TAG_BR:
  case TAG_TR:
  case TAG_SLASHDIV:
  case TAG_SLASHH:
    dummy = 0;
    newline();
    break;
  case TAG_LI:
    newline();
    add_char(ISO_asterisk);
    add_char(ISO_space);
    break;
  case TAG_SCRIPT:
  case TAG_STYLE:
  case TAG_SELECT:
    switch_majorstate(MAJORSTATE_DISCARD);
    break;
  case TAG_SLASHSCRIPT:
  case TAG_SLASHSTYLE:
  case TAG_SLASHSELECT:
    do_word();
    switch_majorstate(s.lastmajorstate);
    break;
  case TAG_BODY:
    s.majorstate = s.lastmajorstate = MAJORSTATE_BODY;
    break;
  case TAG_FRAME:
    if(strncmp(s.tagattr, html_src, sizeof(html_src)) == 0 && s.tagattrparam[0] != 0) {
      switch_majorstate(MAJORSTATE_BODY);
      newline();
      add_char(ISO_rbrack);
      do_word();
      htmlparser_link((char *)html_frame, (unsigned char)strlen(html_frame), s.tagattrparam);
      PRINTF(("Frame [%s]\n", s.tagattrparam));
      add_char(ISO_lbrack);
      newline();
    }
    break;
  case TAG_IMG:
    if(strncmp(s.tagattr, html_alt, sizeof(html_alt)) == 0 && s.tagattrparam[0] != 0) {
      add_char(ISO_lt);
      tagattrparam = &s.tagattrparam[0];
      while(*tagattrparam) {
	add_char(*tagattrparam);
	++tagattrparam;
      }
      add_char(ISO_gt);
      do_word();
    }
    break;
  case TAG_A:
    PRINTF(("A %s %s\n", s.tagattr, s.tagattrparam));
    if(strncmp(s.tagattr, html_href, sizeof(html_href)) == 0 && s.tagattrparam[0] != 0) {
      strcpy(s.linkurl, s.tagattrparam);
      do_word();
      switch_majorstate(MAJORSTATE_LINK);
    }
    break;
  case TAG_SLASHA:
    if(s.majorstate == MAJORSTATE_LINK) {
      switch_majorstate(s.lastmajorstate);
      s.word[s.wordlen] = 0;
      htmlparser_link(s.word, s.wordlen, s.linkurl);
      s.wordlen = 0;
    }
    break;
#if WWW_CONF_FORMS
  case TAG_FORM:
    /* First check if we are called at the end of a form tag. If
       so, we should propagate the form action. */
    if(s.tagattr[0] == 0 && s.formaction[0] != 0) {
      htmlparser_form(s.formaction);
      init_input();
    } else {
      PRINTF(("Form tag\n"));
      switch_majorstate(MAJORSTATE_FORM);
      if(strncmp(s.tagattr, html_action, sizeof(html_action)) == 0) {
        PRINTF(("Form action '%s'\n", s.tagattrparam));
        strncpy(s.formaction, s.tagattrparam, WWW_CONF_MAX_FORMACTIONLEN - 1);
      }
    }
    break;
  case TAG_SLASHFORM:
    switch_majorstate(MAJORSTATE_BODY);
    s.formaction[0] = 0;
    break;
  case TAG_INPUT:
    if(s.majorstate == MAJORSTATE_FORM) {
      /* First check if we are called at the end of an input tag. If
	 so, we should render the input widget. */
      if(s.tagattr[0] == 0 && s.inputname[0] != 0) {
	PRINTF(("Render input type %d\n", s.inputtype));
	switch(s.inputtype) {
	case HTMLPARSER_INPUTTYPE_NONE:
	case HTMLPARSER_INPUTTYPE_TEXT:
	case HTMLPARSER_INPUTTYPE_HIDDEN:
	  htmlparser_inputfield(s.inputtype, s.inputvaluesize, s.inputvalue, s.inputname);
	  break;
	case HTMLPARSER_INPUTTYPE_SUBMIT:
	case HTMLPARSER_INPUTTYPE_IMAGE:
	  htmlparser_submitbutton(s.inputvalue, s.inputname);
	  break;
	}
	init_input();
      } else {
	PRINTF(("Input '%s' '%s'\n", s.tagattr, s.tagattrparam));
	if(strncmp(s.tagattr, html_type, sizeof(html_type)) == 0) {
	  if(strncmp(s.tagattrparam, html_submit, sizeof(html_submit)) == 0) {
	    s.inputtype = HTMLPARSER_INPUTTYPE_SUBMIT;
	  } else if(strncmp(s.tagattrparam, html_image, sizeof(html_image)) == 0) {
	    s.inputtype = HTMLPARSER_INPUTTYPE_IMAGE;
	  } else if(strncmp(s.tagattrparam, html_text, sizeof(html_text)) == 0) {
	    s.inputtype = HTMLPARSER_INPUTTYPE_TEXT;
	  } else if(strncmp(s.tagattrparam, html_hidden, sizeof(html_hidden)) == 0) {
	    s.inputtype = HTMLPARSER_INPUTTYPE_HIDDEN;
	  } else {
	    s.inputtype = HTMLPARSER_INPUTTYPE_OTHER;
	  }
	} else if(strncmp(s.tagattr, html_name, sizeof(html_name)) == 0) {
	  strncpy(s.inputname, s.tagattrparam, WWW_CONF_MAX_INPUTNAMELEN);
	} else if(strncmp(s.tagattr, html_alt, sizeof(html_alt)) == 0 &&
		  s.inputtype == HTMLPARSER_INPUTTYPE_IMAGE) {
	  strncpy(s.inputvalue, s.tagattrparam, WWW_CONF_MAX_INPUTVALUELEN);
	} else if(strncmp(s.tagattr, html_value, sizeof(html_value)) == 0) {
	  strncpy(s.inputvalue, s.tagattrparam, WWW_CONF_MAX_INPUTVALUELEN);
	} else if(strncmp(s.tagattr, html_size, sizeof(html_size)) == 0) {
	  size = 0;
	  if(s.tagattrparam[0] >= '0' &&
	     s.tagattrparam[0] <= '9') {
	    size = s.tagattrparam[0] - '0';
	    if(s.tagattrparam[1] >= '0' &&
	       s.tagattrparam[1] <= '9') {
	      size = size * 10 + (s.tagattrparam[1] - '0');
	    }
	  }
	  if(size >= WWW_CONF_MAX_INPUTVALUELEN) {
	    size = WWW_CONF_MAX_INPUTVALUELEN - 1;
	  }
	  s.inputvaluesize = size;
	}
      }
    }
    break;
#endif /* WWW_CONF_FORMS */
  }
}
/*-----------------------------------------------------------------------------------*/
static uint16_t CC_FASTCALL
parse_word(char *data, uint8_t dlen)
{
  static uint8_t i;
  static uint8_t len;
  unsigned char c;

  len = dlen;

  switch(s.minorstate) {
  case MINORSTATE_TEXT:
    for(i = 0; i < len; ++i) {
      c = data[i];
      if(iswhitespace(c)) {
	do_word();
      } else if(c == ISO_lt) {
	s.minorstate = MINORSTATE_TAG;
	s.tagptr = 0;
	break;
      } else if(c == ISO_ampersand) {
	s.minorstate = MINORSTATE_EXTCHAR;
	break;
      } else {
	add_char(c);
      }
    }
    break;
  case MINORSTATE_EXTCHAR:
    for(i = 0; i < len; ++i) {
      c = data[i];
      if(c == ISO_semicolon) {
	s.minorstate = MINORSTATE_TEXT;
	add_char(' ');
	break;
      } else if(iswhitespace(c)) {
	s.minorstate = MINORSTATE_TEXT;
	add_char('&');
	add_char(' ');
	break;
      }
    }
    break;
  case MINORSTATE_TAG:
    /* We are currently parsing within the name of a tag. We check
       for the end of a tag (the '>' character) or whitespace (which
       indicates that we should parse a tag attr argument
       instead). */
    for(i = 0; i < len; ++i) {
      c = data[i];
      if(c == ISO_gt) {
	/* Full tag found. We continue parsing regular text. */
	s.minorstate = MINORSTATE_TEXT;
	s.tagattrptr = s.tagattrparamptr = 0;
	endtagfound();
	parse_tag();
	break;
      } else if(iswhitespace(c)) {
	/* The name of the tag found. We continue parsing the tag
	   attr.*/
	s.minorstate = MINORSTATE_TAGATTR;
	s.tagattrptr = 0;
	endtagfound();
	break;
      } else {
	/* Keep track of the name of the tag, but convert it to
	   lower case. */
	s.tag[s.tagptr] = lowercase(c);
	++s.tagptr;
	/* Check if the ->tag field is full. If so, we just eat up
	   any data left in the tag. */
	if(s.tagptr == sizeof(s.tag)) {
	  s.minorstate = MINORSTATE_TAGEND;
	  break;
	}
      }

      /* Check for HTML comment, indicated by <!-- */
      if(s.tagptr == 3 &&
	 s.tag[0] == ISO_bang &&
	 s.tag[1] == ISO_dash &&
	 s.tag[2] == ISO_dash) {
	PRINTF(("Starting comment...\n"));
	s.minorstate = MINORSTATE_HTMLCOMMENT;
	s.tagptr = 0;
	endtagfound();
	break;
      }
    }
    break;
  case MINORSTATE_TAGATTR:
    /* We parse the "tag attr", i.e., the "href" in <a
       href="...">. */
    for(i = 0; i < len; ++i) {
      c = data[i];
      if(c == ISO_gt) {
	/* Full tag found. */
	s.minorstate = MINORSTATE_TEXT;
	s.tagattrparamptr = 0;
	s.tagattrptr = 0;
	endtagfound();
	parse_tag();
	s.tagptr = 0;
	endtagfound();
	break;
      } else if(iswhitespace(c)) {
	if(s.tagattrptr == 0) {
	  /* Discard leading spaces. */
	} else {
	  /* A non-leading space is the end of the attribute. */
	  s.tagattrparamptr = 0;
	  endtagfound();
	  parse_tag();
	  s.minorstate = MINORSTATE_TAGATTRSPACE;
	  break;
	}
      } else if(c == ISO_eq) {	
	s.minorstate = MINORSTATE_TAGATTRPARAMNQ;
	s.tagattrparamptr = 0;
	endtagfound();
	break;
      } else {
	s.tagattr[s.tagattrptr] = lowercase(c);
	++s.tagattrptr;
	/* Check if the "tagattr" field is full. If so, we just eat
	   up any data left in the tag. */
	if(s.tagattrptr == sizeof(s.tagattr)) {
	  s.minorstate = MINORSTATE_TAGEND;
	  break;
	}
      }
    }
    break;
  case MINORSTATE_TAGATTRSPACE:
    for(i = 0; i < len; ++i) {
      c = data[i];
      if(iswhitespace(c)) {
	/* Discard spaces. */
      } else if(c == ISO_eq) {
	s.minorstate = MINORSTATE_TAGATTRPARAMNQ;
	s.tagattrparamptr = 0;
	endtagfound();
	parse_tag();
	break;
      } else {
	s.tagattr[0] = lowercase(c);
	s.tagattrptr = 1;
	s.minorstate = MINORSTATE_TAGATTR;
	break;
      }
    }
    break;
  case MINORSTATE_TAGATTRPARAMNQ:
    /* We are parsing the "tag attr parameter", i.e., the link part
       in <a href="link">. */
    for(i = 0; i < len; ++i) {
      c = data[i];
      if(c == ISO_gt) {
	/* Full tag found. */
	endtagfound();
	parse_tag();
	s.minorstate = MINORSTATE_TEXT;
	s.tagattrptr = 0;
	endtagfound();
	parse_tag();
	s.tagptr = 0;
	endtagfound();
	break;
      } else if(iswhitespace(c) && s.tagattrparamptr == 0) {
	/* Discard leading spaces. */
      } else if((c == ISO_citation ||
		 c == ISO_citation2) && s.tagattrparamptr == 0) {
	s.minorstate = MINORSTATE_TAGATTRPARAM;
	s.quotechar = c;
	PRINTF(("tag attr param q found\n"));
	break;
      } else if(iswhitespace(c)) {
	PRINTF(("Non-leading space found at %d\n", s.tagattrparamptr));
	/* Stop parsing if a non-leading space was found */
	endtagfound();
	parse_tag();

	s.minorstate = MINORSTATE_TAGATTR;
	s.tagattrptr = 0;
	endtagfound();
	break;
      } else {
	s.tagattrparam[s.tagattrparamptr] = c;
	++s.tagattrparamptr;
	/* Check if the "tagattr" field is full. If so, we just eat
	   up any data left in the tag. */
	if(s.tagattrparamptr >= sizeof(s.tagattrparam) - 1) {
	  s.minorstate = MINORSTATE_TAGEND;
	  break;
	}
      }
    }
    break;
  case MINORSTATE_TAGATTRPARAM:
    /* We are parsing the "tag attr parameter", i.e., the link
       part in <a href="link">. */
    for(i = 0; i < len; ++i) {
      c = data[i];
      if(c == s.quotechar) {
	/* Found end of tag attr parameter. */
	endtagfound();
	parse_tag();

	s.minorstate = MINORSTATE_TAGATTR;
	s.tagattrptr = 0;
	endtagfound();
	break;
      } else {
	if(iswhitespace(c)) {
	  s.tagattrparam[s.tagattrparamptr] = ISO_space;
	} else {
	  s.tagattrparam[s.tagattrparamptr] = c;
	}

	++s.tagattrparamptr;
	/* Check if the "tagattr" field is full. If so, we just eat
	   up any data left in the tag. */
	if(s.tagattrparamptr >= sizeof(s.tagattrparam) - 1) {
	  s.minorstate = MINORSTATE_TAGEND;
	  break;
	}
      }
    }
    break;
  case MINORSTATE_HTMLCOMMENT:
    for(i = 0; i < len; ++i) {
      c = data[i];
      if(c == ISO_dash) {
	++s.tagptr;
      } else if(c == ISO_gt && s.tagptr > 0) {
	PRINTF(("Comment done.\n"));
	s.minorstate = MINORSTATE_TEXT;
	break;
      } else {
	s.tagptr = 0;
      }
    }
    break;
  case MINORSTATE_TAGEND:
    /* Discard characters until a '>' is seen. */
    for(i = 0; i < len; ++i) {
      if(data[i] == ISO_gt) {
	s.minorstate = MINORSTATE_TEXT;
	s.tagattrptr = 0;
	endtagfound();
	parse_tag();
	break;
      }
    }
    break;
  default:
    i = 0;
    break;
  }
  if(i >= len) {
    return len;
  }
  return i + 1;
}
/*-----------------------------------------------------------------------------------*/
void
htmlparser_parse(char *data, uint16_t datalen)
{
  uint16_t plen;
  
  while(datalen > 0) {
    if(datalen > 255) {
      plen = parse_word(data, 255);
    } else {
      plen = parse_word(data, (uint8_t)datalen);
    }
    datalen -= plen;
    data += plen;
  }
}
/*-----------------------------------------------------------------------------------*/

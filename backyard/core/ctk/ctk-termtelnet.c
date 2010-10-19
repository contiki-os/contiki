/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * @(#)$Id: ctk-termtelnet.c,v 1.2 2010/10/19 18:29:03 adamdunkels Exp $
 */
#include "contiki.h"
#include "loader.h"
#include "memb.h"
#include "ctk-term.h"
#include "contiki-conf.h"

/*-----------------------------------------------------------------------------------*/
/*
 * #defines and enums
 */
/*-----------------------------------------------------------------------------------*/
/* Telnet special characters */
#define TN_NULL   0
#define TN_BL    7
#define TN_BS     8
#define TN_HT     9
#define TN_LF    10
#define TN_VT    11
#define TN_FF    12
#define TN_CR    13

/* Commands preceeded by TN_IAC */
#define TN_SE   240
#define TN_NOP  241
#define TN_DM   242
#define TN_BRK  243
#define TN_IP   244
#define TN_AO   245
#define TN_AYT  246
#define TN_EC   247
#define TN_EL   248
#define TN_GA   249
#define TN_SB   250
#define TN_WILL 251
#define TN_WONT 252
#define TN_DO   253
#define TN_DONT 254
#define TN_IAC  255

#define TNO_BIN   0
#define TNO_ECHO  1
#define TNO_SGA   3
#define TNO_NAWS 31

/* Telnet parsing states */
enum {
  TNS_IDLE,
  TNS_IAC,
  TNS_OPT,
  TNS_SB,
  TNS_SBIAC
};

/* Telnet option negotiation states */
enum {
  TNOS_NO,
  TNOS_WANTNO_EMPTY,
  TNOS_WANTNO_OPPOSITE,
  TNOS_WANTYES_EMPTY,
  TNOS_WANTYES_OPPOSITE,
  TNOS_YES
};

/* Telnet session states */
enum {
  TTS_FREE,        /* Not allocated */
  TTS_IDLE,        /* No data to send and nothing sent */
  TTS_SEND_TNDATA, /* Sending telnet data */
  TTS_SEND_APPDATA /* Sending data from upper layers */
};

/* Number of options supported (we only need ECHO(1) and SGA(3) options) */
#define TNSM_MAX_OPTIONS 4

/* Max option replies in output queue */
#define TNQLEN 20

/* Number of option buffer */
#define OPTION_POOL_SIZE 20

/* Maximum number of telnet sessions */
#ifdef CTK_TERM_CONF_MAX_TELNET_CLIENTS
#define NUM_CONNS CTK_TERM_CONF_MAX_TELNET_CLIENTS
#else
#define NUM_CONNS 1
#endif

#ifdef CTK_TERM_CONF_TELNET_PORT
#define PORT CTK_TERM_CONF_TELNET_PORT
#else
#define PORT 23
#endif

/*-----------------------------------------------------------------------------------*/
/*
 * Structures
 */
/*-----------------------------------------------------------------------------------*/
/* Telnet option state structure */
struct TNOption {
  unsigned char state;
  unsigned char wants;
};

/* Telnet handling state structure */
struct TNSMState
{
  struct TNOption myOpt[TNSM_MAX_OPTIONS];
  struct TNOption hisOpt[TNSM_MAX_OPTIONS];
  unsigned char cmd;
  unsigned char state;
};

/* Telnet session state */
struct telnet_state
{
  unsigned char state;
  unsigned char* sendq[TNQLEN];
  struct TNSMState tnsm;
  struct ctk_term_state* termstate;
};

/*-----------------------------------------------------------------------------------*/
/*
 * Local variables
 */
/*-----------------------------------------------------------------------------------*/
/*static DISPATCHER_UIPCALL(ctk_termtelnet_appcall, state);*/
static void ctk_termtelnet_appcall(void *state);

EK_EVENTHANDLER(eventhandler, ev, data);
EK_PROCESS(p, "CTK telnet server", EK_PRIO_NORMAL,
	   eventhandler, NULL, NULL);
/*static struct dispatcher_proc p =
  {DISPATCHER_PROC("CTK telnet server", NULL, NULL,
  ctk_termtelnet_appcall)};*/

static ek_id_t id = EK_ID_NONE;

/* Option negotiation buffer pool */
struct size_3 {
  char size[3];
};
MEMB(telnetbuf, struct size_3, OPTION_POOL_SIZE);

static int i,j;
static struct telnet_state states[NUM_CONNS];

/*-----------------------------------------------------------------------------------*/
/*
 * Send an option reply on a connection
 */
/*-----------------------------------------------------------------------------------*/
static void
Reply(struct telnet_state* tns, unsigned char cmd, unsigned char opt)
{
  unsigned char* buf = (unsigned char*)memb_alloc(&telnetbuf);
  if (buf != 0) {
    buf[0]=TN_IAC;
    buf[1]=cmd;
    buf[2]=opt;
    for (i=0; i < TNQLEN; i++) {
      if (tns->sendq[i] == 0) {
	tns->sendq[i] = buf;
	return;
      }
    }
    /* Queue is full. Drop it */
    memb_free(&telnetbuf, (char*)buf);
  }
}

/*-----------------------------------------------------------------------------------*/
/*
 * Prepare for enabling one of remote side options.
 */
/*-----------------------------------------------------------------------------------*/
static void
EnableHisOpt(struct telnet_state* tns, unsigned char opt)
{
  switch(tns->tnsm.hisOpt[opt].state) {
  case TNOS_NO:
    tns->tnsm.hisOpt[opt].wants = 1;
    tns->tnsm.hisOpt[opt].state = TNOS_WANTYES_EMPTY;
    Reply(tns, TN_DO, opt);
    break;
  case TNOS_WANTNO_EMPTY:
    tns->tnsm.hisOpt[opt].state = TNOS_WANTNO_OPPOSITE;
    break;
  case TNOS_WANTNO_OPPOSITE:
    break;
  case TNOS_WANTYES_EMPTY:
    tns->tnsm.hisOpt[opt].state = TNOS_YES;
    break;
  case TNOS_WANTYES_OPPOSITE:
    tns->tnsm.hisOpt[opt].state = TNOS_WANTYES_EMPTY;
    break;
  case TNOS_YES:
    break;
  }
}

/*-----------------------------------------------------------------------------------*/
/*
 * Prepare for enabling one of my options
 */
/*-----------------------------------------------------------------------------------*/
static void
EnableMyOpt(struct telnet_state* tns, unsigned char opt)
{
  if (opt < TNSM_MAX_OPTIONS) {
    switch(tns->tnsm.myOpt[opt].state) {
    case TNOS_NO:
      tns->tnsm.myOpt[opt].wants = 1;
      tns->tnsm.myOpt[opt].state = TNOS_WANTYES_EMPTY;
      Reply(tns, TN_WILL, opt);
      break;
    case TNOS_WANTNO_EMPTY:
      tns->tnsm.myOpt[opt].state = TNOS_WANTNO_OPPOSITE;
      break;
    case TNOS_WANTNO_OPPOSITE:
      break;
    case TNOS_WANTYES_EMPTY:
      tns->tnsm.myOpt[opt].state = TNOS_YES;
      break;
    case TNOS_WANTYES_OPPOSITE:
      tns->tnsm.myOpt[opt].state = TNOS_WANTYES_EMPTY;
      break;
    case TNOS_YES:
      break;
    }
  }
}

/*-----------------------------------------------------------------------------------*/
/*
 * Implementation of option negotiation using the Q-method
 */
/*-----------------------------------------------------------------------------------*/
static void
HandleCommand(struct telnet_state* tns, unsigned char cmd, unsigned char opt)
{
  if (opt < TNSM_MAX_OPTIONS) {
    /* Handling according to RFC 1143 "Q Method" */
    switch(cmd) {
    case TN_WILL:
      switch(tns->tnsm.hisOpt[opt].state) {
      case TNOS_NO:
	if (tns->tnsm.hisOpt[opt].wants) {
	  tns->tnsm.hisOpt[opt].state = TNOS_YES;
	  Reply(tns, TN_DO, opt);
	}
	else {
	  Reply(tns, TN_DONT, opt);
	}
	break;
      case TNOS_WANTNO_EMPTY:
	tns->tnsm.hisOpt[opt].state = TNOS_NO;
	break;
      case TNOS_WANTNO_OPPOSITE:
	tns->tnsm.hisOpt[opt].state = TNOS_YES;
	break;
      case TNOS_WANTYES_EMPTY:
	tns->tnsm.hisOpt[opt].state = TNOS_YES;
	break;
      case TNOS_WANTYES_OPPOSITE:
	tns->tnsm.hisOpt[opt].state = TNOS_WANTNO_EMPTY;
	Reply(tns, TN_DONT, opt);
	break;
      case TNOS_YES:
	break;
      }
      break;
      case TN_WONT:
	switch(tns->tnsm.hisOpt[opt].state) {
	case TNOS_NO:
	  break;
	case TNOS_WANTNO_EMPTY:
	case TNOS_WANTYES_EMPTY:
	case TNOS_WANTYES_OPPOSITE:
	  tns->tnsm.hisOpt[opt].state = TNOS_NO;
	  break;
	case TNOS_WANTNO_OPPOSITE:
	  tns->tnsm.hisOpt[opt].state = TNOS_WANTYES_EMPTY;
	  Reply(tns, TN_DO, opt);
	  break;
	case TNOS_YES:
	  tns->tnsm.hisOpt[opt].state = TNOS_NO;
	  Reply(tns, TN_DONT, opt);
	  break;
	}
	break;
	case TN_DO:
	  switch(tns->tnsm.myOpt[opt].state) {
	  case TNOS_NO:
	    if (tns->tnsm.myOpt[opt].wants) {
	      tns->tnsm.myOpt[opt].state = TNOS_YES;
	      Reply(tns, TN_WILL, opt);
	    }
	    else {
	      Reply(tns, TN_WONT, opt);
	    }
	    break;
	  case TNOS_WANTNO_EMPTY:
	    tns->tnsm.myOpt[opt].state = TNOS_NO;
	    break;
	  case TNOS_WANTNO_OPPOSITE:
	    tns->tnsm.myOpt[opt].state = TNOS_YES;
	    break;
	  case TNOS_WANTYES_EMPTY:
	    tns->tnsm.myOpt[opt].state = TNOS_YES;
	    break;
	  case TNOS_WANTYES_OPPOSITE:
	    tns->tnsm.myOpt[opt].state = TNOS_WANTNO_EMPTY;
	    Reply(tns, TN_WONT, opt);
	    break;
	  case TNOS_YES:
	    break;
	  }
	  break;
	  case TN_DONT:
	    switch(tns->tnsm.myOpt[opt].state) {
	    case TNOS_NO:
	      break;
	    case TNOS_WANTNO_EMPTY:
	    case TNOS_WANTYES_EMPTY:
	    case TNOS_WANTYES_OPPOSITE:
	      tns->tnsm.myOpt[opt].state = TNOS_NO;
	      break;
	    case TNOS_WANTNO_OPPOSITE:
	      tns->tnsm.myOpt[opt].state = TNOS_WANTYES_EMPTY;
	      Reply(tns, TN_WILL, opt);
	      break;
	    case TNOS_YES:
	      tns->tnsm.myOpt[opt].state = TNOS_NO;
	      Reply(tns, TN_WONT, opt);
	      break;
	    }
	    break;
    }
  }
  else {
    switch(cmd) {
    case TN_WILL:
      Reply(tns, TN_DONT, opt);
      break;
    case TN_WONT:
      break;
    case TN_DO:
      Reply(tns, TN_WONT, opt);
      break;
    case TN_DONT:
      break;
    }
  }
}

/*-----------------------------------------------------------------------------------*/
/*
 * Telnet data parsing
 */
/*-----------------------------------------------------------------------------------*/
static unsigned char
parse_input(struct telnet_state* tns, unsigned char b)
{
  unsigned char ret = 0;
  switch(tns->tnsm.state) {
  case TNS_IDLE:
    if (b == TN_IAC) tns->tnsm.state = TNS_IAC;
    else ret = 1;
    break;
  case TNS_IAC:
    switch(b) {
    case TN_SE:
    case TN_NOP:
    case TN_DM:
    case TN_BRK:
    case TN_IP:
    case TN_AO:
    case TN_AYT:
    case TN_EC:
    case TN_EL:
    case TN_GA:
      tns->tnsm.state = TNS_IDLE;
      break;
    case TN_SB:
      tns->tnsm.state = TNS_SB;
      break;
    case TN_WILL:
    case TN_WONT:
    case TN_DO:
    case TN_DONT:
      tns->tnsm.cmd = b;
      tns->tnsm.state = TNS_OPT;
      break;
    case TN_IAC:
      tns->tnsm.state = TNS_IDLE;
      ret = 1;
      break;
    default:
      /* Drop unknown IACs */
      tns->tnsm.state = TNS_IDLE;
      break;
    }
    break;
    case TNS_OPT:
      HandleCommand(tns, tns->tnsm.cmd, b);
      tns->tnsm.state = TNS_IDLE;
      break;
    case TNS_SB:
      if (b == TN_IAC) {
	tns->tnsm.state = TNS_SBIAC;
      }
      break;
    case TNS_SBIAC:
      if (b == TN_IAC) {
	tns->tnsm.state = TNS_SB;
      }
      else if (b == TN_SE) {
	tns->tnsm.state = TNS_IDLE;
      }
      else {
	tns->tnsm.state = TNS_IDLE;
      }
      break;
  }
  return ret;
}

/*-----------------------------------------------------------------------------------*/
/*
 * Initialize telnet machine
 */
/*-----------------------------------------------------------------------------------*/
static void
telnet_init(struct telnet_state* tns)
{
  int i;
  for (i = 0; i < TNSM_MAX_OPTIONS; i++) {
    tns->tnsm.myOpt[i].state = TNOS_NO;
    tns->tnsm.myOpt[i].wants = 0;
    tns->tnsm.hisOpt[i].state = TNOS_NO;
    tns->tnsm.hisOpt[i].wants = 0;
  }
  tns->tnsm.state = TNS_IDLE;
}


/*-----------------------------------------------------------------------------------*/
/*
 * Allocate a telnet session structure (including terminal state)
 */
/*-----------------------------------------------------------------------------------*/
static struct telnet_state*
alloc_state()
{
  for (i=0; i < NUM_CONNS; i++) {
    if (states[i].state == TTS_FREE) {
      states[i].termstate = ctk_term_alloc_state();
      if (states[i].termstate != NULL) {
	for (j = 0; j < TNQLEN; j++) {
	  states[i].sendq[j] = 0;
	}
	telnet_init(&states[i]);
        states[i].state = TTS_IDLE;
        return &(states[i]);
      }
    }
  }
  return NULL;
}

/*-----------------------------------------------------------------------------------*/
/*
 * Free a telnet session structure (including terminal state)
 */
/*-----------------------------------------------------------------------------------*/
static void
free_state(struct telnet_state* tns)
{
  if (tns != NULL) {
    ctk_term_dealloc_state(tns->termstate);
    tns->state = TTS_FREE;
  }
}

/*-----------------------------------------------------------------------------------*/
/*
 * A packet is successfully sent
 */
/*-----------------------------------------------------------------------------------*/
static void
acked(struct telnet_state* tns)
{
  /* Were we sending a telnet option packet? */
  if (tns->state == TTS_SEND_TNDATA) {
    /* Yes, free it and update queue */
    if (tns->sendq[0] != 0) {
      memb_free(&telnetbuf, (char*)(tns->sendq[0]));
      for (i=1; i < TNQLEN; i++) {
	tns->sendq[i-1] = tns->sendq[i];
      }
      tns->sendq[TNQLEN-1] = 0;
      /* No options left. Go idle */
      if (tns->sendq[0] == 0) {
	tns->state = TTS_IDLE;
      }
    }
  }
  /* Or were we sending application date ? */
  else if (tns->state == TTS_SEND_APPDATA) {
    /* Inform application that data is sent successfully */
    ctk_term_sent(tns->termstate);
    tns->state = TTS_IDLE;
  }
}

/*-----------------------------------------------------------------------------------*/
/*
 * Send data on a connections
 */
/*-----------------------------------------------------------------------------------*/
static void
senddata(struct telnet_state* tns)
{
  /* Check if there are any option packets to send */
  if (tns->state == TTS_IDLE || tns->state == TTS_SEND_TNDATA) {
    if (tns->sendq[0] != 0) {
      tns->state = TTS_SEND_TNDATA;
      uip_send(tns->sendq[0],3);
    }
  }
  /* Check if terminal wants to send any data */
  if (tns->state == TTS_IDLE || tns->state == TTS_SEND_APPDATA) {
    u16_t len = ctk_term_send(tns->termstate, (unsigned char*)uip_appdata, (unsigned short)uip_mss());
    if (len > 0) {
      tns->state = TTS_SEND_APPDATA;
      uip_send(uip_appdata, len);
    }
  }
}

/*-----------------------------------------------------------------------------------*/
/*
 * uIP callback
 */
/*-----------------------------------------------------------------------------------*/
static void
ctk_termtelnet_appcall(void *state)
{
  struct telnet_state *tns;

  tns = (struct telnet_state*)(state);

  if(uip_connected()) {
    if(tns == NULL) {
      tns = alloc_state();
      if(tns == NULL) {
	uip_close();
	return;
      }
      tcp_markconn(uip_conn, (void *)tns);
    }
    /* Try to negotiate some options */
    EnableHisOpt(tns, TNO_SGA);
    EnableMyOpt(tns,TNO_SGA);
    EnableMyOpt(tns,TNO_ECHO);
    /* Request update of screen */
    ctk_term_redraw(tns->termstate);
    senddata(tns);
  } else if(uip_closed() || uip_aborted()) {
    free_state(tns);
    return;
  }

  if (uip_acked()) {
    acked(tns);
  }

  if (uip_newdata()) {
    for(j = 0; j < uip_datalen(); j++) {
      if (parse_input(tns, uip_appdata[j])) {
	/* Pass it uppwards */
	ctk_term_input(tns->termstate, uip_appdata[j]);
      }
    }
  }

  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked()) {
    senddata(tns);
  } else if(uip_poll()) {
    if (tns->state == TTS_IDLE) {
      senddata(tns);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * Init function
 */
/*-----------------------------------------------------------------------------------*/
LOADER_INIT_FUNC(ctk_termtelnet_init, arg)
{
  arg_free(arg);
  if(id == EK_ID_NONE) {
    memb_init(&telnetbuf);
    for (i=0; i < NUM_CONNS; i++) {
      states[i].state = TTS_FREE;
    }
    id = ek_start(&p);
  }
}
/*-----------------------------------------------------------------------------------*/
EK_EVENTHANDLER(eventhandler, ev, data)
{
  if(ev == EK_EVENT_INIT) {
    tcp_listen(UIP_HTONS(PORT));
  } else if(ev == tcpip_event) {
    ctk_termtelnet_appcall(data);
  }
}

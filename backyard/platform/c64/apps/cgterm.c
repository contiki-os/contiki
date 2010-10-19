#include "contiki-net.h"
#include "ctk/ctk-draw.h"

#include <cbm.h>
#include <c64.h>
#include <string.h>

static struct {
  struct psock sout, sin;
  struct pt inpt, outpt;
  char outputbuf[200];
  char inputbuf[200];
  unsigned short len;
} s;

#define CURSOR_ON()  *(char *)0xcc = 0
#define CURSOR_OFF() *(char *)0xcc = 1
static void ctkmode(void);
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send(void))
{
  PSOCK_BEGIN(&s.sout);
  
  PSOCK_SEND(&s.sout, s.outputbuf, s.len);
  PSOCK_END(&s.sout);    
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(void))
{
  ctk_arch_key_t c;
  char *ptr;
  
  PT_BEGIN(&s.outpt);
  
  while(1) {
    PT_WAIT_UNTIL(&s.outpt, (ctk_mode_get() == CTK_MODE_EXTERNAL) &&
		  kbhit());


    ptr = s.outputbuf;
    s.len = 0;    
    while(kbhit() && s.len < sizeof(s.outputbuf)) {
      c = cgetc();
      *ptr = c;
      ++ptr;
      ++s.len;   
    }

    PT_WAIT_THREAD(&s.outpt, send());
  }
  PT_END(&s.outpt);      
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(void))
{
  unsigned short i;
  char *ptr;
  char next;
  
  next = 1; 
  PT_BEGIN(&s.inpt);
  
  while(1) {
    /* Wait until data arrives. */
    next = 0;
    PT_WAIT_UNTIL(&s.inpt, next && uip_newdata());

    CURSOR_OFF();
    /* Print it out on the screen. */
    ptr = (char *)uip_appdata;
    for(i = 0; i < uip_len; ++i) {
      cbm_k_bsout(*ptr);
      ++ptr;
    }
    CURSOR_ON();
  }  
  PT_END(&s.inpt);
}
/*---------------------------------------------------------------------------*/
static void
appcall(void *state)
{
  if(uip_closed() || uip_aborted() || uip_timedout()) {
    ctkmode();
  } else if(uip_connected()) {
  } else {
    handle_input();
    handle_output();   
  }
}
/*---------------------------------------------------------------------------*/
static struct uip_conn *
connect(u16_t *host, u16_t port)
{
  PSOCK_INIT(&s.sin, s.inputbuf, sizeof(s.inputbuf));
  PSOCK_INIT(&s.sout, s.inputbuf, sizeof(s.inputbuf));
  PT_INIT(&s.inpt);
  PT_INIT(&s.outpt);
  return tcp_connect(host, uip_htons(port), NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS(cgterm_process, "C/G terminal");

static struct uip_conn *conn;
static u16_t serveraddr[2];
static u16_t serverport;
/*---------------------------------------------------------------------------*/
static struct ctk_window window;
static struct ctk_label hostlabel =
  {CTK_LABEL(0, 0, 4, 1, "Host")};
static char host[32];
static struct ctk_textentry hostentry =
  {CTK_TEXTENTRY(5, 0, 20, 1, host, sizeof(host) - 1)};
static struct ctk_label portlabel =
  {CTK_LABEL(27, 0, 4, 1, "Port")};
static char port[7];
static struct ctk_textentry portentry =
  {CTK_TEXTENTRY(32, 0, 4, 1, port, sizeof(port) - 1)};
static struct ctk_button connectbutton =
  {CTK_BUTTON(0, 2, 7, "Connect")};
static struct ctk_button switchbutton =
  {CTK_BUTTON(30, 2, 6, "Switch")};
static struct ctk_label helplabel =
  {CTK_LABEL(0, 4, 37, 1, "RUN/STOP to return from terminal view")};
/*---------------------------------------------------------------------------*/
static void
ctkmode(void)
{
  ctk_mode_set(CTK_MODE_NORMAL);
  ctk_draw_init();
  ctk_desktop_redraw(NULL);
}
/*---------------------------------------------------------------------------*/
static void
textmode(void)
{
  ctk_mode_set(CTK_MODE_EXTERNAL);
  
  VIC.ctrl1 = 0x1b;  /* $D011 */
  VIC.addr  = 0x17;  /* $D018 */
  VIC.ctrl2 = 0xc8;  /* $D016 */
  CIA2.pra  = 0x03;  /* $DD00 */
  
  VIC.bordercolor = 0x00; /* $D020 */
  VIC.bgcolor0 = 0x00; /* $D021 */

  CURSOR_ON();
}     
/*---------------------------------------------------------------------------*/
static void
pollhandler(void)
{
  if(ctk_mode_get() == CTK_MODE_EXTERNAL) {
    if(CIA1.prb == 0x7f) {
      ctkmode();
    } else if(kbhit() && conn != NULL) {
      tcpip_poll_tcp(conn);
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cgterm_process, ev, data)
{
  u16_t *ipaddr;
  char *cptr;

  PROCESS_POLLHANDLER(pollhandler());
  
  PROCESS_BEGIN();

  ctk_window_new(&window, 38, 5, "C/G term");
  CTK_WIDGET_ADD(&window, &hostlabel);
  CTK_WIDGET_ADD(&window, &hostentry);
  CTK_WIDGET_ADD(&window, &portlabel);
  CTK_WIDGET_ADD(&window, &portentry);
  CTK_WIDGET_ADD(&window, &connectbutton);
  CTK_WIDGET_ADD(&window, &switchbutton);
  CTK_WIDGET_ADD(&window, &helplabel);
  ctk_window_open(&window);

  while(1) {
    PROCESS_WAIT_EVENT();
  
    if(ev == tcpip_event) {
      appcall(data);
    } else if(ev == ctk_signal_widget_activate) {
      
      if(data == &switchbutton) {
	textmode();
      } else if(data == &connectbutton) {
	serverport = 0;
	for(cptr = port; *cptr != ' ' && *cptr != 0; ++cptr) {
	  if(*cptr < '0' || *cptr > '9') {
	    continue;
	  }
	  serverport = 10 * serverport + *cptr - '0';
	}
	
	ipaddr = serveraddr;
	if(uiplib_ipaddrconv(host, (u8_t *)serveraddr) == 0) {
	  ipaddr = resolv_lookup(host);
	  if(ipaddr == NULL) {
	    resolv_query(host);
	  } else {
	    uip_ipaddr_copy(serveraddr, ipaddr);
	  }
	}
	if(ipaddr != NULL) {      
	  conn = connect(serveraddr, serverport);
	  if(conn != NULL) {
	    memset((char *)0x0400, 0x20, 40*25);
	    memset((char *)0xd800, 0x01, 40*25);
	    textmode();
	  }
	}
      }
    } else if(ev == resolv_event_found) {
      ipaddr = resolv_lookup(host);
      if(ipaddr != NULL) {
	uip_ipaddr_copy(serveraddr, ipaddr);
	
	conn = connect(serveraddr, serverport);
	if(conn != NULL) {
	  memset((char *)0x0400, 0x20, 40*25);
	  memset((char *)0xd800, 0x01, 40*25);
	  textmode();
	}
      }
    } else if(ev == PROCESS_EVENT_EXIT ||
	      ev == ctk_signal_window_close) {
     
      break;
    }
  }
  ctk_window_close(&window);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

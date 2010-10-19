#include <stepper-process.h>
#include <stepper-steps.h>
#include <stepper.h>
#include <stepper-move.h>
#include <string.h>
#include <interrupt-utils.h>
#include <stdio.h>
#include <unistd.h>
#include <net/uip.h>
#include <dev/cc2420.h>


#undef putchar


static unsigned int
parse_uint_hex(const char **pp, const char *end)
{
  unsigned int v = 0;
  while(*pp < end) {
    char ch;
    if ((ch = **pp) >= '0' && ch <= '9') {
      v = v* 16 + (ch - '0');
    } else if (ch >= 'A' && ch <= 'F') {
      v = v* 16 + (ch - 'A') + 10;
    } else break;
    (*pp)++;
  }
  return v;
}

static int
parse_int_hex(const char **pp, const char *end)
{
  if (*pp == end) return 0;
  if (**pp == '-') {
    (*pp)++;
    return -parse_uint_hex(pp, end);
  } else {
    return parse_uint_hex(pp, end);
  }
}

static void
skip_white(const char **pp, const char *end)
{
  char ch;
  while(*pp < end && ((ch = **pp) == ' ' || ch == '\t'))  (*pp)++;
}

static const char hex_chars[] =
  {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static void
format_uint_hex(char **str, char *end, unsigned int v)
{
  char buffer[10];
  char *p = buffer+10;
  if (*str == end) return;
  if (v == 0) {
    *(*str)++ = '0';
    return;
  }
  while(v > 0) {
    *--p = hex_chars[v&0xf];
    v >>= 4;
  }
  while((p < buffer+10) && (*str < end)) {
    *(*str)++ = *p++;
  }
}

static void
format_int_hex(char **str, char *end, int v)
{
  if (v < 0) {
    if (*str == end) return;
    *(*str)++ = '-';
    v = -v;
  }
  format_uint_hex(str, end, v);
}

static void
format_ull_hex(char **str, char *end, unsigned long long int v)
{
  char buffer[16];
  char *p = buffer+10;
  if (*str == end) return;
  if (v == 0) {
    *(*str)++ = '0';
    return;
  }
  while(v > 0) {
    *--p = hex_chars[v&0xf];
    v >>= 4;
  }
  while((p < buffer+10) && (*str < end)) {
    *(*str)++ = *p++;
  }
}
static void
format_ll_hex(char **str, char *end, long long v)
{
  if (v < 0) {
    if (*str == end) return;
    *(*str)++ = '-';
    v = -v;
  }
  format_ull_hex(str, end, v);
}

typedef struct _ReplyBuffer ReplyBuffer;

struct _ReplyBuffer
{
  char buffer[70]; /* Should be small enough to fit in one packet */
  char *write;
};

static ReplyBuffer tcp_reply;
static ReplyBuffer udp_reply;

#define REPLY_BUFFER_END(reply) ((reply)->buffer+sizeof((reply)->buffer))
#define REPLY_BUFFER_LEFT(reply) \
((reply)->buffer+sizeof((reply)->buffer) - (reply)->write)

static void
reply_char(ReplyBuffer *reply, char c)
{
  if (REPLY_BUFFER_LEFT(reply) > 0) {
    *reply->write++ = c;
  }
}

static void
reply_str(ReplyBuffer *reply, char *str)
{
  while(reply->write < REPLY_BUFFER_END(reply) && *str != '\0')
    *reply->write++ = *str++;
}

static void
stepper_reply(ReplyBuffer *reply, StepperResult res)
{
  switch(res) {
  case STEPPER_OK:
    reply_str(reply, "OK");
    break;
  case STEPPER_ERR_MEM:
    reply_str(reply, "ERR MEM");
    break;
  case STEPPER_ERR_TOO_LATE:
    reply_str(reply, "ERR LATE");
    break;
  case STEPPER_ERR_INDEX:	/* Sholdn't happen here */
    reply_str(reply, "ERR INDEX");
    break;
  default:
    reply_str(reply, "ERR");
  }
  reply_char(reply, '\n');
}
  
#define CHECK_INPUT_LEFT(x) \
do {\
if ((x) > inend - input_line) {reply_str(reply, "ERR\n");return 0;}\
} while(0)
 
static int
handle_line(const char *input_line, const char *inend, ReplyBuffer *reply)
{
  unsigned long when;
#if 0
  {
    const char *p = input_line;
    printf("Got line: '");
    while(p < inend) {
      putchar(*p++);
    }
    printf("'\n");
    fsync(1);
  }
#endif
  skip_white(&input_line, inend);
  CHECK_INPUT_LEFT(1);
  if (*input_line == '#') {
    input_line++;
    reply_char(reply, '#');
    while (input_line < inend &&*input_line != ' ') {
      reply_char(reply, *input_line++);
    }
    reply_char(reply, ' ');
  }
  skip_white(&input_line, inend);
  
  if (*input_line == '@') {
    input_line++;
    when = parse_uint_hex(&input_line, inend);
  } else {
    when = stepper_current_period() + 3;
  }
  skip_white(&input_line, inend);
  CHECK_INPUT_LEFT(1);
  if (input_line[0] == 'L' || input_line[0] == 'R') {
    unsigned int stepper_index = (input_line[0] == 'R' ? 1 : 0);
    CHECK_INPUT_LEFT(1);
    input_line++;
    if (input_line[0] == 'S') {
      int speed;
      input_line++;
      if (input_line == inend) {
	/* printf("Speed: %ld\n",
	   stepper_current_velocity(stepper_index)/VEL_SCALE);*/
	reply_char(reply, input_line[-2]);
	reply_char(reply, 'S');
	format_int_hex(&reply->write, REPLY_BUFFER_END(reply),
		       stepper_current_velocity(stepper_index)/VEL_SCALE); 
	reply_char(reply, '\n');
      } else {
	speed = parse_int_hex(&input_line, inend);
	if (*input_line == ',') {
	  StepperResult res;
	  unsigned int acc;
	  input_line++;
	  acc = parse_uint_hex(&input_line, inend);
	  /* printf("Speed=%d, Acc=%u\n", speed, acc); */
	  res = stepper_set_velocity(stepper_index, &when,
				     acc, speed*VEL_SCALE);
	  
	  stepper_reply(reply, res);
	} else {
	  reply_str(reply, "ERR\n");
	}
      }
    } else if (input_line[0] == 'C') {
      reply_char(reply, input_line[-1]);
      reply_char(reply, 'C');
      format_ll_hex(&reply->write, REPLY_BUFFER_END(reply),
		    stepper_current_step(stepper_index)); 
      reply_char(reply, '\n');
    } else if (input_line[0] == 'M') {
      unsigned int speed;
      unsigned int acc;
      int move;
      input_line++;
      speed = parse_uint_hex(&input_line, inend);
      CHECK_INPUT_LEFT(1);
      if (*input_line == ',') {
	input_line++;
	acc = parse_uint_hex(&input_line, inend);
	if (*input_line == ',') {
	  StepperResult res;
	  input_line++;
	  move = parse_int_hex(&input_line, inend);
	  /*printf("Speed=%u, Acc=%u, Move=%d\n", speed, acc, move);*/
	  res = stepper_move(stepper_index, &when,
			     acc,speed*VEL_SCALE,move*DIST_SCALE);
	  stepper_reply(reply, res);
	} else {
	  reply_str(reply, "ERR\n");
	}
      } else {
	reply_str(reply, "ERR\n");
      }
    } else {
      reply_str(reply, "ERR\n");
    }
  } else if (input_line[0] == 'E') {
    STEPPER_ENABLE();
    printf("Stepper enabled\n");
    reply_str(reply, "OK\n");
  } else if (input_line[0] == 'D') {
    STEPPER_DISABLE();
    printf("Stepper disabled\n");
    reply_str(reply, "OK\n");
  } else if (input_line[0] == 'p') {
    reply_char(reply, 'p');
    format_int_hex(&reply->write, REPLY_BUFFER_END(reply),
		   cc2420_last_rssi); 
    reply_char(reply, ',');
    format_uint_hex(&reply->write, REPLY_BUFFER_END(reply),
		    cc2420_last_correlation);
    reply_char(reply, '\n');
  } else if (input_line[0] == 'T') {
    reply_char(reply, 'T');
    format_int_hex(&reply->write, REPLY_BUFFER_END(reply),
		   stepper_current_period());
    reply_char(reply, '\n');
  } else if (input_line[0] == 'q') {
    return 1;
  } else {
     reply_str(reply, "ERR\n");
  }
  return 0;
}
static unsigned int transmit_len = 0;

static void
send_reply()
{
  if (transmit_len == 0) {
    transmit_len = tcp_reply.write - tcp_reply.buffer;
    if (transmit_len > 0) {
      /* printf("Sending len = %d\n", transmit_len); */
      uip_send(tcp_reply.buffer, transmit_len);
    }
  }
}


static void
handle_connection()
{
  static char exiting = 0;
  static char line_buffer[100];
  static char *line_end;
  if (uip_connected()) {
    exiting = 0;
    transmit_len = 0;
    line_end = line_buffer;
    tcp_reply.write = tcp_reply.buffer;
    reply_str(&tcp_reply, "Ready\n");
    send_reply();
  }
  if (uip_acked()) {
    if (tcp_reply.write - tcp_reply.buffer > transmit_len) {
      memmove(tcp_reply.buffer, tcp_reply.buffer + transmit_len,
	      tcp_reply.write - tcp_reply.buffer - transmit_len);
    }
    tcp_reply.write -= transmit_len;
    /* printf("Acked: %d left\n", reply_buffer.write-reply_buffer.buffer); */
    transmit_len = 0;
    if (exiting && tcp_reply.write == tcp_reply.buffer) {
      uip_close();
      exiting = 0;
    }
  }
  if (uip_newdata()) {
    const char *read_pos = uip_appdata;
    const char *read_end = read_pos + uip_len;
    /* printf("Got data\n"); */
    while(read_pos < read_end) {
      if (line_end == line_buffer+sizeof(line_buffer)) {
	/* Buffer too small, just discard everything */
	line_end = line_buffer;
      }
      *line_end++ = *read_pos++;
      if (line_end[-1] == '\n' || line_end[-1] == '\r' || line_end[-1] == ';'){
	if (line_end - 1 != line_buffer) {
	  if (handle_line(line_buffer, line_end - 1, &tcp_reply)) {
	    send_reply();
	    /* Postpone closing if there's reply data left to be sent. */
	    if (transmit_len == 0)
	      uip_close();
	    else 
	      exiting = 1;
	    break;
	  }
	}
	line_end = line_buffer;
      }
    }
    send_reply();
  }
  
  if (uip_poll()) {
    send_reply();
  }
  if(uip_rexmit()) {
    printf("Retransmit\n");
    if (transmit_len > 0) 
      uip_send(tcp_reply.buffer, transmit_len);
  }
  
}

PROCESS(udp_stepper_process, "UDP stepper process");

PROCESS_THREAD(udp_stepper_process, ev, data)
{
  static struct etimer timer;
  static struct uip_udp_conn *conn;
  static char listening = 1; /* Listen for connections from anyone */
  static uip_ipaddr_t any;
  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();

  printf("udp_stepper_process starting\n");

  uip_ipaddr(&any, 0,0,0,0);
  conn = udp_new(&any, UIP_HTONS(0), NULL);
  if (!conn) goto exit;
  uip_udp_bind(conn, UIP_HTONS(1010));
  etimer_set(&timer, CLOCK_SECOND*2);
  while(1) {
    PROCESS_YIELD();
    
    if(ev == tcpip_event) {
      if (uip_newdata()) {
	struct uip_udpip_hdr *header = (struct uip_udpip_hdr *)uip_buf;
	const char *line_start = uip_appdata;
	const char *line_end = line_start;
	const char *packet_end = line_start + uip_len;
	udp_reply.write = udp_reply.buffer;
	while(line_end < packet_end) {
	  if (*line_end == '\n' || *line_end == '\r' || *line_end == ';' ) {
	    if (line_end != line_start) {
	      handle_line(line_start, line_end, &udp_reply);
	    }
	    line_start = line_end+1;
	  }
	  line_end++;
	}
	/* Check if we are connected to a client, if not reconnect */
	if (listening) {
	  uip_udp_remove(conn);
	  conn = udp_new(&header->srcipaddr, header->srcport, &conn);
	  if (!conn) goto exit;
	  uip_udp_bind(conn, UIP_HTONS(1010));
	  listening = 0;
	}
	etimer_reset(&timer);
	tcpip_poll_udp(conn);
      } else if (uip_poll()) {
	if (data == &conn) {
	  uip_send(udp_reply.buffer, udp_reply.write - udp_reply.buffer);
	  /* printf("sent %ld\n", udp_reply.write - udp_reply.buffer); */
	}
      }
    } else if (ev == PROCESS_EVENT_TIMER) {
      uip_udp_remove(conn);
      conn = udp_new(&any, UIP_HTONS(0), NULL);
      if (!conn) goto exit;
      uip_udp_bind(conn, UIP_HTONS(1010));
      listening = 1;
    }
  }

 exit:
  /* Contiki does automatic garbage collection of uIP state and we
   * need not worry about that. */
  printf("udprecv_process exiting\n");
  PROCESS_END();
}

static const uint32_t stepper0_steps_acc[] = MICRO_STEP(0,3);
static const uint32_t stepper0_steps_run[] = MICRO_STEP(0,2);
static const uint32_t stepper0_steps_hold[] = MICRO_STEP(0,1);

static const uint32_t stepper1_steps_acc[] = MICRO_STEP(1,3);
static const uint32_t stepper1_steps_run[] = MICRO_STEP(1,2);
static const uint32_t stepper1_steps_hold[] = MICRO_STEP(1,1);

static StepperAccSeq seq_heap[40];

static void
init_seq_heap()
{
  unsigned int i;
  for(i = 0; i < sizeof(seq_heap)/sizeof(seq_heap[0]); i++) {
    seq_heap[i].next = NULL;
    stepper_free_seq(&seq_heap[i]);
  }
}

static void
robot_stepper_init()
{
  disableIRQ();
  init_seq_heap();
  stepper_init(AT91C_BASE_TC0, AT91C_ID_TC0);
  *AT91C_PIOA_OER = STEPPER_INHIBIT;
  *AT91C_PIOA_MDER = STEPPER_INHIBIT; /*  | STEPPER0_IOMASK; */
  *AT91C_PIOA_CODR = STEPPER_INHIBIT;
  stepper_init_io(1, STEPPER_IOMASK(0), stepper0_steps_acc,
		  stepper0_steps_run, stepper0_steps_hold,
		  (sizeof(stepper0_steps_run) / sizeof(stepper0_steps_run[0])));
  stepper_init_io(0, STEPPER_IOMASK(1), stepper1_steps_acc,
		  stepper1_steps_run, stepper1_steps_hold,
		  (sizeof(stepper1_steps_run) / sizeof(stepper1_steps_run[0])));
  enableIRQ();
}


PROCESS(stepper_process, "Stepper control process");

PROCESS_THREAD(stepper_process, ev, data)
{
  PROCESS_EXITHANDLER(goto exit);
  PROCESS_BEGIN();
  robot_stepper_init();
  tcp_listen(UIP_HTONS(1010));
  
  process_start(&udp_stepper_process, NULL);
  printf("Stepper starting\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    if(uip_connected()) {
      /* printf("connected\n"); */
      handle_connection(); /* Initialise parser */
      while(!(uip_aborted() || uip_closed() || uip_timedout())) {
	PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
	handle_connection();
      }
    }
    printf("disconnected\n");
  }

 exit:
  /* Contiki does automatic garbage collection of uIP state and we
   * need not worry about that. */
  printf("Stepper exiting\n");
  PROCESS_END();
}


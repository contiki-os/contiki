#include <debug-uart.h>
#include <sys-interrupt.h>
/* #include <strformat.h> */
#include <AT91SAM7S64.h>
#include <string.h>
#include <interrupt-utils.h>

#ifndef DBG_XMIT_BUFFER_LEN
#define DBG_XMIT_BUFFER_LEN 3024
#endif
#ifndef DBG_RECV_BUFFER_LEN
#define DBG_RECV_BUFFER_LEN 256
#endif

static unsigned char dbg_xmit_buffer[DBG_XMIT_BUFFER_LEN];
static unsigned char dbg_recv_buffer[DBG_RECV_BUFFER_LEN];
static unsigned int dbg_recv_buffer_len = 0;

void
dbg_setup_uart()
{
  /* Setup PIO ports */
  *AT91C_PIOA_OER = AT91C_PA10_DTXD;
  *AT91C_PIOA_ODR = AT91C_PA9_DRXD;
  *AT91C_PIOA_ASR = AT91C_PA10_DTXD | AT91C_PA9_DRXD;
  *AT91C_PIOA_PDR = AT91C_PA10_DTXD | AT91C_PA9_DRXD;

  *AT91C_DBGU_MR = AT91C_US_PAR_NONE | AT91C_US_CHMODE_NORMAL;
  *AT91C_DBGU_IDR= 0xffffffff;

  *AT91C_DBGU_BRGR = MCK / (115200 * 16);
  *AT91C_DBGU_CR =  AT91C_US_RXEN | AT91C_US_TXEN;

  *AT91C_DBGU_TPR = (unsigned int)dbg_xmit_buffer;
  *AT91C_DBGU_TNPR = (unsigned int)dbg_xmit_buffer;

  
}

static void (*input_func)(const char *inp, unsigned int len) = NULL;
     
static int dbg_recv_handler_func()
{
  if (!(*AT91C_DBGU_CSR & AT91C_US_RXRDY)) return 0;
  unsigned char c = *AT91C_DBGU_RHR;
  /* Leave one byte for '\0' */
  if (dbg_recv_buffer_len < (DBG_RECV_BUFFER_LEN -1)) {
    dbg_recv_buffer[dbg_recv_buffer_len++] = c;
  }
  if (c == '\n') {
    dbg_recv_buffer[dbg_recv_buffer_len] = '\0';
    if (input_func) input_func((char*)dbg_recv_buffer, dbg_recv_buffer_len);
    dbg_recv_buffer_len = 0;
  }
  return 1;
}

static SystemInterruptHandler dbg_recv_handler = {NULL, dbg_recv_handler_func};

void
dbg_set_input_handler(void (*handler)(const char *inp, unsigned int len))
{
  input_func = handler;
  sys_interrupt_append_handler(&dbg_recv_handler);
  sys_interrupt_enable();
  *AT91C_DBGU_IER = AT91C_US_RXRDY;
}
static volatile unsigned char mutex = 0;

unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len)
{
  unsigned short next_count;
  unsigned short current_count;
  unsigned short left;
  unsigned int save = disableIRQ();
  if (mutex) {
    restoreIRQ(save);
    return 0;	/* Buffer being updated */
  }
  mutex = 1;	/* Prevent interrupts from messing up the transmission */
  *AT91C_DBGU_PTCR =AT91C_PDC_TXTDIS; /* Stop transmitting */
  while(*AT91C_DBGU_PTSR & AT91C_PDC_TXTEN); /* Wait until stopped */
  next_count = *AT91C_DBGU_TNCR;
  current_count = *AT91C_DBGU_TCR;

  left = DBG_XMIT_BUFFER_LEN - next_count - current_count;
  if (left > 0) {
    if (left < len) len = left;
    if (next_count > 0) {
      /* Buffer is wrapped */
      memcpy(&dbg_xmit_buffer[next_count], seq, len);
      *AT91C_DBGU_TNCR = next_count + len;
    } else {
      unsigned char *to = ((unsigned char*)*AT91C_DBGU_TPR) + current_count;
      left = &dbg_xmit_buffer[DBG_XMIT_BUFFER_LEN] - to;
      if (len > left) {
	unsigned int wrapped = len - left;
	memcpy(to, seq, left);
	memcpy(dbg_xmit_buffer, &seq[left], wrapped);
	*AT91C_DBGU_TCR = current_count + left;
	*AT91C_DBGU_TNCR = wrapped;
      } else {
	memcpy(to, seq, len);
	*AT91C_DBGU_TCR = current_count + len;
      }
    }
  } else {
    len = 0;
  }

  *AT91C_DBGU_PTCR =AT91C_PDC_TXTEN; /* Restart transmission */
  mutex = 0;
  restoreIRQ(save);
  return len;
}
static unsigned char dbg_write_overrun = 0;

void
dbg_putchar(const char ch)
{
  if (dbg_write_overrun) {
    if (dbg_send_bytes((const unsigned char*)"^",1) != 1) return;
  }
  dbg_write_overrun = 0;
  if (dbg_send_bytes((const unsigned char*)&ch,1) != 1) {
    dbg_write_overrun = 1;
  }
}

void
dbg_blocking_putchar(const char ch)
{
  if (dbg_write_overrun) {
    while (dbg_send_bytes((const unsigned char*)"^",1) != 1);
  }
  dbg_write_overrun = 0;
  while (dbg_send_bytes((const unsigned char*)&ch,1) != 1);
}

#if 0
static StrFormatResult
dbg_write_cb(void *user_data, const char *data, unsigned int len)
{
  if (dbg_send_bytes((const unsigned char*)data, len) != len) {
    dbg_write_overrun = 1;
    return STRFORMAT_FAILED;
  }
  return STRFORMAT_OK;
}

void
dbg_printf(const char *format, ...)
{
  static const StrFormatContext ctxt = {dbg_write_cb, NULL};
  va_list ap;
  if (dbg_write_overrun) {
    if (dbg_send_bytes((const unsigned char*)"^",1) != 1) return;
  }
  dbg_write_overrun = 0;
  va_start(ap, format);
  format_str_v(&ctxt, format, ap);
  va_end(ap);
}

static StrFormatResult
dbg_write_blocking_cb(void *user_data, const char *data, unsigned int len)
{
  unsigned int left = len;
  while(left > 0) {
    unsigned int sent = dbg_send_bytes((const unsigned char*)data, left);
    left -= sent;
    data += sent;
  }
  return STRFORMAT_OK;
}

void
dbg_blocking_printf(const char *format, ...)
{
  static const StrFormatContext ctxt = {dbg_write_blocking_cb, NULL};
  va_list ap;
  if (dbg_write_overrun) {
    while (dbg_send_bytes((const unsigned char*)"^",1) != 1);
  }
  dbg_write_overrun = 0;
  va_start(ap, format);
  format_str_v(&ctxt, format, ap);
  va_end(ap);
}
#endif
void
dbg_drain()
{
  while(!(*AT91C_DBGU_CSR & AT91C_US_TXBUFE));
}

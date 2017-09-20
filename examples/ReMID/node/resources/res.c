/////////////////////////////////////////////////////////////////////////IMPORTS/////////////////////////////////////////////////////////////
#include "net/ip/ip64-addr.h"
#include "er-coap-engine.h" /* needed for rest-init-engine */
#include "res.h"
#include "../rest-engine/rest-engine.h"
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>


///////////////////////////////////////////////////////////////METHOD DEFINTIONS/////////////////////////////////////////////////////////////
int
res_string_to_linkaddr(char* address, unsigned int size, linkaddr_t* lladdr) {
  char *end;
  char *start = address;
  unsigned int count = 0;
  unsigned char byte;
  while((byte = (unsigned char)strtol(start,&end,16)) && end-start < 3) {
    count++;
    lladdr->u8[count-1] = byte;
    if (count < LINKADDR_SIZE && *end == ':') {
      end++;
    } else if (count == LINKADDR_SIZE && *end != ':') {
      return 1;
    }
    start = end;
  }
  return 0;
}


/* Utility function for json parsing */
int
res_json_find_field(struct jsonparse_state *js, char *field_buf, int field_buf_len)
{
  int state = jsonparse_next(js);
  while(state) {
    switch(state) {
    case JSON_TYPE_PAIR_NAME:
      jsonparse_copy_value(js, field_buf, field_buf_len);
      /* Move to ":" */
      jsonparse_next(js);
      /* Move to value and return its type */
      return jsonparse_next(js);
    default:
      return state;
    }
    state = jsonparse_next(js);
  }
  return 0;
}


uint8_t
res_reply_char_if_possible(char c, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset)
{
  if(*strpos >= *offset && *bufpos < bufsize) {
    buffer[(*bufpos)++] = c;
  }
  ++(*strpos);
  return 1;
}


uint8_t
res_reply_string_if_possible(char *s, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset)
{
  if(*strpos + strlen(s) > *offset) {
    (*bufpos) += snprintf((char*)buffer + (unsigned int)(*bufpos),
                       (unsigned int)bufsize - (unsigned int)(*bufpos) + 1,
                       "%s",
                       s
                       + (*offset - (int32_t)(*strpos) > 0 ?
                          *offset - (int32_t)(*strpos) : 0));
    if(*bufpos >= bufsize) {
      return 0;
    }
  }
  *strpos += strlen(s);
  return 1;
}


uint8_t
res_reply_02hex_if_possible(unsigned int hex, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset)
{
  int hexlen = 0;
  unsigned int temp_hex = hex;
  while(temp_hex > 0) {
    hexlen++;
    temp_hex = temp_hex>>4;
  }
  if(hexlen%2 == 1)
    hexlen++;
  int mask = 0;
  int i = hexlen - (int)*offset + (int)(*strpos);
  while(i>0) {
    mask = mask<<4;
    mask = mask | 0xF;
    i--;
  }
  if(*strpos + hexlen > *offset) {
    if((hexlen - (int)*offset + (int)(*strpos))%2 == 0)
      (*bufpos) += snprintf((char *)buffer + (*bufpos),
                       bufsize - (*bufpos) + 1,
                       "%02x",
                       (*offset - (int32_t)(*strpos) > 0 ?
                          hex & mask : hex));
    else
      (*bufpos) += snprintf((char *)buffer + (*bufpos),
                       bufsize - (*bufpos) + 1,
                       "%x",
                       (*offset - (int32_t)(*strpos) > 0 ?
                          hex & mask : hex));
    if(*bufpos >= bufsize) {
      return 0;
    }
  }
  *strpos += hexlen;
  return 1;
}


uint32_t embedded_pow10(x)
{
  if(x==1)
    return 10;
  else if(x==2)
    return 100;
  else if(x==3)
    return 1000;
  else if(x==4)
    return 10000;
  else if(x==5)
    return 100000;
  else if(x==6)
    return 1000000;
  else
    return 0;
}


uint8_t
res_reply_uint16_if_possible(uint16_t d, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset)
{
  int len = 1;
  uint16_t temp_d = d;
  while(temp_d > 9) {
    len++;
    temp_d /= 10;
  }
  if(*strpos + len > *offset) {
    (*bufpos) += snprintf((char *)buffer + (*bufpos),
                       bufsize - (*bufpos) + 1,
                       "%" PRIu16,
                       (*offset - (int32_t)(*strpos) > 0 ?
                          (uint16_t)d % (uint16_t)embedded_pow10(len - *offset + (int32_t)(*strpos)) : (uint16_t)d));
    if(*bufpos >= bufsize) {
      return 0;
    }
  }
  *strpos += len;
  return 1;
}


void
res_reply_lladdr_if_possible(const linkaddr_t *lladdr, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset)
{
#if LINKADDR_SIZE == 2
    res_reply_02hex_if_possible((unsigned int)((*lladdr)&0xFF), buffer, bufpos, bufsize, strpos, offset);
    res_reply_char_if_possible(':', buffer, bufpos, bufsize, strpos, offset);
    res_reply_02hex_if_possible((unsigned int)(((*lladdr)>>8)&0xFF), buffer, bufpos, bufsize, strpos, offset);
#else
  unsigned int i;
  for(i = 0; i < LINKADDR_SIZE; i++) {
    if(i > 0) {
      res_reply_char_if_possible(':', buffer, bufpos, bufsize, strpos, offset);
    }
    res_reply_02hex_if_possible((unsigned int)lladdr->u8[i], buffer, bufpos, bufsize, strpos, offset);
  }
#endif
}


uint8_t
res_reply_ip_if_possible(const uip_ipaddr_t *addr, uint8_t *buffer, size_t *bufpos, uint16_t bufsize, size_t *strpos, int32_t *offset)
{
#if NETSTACK_CONF_WITH_IPV6
  uint16_t a;
  unsigned int i;
  int f;
#endif /* NETSTACK_CONF_WITH_IPV6 */
  if(addr == NULL) {
    return 0;
  }
#if NETSTACK_CONF_WITH_IPV6
  if(ip64_addr_is_ipv4_mapped_addr(addr)) {
    res_reply_string_if_possible("::FFFF:", buffer, bufpos, bufsize, strpos, offset);
    res_reply_uint16_if_possible((uint16_t)addr->u8[12], buffer, bufpos, bufsize, strpos, offset);
    res_reply_char_if_possible('.', buffer, bufpos, bufsize, strpos, offset);
    res_reply_uint16_if_possible((uint16_t)addr->u8[13], buffer, bufpos, bufsize, strpos, offset);
    res_reply_char_if_possible('.', buffer, bufpos, bufsize, strpos, offset);
    res_reply_uint16_if_possible((uint16_t)addr->u8[14], buffer, bufpos, bufsize, strpos, offset);
    res_reply_char_if_possible('.', buffer, bufpos, bufsize, strpos, offset);
    res_reply_uint16_if_possible((uint16_t)addr->u8[15], buffer, bufpos, bufsize, strpos, offset);

  } else {
    for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
      a = (addr->u8[i] << 8) + addr->u8[i + 1];
      if(a == 0 && f >= 0) {
        if(f++ == 0) {
          res_reply_string_if_possible("::", buffer, bufpos, bufsize, strpos, offset);
//          PRINTA("::");
        }
      } else {
        if(f > 0) {
          f = -1;
        } else if(i > 0) {
          res_reply_char_if_possible(':', buffer, bufpos, bufsize, strpos, offset);
//          PRINTA(":");
        }
        res_reply_02hex_if_possible((uint16_t)a, buffer, bufpos, bufsize, strpos, offset);
//        PRINTA("%x", a);
      }
    }
  }
#else /* NETSTACK_CONF_WITH_IPV6 */
  res_reply_uint16_if_possible((uint16_t)addr->u8[0], buffer, bufpos, bufsize, strpos, offset);
  res_reply_char_if_possible('.', buffer, bufpos, bufsize, strpos, offset);
  res_reply_uint16_if_possible((uint16_t)addr->u8[1], buffer, bufpos, bufsize, strpos, offset);
  res_reply_char_if_possible('.', buffer, bufpos, bufsize, strpos, offset);
  res_reply_uint16_if_possible((uint16_t)addr->u8[2], buffer, bufpos, bufsize, strpos, offset);
  res_reply_char_if_possible('.', buffer, bufpos, bufsize, strpos, offset);
  res_reply_uint16_if_possible((uint16_t)addr->u8[3], buffer, bufpos, bufsize, strpos, offset);
//  PRINTA("%u.%u.%u.%u", addr->u8[0], addr->u8[1], addr->u8[2], addr->u8[3]);
#endif /* NETSTACK_CONF_WITH_IPV6 */
  return 1;
}

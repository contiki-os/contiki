
#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include "8051def.h"
#include "sys/cc.h"
#include <ctype.h>
#include <string.h>
#include "log.h"

/* Time type. */
/*typedef unsigned long clock_time_t;*/
typedef unsigned short clock_time_t;

/* Defines tick counts for a second. */
#define CLOCK_CONF_SECOND		128

#define rtimer_arch_now()	clock_time()

/* Memory filesystem RAM size. */
#define CFS_RAM_CONF_SIZE		512

/* Logging.. */
#define LOG_CONF_ENABLED		0

/* XXX argh, ugly hack to make stuff compile! */
#define snprintf(BUF, SIZE, ...) sprintf(BUF, __VA_ARGS__)

#endif /* __CONTIKI_CONF_H__ */

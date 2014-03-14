#ifndef SDCARD_H_KJBBKS6O57__
#define SDCARD_H_KJBBKS6O57__

#include <sys/process.h>

PROCESS_NAME(sdcard_process);

void
sdcard_init();

/* True if a card is ready to read or write */
int
sdcard_ready();

extern process_event_t sdcard_inserted_event;

extern process_event_t sdcard_removed_event;

void
sdcard_event_process(struct process *p);

#endif /* SDCARD_H_KJBBKS6O57__ */

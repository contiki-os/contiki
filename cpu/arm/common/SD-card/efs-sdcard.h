#ifndef __SDCARD_H__KJBBKS6O57__
#define __SDCARD_H__KJBBKS6O57__

#include <efs.h>
#include <ls.h>
#include <sys/process.h>

PROCESS_NAME(sdcard_process);

void
efs_sdcard_init();

FileSystem *
efs_sdcard_get_fs();

/* True if a card is ready to read or write */
int
sdcard_ready();

extern process_event_t sdcard_inserted_event;

extern process_event_t sdcard_removed_event;

void
sdcard_event_process(struct process *p);

#endif /* __SDCARD_H__KJBBKS6O57__ */

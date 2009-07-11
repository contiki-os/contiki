#include <efs-sdcard.h>
#include <sys/process.h>
#include <sys/etimer.h>
#include <cfs/cfs.h>
#include <debug-uart.h>
#include <efs.h>
#include <ls.h>
#include <stdio.h>



process_event_t sdcard_inserted_event;

process_event_t sdcard_removed_event;

static struct process *event_process = NULL;


#define MAX_FDS 4

static File file_descriptors[MAX_FDS];

static int
find_free_fd()
{
  int fd;
  for (fd = 0; fd < MAX_FDS; fd++) {
    if (!file_getAttr(&file_descriptors[fd], FILE_STATUS_OPEN)) {
      return fd;
    }
  }
  return -1;
}

static File *
get_file(int fd)
{
  if (!sdcard_ready()) return 0;
  if (fd >= MAX_FDS || fd < 0) return NULL;
  if (!file_getAttr(&file_descriptors[fd], FILE_STATUS_OPEN)) return NULL;
  return &file_descriptors[fd];
}

int
cfs_open (const char *name, int flags)
{
  eint8 mode;
  int fd;
  if (!sdcard_ready()) return -1;
  fd = find_free_fd();
  if (fd < 0) return -1;
  if (flags == CFS_READ) {
    mode = MODE_READ;
  } else {
    mode = MODE_APPEND;
  }
  if (file_fopen(&file_descriptors[fd], &sdcard_efs.myFs,
		 (char*)name, mode) < 0) {
    return -1;
  }
  return fd;
}

void
cfs_close(int fd)
{
  File *file = get_file(fd);
  if (!file) return;
  file_fclose(file);
  fs_flushFs(efs_sdcard_get_fs());
}

int
cfs_read (int fd, void *buf, unsigned int len)
{
  File *file = get_file(fd);
  if (!file) return 0;
  return file_read(file, len, (euint8*)buf);
}

int
cfs_write (int fd, const void *buf, unsigned int len)
{
  File *file = get_file(fd);
  if (!file) return 0;
  return file_write(file, len, (euint8*)buf);
}

cfs_offset_t
cfs_seek (int fd, cfs_offset_t offset, int whence)
{
  File *file;
  if (whence != CFS_SEEK_SET) return -1;
  file = get_file(fd);
  if (!file) return 0;
  if (file_setpos(file, offset) != 0) return -1;
  return file->FilePtr;
}


/* Cause a compile time error if expr is false */
#ifdef __GNUC__
#define COMPILE_TIME_CHECK(expr) \
(void) (__builtin_choose_expr ((expr), 0, ((void)0))+3)
#else
#define COMPILE_TIME_CHECK(expr)
#endif

#define MAX_DIR_LISTS 4
DirList dir_lists[MAX_DIR_LISTS];

static DirList *
find_free_dir_list()
{
  unsigned int l;
  for(l = 0; l < MAX_DIR_LISTS; l++) {
    if (dir_lists[l].fs == NULL) {
      return &dir_lists[l];
    }
  }
  return NULL;
}

int
cfs_opendir (struct cfs_dir *dirp, const char *name)
{
  DirList *dirs;
  COMPILE_TIME_CHECK(sizeof(DirList*) <= sizeof(struct cfs_dir));
  if (!sdcard_ready()) return -1;
  dirs = find_free_dir_list();
  if (!dirs) return -1;
  if (ls_openDir(dirs, efs_sdcard_get_fs(), (eint8*)name) != 0) {
    dirs->fs = NULL;
    return -1;
  }
  *(DirList**)dirp = dirs;
  return 0;
}

int
cfs_readdir (struct cfs_dir *dirp, struct cfs_dirent *dirent)
{
  euint8 *start;
  euint8 *end;
  char *to = dirent->name;
  DirList *dirs = *(DirList**)dirp;
  if (!sdcard_ready()) return 1;
  if (ls_getNext(dirs) != 0) return 1;
  start = dirs->currentEntry.FileName;
  end = start + 7;
  while(end > start) {
    if (*end > ' ') {
      end++;
      break;
    }
    end--;
  }
  while(start < end) {
    *to++ = *start++;
  }
  start = dirs->currentEntry.FileName + 8;
  end = start + 3;
  if (*start > ' ') {
    *to++ = '.';
    *to++ = *start++;
    while(start < end && *start > ' ') {
      *to++ = *start++;
    }
  }
  *to = '\0';
  if (dirs->currentEntry.Attribute & ATTR_DIRECTORY) {
    dirent->size = 0;
  } else {
    dirent->size = dirs->currentEntry.FileSize;
  }
  return 0;
}

void
cfs_closedir (struct cfs_dir *dirp)
{
  (*(DirList**)dirp)->fs = NULL;
}

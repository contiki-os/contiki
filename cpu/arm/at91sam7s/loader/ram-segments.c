#ifndef __RAM_SEGMENTS_C__1POIF5E8U4__
#define __RAM_SEGMENTS_C__1POIF5E8U4__

#include <loader/elfloader-otf.h>
#include <loader/codeprop-otf.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>

struct ram_output
{
  struct elfloader_output output;
  char *base;
  unsigned int offset;
  void *text;
  void *rodata;
  void *data;
  void *bss;
};

static void *
allocate_segment(struct elfloader_output * const output,
		 unsigned int type, int size)
{
  struct ram_output * const ram = (struct ram_output *)output;
  void *block = malloc(size);
  if (!block) return NULL;
  switch(type) {
  case ELFLOADER_SEG_TEXT:
    if (ram->text) free(ram->text);
    ram->text = block;
    break;
  case ELFLOADER_SEG_RODATA:
    if (ram->rodata) free(ram->rodata);
    ram->rodata = block;
    break;
  case ELFLOADER_SEG_DATA:
    if (ram->data) free(ram->data);
    ram->data = block;
    break;
  case ELFLOADER_SEG_BSS:
    if (ram->bss) free(ram->bss);
    ram->bss = block;
    break;
  default:
    free(block);
    return NULL;
  }
  return block;
}

static int
start_segment(struct elfloader_output *output,
			unsigned int type, void *addr, int size)
{
  ((struct ram_output*)output)->base = addr;
  ((struct ram_output*)output)->offset = 0;
  return ELFLOADER_OK;
}

static int
end_segment(struct elfloader_output *output)
{
  return ELFLOADER_OK;
}

static int
write_segment(struct elfloader_output *output, const char *buf,
	      unsigned int len)
{
  struct ram_output * const ram = (struct ram_output *)output;
  memcpy(ram->base + ram->offset, buf, len);
  ram->offset += len;
  return len;
}

static unsigned int
segment_offset(struct elfloader_output *output)
{
  return ((struct ram_output*)output)->offset;
}

static const struct elfloader_output_ops elf_output_ops =
  {
    allocate_segment,
    start_segment,
    end_segment,
    write_segment,
    segment_offset
  };


static struct ram_output seg_output = {
  {&elf_output_ops},
  NULL,
  0,
  NULL,
  NULL,
  NULL,
  NULL
};

PROCESS(ram_segments_cleanup_process, "RAM segments cleanup process");

PROCESS_THREAD(ram_segments_cleanup_process, ev, data)
{
  PROCESS_BEGIN();
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXITED
			     || ev == PROCESS_EVENT_EXIT);
    if (ev == PROCESS_EVENT_EXIT) break;
    if (elfloader_autostart_processes ||
	elfloader_autostart_processes[0] == data) {
      PROCESS_PAUSE(); /* Let the process exit */
      if (seg_output.text) {
	free(seg_output.text);
	seg_output.text = NULL;
      }
      if (seg_output.rodata) {
	free(seg_output.rodata);
	seg_output.rodata = NULL;
      }
      if (seg_output.data) {
	free(seg_output.data);
	seg_output.data = NULL;
      }
      
      if (seg_output.bss) {
	free(seg_output.bss);
	seg_output.bss = NULL;
      }
      elfloader_autostart_processes = NULL;
    }
  }
  PROCESS_END();
}
struct elfloader_output *codeprop_output = &seg_output.output;

#endif /* __RAM_SEGMENTS_C__1POIF5E8U4__ */

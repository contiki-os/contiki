#ifndef _IHEX_H
#define _IHEX_H

extern int hexfile_parse(char *line, unsigned char *type, unsigned int *addr, char *buffer);
extern int hexfile_out(char *line, unsigned int type, unsigned int address, unsigned char *data, unsigned int bytes);

#endif /*_IHEX_H*/

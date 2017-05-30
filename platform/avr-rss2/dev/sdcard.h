void
sd_init(void);

uint16_t 
sd_write(char* filename, char *data);

void 
sd_clear_file(char * filename);

void
sd_read_line(char *filename, char [] buf)
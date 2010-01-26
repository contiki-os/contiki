#ifndef CONVERTER_H
#define CONVERTER_H

#define CONVERTER_VERSION "v1.3"

typedef struct {
	int target_type;
	char ihex_file[128];
}conf_opts_t;

enum target { UNDEFINED, VERSION, CONVERT };

#endif

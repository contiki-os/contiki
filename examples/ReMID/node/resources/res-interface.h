#define NOT_ACCEPTABLE_406 "NOT_ACCEPTABLE_406"
static char content[REST_MAX_CHUNK_SIZE];
static int content_len=0;

#define CONTENT_PRINTF(...) { if(content_len < sizeof(content)) content_len += snprintf(content+content_len, sizeof(content)-content_len, __VA_ARGS__); }

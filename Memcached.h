#include <stdio.h>
#include <stdlib.h>

#define BUFLENGTH 1500
int tcp_init();

int add_cache(char *key, int flags, int exptime, int bytes, char *data);
int set_cache(char *key, int flags, int exptime, int bytes, char *data);
int replace_cache(char *key, int flags, int exptime, int bytes, char *data);

void get_cache(char *keys, char *get_cache);

int delete_cache(char *key);

int flush_all();

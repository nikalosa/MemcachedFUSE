#include <stdio.h>
#include <stdlib.h>

int tcp_init();

int add_cache(char *key, int flags, int exptime, int bytes, char *data);
int set_cache(char *key, int flags, int exptime, int bytes, char *data);
int replace_cache(char *key, int flags, int exptime, int bytes, char *data);
int append_cache(char *key, int flags, int exptime, int bytes, char *data);
int prepend_cache(char *key, int flags, int exptime, int bytes, char *data);

char *get_cache(char *keys);
int gets_cache(char *keys);

int delete_cache(char *key);

int incr_cache(char *key, int val);
int decr_cache(char *key, int val);

int touch_cache(char *key, int exptime);

int flush_all();

int stats_cache();

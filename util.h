#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOD 1000000007
#define PRIME 257

int hash_str(char *str);

int string_to_int(char *num);

char *int_to_string(int num);

char *name_from_path(char *path);

char *get_obj(char *resp);

char *parent_from_path(char *path);
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOD 1000000007
#define PRIME 257

int hash_str(char *str);

int string_to_int(char *num);

void int_to_string(int num, char *str_num);

void name_from_path(char *path, char *name);

int get_obj(char *resp, char *obj);

void parent_from_path(char *path, char *name);

void get_attr_hash(char *path, char *key, char *req_key);

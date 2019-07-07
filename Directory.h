#include <stdio.h>
#include <stdlib.h>

struct directory
{
    int dir_hash;
    int chunk_numb;
    char name[50];
};

void init_dir(struct directory *dir, int hash, char *name);

struct directory *make_dir(char *path);

int is_dir(char *path);

void add_object(struct directory *dir, int new_obj_hash);

struct chunk *get_suited_chunk();

char *read_dir(char *path);

struct directory *path_to_dir(char *path);

struct directory *hash_to_dir(int hash);
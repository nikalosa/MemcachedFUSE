#include <stdio.h>
#include <stdlib.h>

// const size_t MAX_LEN = (1024 - sizeof(size_t) - 3 * sizeof(int));

struct chunk
{
    int isdir;
    int hash;
    int size;
    int ind;
    char data[(1024 - 4 * sizeof(int))];
};

void init_chunk(struct chunk *chunk, int isdir, int hash, int ind);

void chunk_add_data(struct chunk *chunk, char *data, int elem_len, int str_size);

struct chunk *create_new_chunk(int num, int isdir, int hash);

char *get_chunk_hash(int numb, int hash);

struct chunk *get_chunk(int ind, int hash);

int can_fit(struct chunk *chunk, int bytes);
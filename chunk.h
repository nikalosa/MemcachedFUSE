#include <stdio.h>
#include <stdlib.h>

#define CHUNK_LEN 1024
#define DATA_LEN 1008

struct chunk
{
    int isdir;
    int hash;
    int size;
    int ind;
    char data[DATA_LEN];
};

void init_chunk(struct chunk *chunk, int isdir, int hash, int ind);

void chunk_add_data(struct chunk *chunk, char *src, int elem_len);

void chunk_write_data(struct chunk *chunk, char *src, int n, off_t chunk_offset, off_t src_offset);

void chunk_get_data(struct chunk *chunk, char *dst, int n, off_t chunk_offset, off_t dst_offset);

void create_new_chunk(int num, int isdir, int hash, struct chunk *chunk);

void get_chunk_hash(int numb, int hash, char *hash_str);

void get_chunk(int ind, int hash, struct chunk *chunk);

int can_fit(struct chunk *chunk, int bytes);

void chunk_replace(struct chunk *chunk);

void chunk_delete(struct chunk *chunk);

void chunk_set(struct chunk *chunk);
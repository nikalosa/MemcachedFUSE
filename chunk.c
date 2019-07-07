#include "chunk.h"
#include "Memcached.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_chunk(struct chunk *chunk, int isdir, int hash, int ind)
{
    chunk->size = 0;
    chunk->hash = hash;
    chunk->ind = ind;
    chunk->isdir = isdir;
}

void chunk_add_data(struct chunk *chunk, char *data, int elem_len, int str_size)
{
    memcpy(chunk->data + chunk->size, data, elem_len);
    // printf("%d ae", chunk->size / 4);
    chunk->size += elem_len;
    // printf("%d \n", chunk->size / 4);
    replace_cache(get_chunk_hash(chunk->ind, chunk->hash), 0, 0, str_size, (char *)chunk);
    chunk = get_chunk(chunk->ind, chunk->hash);
    // printf("%d\n", chunk->size / 4);
}

struct chunk *create_new_chunk(int ind, int isdir, int hash)
{
    char *chunk_hash = get_chunk_hash(ind, hash);
    struct chunk *chunk = malloc(sizeof(struct chunk));
    init_chunk(chunk, isdir, hash, ind);
    // MEMCACHE ADD
    add_cache(chunk_hash, 0, 0, 1024, (char *)chunk);
    return chunk;
}

int can_fit(struct chunk *chunk, int bytes)
{
    return (chunk->size + bytes) < ((1024 - 4 * sizeof(int)));
}

struct chunk *get_chunk(int ind, int hash)
{
    char *chunk_hash = get_chunk_hash(ind, hash);
    return (struct chunk *)get_obj(get_cache(chunk_hash));
}

char *get_chunk_hash(int numb, int hash)
{
    char *hash_str = int_to_string(hash);
    hash_str = realloc(hash_str, sizeof(hash_str) + 11);
    strcat(hash_str, "A");
    strcat(hash_str, int_to_string(numb));
    return hash_str;
}
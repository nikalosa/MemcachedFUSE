#include "chunk.h"
#include "Memcached.h"
#include "util.h"
#include <sys/types.h>

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

void chunk_add_data(struct chunk *chunk, char *src, int elem_len, int str_size)
{
    memcpy(chunk->data + chunk->size, src, elem_len);
    chunk->size += elem_len;
    chunk_replace(chunk);
}

void chunk_write_data(struct chunk *chunk, char *src, int n, off_t chunk_offset, off_t src_offset)
{
    memcpy(chunk->data + chunk_offset, src + src_offset, n);
    if (chunk->size < chunk_offset + n)
        chunk->size = chunk_offset + n;
    chunk_replace(chunk);
}

void chunk_get_data(struct chunk *chunk, char *dst, int n, off_t chunk_offset, off_t dst_offset)
{
    memcpy(dst + dst_offset, chunk->data + chunk_offset, n);
}

void chunk_replace(struct chunk *chunk)
{
    replace_cache(get_chunk_hash(chunk->ind, chunk->hash), 0, 0, sizeof(struct chunk), (char *)chunk);
}

void chunk_delete(struct chunk *chunk)
{
    delete_cache(get_chunk_hash(chunk->ind, chunk->hash));
}

struct chunk *create_new_chunk(int ind, int isdir, int hash)
{
    char *chunk_hash = get_chunk_hash(ind, hash);
    struct chunk *chunk = malloc(sizeof(struct chunk));
    memset(chunk, 0, sizeof(struct chunk));
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
    printf("%d %d\n", ind, hash);
    char *chunk_hash = get_chunk_hash(ind, hash);
    return (struct chunk *)get_obj(get_cache(chunk_hash));
}

char *get_chunk_hash(int numb, int hash)
{
    char *hash_str = int_to_string(hash);
    //int len = strlen(hash_str);
    //hash_str = realloc(hash_str, 30);
    //printf("%d ait\n", len);
    //hash_str[len] = '\0';
    strcat(hash_str, "A");
    strcat(hash_str, int_to_string(numb));
    return hash_str;
}
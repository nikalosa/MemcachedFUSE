#include "chunk.h"
#include "Memcached.h"
#include "Directory.h"
#include "util.h"
#include "hardlink.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_chunk(struct chunk *chunk, int isdir, int hash, int ind)
{
    if (isdir)
        memset(chunk->data, 0, DATA_LEN);
    if (isdir && ind == 1)
    {
        chunk->size = 20;
        char hard_hash[30];
        get_hard_hash(ind, hash, hard_hash);
        char len[4];
        len[0] = len[1] = len[2] = len[3] = '\0';
        sprintf(len, "%d", (int)strlen(hard_hash));
        strcat(len, "S");
        strcat(chunk->data, len);
        strcat(chunk->data, hard_hash);
        // printf("\nRILLY NIGGA? %s\n", chunk->data);
        while (strlen(chunk->data) != chunk->size)
            strcat(chunk->data, "H");
        struct hard_link hlink;
        init_hard(&hlink, hash, ind);
        add_hard(&hlink, hash);
    }
    else
    {
        chunk->size = 0;
    }
    chunk->hash = hash;
    chunk->ind = ind;
    chunk->isdir = isdir;
}

void chunk_add_data(struct chunk *chunk, char *src, int elem_len)
{
    memcpy(chunk->data + chunk->size, src, elem_len);
    chunk->size += elem_len;
    chunk_replace(chunk);
    get_chunk(1, chunk->hash, chunk);
}

void chunk_write_data(struct chunk *chunk, char *src, int n, off_t chunk_offset, off_t src_offset)
{
    memcpy(chunk->data + chunk_offset, src + src_offset, n);
    chunk->size = chunk_offset + n;
    chunk_set(chunk);
}

void chunk_get_data(struct chunk *chunk, char *dst, int n, off_t chunk_offset, off_t dst_offset)
{
    memcpy(dst + dst_offset, chunk->data + chunk_offset, n);
}

void chunk_replace(struct chunk *chunk)
{
    char chunk_hash[30];
    memset(chunk_hash, 0, 30);
    get_chunk_hash(chunk->ind, chunk->hash, chunk_hash);
    replace_cache(chunk_hash, 0, 0, sizeof(struct chunk), (char *)chunk);
}

void chunk_set(struct chunk *chunk)
{
    char chunk_hash[30];
    memset(chunk_hash, 0, 30);
    get_chunk_hash(chunk->ind, chunk->hash, chunk_hash);
    set_cache(chunk_hash, 0, 0, sizeof(struct chunk), (char *)chunk);
}

void chunk_delete(struct chunk *chunk)
{
    char chunk_hash[30];
    memset(chunk_hash, 0, 30);
    get_chunk_hash(chunk->ind, chunk->hash, chunk_hash);
    delete_cache(chunk_hash);
}

void create_new_chunk(int ind, int isdir, int hash, struct chunk *chunk)
{
    char chunk_hash[30];
    memset(chunk_hash, 0, 30);
    get_chunk_hash(ind, hash, chunk_hash);
    memset(chunk, 0, sizeof(struct chunk));
    init_chunk(chunk, isdir, hash, ind);
    add_cache(chunk_hash, 0, 0, CHUNK_LEN, (char *)chunk);
    get_chunk(ind, hash, chunk);
    return;
}

int can_fit(struct chunk *chunk, int bytes)
{
    return (chunk->size + bytes) < DATA_LEN;
}

void get_chunk(int ind, int hash, struct chunk *chunk)
{
    char chunk_hash[30];
    memset(chunk_hash, 0, 30);
    get_chunk_hash(ind, hash, chunk_hash);
    char buf[BUFLENGTH];
    get_cache(chunk_hash, buf);
    get_obj(buf, (char *)chunk);
    return;
}

void get_chunk_hash(int numb, int hash, char *hash_str)
{
    int_to_string(hash, hash_str);
    strcat(hash_str, "A");
    char numbb[10];
    memset(numbb, 0, 10);
    int_to_string(numb, numbb);
    strcat(hash_str, numbb);
}

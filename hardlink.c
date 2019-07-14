#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "chunk.h"
#include "Directory.h"
#include "hardlink.h"
#include "Memcached.h"

void init_hard(struct hard_link *hlink, int hash, int ind)
{
    hlink->hard_link_size = 0;
    hlink->hash = hash;
    hlink->ind = ind;
    char hhash[30];
    memset(hhash, 0, 30);
    get_hard_hash(ind, hash, hhash);
    add_cache(hhash, 0, 0, sizeof(struct hard_link), (char *)hlink);
}

void get_hard_hash(int numb, int hash, char *hash_str)
{
    get_chunk_hash(numb, hash, hash_str);
    strcat(hash_str, "H");
}

int add_hard(struct hard_link *hlink, int hash)
{
    if (hlink->hard_link_size == (CHUNK_LEN - 8) / 4)
        return -1;
    hlink->data[hlink->hard_link_size++] = hash;
    char hhash[30];
    memset(hhash, 0, 30);
    get_hard_hash(hlink->ind, hlink->hash, hhash);
    replace_cache(hhash, 0, 0, sizeof(struct hard_link), (char *)hlink);
    return 0;
}

int del_hard(struct hard_link *hlink, int hash)
{
    for (int i = 0; i < hlink->hard_link_size; i++)
    {
        if (hlink->data[i] == hash)
        {
            hlink->data[i] = hlink->data[--hlink->hard_link_size];
            char hhash[30];
            memset(hhash, 0, 30);
            get_hard_hash(hlink->ind, hlink->hash, hhash);
            replace_cache(hhash, 0, 0, sizeof(struct hard_link), (char *)hlink);
            return (hlink->hard_link_size == 0) ? 1 : 0;
        }
    }
    return -1;
}

void change_hards(struct file *file, int new_chunk_numb)
{
    struct chunk chunk;
    get_chunk(1, file->real_hash, &chunk);
    struct hard_link hlink;
    get_hard_struct(&chunk, &hlink);
    for (int i = 0; i < hlink.hard_link_size; i++)
    {
        struct file filee;
        hash_to_file(hlink.data[i], &filee);
        filee.chunk_numb = new_chunk_numb;
        char numb[10];
        memset(numb, 0, 10);
        int_to_string(filee.file_hash, numb);
        replace_cache(numb, 0, 0, sizeof(struct file), (char *)&filee);
        hash_to_file(hlink.data[i], &filee);
    }
}

void path_to_hard_struct(char *path, struct hard_link *hlink)
{
    struct file file;
    path_to_file((char *)path, &file);
    struct chunk chunk;
    get_chunk(1, file.real_hash, &chunk);
    get_hard_struct(&chunk, hlink);
}

void get_hard_struct(struct chunk *chunk, struct hard_link *hlink)
{
    int len = (chunk->data[2] == 'S') ? 2 : 1;
    char nm[len];
    memset(nm, 0, len);
    strncpy(nm, chunk->data, len);
    int numb = string_to_int(nm);
    char hard_hash[20];
    memset(hard_hash, 0, 20);
    memcpy(hard_hash, chunk->data + len + 1, numb);
    char buf[1500];
    memset(buf, 0, 1500);
    get_cache(hard_hash, buf);
    get_obj(buf, (char *)hlink);
}
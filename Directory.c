#include "util.h"
#include "chunk.h"
#include "Directory.h"
#include "Memcached.h"

#include <stdio.h>
#include <stdlib.h>

void init_dir(struct directory *dir, int hash, char *name)
{
    dir->dir_hash = hash;
    strcpy(dir->name, name);
    dir->chunk_numb = 1;
    add_cache(int_to_string(hash), 0, 0, sizeof(*dir), (char *)dir);
    create_new_chunk(1, 1, hash);
}

struct directory *make_dir(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");

    struct directory *dir = malloc(sizeof(struct directory));
    init_dir(dir, hash_str(path), name_from_path(path));
    if (strlen(path) > 1)
    {
        struct directory *parent_dir = path_to_dir(parent_from_path(path));
        add_object(parent_dir, dir->dir_hash);
    }
    return dir;
}

int is_dir(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");

    return strlen(get_cache(int_to_string(hash_str(path)))) > 10;
}

struct chunk *get_suited_chunk(struct directory *dir)
{
    struct chunk *chunk = get_chunk(dir->chunk_numb, dir->dir_hash);
    if (can_fit(chunk, sizeof(int)))
        return chunk;

    dir->chunk_numb++;
    replace_cache(int_to_string(dir->dir_hash), 0, 0, sizeof(struct directory), (char *)dir);
    return create_new_chunk(dir->chunk_numb, 1, dir->dir_hash);
}

void add_object(struct directory *dir, int new_obj_hash)
{
    struct chunk *suited_chunk = get_suited_chunk(dir);
    chunk_add_data(suited_chunk, (char *)&new_obj_hash, sizeof(int), sizeof(struct chunk));
}

struct directory *path_to_dir(char *path)
{
    return hash_to_dir(hash_str(path));
}

struct directory *hash_to_dir(int hash)
{
    // printf("%d", hash);
    return (struct directory *)get_obj(get_cache(int_to_string(hash)));
}

char *read_dir(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");
    int len = 1024;
    char *read_dir = malloc(1024);
    struct directory *dir = path_to_dir(path);
    printf("%s\n", path);
    for (int i = 1; i <= dir->chunk_numb; i++)
    {
        // printf("%d", i);
        struct chunk *chunk = get_chunk(i, dir->dir_hash);
        printf("%d", chunk->size / 4);
        for (int j = 0; j < chunk->size / 4; j++)
        {
            int key_hash = *((int *)chunk->data + j);
            char *name = hash_to_dir(key_hash)->name;
            while (strlen(read_dir) + strlen(name) + 1 >= len)
            {
                len *= 2;
                read_dir = realloc(read_dir, len);
            }
            strcat(read_dir, " ");
            strcat(read_dir, name);
        }
    }
    return read_dir;
}
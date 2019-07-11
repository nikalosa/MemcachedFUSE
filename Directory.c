#include "util.h"
#include "chunk.h"
#include "Directory.h"
#include "Memcached.h"
#include <assert.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

void init_dir(struct directory *dir, int hash, char *name)
{
    dir->unused = 0;
    dir->dir_hash = hash;
    strcpy(dir->name, name);
    dir->chunk_numb = 1;
    add_cache(int_to_string(hash), 0, 0, sizeof(struct directory), (char *)dir);
    create_new_chunk(1, 1, hash);
}

int make_dir(char *path)
{

    if (strlen(path) == 1)
        path = strdup("");
    //     assert(strlen(path) == 0);
    // assert(strlen(name_from_path(path)) <= 50);
    struct directory *dir = malloc(sizeof(struct directory));
    memset(dir, 0, sizeof(struct directory));
    if (strlen(path) > 0)
    {
        // printf("%s\n", parent_from_path(path));
        struct directory *parent_dir = path_to_dir(parent_from_path(path));
        // printf("%s %d %d aeee\n", path, hash_str(path), hash_str(parent_from_path(path)));
        add_object(parent_dir, hash_str(path));
        dir->chunk_id = parent_dir->chunk_numb;
        // free(parent_dir);
    }
    else
        dir->chunk_id = 0;

    init_dir(dir, hash_str(path), name_from_path(path));
    // free(dir);
    return 0;
}

int is_dir(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");

    if (strlen(get_cache(int_to_string(hash_str(path)))) < 10)
        return 0;

    char *obj = hash_to_struct(hash_str(path));
    return *((int *)obj) == 0;
}

struct chunk *get_suited_chunk(struct directory *dir)
{
    struct chunk *chunk = get_chunk(dir->chunk_numb, dir->dir_hash);
    // printf("yleeeeeeeeeeeeeeeeeeeeee\n");
    if (can_fit(chunk, sizeof(int)))
        return chunk;
    // free(chunk);
    dir->chunk_numb++;
    replace_cache(int_to_string(dir->dir_hash), 0, 0, sizeof(struct directory), (char *)dir);
    return create_new_chunk(dir->chunk_numb, 1, dir->dir_hash);
}

void add_object(struct directory *dir, int new_obj_hash)
{
    struct chunk *suited_chunk = get_suited_chunk(dir);
    printf("Blaaaaaaaa\n");
    chunk_add_data(suited_chunk, (char *)&new_obj_hash, sizeof(int), sizeof(struct chunk));
    // free(suited_chunk);
}

struct directory *path_to_dir(char *path)
{
    return hash_to_dir(hash_str(path));
}

struct directory *hash_to_dir(int hash)
{
    return (struct directory *)hash_to_struct(hash);
}

char *hash_to_struct(int hash)
{
    // printf/("basdasdas\n");
    return get_obj(get_cache(int_to_string(hash)));
}

char *read_dir(char *path)
{
    // assert(0);
    if (strlen(path) == 1)
        path = strdup("");
    int len = 1024;
    char *read_dir = malloc(1024);
    memset(read_dir, 0, 1024);
    struct directory *dir = path_to_dir(path);
    for (int i = 1; i <= dir->chunk_numb; i++)
    {
        struct chunk *chunk = get_chunk(i, dir->dir_hash);
        if (i == 1 && chunk->size == 0)
            return NULL;

        for (int j = 0; j < chunk->size / 4; j++)
        {
            int key_hash = *((int *)chunk->data + j);
            char *name = hash_to_dir(key_hash)->name;
            while (strlen(read_dir) + strlen(name) + 1 >= len)
            {
                len *= 2;
                read_dir = realloc(read_dir, len);
                read_dir[len] = '\0';
            }
            strcat(read_dir, name);
            strcat(read_dir, " ");
        }
        // free(chunk);
    }
    return read_dir;
}

int rm_dir(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");
    int len = 1024;
    char *read_dir = malloc(1024);
    memset(read_dir, 0, 1024);
    struct directory *dir = path_to_dir(path);
    if (dir->chunk_numb > 1 || get_chunk(1, dir->dir_hash)->size > 0)
        return -1;

    delete_cache(int_to_string(dir->dir_hash));
    char *par = parent_from_path(path);
    struct directory *par_dir = path_to_dir(par);
    struct chunk *chunk = get_chunk(dir->chunk_id, par_dir->dir_hash);
    // struct chunk *chunk_last = get_chunk(par_dir->chunk_numb, par_dir->dir_hash);
    for (int j = 0; j < chunk->size / 4; j++)
    {
        int key_hash = *((int *)chunk->data + j);
        if (key_hash == dir->dir_hash)
        {
            memcpy((int *)chunk->data + j, (int *)chunk->data + chunk->size / 4 - 1, sizeof(int));
            // printf("%d ", key_hash);
            // int a_key_hash = *((int *)chunk->data + j);
            // printf("%d\n", key_hash);
            // printf("aeeee %d\n", ((int *)hash_to_struct(key_hash))[1]);
            chunk->size -= sizeof(int);
            // if (a_key_hash != key_hash && chunk_last->size)
            // {
            //     printf("alo\n");
            //     *((int *)hash_to_struct(key_hash)) = dir->chunk_id;
            // }
            // if (chunk_last->size == 0)
            // {
            // par_dir->chunk_numb--;
            // chunk_delete(chunk_last);
            // replace_cache(int_to_string(par_dir->dir_hash), 0, 0, sizeof(struct directory), (char *)par_dir);
            // }
            // else
            // {
            // }
            // chunk_replace(chunk_last);
            chunk_replace(chunk);
            // free(chunk);
            // free(chunk_last);
            // free(par_dir);
            return 0;
        }
    }
    return -1;
}

void init_file(struct file *file, int hash, char *name)
{
    file->unused = 1;
    file->file_hash = hash;
    strcpy(file->name, name);
    file->chunk_numb = 1;
    add_cache(int_to_string(hash), 0, 0, sizeof(struct file), (char *)file);
    create_new_chunk(1, 1, hash);
}

struct file *create_file(char *path)
{
    struct file *file = malloc(sizeof(struct file));
    memset(file, 0, sizeof(struct file));
    struct directory *parent_dir = path_to_dir(parent_from_path(path));
    add_object(parent_dir, hash_str(path));
    file->chunk_id = parent_dir->chunk_numb;
    init_file(file, hash_str(path), name_from_path(path));
    return file;
}

int mwrite(char *path, char *buf, size_t size, off_t offset)
{
    struct file *file = path_to_file(path);
    int written_bytes = 0;
    int chunk_index = offset / (1024 - 4 * sizeof(int)) + 1;
    off_t chunk_offset = offset % (1024 - 4 * sizeof(int));
    int new_chunks = (offset + size) / (1024 - 4 * sizeof(int)) + 1 - file->chunk_numb;
    if (new_chunks > 0)
    {
        for (int i = 1; i <= new_chunks; i++)
            create_new_chunk(file->chunk_numb + i, 0, file->file_hash);
        file->chunk_numb += new_chunks;
        replace_cache(int_to_string(file->file_hash), 0, 0, sizeof(struct file), (char *)file);
    }
    while (written_bytes < size)
    {
        int free_space = (1024 - 4 * sizeof(int)) - chunk_offset;
        struct chunk *chunk = get_chunk(chunk_index, file->file_hash);
        if (size - written_bytes < free_space)
            free_space = size - written_bytes;
        chunk_write_data(chunk, buf, free_space, chunk_offset, written_bytes);
        written_bytes += free_space;
        chunk_offset = 0;
        chunk_index++;
    }
    return written_bytes;
}

int mread(char *path, char *buf, size_t size, off_t offset)
{
    if (size > 100000)
    {
        size = 100000;
    }
    struct file *file = path_to_file(path);
    int read_bytes = 0;
    int chunk_index = offset / (1024 - 4 * sizeof(int)) + 1;
    off_t chunk_offset = offset % (1024 - 4 * sizeof(int));
    while (read_bytes < size)
    {
        if (chunk_index > file->chunk_numb)
            return read_bytes;

        struct chunk *chunk = get_chunk(chunk_index, file->file_hash);
        int free_space = (1024 - 4 * sizeof(int)) - chunk_offset;
        if (chunk_offset >= chunk->size)
            return 0;
        int br = 0;
        if (size - read_bytes < free_space)
            free_space = size - read_bytes;
        if (free_space + chunk_offset > chunk->size)
        {
            br = 1;
            free_space = chunk->size - chunk_offset;
        }
        chunk_get_data(chunk, buf, free_space, chunk_offset, read_bytes);
        read_bytes += free_space;
        buf[read_bytes] = '\0';
        chunk_offset = 0;
        chunk_index++;
        if (br)
            break;
    }
    return read_bytes;
}

int is_file(char *path)
{
    if (strlen(path) == 0 || strlen(get_cache(int_to_string(hash_str(path)))) < 10)
        return 0;
    return !is_dir(path);
}

struct file *path_to_file(char *path)
{
    return hash_to_file(hash_str(path));
}

struct file *hash_to_file(int hash)
{
    return (struct file *)hash_to_struct(hash);
}
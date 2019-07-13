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
    memset(dir->name, 0, 50);
    strcpy(dir->name, name);
    dir->chunk_numb = 1;
    char hash_str[10];
    memset(hash_str, 0, sizeof(10));
    int_to_string(hash, hash_str);
    add_cache(hash_str, 0, 0, sizeof(struct directory), (char *)dir);
    struct chunk chunk;
    create_new_chunk(1, 1, hash, &chunk);
    get_chunk(1, chunk.hash, &chunk);
}

int make_dir(char *path)
{

    if (strlen(path) == 1)
        path = strdup("");
    struct directory dir;
    if (strlen(path) > 0)
    {
        char parent[strlen(path)];
        memset(parent, 0, strlen(path));
        parent_from_path(path, parent);
        struct directory parent_dir;
        path_to_dir(parent, &parent_dir);
        add_object(&parent_dir, hash_str(path));
        dir.chunk_id = parent_dir.chunk_numb;
        // free(&parent_dir);
    }
    else
        dir.chunk_id = 0;

    char name[50];
    memset(name, 0, sizeof(50));
    name_from_path(path, name);
    init_dir(&dir, hash_str(path), name);
    // free(&dir);
    return 0;
}

int is_dir(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");

    char numb[10];
    memset(numb, 0, 10);
    int_to_string(hash_str(path), numb);
    // printf("%s :hash\n", numb);
    char buf[BUFLENGTH];
    memset(buf, 0, BUFLENGTH);
    get_cache(numb, buf);
    // printf("ait she bozo %d\n", strlen(buf));
    if (strlen(buf) < 10)
        return 0;

    char obj[CHUNK_LEN];
    hash_to_struct(hash_str(path), obj);
    return *((int *)obj) == 0;
}

void get_suited_chunk(struct directory *dir, struct chunk *chunk)
{
    get_chunk(dir->chunk_numb, dir->dir_hash, chunk);
    if (can_fit(chunk, sizeof(int)))
        return;

    dir->chunk_numb++;
    char numb[10];
    memset(numb, 0, 10);
    int_to_string(dir->dir_hash, numb);
    replace_cache(numb, 0, 0, sizeof(struct directory), (char *)dir);
    memset(chunk, 0, sizeof(struct chunk));
    create_new_chunk(dir->chunk_numb, 1, dir->dir_hash, chunk);
}

void add_object(struct directory *dir, int new_obj_hash)
{
    struct chunk *suited_chunk = malloc(sizeof(struct chunk));
    get_suited_chunk(dir, suited_chunk);
    chunk_add_data(suited_chunk, (char *)&new_obj_hash, sizeof(int));
    free(suited_chunk);
}

void path_to_dir(char *path, struct directory *dir)
{
    hash_to_dir(hash_str(path), dir);
}

void hash_to_dir(int hash, struct directory *dir)
{
    hash_to_struct(hash, (char *)dir);
}

void hash_to_struct(int hash, char *obj)
{
    char numb[10];
    memset(numb, 0, 10);
    int_to_string(hash, numb);
    char buf[BUFLENGTH];
    memset(buf, 0, BUFLENGTH);
    get_cache(numb, buf);
    get_obj(buf, obj);
}

char *read_dir(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");
    int len = CHUNK_LEN;
    char *read_dir = malloc(CHUNK_LEN);
    memset(read_dir, 0, CHUNK_LEN);
    struct directory dir;
    path_to_dir(path, &dir);
    for (int i = 1; i <= dir.chunk_numb; i++)
    {
        struct chunk chunk;
        get_chunk(i, dir.dir_hash, &chunk);
        if (i == 1 && chunk.size == 0)
            return NULL;

        for (int j = 0; j < chunk.size / 4; j++)
        {
            int key_hash = *((int *)chunk.data + j);
            char *name;

            struct directory d;
            hash_to_dir(key_hash, &d);
            if (*((int *)&d) == 0)
                name = d.name;
            else
                name = d.name;

            while (strlen(read_dir) + strlen(name) + 1 >= len)
            {
                len *= 2;
                read_dir = realloc(read_dir, len);
                memset(read_dir + len / 2, 0, len / 2);
            }
            strcat(read_dir, name);
            strcat(read_dir, " ");
        }
    }
    return read_dir;
}

int rm_dir(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");
    struct directory dir;
    path_to_dir(path, &dir);
    struct chunk chunk;
    get_chunk(1, dir.dir_hash, &chunk);
    if (dir.chunk_numb > 1 || chunk.size > 0)
        return -1;

    char numb[10], par[strlen(path)];
    memset(numb, 0, 10);
    memset(par, 0, strlen(path));
    int_to_string(dir.dir_hash, numb);
    parent_from_path(path, par);
    struct directory par_dir;

    path_to_dir(par, &par_dir);
    get_chunk(dir.chunk_id, par_dir.dir_hash, &chunk);
    for (int j = 0; j < chunk.size / 4; j++)
    {
        int key_hash = *((int *)chunk.data + j);
        if (key_hash == dir.dir_hash)
        {
            memcpy((int *)chunk.data + j, (int *)chunk.data + chunk.size / 4 - 1, sizeof(int));
            chunk.size -= sizeof(int);
            chunk_replace(&chunk);
            delete_cache(numb);
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
    char numb[10];
    memset(numb, 0, 10);
    int_to_string(hash, numb);
    add_cache(numb, 0, 0, sizeof(struct file), (char *)file);
    struct chunk chunk;
    create_new_chunk(1, 1, hash, &chunk);
}

void create_file(char *path, struct file *file)
{
    char par[strlen(path)], name[50];
    memset(par, 0, strlen(path));
    memset(name, 0, 50);

    parent_from_path(path, par);
    name_from_path(path, name);
    struct directory parent_dir;
    path_to_dir(par, &parent_dir);
    add_object(&parent_dir, hash_str(path));
    file->chunk_id = parent_dir.chunk_numb;
    init_file(file, hash_str(path), name);
}

int mwrite(char *path, char *buf, size_t size, off_t offset)
{
    struct file file;
    path_to_file(path, &file);
    int written_bytes = 0;
    int chunk_index = offset / (DATA_LEN) + 1;
    off_t chunk_offset = offset % (DATA_LEN);
    int new_chunks = (offset + size) / (DATA_LEN) + 1 - file.chunk_numb;
    if (new_chunks > 0)
    {
        for (int i = 1; i <= new_chunks; i++)
        {
            struct chunk chunk;
            create_new_chunk(file.chunk_numb + i, 0, file.file_hash, &chunk);
        }
        file.chunk_numb += new_chunks;
        char numb[10];
        memset(numb, 0, 10);
        int_to_string(file.file_hash, numb);
        replace_cache(numb, 0, 0, sizeof(struct file), (char *)&file);
    }
    while (written_bytes < size)
    {
        int free_space = (DATA_LEN)-chunk_offset;
        struct chunk chunk;
        get_chunk(chunk_index, file.file_hash, &chunk);
        if (size - written_bytes < free_space)
            free_space = size - written_bytes;
        chunk_write_data(&chunk, buf, free_space, chunk_offset, written_bytes);
        written_bytes += free_space;
        chunk_offset = 0;
        chunk_index++;
    }
    return written_bytes;
}

int mread(char *path, char *buf, size_t size, off_t offset)
{
    if (offset == -1)
    {
        offset = file_size(path);
    }
    struct file file;
    path_to_file(path, &file);
    int read_bytes = 0;
    int chunk_index = offset / (DATA_LEN) + 1;
    off_t chunk_offset = offset % (DATA_LEN);
    while (read_bytes < size)
    {
        if (chunk_index > file.chunk_numb)
            return read_bytes;

        struct chunk chunk;
        get_chunk(chunk_index, file.file_hash, &chunk);
        int free_space = (DATA_LEN)-chunk_offset;
        if (chunk_offset >= chunk.size)
            return 0;
        int br = 0;
        if (size - read_bytes < free_space)
            free_space = size - read_bytes;
        if (free_space + chunk_offset > chunk.size)
        {
            br = 1;
            free_space = chunk.size - chunk_offset;
        }
        chunk_get_data(&chunk, buf, free_space, chunk_offset, read_bytes);
        read_bytes += free_space;
        chunk_offset = 0;
        chunk_index++;
        if (br)
            break;
    }
    return read_bytes;
}

int del_file_data(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");
    struct file file;
    path_to_file(path, &file);
    struct chunk chunk;
    char ch_hash[30], numb[10];
    memset(ch_hash, 0, 30);
    memset(numb, 0, 10);
    int_to_string(file.file_hash, numb);
    for (int i = 1; i <= file.chunk_numb; i++)
    {
        get_chunk_hash(i, file.file_hash, ch_hash);
        delete_cache(ch_hash);
    }
    file.chunk_numb = 1;
    replace_cache(numb, 0, 0, sizeof(struct file), (char *)&file);
    return -1;
}

int file_size(char *path)
{
    struct file file;
    path_to_file(path, &file);
    int len = (file.chunk_numb - 1) * (DATA_LEN);
    struct chunk chunk;

    get_chunk(file.chunk_numb, file.file_hash, &chunk);
    return len + chunk.size;
}

int is_file(char *path)
{
    char numb[10];
    memset(numb, 0, 10);
    int_to_string(hash_str(path), numb);
    char buf[BUFLENGTH];
    memset(buf, 0, BUFLENGTH);
    get_cache(numb, buf);
    if (strlen(path) == 0 || strlen(buf) < 10)
        return 0;
    return !is_dir(path);
}

void path_to_file(char *path, struct file *file)
{
    hash_to_file(hash_str(path), file);
}

void hash_to_file(int hash, struct file *file)
{
    hash_to_struct(hash, (char *)file);
}
#include "util.h"
#include "chunk.h"
#include "Directory.h"
#include "Memcached.h"
#include "hardlink.h"
#include <assert.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

void init_dir(struct directory *dir, int hash, char *name, mode_t mode, uid_t uid, gid_t gid)
{
    dir->unused = 0;
    dir->dir_hash = hash;
    memset(dir->name, 0, 256);
    strcpy(dir->name, name);
    dir->chunk_numb = 1;
    dir->mode = mode;
    dir->uid = uid;
    dir->gid = gid;
    char hash_str[10];
    memset(hash_str, 0, sizeof(10));
    int_to_string(hash, hash_str);
    add_cache(hash_str, 0, 0, sizeof(struct directory), (char *)dir);
    struct chunk chunk;
    create_new_chunk(1, 0, hash, &chunk);
    get_chunk(1, chunk.hash, &chunk);
}

int make_dir(char *path, mode_t mode, uid_t uid, gid_t gid)
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
    }
    else
        dir.chunk_id = 0;

    char name[256];
    memset(name, 0, sizeof(256));
    name_from_path(path, name);
    init_dir(&dir, hash_str(path), name, mode, uid, gid);
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
    char buf[BUFLENGTH];
    memset(buf, 0, BUFLENGTH);
    get_cache(numb, buf);
    if (strlen(buf) < 10)
        return 0;

    char obj[CHUNK_LEN];
    hash_to_struct(hash_str(path), obj);
    // if()
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
    create_new_chunk(dir->chunk_numb, 0, dir->dir_hash, chunk);
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

void init_file(struct file *file, int hash, char *name, int sym)
{
    file->unused = 1;
    file->file_hash = file->real_hash = hash;
    memset(file->name, 0, 256);
    strcpy(file->name, name);
    file->is_sym = sym;
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
    char par[strlen(path)], name[256];
    memset(par, 0, strlen(path));
    memset(name, 0, 256);

    parent_from_path(path, par);
    name_from_path(path, name);
    struct directory parent_dir;
    path_to_dir(par, &parent_dir);
    add_object(&parent_dir, hash_str(path));
    file->chunk_id = parent_dir.chunk_numb;
    init_file(file, hash_str(path), name, 0);
}

int mwrite(char *path, char *buf, size_t size, off_t offset)
{
    if (offset == -1)
    {
        offset = file_size(path);
    }
    struct file file;
    path_to_file(path, &file);
    int written_bytes = 0;
    int start_index = (offset + 20) / (DATA_LEN) + 1;
    int chunk_index = start_index;
    off_t chunk_offset = (offset - DATA_LEN + 20) % (DATA_LEN);
    if (chunk_index == 1)
        chunk_offset = 20 + offset;
    int start_chunk_numb = file.chunk_numb;
    int last_chunk = (offset + size + 20) / (DATA_LEN) + 1;
    int new_chunks = last_chunk - file.chunk_numb;

    change_hards(&file, last_chunk);
    file.chunk_numb = last_chunk;
    struct chunk chunk;
    chunk.hash = file.file_hash;
    chunk.isdir = 1;
    while (written_bytes < size)
    {
        int free_space = (DATA_LEN)-chunk_offset;
        if (size - written_bytes < free_space)
            free_space = size - written_bytes;
        if ((chunk_index > start_index && chunk_index < last_chunk) || (chunk_index == last_chunk && new_chunks > 0))
        {
            chunk.ind = chunk_index;
            chunk.size = 0;
            if (chunk.size < DATA_LEN)
                memset(chunk.data, 0, DATA_LEN);
        }
        else
        {
            get_chunk(chunk_index, file.real_hash, &chunk);
        }
        chunk_write_data(&chunk, buf, free_space, chunk_offset, written_bytes);
        written_bytes += free_space;
        chunk_offset = 0;
        chunk_index++;
    }
    return written_bytes;
}

int mread(char *path, char *buf, size_t size, off_t offset)
{
    struct file file;
    path_to_file(path, &file);
    int read_bytes = 0;
    int chunk_index = (offset + 20) / (DATA_LEN) + 1;
    off_t chunk_offset = (offset - DATA_LEN + 20) % (DATA_LEN);
    if (chunk_index == 1)
        chunk_offset = 20 + offset;
    while (read_bytes < size)
    {
        if (chunk_index > file.chunk_numb)
            return read_bytes;

        struct chunk chunk;
        get_chunk(chunk_index, file.real_hash, &chunk);
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
    for (int i = 2; i <= file.chunk_numb; i++)
    {
        get_chunk_hash(i, file.real_hash, ch_hash);
        delete_cache(ch_hash);
    }
    get_chunk_hash(1, file.real_hash, ch_hash);

    get_chunk(1, file.real_hash, &chunk);
    chunk.size = 20;
    memset(chunk.data + 20, 0, DATA_LEN - 20);
    chunk_replace(&chunk);
    change_hards(&file, 1);
    replace_cache(numb, 0, 0, sizeof(struct file), (char *)&file);
    return -1;
}

int file_size(char *path)
{
    struct file file;
    path_to_file(path, &file);
    int len = (file.chunk_numb - 1) * (DATA_LEN);
    struct chunk chunk;

    get_chunk(file.chunk_numb, file.real_hash, &chunk);
    len -= 20;
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

int rm_file(char *path)
{
    if (strlen(path) == 1)
        path = strdup("");
    struct file file;
    path_to_file(path, &file);
    struct chunk chunk;
    get_chunk(1, file.file_hash, &chunk);

    char numb[10], par[strlen(path)];
    memset(numb, 0, 10);
    memset(par, 0, strlen(path));
    int_to_string(file.file_hash, numb);
    parent_from_path(path, par);
    struct directory par_dir;
    path_to_dir(par, &par_dir);
    get_chunk(file.chunk_id, par_dir.dir_hash, &chunk);
    for (int j = 0; j < chunk.size / 4; j++)
    {
        int key_hash = *((int *)chunk.data + j);
        if (key_hash == file.file_hash)
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

void munlink(char *path)
{
    struct hard_link hlink;
    path_to_hard_struct(path, &hlink);
    if (del_hard(&hlink, hash_str(path)))
        del_file_data(path);
    rm_file(path);
}
/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */

#define FUSE_USE_VERSION 31

#include "Directory.h"
#include "Memcached.h"
#include "util.h"
#include "hardlink.h"
#include "chunk.h"

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */

static void *mem_init(struct fuse_conn_info *conn,
                      struct fuse_config *cfg)
{
    (void)conn;
    tcp_init();
    flush_all();
    make_dir("/");
    return NULL;
}

static int mem_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi)
{

    // printf("getattr: %s\n", path);

    (void)fi;
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    if (is_dir((char *)path))
    {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if (is_file((char *)path))
    {
        struct hard_link hlink;
        path_to_hard_struct((char *)path, &hlink);
        // printf("%s %d\n", path, hash_str(path));
        // printf("%s %d\n", path, hlink.hard_link_size);

        stbuf->st_mode = S_IFREG | 0444;
        struct file file;
        path_to_file((char *)path, &file);
        if (file.is_sym)
        {
            stbuf->st_mode = S_IFLNK | 0777;
        }
        stbuf->st_nlink = hlink.hard_link_size;
        stbuf->st_size = file_size((char *)path);
    }
    else
    {
        res = -ENOENT;
    }

    return res;
}

static int mem_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi,
                       enum fuse_readdir_flags flags)
{

    (void)offset;
    (void)fi;
    (void)flags;

    if (is_dir((char *)path) == 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    char *dir_stuff = read_dir((char *)path);
    if (dir_stuff == NULL)
    {
        return 0;
    }
    char *tok = strtok(dir_stuff, " ");
    while (tok != NULL)
    {
        // printf("%s\n", tok);
        filler(buf, tok, NULL, 0, 0);
        tok = strtok(NULL, " ");
    }

    return 0;
}
static int mem_open(const char *path, struct fuse_file_info *fi)
{

    // printf("open: %s\n", path);
    if (fi->flags & O_CREAT)
    {
        struct file file;
        create_file((char *)path, &file);
    }
    else if (!is_file((char *)path))
        return -ENOENT;
    if (O_TRUNC & fi->flags)
    {
        // printf("aeeeee\n");
        del_file_data((char *)path);
    }
    // printf("ASDasdasdasda\n");
    // if ((fi->flags & O_ACCMODE) != O_RDONLY)
    //     return -EACCES;

    return 0;
}

static int mem_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
    return mem_open(path, fi);
}

static int mem_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
    // printf("%s %d\n", path, size);
    (void)fi;
    if (!is_file((char *)path))
        return -ENOENT;
    int res = mread((char *)path, buf, size, offset);
    // printf("%s %d\n", buf, res);
    return res;
}

static int mem_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    if (!is_file((char *)path))
        return -ENOENT;

    if (O_APPEND & fi->flags)
    {
        offset = -1;
    }

    return mwrite((char *)path, (char *)buf, size, offset);
}

static int mem_mkdir(const char *path, mode_t mode)
{
    make_dir((char *)path);
    return 0;
}

static int mem_rmdir(const char *path)
{
    return rm_dir((char *)path);
}

static int mem_link(const char *from, const char *to)
{
    if (!is_file((char *)from) || is_dir((char *)to) || is_file((char *)to))
        return -ENOENT;

    struct hard_link hlink;
    path_to_hard_struct((char *)from, &hlink);

    struct file file;
    path_to_file((char *)from, &file);
    struct file link;
    memcpy(&link, &file, sizeof(struct file));
    link.file_hash = hash_str((char *)to);
    name_from_path((char *)to, link.name);
    add_hard(&hlink, link.file_hash);

    char hash[10];
    memset(hash, 0, 10);
    int_to_string(hash_str((char *)to), hash);
    add_cache(hash, 0, 0, sizeof(file), (char *)&link);
    struct directory parent_dir;
    char name[50];
    memset(name, 0, 50);
    parent_from_path((char *)to, name);
    path_to_dir(name, &parent_dir);
    add_object(&parent_dir, hash_str((char *)to));
    return 0;
}

static int mem_symlink(const char *from, const char *to)
{
    // printf("%d %d %d\n", is_file((char *)from), is_dir((char *)to), is_file((char *)to));
    // if (!is_file((char *)from) || is_dir((char *)to) || is_file((char *)to))
    // return -ENOENT;

    // printf("SYMLINK: %s %s\n", from, to);
    struct file file;
    path_to_file((char *)from, &file);

    struct file sym_file;
    memset(&sym_file, 0, sizeof(struct file));
    sym_file.is_sym = 1;
    char name[50];
    memset(name, 0, 50);
    name_from_path((char *)to, name);

    char par[strlen((char *)to)];
    memset(par, 0, strlen((char *)to));
    parent_from_path((char *)to, par);
    struct directory parent_dir;
    path_to_dir(par, &parent_dir);
    add_object(&parent_dir, hash_str((char *)to));
    sym_file.chunk_id = parent_dir.chunk_numb;

    init_file(&sym_file, hash_str((char *)to), name, 1);
    struct chunk chunk;
    get_chunk(1, hash_str((char *)to), &chunk);
    chunk.size += strlen(from);
    memcpy(chunk.data + 20, from, strlen(from));
    chunk.data[chunk.size] = 0;
    chunk_replace(&chunk);
    return 0;
}

static int mem_readlink(const char *path, char *buf, size_t size)
{
    struct file file;
    path_to_file((char *)path, &file);

    if (!file.is_sym)
    {
        return -ENOENT;
    }

    struct chunk chunk;
    get_chunk(1, hash_str((char *)path), &chunk);
    memcpy(buf, chunk.data + 20, size);

    return 0;
}

static int mem_unlink(const char *path)
{
    munlink((char *)path);
}

static struct fuse_operations mem_oper = {
    .init = mem_init,
    .getattr = mem_getattr,
    .readdir = mem_readdir,
    .open = mem_open,
    .create = mem_create,
    .read = mem_read,
    .write = mem_write,
    .mkdir = mem_mkdir,
    .rmdir = mem_rmdir,
    .link = mem_link,
    .unlink = mem_unlink,
    .symlink = mem_symlink,
    .readlink = mem_readlink,
};

int main(int argc, char *argv[])
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    ret = fuse_main(args.argc, args.argv, &mem_oper, NULL);
    fuse_opt_free_args(&args);
    return ret;
}

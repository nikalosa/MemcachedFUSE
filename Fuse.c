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
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = 6;
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
    if (!is_file((char *)path))
        return -ENOENT;

    // if ((fi->flags & O_ACCMODE) != O_RDONLY)
    //     return -EACCES;

    return 0;
}

static int mem_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
    create_file((char *)path);
    return mem_open(path, fi);
}

static int mem_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
    (void)fi;
    if (!is_file((char *)path))
        return -ENOENT;
    int res = mread((char *)path, buf, size, offset);
    printf("%s %d\n", buf, res);
    return res - 1;
}

static int mem_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    (void)fi;
    if (!is_file((char *)path))
        return -ENOENT;

    printf("oeeee\n");
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
};

int main(int argc, char *argv[])
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    ret = fuse_main(args.argc, args.argv, &mem_oper, NULL);
    printf("bla\n");
    fuse_opt_free_args(&args);
    return ret;
}
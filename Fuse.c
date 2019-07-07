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
static struct options
{
    const char *filename;
    const char *contents;
    int show_help;
} options;

#define OPTION(t, p)                      \
    {                                     \
        t, offsetof(struct options, p), 1 \
    }
static const struct fuse_opt option_spec[] = {
    OPTION("--name=%s", filename),
    OPTION("--contents=%s", contents),
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    FUSE_OPT_END};

static void *mem_init(struct fuse_conn_info *conn,
                      struct fuse_config *cfg)
{
    (void)conn;
    tcp_init();
    flush_all();
    make_dir("/");
    cfg->kernel_cache = 1;
    // assert(1 == 2)
    return NULL;
}

static int mem_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi)
{
    (void)fi;
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    //is_dir((char *)path)
    if (is_dir((char *)path))
    {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if (0)
    {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(options.contents);
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
    // printf("aeee");
    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    char *dir_stuff = read_dir((char *)path);
    char *tok = strtok(dir_stuff, " ");
    while (tok != NULL)
    {
        filler(buf, tok, NULL, 0, 0);
        tok = strtok(NULL, " ");
    }

    return 0;
}

static int mem_open(const char *path, struct fuse_file_info *fi)
{
    if (strcmp(path + 1, options.filename) != 0)
        return -ENOENT;

    if ((fi->flags & O_ACCMODE) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int mem_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
    size_t len;
    (void)fi;
    if (strcmp(path + 1, options.filename) != 0)
        return -ENOENT;

    len = strlen(options.contents);
    if (offset < len)
    {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, options.contents + offset, size);
    }
    else
        size = 0;

    return size;
}

static int mem_mkdir(const char *path, mode_t mode)
{

    // (void)path;
    make_dir((char *)path);
    return 0;
}

static struct fuse_operations hello_oper = {
    .init = mem_init,
    .getattr = mem_getattr,
    .readdir = mem_readdir,
    .open = mem_open,
    .read = mem_read,
    .mkdir = mem_mkdir,
};

static void show_help(const char *progname)
{
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf("File-system specific options:\n"
           "    --name=<s>          Name of the \"hello\" file\n"
           "                        (default: \"hello\")\n"
           "    --contents=<s>      Contents \"hello\" file\n"
           "                        (default \"Hello, World!\\n\")\n"
           "\n");
}

int main(int argc, char *argv[])
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    options.filename = strdup("hello");
    options.contents = strdup("Hello World!\n");
    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
        return 1;

    ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
    fuse_opt_free_args(&args);
    return ret;
}

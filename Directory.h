#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

struct directory
{
    int unused;
    int chunk_id;
    int chunk_numb;
    int dir_hash;
    char name[50];
};

struct file
{
    int unused;
    int chunk_id;
    int chunk_numb;
    int file_hash;
    char name[50];
};

void init_dir(struct directory *dir, int hash, char *name);
struct directory *make_dir(char *path);
int is_dir(char *path);
char *read_dir(char *path);
int rm_dir(char *path);
void add_object(struct directory *dir, int new_obj_hash);

struct directory *path_to_dir(char *path);
struct directory *hash_to_dir(int hash);

struct chunk *get_suited_chunk();
char *hash_to_struct(int hash);

void init_file(struct file *file, int hash, char *name);
int is_file(char *path);
struct file *create_file(char *path);
int mwrite(char *path, char *buf, size_t size, off_t offset);
int mread(char *path, char *buf, size_t size, off_t offset);
struct file *path_to_file(char *path);
struct file *hash_to_file(int hash);
struct file *create_file(char *path);
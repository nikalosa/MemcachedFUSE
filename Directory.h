#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// #include "chunk.h"

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
    int real_hash;
    int is_sym;
};

void init_dir(struct directory *dir, int hash, char *name);
int make_dir(char *path);
int is_dir(char *path);
char *read_dir(char *path);
int rm_dir(char *path);
void add_object(struct directory *dir, int new_obj_hash);

void path_to_dir(char *path, struct directory *dir);
void hash_to_dir(int hash, struct directory *dir);

void get_suited_chunk(struct directory *dir, struct chunk *chunk);
void hash_to_struct(int hash, char *obj);

void init_file(struct file *file, int hash, char *name, int sym);
int is_file(char *path);
int mwrite(char *path, char *buf, size_t size, off_t offset);
int mread(char *path, char *buf, size_t size, off_t offset);
void path_to_file(char *path, struct file *file);
void hash_to_file(int hash, struct file *file);
void create_file(char *path, struct file *file);
int file_size(char *path);
int del_file_data(char *path);
void munlink(char *path);
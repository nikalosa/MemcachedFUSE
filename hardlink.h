#include <stdio.h>
#include <stdlib.h>

struct hard_link
{
    int unused;
    int hash;
    int ind;
    int hard_link_size;
    int data[(1024 - 8) / 4];
};

void get_hard_hash(int numb, int hash, char *hash_str);
void init_hard(struct hard_link *hlink, int hash, int ind);
int add_hard(struct hard_link *hlink, int hash);
int del_hard(struct hard_link *hlink, int hash);
void change_hards(struct file *file, int new_chunk_numb);
void path_to_hard_struct(char *path, struct hard_link *hlink);
void get_hard_struct(struct chunk *chunk, struct hard_link *hlink);
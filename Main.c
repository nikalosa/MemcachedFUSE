#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "Memcached.h"
#include "chunk.h"
#include "util.h"
#include "Directory.h"
int main()
{
  tcp_init();

  flush_all();
  make_dir("/");

  // struct chunk *dir = (struct chunk *)get_obj(str);
  // printf("%d %d %d\n", dir->size, dir->hash, dir->ind);
  // printf("ae");
  for (int i = 0; i < 10; i++)
  {
    char *path = malloc(4);
    path[0] = '/';

    strcat(path, int_to_string(i));
    make_dir(path);
  }
  create_file("/a.txt");
  mwrite("/a.txt", "nikalosaberidze", 15, 0);
  char *buf = malloc(20);
  mwrite("/a.txt", "aez", 3, 4);
  mread("/a.txt", buf, 15, 0);
  printf("%s", buf);
  // printf("%s\n", read_dir("/"));
  // printf("%d\n", rm_dir("/2"));
  // printf("%s\n", read_dir("/"));
  // printf("\n%s", read_dir("/ae/yle/yle"));
}

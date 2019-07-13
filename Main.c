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

  char *path = malloc(1000);
  memset(path, 0, 1000);
  // struct chunk *dir = (struct chunk *)get_obj(str);
  // printf("%d %d %d\n", dir->size, dir->hash, dir->ind);
  // printf("ae");
  for (int i = 0; i < 100; i++)
  {
    // printf("\n%d\n", i);
    // path[0] = '\0';
    strcat(path, "/");
    strcat(path, "dir_rec_");
    sprintf(path, "%d", i);
    printf("SUCCESS: %d %s\n", i, read_dir(path));
    make_dir(path);
  }
  // printf("bla\n");
  // create_file("/a.txt");

  // mwrite("/a.txt", "nikalosaberidze", 15, 0);
  // char *buf = malloc(20);
  // mwrite("/a.txt", "aez", 3, 4);
  // mread("/a.txt", buf, 15, 0);
  // printf("%s", buf);
  // printf("%s\n", read_dir("/"));
  // printf("%d\n", rm_dir("/2"));
  // printf("%s\n", read_dir("/"));
  // printf("\n%s", read_dir("/ae/yle/yle"));
}

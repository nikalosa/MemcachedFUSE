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
  char *str = get_cache("0A1");

  // struct chunk *dir = (struct chunk *)get_obj(str);
  // printf("%d %d %d\n", dir->size, dir->hash, dir->ind);
  // printf("ae");
  for (int i = 0; i < 100; i++)
  {
    char *path = malloc(4);
    path[0] = '/';

    strcat(path, int_to_string(i));
    make_dir(path);
  }
  printf("\n%s", read_dir("/ae/yle/yle"));
  // printf("%d %d", is_dir("bla"), is_dir("/nikalosa/bla"));
  // str = get_cache("0A1");

  // dir = (struct chunk *)get_obj(str);
  // printf("%d %d %d %d", dir->size, dir->hash, dir->ind, *((int *)dir->data));

  // str = get_cache("602417405");
  // struct directory *d = (struct directory *)get_obj(str);
  // printf("%s", d->name);
}

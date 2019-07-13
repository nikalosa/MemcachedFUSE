#include "Memcached.h"
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "chunk.h"

int clientfd;
struct sockaddr_in addr;
int create_connection()
{
    connect(clientfd, (const struct sockaddr *)&addr, sizeof(addr));
}

int tcp_init()
{
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    struct in_addr s_addr;
    inet_pton(AF_INET, "127.0.0.1", &s_addr.s_addr);

    addr.sin_addr = s_addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(11211);
    create_connection();
}

void get_response(char *buf)
{
    int length;
    char b[1500];
    int l = 0;
    while (length = read(clientfd, buf, 1500))
    {
        // memcpy(buf + l, b, length);
        // l += length;
        if (length < 1500)
        {
            // printf("ylleee %d\n", length);
            return;
        }
    }
    return;
}

void conact_args(char *type, char *key, int flags, int exptime, int bytes, char *data)
{
    char fl[30];
    sprintf(fl, "%d %d %d\r\n", flags, exptime, bytes);
    char req[4 + strlen(type) + strlen(key) + strlen(fl) + bytes];
    sprintf(req, "%s %s %s", type, key, fl);
    int l = strlen(req) + bytes;
    memcpy(req + strlen(req), data, bytes);
    req[l] = '\r';
    req[l + 1] = '\n';
    write(clientfd, req, 4 + strlen(type) + strlen(key) + strlen(fl) + bytes);
    char buf[CHUNK_LEN];
    get_response(buf);
}

int add_cache(char *key, int flags, int exptime, int bytes, char *data)
{
    conact_args("add", key, flags, exptime, bytes, data);
}
int set_cache(char *key, int flags, int exptime, int bytes, char *data)
{
    conact_args("set", key, flags, exptime, bytes, data);
}
int replace_cache(char *key, int flags, int exptime, int bytes, char *data)
{
    conact_args("replace", key, flags, exptime, bytes, data);
}
void get_cache(char *keys, char *buf)
{
    char req[30];
    sprintf(req, "get %s\r\n", keys);
    write(clientfd, req, strlen(req));
    get_response(buf);
    return;
}

int flush_all()
{
    write(clientfd, "flush_all\r\n", 11);
    char buf[BUFLENGTH];
    get_response(buf);
    return 1;
}

int delete_cache(char *key)
{
    create_connection();
    char req[24], buf[BUFLENGTH];
    sprintf(req, "delete %s\r\n", key);
    write(clientfd, req, strlen(req));
    get_response(buf);
}

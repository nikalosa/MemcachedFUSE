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
#define BUFLENGTH 1500

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

char *get_response()
{
    char *buf = malloc(BUFLENGTH);
    int length;
    while (length = read(clientfd, buf, BUFLENGTH))
    {
        buf[length] = '\0';
        // printf("%s", buf);
        if (length < BUFLENGTH)
            break;
    }
    return buf;
}

char *conact_args(char *type, char *key, int flags, int exptime, int bytes, char *data)
{
    char fl[30];
    sprintf(fl, "%d %d %d\r\n", flags, exptime, bytes);
    char *req = malloc(4 + strlen(type) + strlen(key) + strlen(fl) + bytes);
    sprintf(req, "%s %s %s", type, key, fl);
    int l = strlen(req) + bytes;
    // printf("%d\n", l);
    memcpy(req + strlen(req), data, bytes);
    req[l] = '\r';
    req[l + 1] = '\n';
    write(clientfd, req, 4 + strlen(type) + strlen(key) + strlen(fl) + bytes);
    return get_response();
}

int add_cache(char *key, int flags, int exptime, int bytes, char *data)
{
    char *resp = conact_args("add", key, flags, exptime, bytes, data);
    // create_connection();
    return (strcmp(resp, "STORED") == 0) ? 1 : 0;
}
int set_cache(char *key, int flags, int exptime, int bytes, char *data)
{
    char *resp = conact_args("set", key, flags, exptime, bytes, data);
    // create_connection();
    return (strcmp(resp, "STORED") == 0) ? 1 : 0;
}
int replace_cache(char *key, int flags, int exptime, int bytes, char *data)
{
    char *resp = conact_args("replace", key, flags, exptime, bytes, data);
    // create_connection();
    return (strcmp(resp, "STORED") == 0) ? 1 : 0;
}
int append_cache(char *key, int flags, int exptime, int bytes, char *data)
{
    char *resp = conact_args("append", key, flags, exptime, bytes, data);
    // create_connection();
    return (strcmp(resp, "STORED") == 0) ? 1 : 0;
}
int prepend_cache(char *key, int flags, int exptime, int bytes, char *data)
{
    char *req = conact_args("prepend", key, flags, exptime, bytes, data);
    create_connection();
    write(clientfd, req, strlen(req));
    char *resp = get_response();
    return (strcmp(resp, "STORED") == 0) ? 1 : 0;
}
char *get_cache(char *keys)
{
    // create_connection();
    char req[50];
    sprintf(req, "get %s\r\n", keys);
    write(clientfd, req, strlen(req));
    char *resp = get_response();
    return resp;
}

int flush_all()
{
    write(clientfd, "flush_all\r\n", 11);
    char *resp = get_response();
    return 1;
}

int gets_cache(char *keys)
{
}

int delete_cache(char *key)
{
    create_connection();
    char req[24];
    sprintf(req, "delete %s\r\n", key);
    write(clientfd, req, strlen(req));
    get_response();
}

int incr_cache(char *key, int val)
{
    create_connection();
    char req[24];
    sprintf(req, "incr %s %d\r\n", key, val);
    write(clientfd, req, strlen(req));
    get_response();
}

int decr_cache(char *key, int val)
{
    create_connection();
    char req[24];
    sprintf(req, "decr %s %d\r\n", key, val);
    write(clientfd, req, strlen(req));
    get_response();
}

int touch_cache(char *key, int exptime)
{
    create_connection();
    char req[24];
    sprintf(req, "touch %s %d\r\n", key, exptime);
    write(clientfd, req, strlen(req));
    get_response();
}

int stats_cache()
{
    create_connection();
    write(clientfd, "stats\r\n", 7);
    get_response();
}

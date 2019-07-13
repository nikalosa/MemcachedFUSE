#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hash_str(char *str)
{
    int hash = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        hash = ((long long)hash * PRIME + (long long)str[i]) % MOD;
    }
    return hash;
}

int string_to_int(char *num)
{
    return atoi(num);
}

void int_to_string(int num, char *str_num)
{
    sprintf(str_num, "%d", num);
}

void name_from_path(char *path, char *name)
{
    for (int i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            strcpy(name, path + i + 1);
            return;
        }
    }
}

void parent_from_path(char *path, char *name)
{
    for (int i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            if (i > 0)
                strncpy(name, path, i);
            else
                name[0] = '\0';
            return;
        }
    }
}

void get_obj(char *resp, char *obj)
{
    char *tok = strtok(resp, " ");
    int ind = 0;
    int len = 0;
    int offset = 0;
    while (tok != NULL)
    {
        if (ind == 3)
        {
            tok = strtok(tok, "\r");
            offset += strlen(tok) + 2;
            len = string_to_int(tok);
            memcpy(obj, resp + offset, len);
            return;
        }
        offset += strlen(tok) + 1;
        tok = strtok(NULL, " ");
        ind++;
    }
    return;
}
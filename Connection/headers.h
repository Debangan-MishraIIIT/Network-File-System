#ifndef _HEADERS_H_
#define _HEADERS_H_

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <semaphore.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "socket.h"
#include "api.h"
#include "search.h"
#include "cache.h"
#include "colors.h"
#include "errors.h"

#define SA struct sockaddr
#define MAX_SIZE 1024
#define maxlen 1000
void handle_errors(char* error);

struct pair
{
    int first, second;
};

struct hostDetails
{
    char ip[16];
    int port;
    int connfd;
};

struct cDetails
{
    int id;
    int connfd;
};

struct ssDetails
{
    int id;
    char ip[16];
    int nmPort;
    int cliPort;
    int connfd;
    int addPathfd;
};

struct accessibleFile
{
    char *path;
    char perms[11];
};

struct record
{
    char * path;
    struct ssDetails *orignalSS;
    char originalPerms[11];
    bool isDir;
    char currentPerms[11]; // for backup
    struct ssDetails *backupSS1;
    struct ssDetails *backupSS2;
    size_t size;
    time_t creationTime;
    time_t lastModifiedTime;

    // for n-ary tree
    struct record *firstChild;
    struct record * nextSibling;
    struct record * parent;
    struct record * prevSibling;
};

struct fileDetails
{
    char path[4096];
    char perms[11];
    size_t size;
    bool isDir;
};
#endif
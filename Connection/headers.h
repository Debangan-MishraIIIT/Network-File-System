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

#include "socket.h"
#include "api.h"

struct cDetails
{
    int clientID;
    int connfd;
};

struct ssDetails
{
    int ssID;
    char ip[16];
    int nmPort;
    int cliPort;
    int connfd;
    int totalSize;
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
    int size;
};
#endif
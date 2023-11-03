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

#include <semaphore.h>
#include <pthread.h>

#include "socket.h"

#define SA struct sockaddr
#define MAX_SIZE 1024

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

#endif
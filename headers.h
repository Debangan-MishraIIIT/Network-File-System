#ifndef _HEADERS_H_
#define _HEADERS_H_

#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <fcntl.h>

#include "socket.h"

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 
#define MAX_SIZE 1024

struct pair{
    int first, second;
};

#endif
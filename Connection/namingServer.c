#include "headers.h"

// TODO: to be modified to a more efficient (non restrictive) method of storage. Implementing simple array for now

struct hostDetails storageServers[100];
int storageServerCount = 0;

pthread_t clientThreads[100];
struct hostDetails clientDetails[100];
int clientCount = 0;

void *acceptClientRequests(void *args)
{
    struct hostDetails *cd = ((struct hostDetails *)(args));
    char client_ip[16];
    strcpy(client_ip, cd->ip);
    int client_port = cd->port;
    int connfd = cd->connfd;
    printf("Accepting client requests from %s:%d\n", client_ip, client_port);

    while (1)
    {
        // recieve the client request
        char request[4096];
        int bytesRecv = recv(connfd, request, sizeof(request), 0);
        if (bytesRecv == -1)
        {
            perror("recv");
        }

        struct hostDetails ss = storageServers[atoi(request)];

        // send storage server details
        int bytesSent = send(connfd, &ss, sizeof(ss), 0);
        if (bytesSent == -1)
        {
            perror("send");
        }
        printf("SS Details: %s:%d\n", storageServers[atoi(request)].ip, storageServers[atoi(request)].port);
    }
}

void addClient(int connfd)
{
    struct sockaddr_in cli_addr;
    socklen_t addrlen = sizeof(cli_addr);

    char client_ip[16];
    int client_port;
    if (getpeername(connfd, (struct sockaddr *)&cli_addr, &addrlen) == 0)
    {
        inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        client_port = ntohs(cli_addr.sin_port);
        printf("Client connected from %s:%d\n", client_ip, client_port);
    }
    else
    {
        perror("getpeername");
    }
    strcpy(clientDetails[clientCount].ip, client_ip);
    clientDetails[clientCount].port = client_port;
    clientDetails[clientCount].connfd = connfd;
    clientCount++;
    pthread_create(&clientThreads[clientCount - 1], NULL, acceptClientRequests, (void *)&clientDetails[clientCount - 1]);
}

void addStorageServer(int connfd)
{
    struct sockaddr_in ss_addr;
    socklen_t addrlen = sizeof(ss_addr);

    char ss_ip[16];
    int ss_port;
    if (getpeername(connfd, (struct sockaddr *)&ss_addr, &addrlen) == 0)
    {
        inet_ntop(AF_INET, &ss_addr.sin_addr, ss_ip, INET_ADDRSTRLEN);
        ss_port = ntohs(ss_addr.sin_port);
        printf("Storage Server connected from %s:%d\n", ss_ip, ss_port);
    }
    else
    {
        perror("getpeername");
    }
    strcpy(storageServers[storageServerCount].ip, ss_ip);
    storageServers[storageServerCount].port = ss_port;
    storageServers[storageServerCount].connfd = connfd;
    storageServerCount++;
}

void *acceptHost(void *args)
{
    int sockfd = *((int *)args);
    // need to change limit
    if (listen(sockfd, 128) == 0)
        printf("Listening\n");
    else
        printf("Error\n");

    struct sockaddr_in cli;
    int len = sizeof(cli);
    while (1)
    {
        int connfd = accept(sockfd, &cli, &len);
        if (connfd < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        char buffer[4096];
        int bytesRecv = recv(connfd, buffer, sizeof(buffer), 0);
        if (bytesRecv == -1)
        {
            perror("recv");
        }

        if (strcmp(buffer, "JOIN_AS Storage Server") == 0)
        {
            addStorageServer(connfd);
        }
        if (strcmp(buffer, "JOIN_AS Client") == 0)
        {
            addClient(connfd);
        }
    }
    return NULL;
}

int initializeNamingServer(int port)
{
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        perror("socket");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        perror("bind");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    return sockfd;
}

int main()
{
    int port = 6969;
    int sockfd = initializeNamingServer(port);

    pthread_t acceptHostThread;
    pthread_create(&acceptHostThread, NULL, acceptHost, &sockfd);

    // accpeting server and clinet are infine loops and continue till naming server terminated
    pthread_join(acceptHostThread, NULL);
    for (int i = 0; i < clientCount; i++)
    {
        pthread_join(clientThreads[i], NULL);
    }

    return 0;
}
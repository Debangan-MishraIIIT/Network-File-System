#include "headers.h"

// TODO: to be modified to a more efficient (non restrictive) method of storage. Implementing simple array for now
struct pair clientsList[100];
struct pair serversList[100];
int clientsCount = 0;
int serversCount = 0;

void addClient(int connfd)
{
    struct sockaddr_in cli_addr;
    socklen_t addrlen = sizeof(cli_addr);

    if (getpeername(connfd, (struct sockaddr *)&cli_addr, &addrlen) == 0)
    {
        char client_ip[16];
        inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(cli_addr.sin_port);
        printf("Client connected from %s:%d\n", client_ip, client_port);
    }
    else
    {
        perror("getpeername");
    }
}

void addStorageServer(int connfd)
{
    struct sockaddr_in ss_addr;
    socklen_t addrlen = sizeof(ss_addr);

    if (getpeername(connfd, (struct sockaddr *)&ss_addr, &addrlen) == 0)
    {
        char ss_ip[16];
        inet_ntop(AF_INET, &ss_addr.sin_addr, ss_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(ss_addr.sin_port);
        printf("Storage Server connected from %s:%d\n", ss_ip, client_port);
    }
    else
    {
        perror("getpeername");
    }
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

    return 0;
}
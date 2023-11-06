#include "headers.h"

// TODO: to be modified to a more efficient (non restrictive) method of storage. Implementing simple array for now

struct ssDetails storageServers[100];
int storageServerCount = 0;
struct cDetails clientDetails[100];

struct record
{
    struct accessibleFile file;
    struct ssDetails storageServer;
};
struct record records[100]; // need to change
int recordCount = 0;

pthread_t clientThreads[100];
// struct hostDetails clientDetails[100];
int clientCount = 0;

struct ssDetails *getSSfromPath(char *path)
{
    for (int i = 0; i < recordCount; i++)
    {
        // if (strcmp(records[i].file, path) == 0)
        // {
        //     return &records[i].storageServer;
        // }
    }
    return NULL;
}

void sendRequestToSS(struct ssDetails *ss, char *request)
{
    int bytesSent = send(ss->connfd, request, sizeof(request), 0);
    if (bytesSent == -1)
    {
        perror("send");
    }
}

void *acceptClientRequests(void *args)
{
    struct cDetails *cli = (struct cDetails *)args;
    printf("%d cli connfd\n", cli->connfd);
    while (1)
    {
        // recieve the client request
        char request[4096];
        int bytesRecv = recv(cli->connfd, request, sizeof(request), 0);
        if (bytesRecv == -1)
        {
            // perror("recv");
            break;
        }
        printf("Recieved from client - \'%s\'\n", request);
        struct ssDetails *ss = &storageServers[atoi(request)];
        // struct ssDetails* ss = getSSfromPath(request);

        printf("SS Details: %s:%d\n", ss->ip, ss->cliPort);
        sendRequestToSS(ss, request);

        // send storage server details
        int bytesSent = send(cli->connfd, ss, sizeof(struct ssDetails), 0);
        if (bytesSent == -1)
        {
            perror("send");
        }

        printf("sent ss detials to client\n");
    }
    return NULL;
}

void addClient(int connfd)
{
    clientDetails[clientCount].connfd = connfd;
    clientCount++;
    printf("Client Joined\n");

    pthread_create(&clientThreads[clientCount - 1], NULL, acceptClientRequests, &clientDetails[clientCount - 1]);
}
void *addPathFromSS(void *args){
    
}

void addStorageServer(int connfd)
{
    int bytesRecv = recv(connfd, &storageServers[storageServerCount], sizeof(storageServers[storageServerCount]), 0);
    if (bytesRecv == -1)
    {
        perror("recv");
        return;
    }
    storageServers[storageServerCount].connfd = connfd;
    storageServerCount++;
    printf("SS Joined %s:%d %d\n", storageServers[storageServerCount - 1].ip, storageServers[storageServerCount - 1].cliPort, storageServers[storageServerCount - 1].nmPort);

    pthread_t addpathThread;
    pthread_create(&addpathThread, NULL, addPathFromSS, (void*)&storageServers[storageServerCount-1]);
    // TODO: add storage server disconnection message
}

void *acceptHost(void *args)
{
    int sockfd = *((int *)args);
    // need to change limit
    if (listen(sockfd, 12) == 0)
        printf("Listening...\n");
    else
        printf("Error\n");

    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    while (1)
    {
        int connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
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

        char joinAcceptedMsg[100] = "ACCEPTED JOIN";
        int bytesSent = send(connfd, joinAcceptedMsg, sizeof(joinAcceptedMsg), 0);
        if (bytesSent == -1)
        {
            perror("send");
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
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        perror("bind");
        exit(0);
    }

    return sockfd;
}

int main()
{
    int port = 6969;
    int nmSock = initializeNamingServer(port);

    pthread_t acceptHostThread;
    pthread_create(&acceptHostThread, NULL, acceptHost, &nmSock);

    // accpeting server and clinet are infine loops and continue till naming server terminated
    pthread_join(acceptHostThread, NULL);
    for (int i = 0; i < clientCount; i++)
    {
        pthread_join(clientThreads[i], NULL);
    }

    return 0;
}
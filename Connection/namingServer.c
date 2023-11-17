#include "headers.h"

struct ssDetails storageServers[10000];
bool validSS[10000];
int storageServerCount = 0;

struct cDetails clientDetails[10000];
bool validCli[10000];

struct record records[10000];
int recordCount = 0;

pthread_t clientThreads[10000];
int clientCount = 0;
TrieNode *root;
LRUCache *myCache;

pthread_mutex_t hostLock;
pthread_mutex_t recordsLock;
int nmSock;

struct ssDetails *getRecord(char *path)
{
    struct record *tableEntry;
    // first check if path exists in cache or not
    tableEntry = searchFileInCache(myCache, path);
    // printCache(myCache);
    if (tableEntry)
        return tableEntry->orignalSS;

    // printf("LOL!\n");

    tableEntry = search(root, path);
    if (tableEntry)
    {
        // add the record to cache since it was not present before
        addFile(myCache, tableEntry);
        printf("Record added to cache!\n");
        return tableEntry->orignalSS;
    }
    else
        return NULL;
}

void sendRequestToSS(struct ssDetails *ss, char *request)
{
    int bytesSent = send(ss->connfd, request, sizeof(request), 0);
    if (bytesSent == -1)
    {
        handleNetworkErrors("send");
    }
}

void *acceptClientRequests(void *args)
{
    struct cDetails *cli = (struct cDetails *)args;
    while (1)
    {
        // recieve the client request
        char request[4096];
        bzero(request, sizeof(request));
        int bytesRecv = recv(cli->connfd, request, sizeof(request), 0);
        if (bytesRecv == -1)
        {
            handleNetworkErrors("recv");
            break;
        }
        if (bytesRecv == 0)
        {
            // client disconnects
            validCli[cli->id] = false;
            printf(RED_COLOR "[-] Client %d has disconnected!\n" RESET_COLOR, cli->id);

            close(cli->connfd);
            break;
        }

        printf("Recieved from client - \'%s\'\n", request);
        // struct ssDetails *ss = &storageServers[atoi(request)];

        struct ssDetails *ss = getRecord(request);

        printf("SS Details: %s:%d\n", ss->ip, ss->cliPort);
        sendRequestToSS(ss, request);

        // send storage server details
        int bytesSent = send(cli->connfd, ss, sizeof(struct ssDetails), 0);
        if (bytesSent == -1)
        {
            handleNetworkErrors("send");
        }

        printf("sent ss detials to client\n");
    }
    return NULL;
}

void *addToRecord(void *args)
{
    while (1)
    {
        struct ssDetails *ss = (struct ssDetails *)(args);
        struct fileDetails det;
        bzero(&det, sizeof(det));
        int bytesRecv = recv(ss->addPathfd, &det, sizeof(det), 0);
        if (bytesRecv == -1)
        {
            handleNetworkErrors("recv");
            break;
        }
        if (bytesRecv == 0)
        {
            // ss disconnects
            validSS[ss->id] = false;
            close(ss->connfd);
            close(ss->addPathfd);
            printf(RED_COLOR "[-] Storage Server %d has disconnected!\n" RESET_COLOR, ss->id);
            break;
        }
        pthread_mutex_lock(&recordsLock);
        int i = recordCount;

        records[i].size = det.size;

        records[i].backupSS1 = NULL;
        records[i].backupSS2 = NULL;
        strcpy(records[i].currentPerms, det.perms);
        strcpy(records[i].originalPerms, det.perms);
        records[i].isDir = det.isDir;
        records[i].orignalSS = ss;
        records[i].path = malloc(sizeof(char) * 4096);
        strcpy(records[i].path, det.path);

        printf(BLUE_COLOR"Added %s as accessible path in Storage Server %d\nPermissions: %s\n"RESET_COLOR, records[i].path, records[i].orignalSS->id, records[i].originalPerms);

        insertRecordToTrie(root, &records[i]);
        recordCount++;
        pthread_mutex_unlock(&recordsLock);
    }

    return NULL;
}

void addClient(int connfd)
{
    while (validCli[clientCount + 1])
    {
        clientCount++;
        clientCount %= 10000;
    }
    clientDetails[clientCount].connfd = connfd;
    clientDetails[clientCount].id = clientCount + 1;
    validCli[clientCount + 1] = true;

    printf(YELLOW_COLOR "[+] Client %d Joined\n" RESET_COLOR, clientDetails[clientCount].id);

    pthread_create(&clientThreads[clientCount], NULL, acceptClientRequests, &clientDetails[clientCount]);
}

void addStorageServer(int connfd)
{
    while (validSS[storageServerCount + 1])
    {
        storageServerCount++;
        storageServerCount %= 10000;
    }

    bzero(&storageServers[storageServerCount], sizeof(storageServers[storageServerCount]));
    int bytesRecv = recv(connfd, &storageServers[storageServerCount], sizeof(storageServers[storageServerCount]), 0);
    if (bytesRecv == -1)
    {
        // perror("recv");
        handleNetworkErrors("recv");
        return;
    }
    storageServers[storageServerCount].connfd = connfd;
    storageServers[storageServerCount].id = storageServerCount + 1;
    validSS[storageServerCount + 1] = true;

    // create a new sockfd for adding paths dynamically
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    connfd = accept(nmSock, (struct sockaddr *)&cli, &len);
    if (connfd < 0)
    {
        // perror("accept");
        handleNetworkErrors("accept");
        exit(EXIT_FAILURE);
    }

    storageServers[storageServerCount].addPathfd = connfd;

    printf(YELLOW_COLOR "[+] Storage Server %d Joined from %s:%d\n    Port for Client Communication: %d\n" RESET_COLOR, storageServers[storageServerCount].id, storageServers[storageServerCount].ip, storageServers[storageServerCount].nmPort, storageServers[storageServerCount].cliPort);

    pthread_t addpathThread;
    pthread_create(&addpathThread, NULL, addToRecord, (void *)&storageServers[storageServerCount]);
}

void *acceptHost(void *args)
{
    int sockfd = *((int *)args);
    // need to change limit
    if (listen(sockfd, 12) != 0)
    {
        // printf(RED_COLOR "Error in listen\n" RESET_COLOR);
        handleNetworkErrors("listen");
        exit(0);
    }
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    while (1)
    {
        int connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (connfd < 0)
        {
            // perror("accept");
            handleNetworkErrors("accept");
            exit(0);
        }
        char buffer[4096];
        bzero(buffer, sizeof(buffer));
        int bytesRecv = recv(connfd, buffer, sizeof(buffer), 0);
        if (bytesRecv == -1)
        {
            handleNetworkErrors("recv");
            // perror("recv");
        }

        char joinAcceptedMsg[100] = "ACCEPTED JOIN";
        int bytesSent = send(connfd, joinAcceptedMsg, sizeof(joinAcceptedMsg), 0);
        if (bytesSent == -1)
        {
            handleNetworkErrors("send");
            // perror("send");
        }

        if (strcmp(buffer, "JOIN_AS Storage Server") == 0)
        {
            pthread_mutex_lock(&hostLock);
            addStorageServer(connfd);
            pthread_mutex_unlock(&hostLock);
        }
        if (strcmp(buffer, "JOIN_AS Client") == 0)
        {
            pthread_mutex_lock(&hostLock);
            addClient(connfd);
            pthread_mutex_unlock(&hostLock);
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
        // perror("socket");
        handleNetworkErrors("socket");
        exit(0);
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // to avoid bind error
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        // perror("bind");
        handleNetworkErrors("bind");
        exit(0);
    }
    printf(GREEN_COLOR "[+] Naming Server Initialized\n" RESET_COLOR);

    return sockfd;
}

int main(int argc, char *argv[])
{
    // int port = 6969;
    if (argc != 2)
    {
        printf(RED_COLOR "[-] Invalid Arguments!\n" RESET_COLOR);
        exit(0);
    }
    int port = atoi(argv[1]);
    nmSock = initializeNamingServer(port);
    root = initTrieNode();
    myCache = initCache();

    for (int i = 0; i < 10000; i++)
    {
        validCli[i] = false;
        validSS[i] = false;
    }
    int rc = pthread_mutex_init(&hostLock, NULL);
    assert(rc == 0);
    pthread_mutex_init(&recordsLock, NULL);
    assert(rc == 0);

    pthread_t acceptHostThread;
    pthread_create(&acceptHostThread, NULL, acceptHost, &nmSock);

    // accpeting server and clinet are infine loops and continue till naming server terminated
    pthread_join(acceptHostThread, NULL);

    freeCache(myCache);
    return 0;
}

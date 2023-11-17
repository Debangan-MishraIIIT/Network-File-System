#include "headers.h"

// TODO: to be modified to a more efficient (non restrictive) method of storage. Implementing simple array for now

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

int nmSock;

struct ssDetails *getRecord(char *path)
{
    struct record *tableEntry;
    // first check if path exists in cache or not
    tableEntry = searchFileInCache(myCache, path);
    if (tableEntry)
    {
        tableEntry->lastModified = time(NULL);
        return tableEntry->orignalSS;
    }

    tableEntry = search(root, path);
    if (tableEntry)
    {
        // add the record to cache since it was not present before
        addFile(myCache, tableEntry);
        printf("Record added to cache!\n");
        tableEntry->lastModified = time(NULL);
        return tableEntry->orignalSS;
    }
    else
    {
        return NULL;
    }
}

void sendRequestToSS(struct ssDetails *ss, char *request)
{
    int bytesSent = send(ss->connfd, request, sizeof(request), 0);
    if (bytesSent == -1)
    {
        perror("send");
    }
}

void removeToken(char *str)
{
    char *lastSlash = strrchr(str, '/');
    if (lastSlash != NULL)
    {
        *lastSlash = '\0';
    }
}

void splitPath(char *inputPath, char *outputParts[2])
{
    char path[1024];
    strcpy(path, inputPath);

    char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL)
    {
        *lastSlash = '\0';
        outputParts[0] = path;
        outputParts[1] = lastSlash + 1;
    }
    else
    {
        outputParts[0] = path;
        outputParts[1] = NULL;
    }
}

struct ssDetails *find_final_path(char *path)
{
    char *temp_request = malloc(sizeof(char) * 1000);
    strcpy(temp_request, path);
    printf("%s\n", temp_request);
    struct ssDetails *ss = getRecord(temp_request);
    if (ss != NULL)
    {
        printf("found\n");
        return ss;
    }
    while (strchr(temp_request, '/') != NULL)
    {
        removeToken(temp_request);
        printf("%s\n", temp_request);
        struct ssDetails *ss = getRecord(temp_request);
        if (ss != NULL)
        {
            printf("found\n");
            return ss;
        }
    }
    ss = malloc(sizeof(struct ssDetails));
    ss->id = -1;
    return ss;
}

void *acceptClientRequests(void *args)
{
    struct cDetails *cli = (struct cDetails *)args;
    char request[4096];
    while (1)
    {
        // recieve the client request
        bzero(request, sizeof(request));
        int bytesRecv = recv(cli->connfd, request, sizeof(request), 0);
        if (bytesRecv == -1)
        {
            // handle_errors("recv");
            break;
        }

        printf("Recieved from client - \'%s\'\n", request);
        // struct ssDetails *ss = &storageServers[atoi(request)];

        // get path-index 1 and command- index 0 from address
        char *argument_array[3];
        parse_input(argument_array, request);
        char *request_command = argument_array[0];
        char *request_path = argument_array[1];

        // changes starting here
        // handle null values here
        struct ssDetails *ss = find_final_path(request_path);
        if (ss->id == -1)
        {
            char buffer[1000];
            strcpy(buffer, "error");
            int bytesSent = send(cli->connfd, buffer, sizeof(buffer), 0);
            if (bytesSent == -1)
            {
                handle_errors("send");
            }
            continue;
        }
        printf("SS Details: %s:%d\n", ss->ip, ss->cliPort);

        // if privileged
        if (strcmp(request_command, "MKDIR") == 0 || strcmp(request_command, "MKFILE") == 0 || strcmp(request_command, "RMFILE") == 0 || strcmp(request_command, "RMDIR") == 0)
        {
            sendRequestToSS(ss, request);

            char buffer[1000];
            bzero(buffer, sizeof(buffer));
            int bytesRecv = recv(ss->connfd, buffer, sizeof(buffer), 0);
            printf("receieve from server: %s\n", buffer);

            int bytesSent = send(cli->connfd, buffer, sizeof(buffer), 0);
            printf("sent execution to client\n");
        }
        else if (strcmp(request_command, "COPYDIR") == 0 || strcmp(request_command, "COPYFILE") == 0)
        {
            // first listen and copy locally
            sendRequestToSS(ss, request);
            receive_directory(ss->connfd);

            // acknowledgement window not opened
            char buffer[1000];
            bzero(buffer, sizeof(buffer));
            int bytesRecv = recv(ss->connfd, buffer, sizeof(buffer), 0);
            printf("receieve from server: %s\n", buffer);

            bzero(buffer, sizeof(buffer));
            int bytesSent = send(cli->connfd, buffer, sizeof(buffer), 0);
            printf("sent execution to client\n");
            bzero(buffer, sizeof(buffer));

            // now send the received files
            printf("HERE!!!\n");
            char *temparr[2];
            splitPath(request_path, temparr);
            char *lastToken = temparr[1];
            char send_buffer[1000];
            char *final_path = argument_array[2];
            snprintf(send_buffer, sizeof(send_buffer), "RECDIR %s", final_path);
            printf("%s\n", final_path);
            struct ssDetails *ss2 = find_final_path(final_path);
            printf("SS recipient Details: %s:%d\n", ss2->ip, ss2->cliPort);
            sendRequestToSS(ss2, send_buffer);

            int resp = recursive_directory_sending(lastToken, ss2->connfd);
            if (resp == -1)
            {
                handleFileOperationError("recursive_directory_sending");
            }
            send(ss2->connfd, "END", sizeof("END"), 0);
        }
        // not privileged
        else
        {
            sendRequestToSS(ss, request);
            // send storage server details
            int bytesSent = send(cli->connfd, ss, sizeof(struct ssDetails), 0);
            if (bytesSent == -1)
            {
                handle_errors("send");
            }
            printf("sent ss details to client\n");
        }
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
            perror("recv");
            break;
        }
        if (bytesRecv == 0)
        {
            // ss disconnects
            printf("SS %d Disconnected\n", ss->id);
            break;
        }
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

        insertRecordToTrie(root, &records[i]);
        recordCount++;
    }

    return NULL;
}

void addClient(int connfd)
{
    while (validSS[clientCount])
    {
        clientCount++;
        clientCount %= 10000;
    }
    clientDetails[clientCount].connfd = connfd;
    clientDetails[clientCount].id = clientCount + 1;
    clientCount++;
    clientCount %= 10000;

    printf("Client Joined\n");

    pthread_create(&clientThreads[clientCount - 1], NULL, acceptClientRequests, &clientDetails[clientCount - 1]);
}

void addStorageServer(int connfd)
{
    while (validSS[storageServerCount])
    {
        storageServerCount++;
        storageServerCount %= 10000;
    }

    bzero(&storageServers[storageServerCount], sizeof(storageServers[storageServerCount]));
    int bytesRecv = recv(connfd, &storageServers[storageServerCount], sizeof(storageServers[storageServerCount]), 0);
    if (bytesRecv == -1)
    {
        perror("recv");
        return;
    }
    storageServers[storageServerCount].connfd = connfd;
    storageServers[storageServerCount].id = storageServerCount + 1;

    // create a new sockfd for adding paths dynamically
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    connfd = accept(nmSock, (struct sockaddr *)&cli, &len);
    if (connfd < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    storageServers[storageServerCount].addPathfd = connfd;

    storageServerCount++;
    storageServerCount %= 10000;

    printf("SS Joined %s:%d %d\n", storageServers[storageServerCount - 1].ip, storageServers[storageServerCount - 1].cliPort, storageServers[storageServerCount - 1].nmPort);

    pthread_t addpathThread;
    pthread_create(&addpathThread, NULL, addToRecord, (void *)&storageServers[storageServerCount - 1]);
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
        bzero(buffer, sizeof(buffer));
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

    // to avoid bind error
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        perror("bind");
        exit(0);
    }

    return sockfd;
}

int main(int argc, char *argv[])
{
    // int port = 6969;
    if (argc != 2)
    {
        handleSYSandInputErrors("invalid_input");
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

    pthread_t acceptHostThread;
    pthread_create(&acceptHostThread, NULL, acceptHost, &nmSock);

    // accpeting server and clinet are infine loops and continue till naming server terminated
    pthread_join(acceptHostThread, NULL);
    for (int i = 0; i < clientCount; i++)
    {
        pthread_join(clientThreads[i], NULL);
    }

    freeCache(myCache);
    return 0;
}

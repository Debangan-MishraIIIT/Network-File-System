#include "headers.h"

struct ssDetails storageServers[10000];
bool validSS[10000];
int storageServerCount = 0;

struct cDetails clientDetails[10000];
bool validCli[10000];

// struct record records[10000];
// int recordCount = 0;
struct record *root;

pthread_t clientThreads[10000];
int clientCount = 0;
TrieNode *trieRoot;
LRUCache *myCache;

pthread_mutex_t hostLock;
pthread_mutex_t recordsLock;
pthread_mutex_t loggingLock;

int nmSock;

struct record *getRecord(char *path)
{
    pthread_mutex_lock(&recordsLock);
    struct record *tableEntry;
    // first check if path exists in cache or not
    tableEntry = searchFileInCache(myCache, path);
    // printCache(myCache);
    if (tableEntry && validSS[tableEntry->orignalSS->id])
    {
        pthread_mutex_unlock(&recordsLock);
        return tableEntry;
    }

    tableEntry = search(trieRoot, path);
    if (tableEntry && validSS[tableEntry->orignalSS->id])
    {
        // add the record to cache since it was not present before
        addFile(myCache, tableEntry);
        pthread_mutex_unlock(&recordsLock);
        return tableEntry;
    }
    else
    {
        pthread_mutex_unlock(&recordsLock);
        return NULL;
    }
}

void addToRecords(struct record *r)
{
    pthread_mutex_lock(&recordsLock);
    if (root == NULL)
    {
        pthread_mutex_unlock(&recordsLock);
        return;
    }

    char *finalPath = strdup(r->path);
    char *token = strtok(finalPath, "/");
    struct record *parentNode = root;

    while (token != NULL)
    {
        // Check if a child node with the same path already exists
        char currPath[4096];
        bzero(currPath, sizeof(currPath));

        strcpy(currPath, parentNode->path);
        if (strlen(currPath) != 0)
        {
            currPath[strlen(currPath)] = '/';
        }
        strcpy(currPath + strlen(currPath), token);

        // printf("Curr path: %s\n", currPath);
        struct record *child = parentNode->firstChild;
        while (child != NULL)
        {
            if (strcmp(child->path, currPath) == 0)
            {
                break;
            }
            child = child->nextSibling;
        }

        if (child == NULL)
        {
            r->nextSibling = parentNode->firstChild;
            if (parentNode->firstChild != NULL)
            {
                parentNode->firstChild->prevSibling = r;
            }
            parentNode->firstChild = r;
            r->parent = parentNode;
            parentNode = r;
        }
        else
        {
            parentNode = child;
        }

        token = strtok(NULL, "/");
    }
    insertRecordToTrie(trieRoot, r);
    printf(BLUE_COLOR "Added %s as accessible path in Storage Server %d\nPermissions: %s\n" RESET_COLOR, r->path, r->orignalSS->id, r->originalPerms);

    pthread_mutex_unlock(&recordsLock);
    return;
}

void removeFromRecords(char *path)
{
    struct record *r = getRecord(path);
    pthread_mutex_lock(&recordsLock);
    removeFileFromCache(myCache, path);
    deleteTrieNode(trieRoot, path);

    if (r->prevSibling != NULL)
    {
        r->prevSibling->nextSibling = r->nextSibling;
        if (r->nextSibling != NULL)
        {
            r->nextSibling->prevSibling = r->prevSibling;
        }
    }
    else
    {
        r->parent->firstChild = r->nextSibling;
        if (r->nextSibling != NULL)
        {
            r->nextSibling->prevSibling = NULL;
        }
    }
    free(r->path);
    free(r);
    pthread_mutex_unlock(&recordsLock);
}

void removePrefix(char *str, const char *prefix)
{
    size_t prefixLen = strlen(prefix);
    size_t strLen = strlen(str);

    if (strLen >= prefixLen && strncmp(str, prefix, prefixLen) == 0)
    {
        memmove(str, str + prefixLen, strLen - prefixLen + 1); // +1 to include the null terminator
        if (str[0] == '/')
        {
            memmove(str, str + 1, strLen - prefixLen); // Remove the leading '/'
        }
    }
}

void concatenateStrings(char *result, const char *A, const char *B, const char *C, const char *D)
{
    strcpy(result, A);
    strcat(result, " ");
    strcat(result, B);
    strcat(result, "/");
    strcat(result, C);
    strcat(result, " ");
    strcat(result, D);
}

void makeAccessibleAferCopy(struct record *r)
{
    if (r == NULL)
    {
        return;
    }
    struct record *child = r->firstChild;
    while (child != NULL)
    {
        if (child->isValid == false)
        {
            makeAccessibleAferCopy(child);
        }
        child = child->nextSibling;
    }
}

int copyLocally(struct record *curr_rec, char *mdir, char *base_dir, struct ssDetails *ss, struct ssDetails *ss_read)
{
    if (curr_rec == NULL || curr_rec->isValid == false)
    {
        return 0;
    }
    char *dup = strdup(curr_rec->path);
    int res1 = 0, res3 = 0;

    char new_request[4096];
    bzero(new_request, sizeof(new_request));
    int bytesRecv;

    if (curr_rec->isDir)
    {
        removePrefix(dup, mdir);
        concatenateStrings(new_request, "MKDIR", base_dir, dup, curr_rec->originalPerms);

        res3 = send(ss->connfd, new_request, sizeof(new_request), 0);
        char ackStatus[4096];
        bzero(ackStatus, sizeof(ackStatus));
        bytesRecv = recv(ss->connfd, ackStatus, sizeof(ackStatus), 0);

        // add to record
        char *arg_arr[3];
        parse_input(arg_arr, new_request);
        struct record *r = malloc(sizeof(struct record));
        r->path = malloc(sizeof(char) * 4096);
        strcpy(r->path, arg_arr[1]);
        r->orignalSS = ss;
        strcpy(r->originalPerms, arg_arr[2]);
        r->isDir = true;
        strcpy(r->currentPerms, arg_arr[2]);
        r->size = 0;
        r->firstChild = NULL;
        r->nextSibling = NULL;
        r->parent = NULL;
        r->prevSibling = NULL;
        r->isValid = false;
        pthread_mutex_init(&(r->record_lock), NULL);
        addToRecords(r);

        res1 = copyLocally(curr_rec->firstChild, mdir, base_dir, ss, ss_read);
    }
    else
    {
        removePrefix(dup, mdir);
        concatenateStrings(new_request, "MKFILE", base_dir, dup, curr_rec->originalPerms);

        res3 = send(ss->connfd, new_request, sizeof(new_request), 0);
        char ackStatus1[4096];
        bzero(ackStatus1, sizeof(ackStatus1));
        bytesRecv = recv(ss->connfd, ackStatus1, sizeof(ackStatus1), 0);

        // add to record
        char *arg_arr[3];
        parse_input(arg_arr, new_request);
        struct record *r = malloc(sizeof(struct record));
        r->path = malloc(sizeof(char) * 4096);
        strcpy(r->path, arg_arr[1]);
        r->orignalSS = ss;
        strcpy(r->originalPerms, arg_arr[2]);
        r->isDir = false;
        strcpy(r->currentPerms, arg_arr[2]);
        r->size = 0;
        r->firstChild = NULL;
        r->nextSibling = NULL;
        r->parent = NULL;
        r->prevSibling = NULL;
        r->isValid = false;
        pthread_mutex_init(&(r->record_lock), NULL);
        addToRecords(r);

        // send request for downloading file locally
        char file_buffer[4096];
        bzero(file_buffer, sizeof(file_buffer));

        bzero(file_buffer, sizeof(file_buffer));
        strcpy(file_buffer, "WRITEFILE");
        strcat(file_buffer, " ");
        strcat(file_buffer, curr_rec->path);
        // printf("file request: %s\n", file_buffer);

        char recvFileName[1024];
        strcpy(recvFileName, basename(curr_rec->path));
        strcpy(recvFileName + strlen(recvFileName), "_copy");

        // receive file
        int res4 = send(ss_read->connfd, file_buffer, sizeof(file_buffer), 0);                    // error
        int resp = receiveFileCopy(recvFileName, ss_read->connfd, curr_rec->originalPerms, true); // error

        char ackStatus2[4096];
        bzero(ackStatus2, sizeof(ackStatus2));
        bytesRecv = recv(ss_read->connfd, ackStatus2, sizeof(ackStatus2), 0);

        // send request for accepting file in server
        char file_buffer2[4096];
        bzero(file_buffer2, sizeof(file_buffer2));

        bzero(file_buffer2, sizeof(file_buffer));
        strcpy(file_buffer2, "READFILE");
        strcat(file_buffer2, " ");
        strcat(file_buffer2, base_dir);
        strcat(file_buffer2, "/");
        strcat(file_buffer2, dup);
        strcat(file_buffer2, " ");
        strcat(file_buffer2, curr_rec->originalPerms);
        // printf("file request: %s\n", file_buffer2);

        // send file
        int res5 = send(ss->connfd, file_buffer2, sizeof(file_buffer2), 0); // error
        int resp2 = sendFileCopy(recvFileName, ss->connfd, false);          // error

        char ackStatus3[4096];
        bzero(ackStatus3, sizeof(ackStatus3));
        bytesRecv = recv(ss->connfd, ackStatus3, sizeof(ackStatus3), 0);

        remove(recvFileName);
    }
    int res2 = copyLocally(curr_rec->nextSibling, mdir, base_dir, ss, ss_read);
    if (res1 == 0 && res2 == 0 && res3 > 0)
    {
        return 0;
    }
    else
    {
        return -1; // error
    }
}

void logMessage(const char *message, const char *ip, int port)
{
    pthread_mutex_lock(&loggingLock);
    time_t now;
    struct tm *timestamp;
    char buffer[80];

    time(&now);
    timestamp = localtime(&now);

    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timestamp);

    printf(CYAN_COLOR "[%s] [%s:%d] -> %s\n" RESET_COLOR, buffer, ip, port, message);
    pthread_mutex_unlock(&loggingLock);
}

void *acceptClientRequests(void *args)
{
    struct cDetails *cli = (struct cDetails *)args;
    int bytesRecv, bytesSent;
    while (1)
    {
        // recieve the client request
        char request[4096];
        bzero(request, sizeof(request));
        bytesRecv = recv(cli->connfd, request, sizeof(request), 0);
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

        printf(YELLOW_COLOR "Comamnd from client %d: \'%s\'\n" RESET_COLOR, cli->id, request);

        char *arg_arr[3];
        parse_input(arg_arr, request);
        char *request_command = arg_arr[0];

        if (strcmp(request_command, "RMFILE") == 0 || strcmp(request_command, "RMDIR") == 0)
        {
            // RMFILE/RMDIR path
            struct record *r = getRecord(arg_arr[1]);
            if (r == NULL)
            {
                // sending ack status to client
                handleFileOperationError("no_path");
                bytesSent = send(cli->connfd, "no_path", sizeof("no_path"), 0);
                if (bytesSent == -1)
                {
                    handleNetworkErrors("send");
                }
                continue;
            }

            pthread_mutex_lock(&r->record_lock);
            struct ssDetails *ss = r->orignalSS;
            // sending to ss
            bytesSent = send(ss->connfd, request, sizeof(request), 0);
            if (bytesSent == -1)
            {
                handleNetworkErrors("send");
            }

            // receiving ack status from ss
            char ackStatus[4096];
            bzero(ackStatus, sizeof(ackStatus));
            bytesRecv = recv(ss->connfd, ackStatus, sizeof(ackStatus), 0);
            pthread_mutex_unlock(&r->record_lock);

            if (bytesRecv == -1)
            {
                handleNetworkErrors("recv");
            }

            if (strcmp(ackStatus, "SUCCESS") == 0)
            {
                printf(YELLOW_COLOR "Command successfully executed\n" RESET_COLOR);
                removeFromRecords(arg_arr[1]);
            }
            else
            {
                handleAllErrors(ackStatus);
                printf(YELLOW_COLOR "Command failed\n" RESET_COLOR);
            }
            // sending ack status to client
            bytesSent = send(cli->connfd, ackStatus, sizeof(ackStatus), 0);
            if (bytesSent == -1)
            {
                handleNetworkErrors("send");
            }

            continue;
        }

        else if (strcmp(request_command, "MKDIR") == 0 || strcmp(request_command, "MKFILE") == 0)
        {
            char *arg_copy = strdup(arg_arr[1]);

            // file or dir already exists
            if (getRecord(arg_copy) != NULL)
            {
                if (strcmp(request_command, "MKDIR") == 0)
                {
                    handleFileOperationError("dir_exists");
                    bytesSent = send(cli->connfd, "dir_exists", sizeof("dir_exists"), 0);
                }
                if (strcmp(request_command, "MKFILE") == 0)
                {
                    handleFileOperationError("file_exists");
                    bytesSent = send(cli->connfd, "file_exists", sizeof("file_exists"), 0);
                }
                if (bytesSent == -1)
                {
                    handleNetworkErrors("send");
                }
                continue;
            }

            char *baseDir = dirname(arg_copy);
            struct record *r = getRecord(baseDir);

            if (r == NULL)
            {
                // sending ack status to client
                handleFileOperationError("no_path");
                bytesSent = send(cli->connfd, "no_path", sizeof("no_path"), 0);
                if (bytesSent == -1)
                {
                    handleNetworkErrors("send");
                }
                continue;
            }

            pthread_mutex_lock(&r->record_lock);
            struct ssDetails *ss = r->orignalSS;

            // sending to ss
            bytesSent = send(ss->connfd, request, sizeof(request), 0);
            if (bytesSent == -1)
            {
                handleNetworkErrors("send");
            }

            // receiving ack status from ss
            char ackStatus[4096];
            bzero(ackStatus, sizeof(ackStatus));
            bytesRecv = recv(ss->connfd, ackStatus, sizeof(ackStatus), 0);
            pthread_mutex_unlock(&r->record_lock);

            if (bytesRecv == -1)
            {
                handleNetworkErrors("recv");
            }

            if (strcmp(ackStatus, "SUCCESS") == 0)
            {
                printf(YELLOW_COLOR "Command successfully executed\n" RESET_COLOR);

                // need to add record
                struct record *r = malloc(sizeof(struct record));

                r->path = malloc(sizeof(char) * 4096);
                strcpy(r->path, arg_arr[1]);
                r->orignalSS = ss;
                if (strcmp(request_command, "MKDIR") == 0)
                {
                    strcpy(r->originalPerms, "drwxr-xr-x");
                    r->isDir = true;
                    strcpy(r->currentPerms, "drwxr-xr-x");
                }
                if (strcmp(request_command, "MKFILE") == 0)
                {
                    strcpy(r->originalPerms, "-rw-r--r--");
                    r->isDir = false;
                    strcpy(r->currentPerms, "-rw-r--r--");
                }
                r->backupSS1 = NULL;
                r->backupSS2 = NULL;
                r->size = 0;
                
                r->firstChild = NULL;
                r->nextSibling = NULL;
                r->parent = NULL;
                r->prevSibling = NULL;
                r->isValid = true;
                pthread_mutex_init(&(r->record_lock), NULL);

                addToRecords(r);
            }
            else
            {
                handleAllErrors(ackStatus);
                printf(YELLOW_COLOR "Command failed\n" RESET_COLOR);
            }
            // sending ack status to client
            bytesSent = send(cli->connfd, ackStatus, sizeof(ackStatus), 0);
            if (bytesSent == -1)
            {
                handleNetworkErrors("send");
            }

            continue;
        }

        else if (strcmp(request_command, "COPY") == 0)
        {
            struct record *r1 = getRecord(arg_arr[1]);
            struct record *r2 = getRecord(arg_arr[2]);
            if (r1 == NULL || r2 == NULL)
            {
                // sending ack status to client
                handleFileOperationError("no_path");
                bytesSent = send(cli->connfd, "no_path", sizeof("no_path"), 0);
                if (bytesSent == -1)
                {
                    handleNetworkErrors("send");
                }
                continue;
            }
            else if (r2->isDir == 0)
            {
                handleFileOperationError("not_dir");
                bytesSent = send(cli->connfd, "not_dir", sizeof("not_dir"), 0);
                if (bytesSent == -1)
                {
                    handleNetworkErrors("send");
                }
                continue;
            }

            char *mdir = dirname(strdup(r1->path));

            int resp = copyLocally(r1, mdir, r2->path, r2->orignalSS, r1->orignalSS); // error

            // make copied paths accessible
            makeAccessibleAferCopy(getRecord(r2->path));

            char ackStatus[4096];
            if (resp == 0)
            {
                strcpy(ackStatus, "SUCCESS");
                printf(YELLOW_COLOR "Command successfully executed\n" RESET_COLOR);
            }
            else
            {
                strcpy(ackStatus, "ERROR");
            }

            bytesSent = send(cli->connfd, ackStatus, sizeof(ackStatus), 0);
            if (bytesSent == -1)
            {
                handleNetworkErrors("send");
            }
            continue;
        }

        else if (strcmp(request_command, "READ") == 0 || strcmp(request_command, "WRITE") == 0 || strcmp(request_command, "FILEINFO") == 0)
        {
            bool error = false;
            char *path = arg_arr[1];
            // check if record exists

            struct record *r = getRecord(path);
            // if(strcmp(request_command, "WRITE") == 0) pthread_mutex_lock(&r->record_lock);
            struct ssDetails *ss;
            if (r == NULL)
            {
                ss = malloc(sizeof(struct ssDetails));
                ss->id = -1; // invalid record
                handleFileOperationError("no_path");
                error = true;
            }
            else if (r->isDir == true)
            {
                ss = malloc(sizeof(struct ssDetails));
                ss->id = -2; // invalid record
                handleFileOperationError("not_file");
                error = true;
            }
            else
            {
                ss = r->orignalSS;
            }
            if (strcmp(request_command, "WRITE") == 0)
            {
                pthread_mutex_lock(&r->record_lock);
            }
            // printf("SS Details: %s:%d\n", ss->ip, ss->cliPort);
            // send ss details
            int bytesSent = send(cli->connfd, ss, sizeof(struct ssDetails), 0);
            if (bytesSent == -1)
            {
                handleNetworkErrors("send");
            }
            if (!error)
            {
                printf(YELLOW_COLOR "Storage Server details sent to client\n" RESET_COLOR);
            }
            if (strcmp(request_command, "WRITE") == 0)
            {
                pthread_mutex_unlock(&r->record_lock);
            }
        }
    }
    return NULL;
}

void *addPaths(void *args)
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
        char logMsg[100] = "Recieved: Path - ";
        strcpy(logMsg + strlen(logMsg), det.path);
        logMessage(logMsg, ss->ip, ss->nmPort);

        if (getRecord(det.path) != NULL)
        {
            int bytesSent = send(ss->addPathfd, "EXISTS", strlen("EXISTS"), 0);
            if (bytesSent == -1)
            {
                handleNetworkErrors("send");
            }
            logMessage("Sent: EXISTS", ss->ip, ss->nmPort);

            continue;
        }

        struct record *r = malloc(sizeof(struct record));

        r->path = malloc(sizeof(char) * 4096);
        strcpy(r->path, det.path);
        r->orignalSS = ss;
        strcpy(r->originalPerms, det.perms);
        r->isDir = det.isDir;
        strcpy(r->currentPerms, det.perms);
        r->backupSS1 = NULL;
        r->backupSS2 = NULL;
        r->size = det.size;

        r->firstChild = NULL;
        r->nextSibling = NULL;
        r->parent = NULL;
        r->prevSibling = NULL;
        r->isValid = true;
        pthread_mutex_init(&(r->record_lock), NULL);

        addToRecords(r);
        int bytesSent = send(ss->addPathfd, "ADDED", strlen("ADDED"), 0);
        if (bytesSent == -1)
        {
            handleNetworkErrors("send");
        }
        logMessage("Sent: ADDED", ss->ip, ss->nmPort);
    }

    return NULL;
}

void addClient(int connfd, char *hostIP, int hostPort)
{
    while (validCli[clientCount + 1])
    {
        clientCount++;
        clientCount %= 10000;
    }
    clientDetails[clientCount].connfd = connfd;
    strcpy(clientDetails[clientCount].ip, hostIP);
    clientDetails[clientCount].port = hostPort;
    clientDetails[clientCount].id = clientCount + 1;
    validCli[clientCount + 1] = true;

    printf(YELLOW_COLOR "[+] Client %d Joined\n" RESET_COLOR, clientDetails[clientCount].id);

    pthread_create(&clientThreads[clientCount], NULL, acceptClientRequests, &clientDetails[clientCount]);
}

void addStorageServer(int connfd, char *hostIP, int hostPort)
{
    storageServerCount++;
    bzero(&storageServers[storageServerCount], sizeof(storageServers[storageServerCount]));
    int bytesRecv = recv(connfd, &storageServers[storageServerCount], sizeof(storageServers[storageServerCount]), 0);
    if (bytesRecv == -1)
    {
        // perror("recv");
        handleNetworkErrors("recv");
        return;
    }
    storageServers[storageServerCount].connfd = connfd;
    storageServers[storageServerCount].id = storageServerCount;
    validSS[storageServerCount] = true;
    logMessage("Recieved Storage Server Details", hostIP, hostPort);

    // create a new sockfd for adding paths dynamically
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    connfd = accept(nmSock, (struct sockaddr *)&cli, &len);
    if (connfd < 0)
    {
        // perror("accept");
        handleNetworkErrors("accept");
        exit(0);
    }

    storageServers[storageServerCount].addPathfd = connfd;

    // check for reconnection
    bool reconnect = false;
    int i = 0;
    for (i = 1; i < storageServerCount; i++)
    {
        if (validSS[storageServers[i].id] == false && strcmp(storageServers[i].ip, storageServers[storageServerCount].ip) == 0 && storageServers[i].nmPort == storageServers[storageServerCount].nmPort)
        {
            validSS[storageServers[i].id] = true;
            reconnect = true;
            storageServers[i].addPathfd = storageServers[storageServerCount].addPathfd;
            storageServers[i].connfd = storageServers[storageServerCount].connfd;
            break;
        }
    }

    if (reconnect)
    {
        validSS[storageServerCount] = true;
        printf(YELLOW_COLOR "[+] Storage Server %d Reconnected from %s:%d\n    Port for Client Communication: %d\n" RESET_COLOR, storageServers[i].id, storageServers[i].ip, storageServers[i].nmPort, storageServers[i].cliPort);
        storageServerCount--;
    }
    else
    {
        i = storageServerCount;

        printf(YELLOW_COLOR "[+] Storage Server %d Joined from %s:%d\n    Port for Client Communication: %d\n" RESET_COLOR, storageServers[storageServerCount].id, storageServers[storageServerCount].ip, storageServers[storageServerCount].nmPort, storageServers[storageServerCount].cliPort);
    }

    pthread_t addpathThread;
    pthread_create(&addpathThread, NULL, addPaths, (void *)&storageServers[i]);
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
        // log
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli.sin_addr), ip, INET_ADDRSTRLEN);
        logMessage("Recieved accept request", ip, ntohs(cli.sin_port));

        char buffer[4096];
        bzero(buffer, sizeof(buffer));
        int bytesRecv = recv(connfd, buffer, sizeof(buffer), 0);
        if (bytesRecv == -1)
        {
            handleNetworkErrors("recv");
            // perror("recv");
        }
        char logMsg[100];
        strcpy(logMsg, "Recieved: ");
        strcpy(logMsg + strlen(logMsg), buffer);
        logMessage(logMsg, ip, ntohs(cli.sin_port));

        char joinAcceptedMsg[100] = "ACCEPTED JOIN";
        int bytesSent = send(connfd, joinAcceptedMsg, sizeof(joinAcceptedMsg), 0);
        if (bytesSent == -1)
        {
            handleNetworkErrors("send");
            // perror("send");
        }
        logMessage("Sent: ACCEPTED JOIN", ip, ntohs(cli.sin_port));

        if (strcmp(buffer, "JOIN_AS Storage Server") == 0)
        {
            pthread_mutex_lock(&hostLock);
            addStorageServer(connfd, ip, ntohs(cli.sin_port));
            pthread_mutex_unlock(&hostLock);
        }
        if (strcmp(buffer, "JOIN_AS Client") == 0)
        {
            pthread_mutex_lock(&hostLock);
            addClient(connfd, ip, ntohs(cli.sin_port));
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
    trieRoot = initTrieNode();
    myCache = initCache();
    root = malloc(sizeof(struct record));
    root->firstChild = NULL;
    root->nextSibling = NULL;
    root->path = malloc(sizeof(char) * 1);
    root->path[0] = '\0';

    for (int i = 0; i < 10000; i++)
    {
        validCli[i] = false;
        validSS[i] = false;
    }
    int rc = pthread_mutex_init(&hostLock, NULL);
    assert(rc == 0);
    rc = pthread_mutex_init(&recordsLock, NULL);
    assert(rc == 0);
    rc = pthread_mutex_init(&loggingLock, NULL);
    assert(rc == 0);

    pthread_t acceptHostThread;
    pthread_create(&acceptHostThread, NULL, acceptHost, &nmSock);

    // accpeting server and clinet are infine loops and continue till naming server terminated
    pthread_join(acceptHostThread, NULL);

    freeCache(myCache);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CACHE_SIZE 3
#define MAX_FILE_PATH_LENGTH 256

typedef struct cacheCell
{
    char filePath[MAX_FILE_PATH_LENGTH];
    char ip[16];
    int port;
    struct cacheCell *next;
    struct cacheCell *prev;
} cacheCell;

typedef struct LRUCache
{
    cacheCell *head;
    cacheCell *tail;
    int numFiles;
    int maxFiles;
} LRUCache;

bool isCacheEmpty(LRUCache *lrucache)
{
    if (lrucache->numFiles == 0)
        return 1;
    return 0;
}

bool isCacheFull(LRUCache *lrucache)
{
    if (lrucache->numFiles == lrucache->maxFiles)
        return 1;
    return 0;
}

LRUCache *initCache()
{
    LRUCache *myCache = (LRUCache *)malloc(sizeof(LRUCache));
    myCache->head = NULL;
    myCache->tail = NULL;
    myCache->numFiles = 0;
    myCache->maxFiles = MAX_CACHE_SIZE;

    return myCache;
}

void printCache(LRUCache *myCache)
{
    cacheCell *temp = myCache->head;
    while (temp != NULL)
    {
        printf("(%s, %s, %d)\n", temp->filePath, temp->ip, temp->port);
        temp = temp->next;
    }
}

cacheCell *createCacheCell(char *filePath, char *ip, int port)
{
    cacheCell *newNode = (cacheCell *)malloc(sizeof(cacheCell));
    strcpy(newNode->filePath, filePath);
    strcpy(newNode->ip, ip);
    newNode->port = port;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

void moveCelltoStart(LRUCache *myCache, cacheCell *cell)
{
    if (cell == myCache->head)
        return;

    if (cell->prev != NULL)
        cell->prev->next = cell->next;

    if (cell->next != NULL)
        cell->next->prev = cell->prev;

    // Update head and tail
    if (cell == myCache->tail)
        myCache->tail = cell->prev;

    cell->next = myCache->head;
    cell->prev = NULL;

    if (myCache->head != NULL)
        myCache->head->prev = cell;

    myCache->head = cell;

    // If the list was empty, update the tail
    if (myCache->tail == NULL)
        myCache->tail = myCache->head;
}

void addFile(LRUCache *myCache, char *filePath, char *ip, int port)
{
    cacheCell *newNode = createCacheCell(filePath, ip, port);

    newNode->next = myCache->head;

    if (myCache->head != NULL)
    {
        myCache->head->prev = newNode;
    }

    myCache->head = newNode;

    if (myCache->tail == NULL)
    {
        myCache->tail = myCache->head;
    }

    // If the cache is full, remove the last element
    if (isCacheFull(myCache))
    {
        cacheCell *lastNode = myCache->tail;
        myCache->tail = lastNode->prev;
        if (myCache->tail != NULL)
        {
            myCache->tail->next = NULL;
        }

        free(lastNode);
    }
    else
    {
        myCache->numFiles++;
    }
}

int searchFileInCache(LRUCache *myCache, char *filePath, char *ip, int *port)
{
    cacheCell *current = myCache->head;

    while (current != NULL)
    {
        if (strcmp(current->filePath, filePath) == 0)
        {
            moveCelltoStart(myCache, current);
            strcpy(ip, current->ip);
            *port = current->port;
            printf("Retrieved from cache!\n");
            return 1;
        }
        current = current->next;
    }

    return 0;
}

void freeCache(LRUCache *myCache)
{
    cacheCell *temp = myCache->head;
    cacheCell *temp2;

    while (temp != NULL)
    {
        temp2 = temp->next;
        free(temp);
        temp = temp2;
    }

    free(myCache);
}

int main()
{
    LRUCache *myCache = initCache();

    addFile(myCache, "/path/file1.txt", "192.168.1.1", 8080);
    addFile(myCache, "/path/file2.txt", "192.168.1.2", 9090);
    addFile(myCache, "/path/file3.txt", "192.168.1.3", 6060);

    printf("\n\n\n");
    printCache(myCache);
    printf("\n\n\n");

    char ip[16];
    int port;
    if (searchFileInCache(myCache, "/path/file2.txt", ip, &port))
        printf("IP: %s, Port: %d\n", ip, port);
    else
        printf("File path not found.\n");

    if (searchFileInCache(myCache, "/path/nonexistent.txt", ip, &port))
        printf("IP: %s, Port: %d\n", ip, port);
    else
        printf("File path not found.\n");

    printf("\n\n\n");
    printCache(myCache);
    printf("\n\n\n");

    addFile(myCache, "/path/file4.txt", "192.168.1.4", 7070);
    printf("\n\n\n");
    printCache(myCache);
    printf("\n\n\n");

    freeCache(myCache);

    return 0;
}

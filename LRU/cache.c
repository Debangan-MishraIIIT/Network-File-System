#include "headers.h"

typedef struct cacheCell
{
    struct cacheCell *prev;
    struct cacheCell *next;
    int ip;
    int port;
} cacheCell;

typedef struct cache
{
    int numFiles;
    int maxFiles;
    cacheCell *front;
    cacheCell *rear;
} cache;

cacheCell *createCacheCell(int ip, int port)
{
    cacheCell *cnew = (cacheCell *)malloc(sizeof(cacheCell));
    cnew->next = NULL;
    cnew->prev = NULL;
    cnew->ip = ip;
    cnew->port = port;
    return cnew;
}

cache *createCache(int size)
{
    cache *cnew = (cache *)malloc(sizeof(cache));
    cnew->front = NULL;
    cnew->rear = NULL;
    cnew->maxFiles = size;
    cnew->numFiles = 0;
    return cnew;
}

bool isCacheFull(cache *c)
{
    return (c->numFiles == c->maxFiles);
}

bool isCacheEmpty(cache *c)
{
    return (c->numFiles == 0);
}

void printCache(cache *c)
{
    if (!isCacheEmpty(c))
    {
        cacheCell *temp = c->front;
        int i = 1;
        do
        {
            printf("%d ---- %d\t%d\n", i, temp->ip, temp->port);
            i++;
            temp = temp->next;
        } while (temp != c->front);
    }
    else
    {
        printf("Cache is empty!\n");
    }
}

void removeFileFromCache(cache *c)
{
    if (!isCacheEmpty(c))
    {
        if (c->front == c->rear)
        {
            free(c->front);
            c->front = NULL;
            c->rear = NULL;
        }
        else
        {
            cacheCell *temp = c->rear;
            c->rear = c->rear->prev;
            c->rear->next = NULL;
            free(temp);
        }
        c->numFiles--;
    }
}

void addFileToCache(cache *c, int ip, int port)
{
    if (isCacheFull(c))
    {
        removeFileFromCache(c);
    }

    cacheCell *cnew = createCacheCell(ip, port);
    if (isCacheEmpty(c))
    {
        c->front = cnew;
        c->rear = cnew;
    }
    else
    {
        cnew->next = c->front;
        cnew->prev = c->rear;
        c->rear->next = cnew;
        cnew->next = c->front;
        c->front->prev = cnew;
        c->front = cnew;
    }

    c->numFiles++;
}

int main()
{
    cache *c = createCache(3);

    // printCache(c);

    addFileToCache(c, 192, 80);
    addFileToCache(c, 203, 443);
    addFileToCache(c, 127, 8080);
    printCache(c);
    printf("\n\n\n");
    addFileToCache(c, 169, 69);
    printCache(c);
    printf("\n\n\n");
    addFileToCache(c, 156, 22);
    printCache(c);
    printf("\n\n\n");
    addFileToCache(c, 127, 8080);
    printCache(c);
    printf("\n\n\n");

    // printCache(c);

    printf("lol\n");

    return 0;
}

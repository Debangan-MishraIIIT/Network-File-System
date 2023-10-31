#include "headers.h"

// replace data type of "data" with relevant/suitable data type.
// In the case of NFS, we will be storing multiple data types like file path, IP of SS_x, port of SS_x, etc.
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
    // bool *presentArray;
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
    // cnew->presentArray = (bool *)malloc(sizeof(bool) * cnew->maxFiles);
    // for (int i = 0; i < cnew->maxFiles; i++)
    //     cnew->presentArray[i] = false;
    return cnew;
}

bool isCacheFull(cache *c)
{
    return (c->maxFiles == c->numFiles);
}

bool isCacheEmpty(cache *c)
{
    return (c->numFiles == 0);
}

void printCache(cache *c)
{
    if (!isCacheEmpty(c))
    {
        cacheCell *temp = c->front->next;
        int i = 1;
        while (temp != c->front)
        {
            printf("%d ---- %d\t\t%d\n", i, temp->ip, temp->port);
            i++;
            temp = temp->next;
        }
    }
    else
        printf("Cache is empty!\n");
}

void dequeueCell(cache *c)
{
    if (!isCacheEmpty(c))
    {
        if (c->front == c->rear)
            c->front = NULL;
        cacheCell *temp = c->rear;
        c->rear = c->rear->prev;
        if (c->rear != NULL)
            c->rear->next = NULL;
        free(temp);
        c->numFiles--;
    }
}

void enqueueCell(cache *c, int ip, int port)
{
    if (isCacheFull(c))
        dequeueCell(c);

    cacheCell *cnew = createCacheCell(ip, port);
    cnew->next = c->front;
    if (c->front != NULL)
    {
        c->front->prev = cnew;
    }
    c->front = cnew;

    if (c->rear == NULL)
    {
        c->rear = cnew;
    }

    c->numFiles++;
    
}

int main()
{
}

#include "headers.h"

bool isCacheEmpty(LRUCache *myCache)
{
    if (myCache->numFiles == 0)
        return 1;
    return 0;
}

bool isCacheFull(LRUCache *myCache)
{
    if (myCache->numFiles == myCache->maxFiles)
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
        printf("(%s, %s, %d)\n", temp->tableEntry->path, temp->tableEntry->orignalSS->ip, temp->tableEntry->orignalSS->cliPort);
        temp = temp->next;
    }
}

cacheCell *createCacheCell(struct record *newTableEntry)
{
    cacheCell *newNode = (cacheCell *)malloc(sizeof(cacheCell));
    newNode->tableEntry = newTableEntry;
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

void addFile(LRUCache *myCache, struct record *newTableEntry)
{
    cacheCell *newNode = createCacheCell(newTableEntry);

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
            myCache->tail->next = NULL;
        free(lastNode);
    }
    else
        myCache->numFiles++;
}

struct record *searchFileInCache(LRUCache *myCache, char *filePath)
{
    cacheCell *temp = myCache->head;

    while (temp != NULL)
    {
        if (strcmp(temp->tableEntry->path, filePath) == 0)
        {
            moveCelltoStart(myCache, temp);
            // printf("Retrieved from cache!\n");
            return temp->tableEntry;
        }
        temp = temp->next;
    }
    return NULL;
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

// int removeFileFromCache(LRUCache *myCache, char *filePath)
// {
//     cacheCell *temp = myCache->head;

//     if (temp == NULL)
//     {
//         return 1;
//     }
//     else if (temp->next == NULL)
//     {
//         cacheCell *temp2 = temp;
//         myCache->head = NULL;
//         myCache->numFiles--;
//         // printf("Deleted from cache!\n");
//         free(temp2);

//         return 1;
//     }

//     while (temp->next != NULL)
//     {
//         if (strcmp(temp->tableEntry->path, filePath) == 0)
//         {
//             // moveCelltoStart(myCache, temp);
//             cacheCell *temp2 = temp;
//             if (temp->next != NULL)
//             {
//                 temp->next = temp->next->next;
//             }
//             myCache->numFiles--;
//             // printf("Deleted from cache!\n");
//             free(temp2);
//             return 1;
//         }
//         temp = temp->next;
//     }
//     return 0;
// }
int removeFileFromCache(LRUCache *myCache, char *filePath)
{
    cacheCell *temp = myCache->head;

    while (temp != NULL)
    {
        if (strcmp(temp->tableEntry->path, filePath) == 0)
        {
            if (temp == myCache->head)
            {
                myCache->head = temp->next;
                if (myCache->head != NULL)
                    myCache->head->prev = NULL;
            }
            else if (temp == myCache->tail)
            {
                myCache->tail = temp->prev;
                if (myCache->tail != NULL)
                    myCache->tail->next = NULL;
            }
            else
            {
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev;
            }

            free(temp);
            myCache->numFiles--;
            return 0;
        }
        temp = temp->next;
    }
    return 0;
}

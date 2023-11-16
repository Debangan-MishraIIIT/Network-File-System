#ifndef __CACHE_H
#define __CACHE_H

#define MAX_CACHE_SIZE 5
#define MAX_FILE_PATH_LENGTH 256

typedef struct cacheCell
{
    struct record *tableEntry;
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

LRUCache *initCache();
bool isCacheEmpty(LRUCache *myCache);
bool isCacheFull(LRUCache *myCache);
void printCache(LRUCache *myCache);
cacheCell *createCacheCell(struct record *newTableEntry);
void moveCelltoStart(LRUCache *myCache, cacheCell *cell);
void addFile(LRUCache *myCache, struct record *newTableEntry);
struct record *searchFileInCache(LRUCache *myCache, char *filePath);
void freeCache(LRUCache *myCache);

#endif
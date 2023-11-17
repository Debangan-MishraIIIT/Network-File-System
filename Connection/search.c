#include "headers.h"

TrieNode *initTrieNode()
{
    TrieNode *newNode = (TrieNode *)malloc(sizeof(TrieNode));
    if (newNode)
    {
        for (int i = 0; i < NUM_CHILDREN; i++)
        {
            newNode->children[i] = NULL;
        }
        newNode->tableEntry = NULL;
        return newNode;
    }
    else
    {
        handleSYSandInputErrors("malloc");
        return NULL;
    }
}

void insertRecordToTrie(TrieNode *root, struct record *newTableEntry)
{
    TrieNode *temp = root;
    for (int i = 0; newTableEntry->path[i] != '\0'; i++)
    {
        int index = (int)newTableEntry->path[i];
        if (temp->children[index] == NULL)
        {
            temp->children[index] = initTrieNode(); // for each character in the file path, create another trienode....You know how tries work :)
        }
        temp = temp->children[index];
    }
    temp->tableEntry = newTableEntry;
    temp->tableEntry->creationTime = time(NULL); // 
}

// can modify the search algo accordingly to return appropriate values.
// NULL if file not found, a struct which contains the ip and port or something.
struct record *search(TrieNode *root, char *file_path)
{
    TrieNode *temp = root;
    for (int i = 0; file_path[i] != '\0'; i++)
    {
        int index = (int)file_path[i];
        if (temp->children[index] == NULL)
        {
            // if at any time, you find that there does not exist an trienode with any character of the file path, the search fails.
            handleFileOperationError("search");
            return NULL;
        }
        temp = temp->children[index];
    }
    if (temp->tableEntry)
    {
        printf("File is located at %s in SS where IP: %s, Port: %d\n", temp->tableEntry->path, temp->tableEntry->orignalSS->ip, temp->tableEntry->orignalSS->cliPort);
        return temp->tableEntry;
    }
    else
    {
        handleFileOperationError("search");
        return NULL;
    }
}
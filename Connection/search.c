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
    }
    return newNode;
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
    // temp->rec = strdup(ip); // copy the ip and port into the struct
    // temp->port = port;
    temp->tableEntry = newTableEntry;
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
            // printf("File not found\n");
            return NULL;
        }
        temp = temp->children[index];
    }
    if (temp->tableEntry)
    {
        // printf("File is located at %s in SS where IP: %s, Port: %d\n", temp->tableEntry->path, temp->tableEntry->orignalSS->ip, temp->tableEntry->orignalSS->cliPort);
        return temp->tableEntry;
    }
    else
    {
        // printf("File not found\n");
        return NULL;
    }
}

int deleteTrieNode(TrieNode *root, char *file_path)
{
    TrieNode *temp = root;
    TrieNode *prev = NULL;
    int i;
    for (i = 0; file_path[i] != '\0'; i++)
    {
        int index = (int)file_path[i];
        if (temp->children[index] == NULL)
        {
            return 0;
        }
        prev = temp;
        temp = temp->children[index];
    }

    // // If the TrieNode has a table entry, delete it
    // if (temp->tableEntry)
    // {
    //     free(temp->tableEntry);
    //     temp->tableEntry = NULL;
    // }

    // Remove the TrieNode from its parent
    free(prev->children[(int)file_path[i - 1]]);
    prev->children[(int)file_path[i - 1]] = NULL;
    return 1;
}
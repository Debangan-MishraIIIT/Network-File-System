#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Note - 1: All the storage servers should send the file paths of each file in their respective directories to the naming server.
Regular updation should be done by the storage servers to ensure correct file paths.

Note - 2: This search algo only returns the ip and ports of the storage server in which the file path is specified.
After the client receives the credentials of the storage server, the client should make another request to this ip and port with the required file path.
This enables the direct communication of the client with the storage server.
*/

#define NUM_CHILDREN 256

typedef struct TrieNode
{
    char *ip;                                // ip address
    int port;                                // port of the strorage server
    struct TrieNode *children[NUM_CHILDREN]; // trie node array
} TrieNode;

TrieNode *initTrieNode()
{
    TrieNode *newNode = (TrieNode *)malloc(sizeof(TrieNode));
    if (newNode)
    {
        for (int i = 0; i < NUM_CHILDREN; i++)
        {
            newNode->children[i] = NULL;
        }
        newNode->ip = NULL;
        newNode->port = -1;
    }
    return newNode;
}

void insert(TrieNode *root, const char *file_path, const char *ip, int port)
{
    TrieNode *temp = root;
    for (int i = 0; file_path[i] != '\0'; i++)
    {
        int index = (int)file_path[i];
        if (temp->children[index] == NULL)
        {
            temp->children[index] = initTrieNode(); // for each character in the file path, create another trienode....You know how tries work :)
        }
        temp = temp->children[index];
    }
    temp->ip = strdup(ip); // copy the ip and port into the struct
    temp->port = port;
}

// can modify the search algo accordingly to return appropriate values.
// NULL if file not found, a struct which contains the ip and port or something.
void search(TrieNode *root, const char *file_path)
{
    TrieNode *temp = root;
    for (int i = 0; file_path[i] != '\0'; i++)
    {
        int index = (int)file_path[i];
        if (temp->children[index] == NULL)
        {
            // if at any time, you find that there does not exist an trienode with any character of the file path, the search fails.
            printf("File not found\n");
            return;
        }
        temp = temp->children[index];
    }
    if (temp->ip)
    {
        printf("File is located at IP: %s, Port: %d\n", temp->ip, temp->port);
    }
    else
    {
        printf("File not found\n");
    }
}

int main()
{
    TrieNode *root = initTrieNode();

    insert(root, "/path/to/file1", "192.168.1.10", 8001);
    insert(root, "/path/to/file2", "192.168.1.11", 8002);
    insert(root, "/path/to/file3", "192.168.1.12", 8003);
    insert(root, "/path/to/file4", "192.168.1.13", 8004);
    insert(root, "/path/to/dir5/dir4/file5", "192.168.1.14", 8005);
    insert(root, "/path/to/dir1/file6", "192.168.1.15", 8006);
    insert(root, "/path/to/dir2/file7", "192.168.1.16", 8007);

    const char *requested_file_path1 = "/path/to/dir5/dir4/file5";
    const char *requested_file_path2 = "/path/to/dir1/file6";
    const char *requested_file_path3 = "/path/to/file2";
    const char *requested_file_path4 = "//path/to/dir2/file69";

    search(root, requested_file_path1);
    search(root, requested_file_path2);
    search(root, requested_file_path3);
    search(root, requested_file_path4);

    return 0;
}

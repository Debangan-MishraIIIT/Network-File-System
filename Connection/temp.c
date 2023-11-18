
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure for a node in the n-ary tree
struct Node {
    char* path;
    struct Node* child;
    struct Node* sibling;
};

// Function to create a new node
struct Node* createNode(char* path) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->path = strdup(path);
    newNode->child = NULL;
    newNode->sibling = NULL;
    return newNode;
}

// Function to add a file path to the n-ary tree
void addFilePath(struct Node* root, char* filePath) {
    if (root == NULL) {
        return;
    }

    // Split the file path into individual directories
    char* token = strtok(filePath, "/");
    struct Node* parentNode = root;

    // Traverse the directories in the file path
    while (token != NULL) {
        // Check if a child node with the same path already exists
        struct Node* child = parentNode->child;
        while (child != NULL) {
            if (strcmp(child->path, token) == 0) {
                break;
            }
            child = child->sibling;
        }

        // If the child node doesn't exist, create a new node and add it as a child of the parent node
        if (child == NULL) {
            struct Node* newNode = createNode(token);
            newNode->sibling = parentNode->child;
            parentNode->child = newNode;
            parentNode = newNode;
        } else {
            parentNode = child;
        }

        token = strtok(NULL, "/");
    }
}

// Function to print the file paths in the n-ary tree
void printFilePaths(struct Node* root, int level) {
    if (root == NULL) {
        return;
    }

    // Print the current file path
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    printf("%s\n", root->path);

    // Recursively print the file paths of the children
    struct Node* child = root->child;
    while (child != NULL) {
        printFilePaths(child, level + 1);
        child = child->sibling;
    }
}

int main() {
    // Create the root node
    struct Node* root = createNode("");

    // // Add file paths to the n-ary tree
    addFilePath(root, "dirA/file1");
    addFilePath(root, "dirA/dirB/file2");
    addFilePath(root, "dirA/dirB/dirC/file3");

    // // Print the file paths in the n-ary tree
    // printFilePaths(root, 0);

    return 0;
}
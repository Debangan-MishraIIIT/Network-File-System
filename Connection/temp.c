#include <stdio.h>
#include <string.h>

void splitPath(char *inputPath, char *outputParts[2])
{
    char path[1024];
    strcpy(path, inputPath);

    char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL)
    {
        *lastSlash = '\0';
        outputParts[0] = path;
        outputParts[1] = lastSlash + 1;
    }
    else
    {
        outputParts[0] = path;
        outputParts[1] = NULL;
    }
}

int main()
{
    char path[] = "a/b/c";
    char *parts[2];

    splitPath(path, parts);

    printf("First Part: %s\n", parts[0]);
    printf("Second Part: %s\n", parts[1]);

    return 0;
}
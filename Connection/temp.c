#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>

int main() {
    const char *originalPath = "a/b/c";

    char *directoryPath = dirname(strdup(originalPath));

    printf("Original Path: %s\n", originalPath);
    printf("Directory Path: %s\n", directoryPath);

    free(directoryPath);  // Remember to free the memory allocated by strdup

    return 0;
}

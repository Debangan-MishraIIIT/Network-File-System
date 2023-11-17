#include "headers.h"

/*

// file error ===> starting with 5 (500,501,502,503....) ------------> RED
// 		1. File missing =
// 		2. No read Perms =
// 		3. No write Perms =
// 		4. convertPerms =
// 		5. mkdir , mkfile , etc errors =

// network error ===> starting with 9 (900,901,902,903...) -----------> YELLOW
// 		1. Receive Error =
// 		2. Send Error =
// 		3. Individual Error codes for helper functions like sendPathToNS , check_path_exists , etc
// 		4.

// syscall errors ===> starting with 6(600,601,602,603...) -----------> BLUE
// 		1. malloc =
// 		2. input errors =

*/

void handle_errors(char *error)
{
    printf("\e[0;31mError in: %s\n", error);
}

// file operations errors (all in red)

void handleFileOperationError(char *error)
{
    if (!strcmp(error, "make_directory"))
        printf(RED "ERROR 500: UNABLE TO CREATE A DIRECTORY (make_directory)\n" reset);

    if (!strcmp(error, "make_file"))
        printf(RED "ERROR 501: UNABLE TO CREATE A FILE (make_file)\n" reset);

    if (!strcmp(error, "opendir"))
        printf(RED "ERROR 502: UNABLE TO OPEN DIRECTORY (opendir)\n" reset);

    if (!strcmp(error, "recursive_directory_deletion"))
        printf(RED "ERROR 503: UNABLE TO DELETE DIRECTORY (recursive_directory_deletion)\n" reset);

    if (!strcmp(error, "remove"))
        printf(RED "ERROR 504: UNABLE TO REMOVE FILE (remove)\n" reset);

    if (!strcmp(error, "no_path"))
        printf(RED "ERROR 505: PATH MISSING\n" reset);

    if (!strcmp(error, "chdir"))
        printf(RED "ERROR 506: UNABLE TO CHANGE DIRECTORY (chdir)\n" reset);

    if (!strcmp(error, "opendir"))
        printf(RED "ERROR 507: UNABLE TO OPEN DIRECTORY (opendir)\n" reset);

    if (!strcmp(error, "getRecord"))
        printf(RED "ERROR 508: RECORD MISSING (getRecord)\n" reset);

    if (!strcmp(error, "search"))
        printf(RED "ERROR 509: FILE MISSING (search)\n" reset);

    if (!strcmp(error, "stat"))
        printf(RED "ERROR 510: UNABLE TO EXTRACT FILE DETAILS (stat)\n" reset);

    if (!strcmp(error, "no_file"))
        printf(RED "ERROR 511: FILE MISSING (no_file)\n" reset);

    if (!strcmp(error, "getcwd"))
        printf(RED "ERROR 512: UNABLE TO GET getcwd (getcwd)\n" reset);

    if (!strcmp(error, "open"))
        printf(RED "ERROR 513: UNABLE TO OPEN FILE (open)\n" reset);

    if (!strcmp(error, "read"))
        printf(RED "ERROR 514: UNABLE TO READ FILE (read)\n" reset);

    if (!strcmp(error, "write"))
        printf(RED "ERROR 515: UNABLE TO WRITE FILE (write)\n" reset);

    if (!strcmp(error, "remove_files_and_directory"))
        printf(RED "ERROR 516: UNABLE DELETE FILE (remove_files_and_directory)\n" reset);

    if (!strcmp(error, "recursive_directory_sending"))
        printf(RED "ERROR 517: UNABLE TO SEND DIRECTORY (recursive_directory_sending)\n" reset);
}

// syscall and input errors (all in blue)

void handleSYSandInputErrors(char *error)
{
    if (!strcmp(error, "invalid_input"))
        printf(BLU "ERROR 600: INVALID INPUT\n" reset);

    if (!strcmp(error, "malloc"))
        printf(BLU "ERROR 601: MALLOC FAILED\n" reset);
}

// network errors (all in yellow)

void handleNetworkErrors(char *error)
{
}
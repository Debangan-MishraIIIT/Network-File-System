#include "headers.h"

/*
API CALLS:
MKDIR
RMFIL
MKFIL
*/

int make_directory(char *dirname)
{
    int check;
    check = mkdir(dirname, 0777);

    // check if directory is created or not
    if (!check)
    {
        printf("Directory created\n");
        return 0;
    }
    else
    {
        // printf("Unable to create directory\n");
        return -1;
    }
}

int make_file(char *dirname)
{
    int check;
    check = creat(dirname, 0777);

    // check if file is created or not
    // if file is already existing, it is cleared, just like a normal write
    if (check == -1)
    {
        // printf("Unable to create file\n");
        return -1;
    }
    else
    {
        printf("File created\n");
    }
    return 0;
}

int isDirectory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

int recursive_directory_deletion(char *curr_address)
{
    DIR *dir = opendir(curr_address);
    if (dir == NULL)
    {
        handleFileOperationError("opendir");
        return -1;
    }

    struct dirent *dir_read;
    int retval = 0; // Initialize the return value

    while ((dir_read = readdir(dir)) != 0)
    {
        char buffer[maxlen];
        snprintf(buffer, sizeof(buffer), "%s/%s", curr_address, dir_read->d_name);
        if (isDirectory(buffer) == 1 && strcmp(dir_read->d_name, ".") != 0 && strcmp(dir_read->d_name, "..") != 0)
        {
            retval = recursive_directory_deletion(buffer);

            // Check the return value and continue with the loop if successful
            if (retval != 0)
            {
                handleFileOperationError("recursive_directory_deletion");
                closedir(dir);
                return retval;
            }
        }
        else if (isDirectory(buffer) == 0)
        {
            retval = remove(buffer);

            // Check the return value and continue with the loop if successful
            if (retval != 0)
            {
                handleFileOperationError("remove");
                closedir(dir);
                return retval;
            }
        }
    }

    retval = remove(curr_address);
    closedir(dir);
    return retval;
}

void get_request(char request[], char actual_request[])
{
    if (strlen(request) < 5)
    {
        printf("Invalid Request\n");
    }
    for (int i = 0; i < 5; i++)
    {
        actual_request[i] = request[i + 1];
    }
}

void get_path(char request[], char path[])
{
    if (strlen(request) < 5)
    {
        printf("Invalid Request\n");
    }
    for (int i = 6; i < strlen(request); i++)
    {
        path[i - 6] = request[i];
    }
    printf("\n");
}

int check_path_exists(const char *directoryPath)
{
    struct stat dirStat;
    if (stat(directoryPath, &dirStat) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int remove_files_and_directory(char *path)
{

    if (!check_path_exists(path))
    {
        return -1;
    }
    else
    {
        if (isDirectory(path))
        {
            recursive_directory_deletion(path);
        }
        else
        {
            remove(path);
        }
        return 0;
    }
}

int handle_naming_server_commands(char *command, char *inputS)
{
    // Input string
    if (strcmp("RMDIR", command) != 0 && strcmp("MKFIL", command) != 0 && strcmp("MKDIR", command) != 0)
    {
        handleSYSandInputErrors("invalid_input");
        return -1;
    }

    char *inputString = strdup(inputS);
    char *token = strtok(inputString, "/");
    char *lastToken = NULL;
    int count = 0;

    // char cwd[1000];

    while (token != NULL)
    {
        if (lastToken != NULL)
        {
            // if it is a make directory or file command, I have to make the previous directories
            if (!isDirectory(lastToken) && (strcmp(command, "MKFIL") == 0 || strcmp(command, "MKDIR") == 0))
            {
                printf("Creating directory: %s\n", lastToken);
                int err1 = make_directory(lastToken);
                if (err1 == -1)
                    handleFileOperationError("make_directory");
            }
            else if (!isDirectory(lastToken) && strcmp(command, "RMDIR") == 0)
            {
                handleFileOperationError("no_path");
                return -1;
            }
            // getcwd(cwd, sizeof(cwd));
            // printf("current directory: %s\n", cwd);
            int err2 = chdir(lastToken);
            if (err2 == -1)
                handleFileOperationError("chdir");
        }
        lastToken = token;
        token = strtok(NULL, "/");
    }

    // getcwd(cwd, sizeof(cwd));
    // printf("current directory: %s\n", cwd);
    int err3 = 0;

    if (lastToken != NULL)
    {
        if (strcmp(command, "MKFIL") == 0)
        {
            err3 = make_file(lastToken);
            if (err3 == -1)
                handleFileOperationError("make_file");
        }
        else if (strcmp(command, "MKDIR") == 0)
        {
            err3 = make_directory(lastToken);
            if (err3 == -1)
                handleFileOperationError("make_directory");
        }
        else if (strcmp(command, "RMDIR") == 0)
        {
            printf("HERE %s\n", lastToken);
            err3 = remove_files_and_directory(lastToken);
            if (err3 == -1)
                handleFileOperationError("recursive_directory_deletion");
        }
        if (err3 == 0)
        {
            printf("\e[0;32m%s SUCCESS!\n", command);
        }
    }
    return err3;
}

void parse_input(char *array[], char *inputS)
{
    char *inputString = strdup(inputS);
    char *token = strtok(inputString, " ");
    int count = 0;

    while (token != NULL)
    {
        if (count == 2)
        {
            handleSYSandInputErrors("invalid_input");
            break;
        }
        array[count] = malloc(sizeof(char) * 100);
        strcpy(array[count], token);
        count++;
        token = strtok(NULL, " ");
    }
}
#include "headers.h"

/*
API CALLS:
MKDIR
RMFILE
MKFILE
COPYDIR
*/

int send_file(char *filename, int sockfd)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }
    char buf[MAX_SIZE];
    ssize_t bytes_read = read(fd, buf, MAX_SIZE);
    while (bytes_read > 0)
    {
        ssize_t resp = write(sockfd, buf, bytes_read);
        if (resp == -1)
        {
            perror("write");
            return -1;
        }
        bzero(buf, MAX_SIZE);
        bytes_read = read(fd, buf, MAX_SIZE);
    }
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }
    return 0; // Success
}

int receive_file(char *filename, int sockfd)
{
    int fd = open(filename, O_WRONLY | O_CREAT, 0644);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }
    char buf[MAX_SIZE];
    ssize_t bytes_read = read(sockfd, buf, MAX_SIZE);
    while (bytes_read > 0)
    {
        ssize_t resp = write(fd, buf, bytes_read);
        if (resp == -1)
        {
            perror("write");
            return -1;
        }
        bzero(buf, MAX_SIZE);
        bytes_read = read(sockfd, buf, MAX_SIZE);
    }
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }
    return 0; // Success
}

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
        printf("Unable to create directory\n");
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
        printf("Unable to create file\n");
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
        handle_errors("opendir");
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
                closedir(dir);
                return retval;
            }
        }
    }

    retval = remove(curr_address);
    closedir(dir);
    return retval;
}

int recursive_directory_sending(char *curr_address, int nmfd)
{
    DIR *dir = opendir(curr_address);
    if (dir == NULL)
    {
        handle_errors("opendir");
        return -1;
    }

    struct dirent *dir_read;
    int retval = 0;

    char *sender_buffer = (char *)malloc(maxlen);
    if (sender_buffer == NULL)
    {
        perror("malloc");
        closedir(dir);
        return -1;
    }

    while ((dir_read = readdir(dir)) != NULL)
    {
        char buffer[maxlen];
        char send_buffer[maxlen];
        char recv_buffer[100];
        snprintf(buffer, sizeof(buffer), "%s/%s", curr_address, dir_read->d_name);

        if (strcmp(dir_read->d_name, ".") != 0 && strcmp(dir_read->d_name, "..") != 0 && dir_read->d_name[0] != '.')
        {
            if (isDirectory(buffer))
            {
                bzero(send_buffer, sizeof(send_buffer));
                snprintf(send_buffer, maxlen, "dir %s", buffer);
                printf("%s\n", send_buffer);

                send(nmfd, send_buffer, strlen(send_buffer), 0);
                recv(nmfd, recv_buffer, sizeof(recv_buffer), 0);
                printf("%s\n", recv_buffer);

                retval = recursive_directory_sending(buffer, nmfd);

                if (retval != 0)
                {
                    free(sender_buffer);
                    closedir(dir);
                    return retval;
                }
            }
            else
            {
                bzero(send_buffer, sizeof(send_buffer));
                snprintf(send_buffer, maxlen, "file %s", buffer);
                printf("%s\n", send_buffer);

                send(nmfd, send_buffer, strlen(send_buffer), 0);
                recv(nmfd, recv_buffer, sizeof(recv_buffer), 0);
                printf("%s\n", recv_buffer);

                if (retval != 0)
                {
                    free(sender_buffer);
                    closedir(dir);
                    return retval;
                }
            }
        }
    }

    free(sender_buffer);
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
    }
}

int receive_directory(int connfd)
{
    while (1)
    {
        int err;
        char buffer[4096];
        bzero(buffer, sizeof(buffer));
        int bytesRecv = recv(connfd, buffer, sizeof(buffer), 0);

        if (bytesRecv == -1)
        {
            handle_errors("recv");
        }
        else if (strcmp(buffer, "END") == 0)
        {
            break;
        }
        else
        {
            send(connfd, "ACK", sizeof("ACK"), 0);
        }

        char *file_array[3];
        file_separator(file_array, buffer);

        // adding socket -1 here as I do not wish to deal with copy and any form of sending
        printf("receieve from server: %s %s\n", file_array[0], file_array[1]);
        if (strcmp(file_array[0], "file") == 0)
        {
            err = handle_naming_server_commands("MKFILE", file_array[1], -1);
        }
        else if (strcmp(file_array[0], "dir") == 0)
        {
            err = handle_naming_server_commands("MKDIR", file_array[1], -1);
        }
        if (err == -1)
            return -1;
    }

    return 0;
}

int handle_naming_server_commands(char *command, char *inputS, int nmfd)
{
    // Input string
    char main_cwd[1000];
    if (getcwd(main_cwd, sizeof(main_cwd)) == NULL)
    {
        handle_errors("getcwd error");
        return -1;
    }

    if (strcmp("RMDIR", command) != 0 && strcmp("MKFILE", command) != 0 && strcmp("MKDIR", command) != 0 && strcmp("COPYFILE", command) != 0 && strcmp("COPYDIR", command) != 0 && strcmp("RMFILE", command) != 0)
    {
        handle_errors("Invalid Command");
        return -1;
    }

    char *inputString = strdup(inputS);
    char *token = strtok(inputString, "/");
    char *lastToken = NULL;
    int count = 0;

    while (token != NULL)
    {
        if (lastToken != NULL)
        {
            // if it is a make directory or file command, I have to make the previous directories
            if (!isDirectory(lastToken) && (strcmp(command, "MKFILE") == 0 || strcmp(command, "MKDIR") == 0))
            {
                printf("Creating directory: %s\n", lastToken);
                int err1 = make_directory(lastToken);
                if (err1 == -1)
                    handle_errors("make_directory");
            }
            else if (!isDirectory(lastToken) && strcmp(command, "RMDIR") == 0)
            {
                handle_errors("Path does not exist");
                return -1;
            }
            // getcwd(cwd, sizeof(cwd));
            // printf("current directory: %s\n", cwd);
            int err2 = chdir(lastToken);
            if (err2 == -1)
                handle_errors("chdir");
        }
        lastToken = token;
        token = strtok(NULL, "/");
    }

    int err3 = 0;
    if (lastToken != NULL)
    {
        if (strcmp(command, "MKFILE") == 0)
        {
            err3 = make_file(lastToken);
            if (err3 == -1)
                handle_errors("make file");
        }
        else if (strcmp(command, "MKDIR") == 0)
        {
            err3 = make_directory(lastToken);
            if (err3 == -1)
                handle_errors("make directory");
        }
        else if (strcmp(command, "RMDIR") == 0 || strcmp(command, "RMFILE") == 0)
        {
            printf("HERE %s\n", lastToken);
            err3 = remove_files_and_directory(lastToken);
            if (err3 == -1)
                handle_errors("delete file and directory");
        }
        else if (strcmp(command, "COPYDIR") == 0 || strcmp(command, "COPYFILE") == 0)
        {
            //
            int resp = recursive_directory_sending(lastToken, nmfd);
            if (resp == -1)
            {
                handle_errors("recursive directory sending");
            }
            send(nmfd, "END", sizeof("END"), 0);
        }
        if (err3 == 0)
        {
            printf("\e[0;32m%s SUCCESS!\n\e[0;37m", command);
        }
    }
    int err4 = chdir(main_cwd);
    if (err4 == -1)
    {
        handle_errors("chdir");
        return -1;
    }

    return err3;
}

void parse_input(char *array[], char *inputS)
{
    for (int i = 0; i < 3; i++)
    {
        array[i] = NULL;
    }
    char *inputString = strdup(inputS);
    char *token = strtok(inputString, " \t\n");
    int count = 0;

    while (token != NULL)
    {
        if (count >= 3)
        {
            handle_errors("invalid input");
            break;
        }
        array[count] = malloc(sizeof(char) * 100);
        strcpy(array[count], token);
        count++;
        token = strtok(NULL, " \t\n");
    }
}

void file_separator(char *array[], char *inputS)
{
    char *inputString = strdup(inputS);
    char *token = strtok(inputString, " ");
    int count = 0;
    ;

    while (token != NULL)
    {
        if (count >= 3)
        {
            handle_errors("invalid input");
            break;
        }
        array[count] = malloc(sizeof(char) * 10000);
        strcpy(array[count], token);
        count++;
        token = strtok(NULL, " ");
    }
}

mode_t reversePermissions(char *perms)
{
    mode_t mode = 0;

    // Check if the file is a directory
    mode |= (perms[0] == 'd') ? S_IFDIR : 0;

    // Owner permissions
    mode |= (perms[1] == 'r') ? S_IRUSR : 0;
    mode |= (perms[2] == 'w') ? S_IWUSR : 0;
    mode |= (perms[3] == 'x') ? S_IXUSR : 0;

    // Group permissions
    mode |= (perms[4] == 'r') ? S_IRGRP : 0;
    mode |= (perms[5] == 'w') ? S_IWGRP : 0;
    mode |= (perms[6] == 'x') ? S_IXGRP : 0;

    // Others permissions
    mode |= (perms[7] == 'r') ? S_IROTH : 0;
    mode |= (perms[8] == 'w') ? S_IWOTH : 0;
    mode |= (perms[9] == 'x') ? S_IXOTH : 0;

    return mode;
}

// void convertPermissions(mode_t st_mode, char *perms)
// {
//     perms[0] = (S_ISDIR(st_mode)) ? 'd' : '-';
//     perms[1] = (st_mode & S_IRUSR) ? 'r' : '-';
//     perms[2] = (st_mode & S_IWUSR) ? 'w' : '-';
//     perms[3] = (st_mode & S_IXUSR) ? 'x' : '-';
//     perms[4] = (st_mode & S_IRGRP) ? 'r' : '-';
//     perms[5] = (st_mode & S_IWGRP) ? 'w' : '-';
//     perms[6] = (st_mode & S_IXGRP) ? 'x' : '-';
//     perms[7] = (st_mode & S_IROTH) ? 'r' : '-';
//     perms[8] = (st_mode & S_IWOTH) ? 'w' : '-';
//     perms[9] = (st_mode & S_IXOTH) ? 'x' : '-';
//     perms[10] = '\0'; // Null-terminate the string
// }

void convertPermissions(mode_t mode, char* str) {
    str[0] = S_ISDIR(mode) ? 'd' : '-';
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

int removeFile(char *path)
{
    if (!check_path_exists(path))
    {
        return -1; // file_not_found
    }
    else
    {
        if (isDirectory(path))
        {
            return -2; // not_file
        }
        else
        {
            int ret = remove(path);
            if (ret == -1)
            {
                return -3; // remove system call
            }
        }
    }
    return 0;
}

int removeDirectory(char *path)
{
    if (!check_path_exists(path))
    {
        return -1; // dir_not_found
    }
    else
    {
        if (!isDirectory(path))
        {
            return -2; // not_dir
        }
        else
        {
            int ret = rmdir(path);
            if (ret == -1)
            {
                return -3; // rmdir system call
            }
        }
    }
    return 0;
}

int makeDirectory(char *path, char *perms)
{
    mode_t mode = reversePermissions(perms);
    char *baseDir = dirname(strdup(path));
    int retval = 0;
    if (check_path_exists(path))
    {
        retval = 0; // just need to make it accessible
    }
    else
    {
        if (!check_path_exists(baseDir))
        {
            retval = -1; // dir_not_found
        }
        else
        {
            if (!isDirectory(baseDir))
            {
                retval = -2; // not_dir
            }
            else
            {
                int ret = mkdir(path, mode);
                if (ret == -1)
                {
                    retval = -3; // mkdir syscall
                }
            }
        }
    }
    return retval;
}

int makeFile(char *path, char *perms)
{
    mode_t mode = reversePermissions(perms);
    char *baseDir = dirname(strdup(path));
    int retval = 0;
    if (check_path_exists(path))
    {
        retval = 0; // just need to make it accessible
    }
    else
    {
        if (!check_path_exists(baseDir))
        {
            retval = -1; // file_not_found
        }
        else
        {
            if (!isDirectory(baseDir))
            {
                retval = -1; // not_file
            }
            else
            {
                int ret = creat(path, mode);
                if (ret == -1)
                {
                    retval = -1; // creat syscall
                }
            }
        }
    }
    return retval;
}
#include "../Connection/headers.h"

int main(int argc, char *argv[])
{
    char file[2048] = "temp.txt";
    pid_t pid = fork();

    int fd = open("hello.txt", O_CREAT | O_APPEND | O_TRUNC | O_WRONLY);

    if (pid == 0) // Child process
    {
        char *gedit_args[] = {"gedit", file, NULL};
        if (execvp("gedit", gedit_args) == -1)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    else // Parent process
    {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("LOL!\n");

    remove("hello.txt");

    return 0;
}
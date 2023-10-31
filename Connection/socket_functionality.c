#include "headers.h"

int client_establish_connection(int PORT, char *IP)
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("connect");
        exit(0);
    }
    else
        printf("connected to the server..\n");
    return sockfd;
}

struct pair server_establish_connection(int PORT)
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        perror("bind");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    if ((listen(sockfd, 5)) != 0)
    {
        perror("listen");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0)
    {
        perror("accept");
        exit(0);
    }
    else
        printf("server accept the client...\n");

    struct pair p;
    p.first = sockfd;
    p.second = connfd;
    return p;
}

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

int receive_file(int sockfd)
{
    int fd = open("temp.txt", O_WRONLY | O_CREAT, 0644);
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

// added new
int joinNamingServer(char *ip, int port, char *joinas)
{
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("connect");
        exit(0);
    }

    char joinMsg[4096] = "JOIN_AS ";
    // joinMsg = strcat(joinMsg, joinas);
    strcat(joinMsg, joinas);
    int bytesSent = send(sockfd, joinMsg, sizeof(joinMsg), 0);
    if (bytesSent == -1)
    {
        perror("send");
    }
    printf("%s connected to naming server.\n", joinas);

    return sockfd;
}
#include "headers.h"

int joinSS(struct ssDetails ss)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ss.ip);
	servaddr.sin_port = htons(ss.cliPort);

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		perror("connect");
		exit(0);
	}
	return sockfd;
}

void sendRequest(char *input, int sockfd)
{
	// send request
	char request[4096];
	strcpy(request, input);
	// printf("req: %s\nin: %s\n", request, input);
	// printf(".%d,%d,\n", strlen(request), strlen(input));
	int bytesSent = send(sockfd, request, strlen(request), 0);
	if (bytesSent == -1)
	{
		perror("send");
	}
	// printf("here");
	// recieve the storage server details
	struct ssDetails ss;
	int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
	if (bytesRecv == -1)
	{
		perror("recv");
	}
	printf("Recieved from NM - SS %s:%d\n", ss.ip, ss.cliPort);

	int connfd = joinSS(ss);
	bytesSent = send(connfd, request, sizeof(request), 0);
	if (bytesSent == -1)
	{
		perror("recv");
	}
	printf("\'%s\' sent to SS %s:%d\n", request, ss.ip, ss.cliPort);
}

int joinNamingServerAsClient(char *ip, int port)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("socket");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		perror("connect");
		exit(0);
	}

	char joinMsg[4096] = "JOIN_AS Client";
	int bytesSent = send(sockfd, joinMsg, sizeof(joinMsg), 0);
	if (bytesSent == -1)
	{
		perror("send");
	}
	
	char buffer[4096];
	int bytesRecv = recv(sockfd, buffer, sizeof(buffer), 0);
	if (bytesRecv == -1)
	{
		perror("recv");
	}
	if (strcmp(buffer, "ACCEPTED JOIN")!=  0)
	{
		return -1;
	}
	printf("Client connected to naming server.\n");


	return sockfd;
}

int main()
{
	int namingServerPort = 6969;
	int sockfd = joinNamingServerAsClient("127.0.0.1", namingServerPort);
	while (1)
	{
		// char input[4096];
		char *input = malloc(sizeof(char) * 4096);
		if (fgets(input, 4096, stdin) == NULL)
		{
			break;
		}

		if (input[strlen(input) - 1] == '\n')
		{
			input[strlen(input) - 1] = '\0'; // removing the \n char
		}

		// printf(".%ld.%s.\n", strlen(input), input);
		sendRequest(input, sockfd);

		free(input);
	}

	return 0;
}
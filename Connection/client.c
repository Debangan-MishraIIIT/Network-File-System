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

int sendRequest(char *input, int sockfd)
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
	if (bytesSent == 0)
	{
		// nm has disconnected
		return -1;
	}

	// printf("here");
	// recieve the storage server details
	struct ssDetails ss;
	int bytesRecv = recv(sockfd, &ss, sizeof(ss), 0);
	if (ss.id <= 0)
	{
		// nm has disconnected
		return -1;
	}

	if (bytesRecv == -1)
	{
		perror("recv");
	}
	if (bytesRecv == 0)
	{
		return -1;
	}

	printf("Recieved from NM - SS %s:%d\n", ss.ip, ss.cliPort);

	int connfd = joinSS(ss);
	bytesSent = send(connfd, request, sizeof(request), 0);
	if (bytesSent == -1)
	{
		perror("recv");
	}
	printf("\'%s\' sent to SS %s:%d\n", request, ss.ip, ss.cliPort);
	return 0;
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
	if (strcmp(buffer, "ACCEPTED JOIN") != 0)
	{
		return -1;
	}
	// printf("Client connected to naming server.\n");

	return sockfd;
}

void *isNMConnected(void *args)
{
	int sockfd = *(int *)(args);
	while (1)
	{
		sleep(1);
		int error;
		socklen_t len = sizeof(error);
		int ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

		if (ret == 0 && error == 0)
		{
			sleep(1);
			continue;
		}
		else
		{
			printf(RED_COLOR "[-] Naming Server has disconnected!\n" RESET_COLOR);
			exit(0);
		}
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf(RED_COLOR "[-] Invalid Arguments!\n" RESET_COLOR);
		exit(0);
	}

	int namingServerPort = atoi(argv[1]);
	// int namingServerPort = 6969;
	int sockfd = joinNamingServerAsClient("127.0.0.1", namingServerPort);
	printf(GREEN_COLOR "[+] Client connected to Naming Server\n" RESET_COLOR);

	pthread_t disconnectionThread;
	pthread_create(&disconnectionThread, NULL, isNMConnected, &sockfd);

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
		int status = sendRequest(input, sockfd);
		if (status == -1)
		{
			printf(RED_COLOR "[-] Naming Server has disconnected!\n" RESET_COLOR);
			exit(0);
			break;
		}
		if (status == 0)
		{
			printf("Successful Request \n");
		}
		free(input);
	}
	pthread_join(disconnectionThread, NULL);
	return 0;
}
